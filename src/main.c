/**
 * Pico Firewall Monitor - Main Application
 * 
 * A powerful network monitoring and firewall management system
 * for Raspberry Pi Pico W with web-based dashboard.
 * 
 * Features:
 * - Real-time network monitoring
 * - Dynamic IP blocking/unblocking
 * - Web-based dashboard with modern UI
 * - JSON API for integration
 * - Persistent configuration storage
 * - Connection statistics and traffic monitoring
 * - LED status indicators
 */

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/watchdog.h"
#include <stdio.h>
#include <string.h>

// Module headers
#include "config.h"
#include "network_monitor.h"
#include "firewall.h"
#include "http_server.h"
#include "system_monitor.h"

// Wi-Fi connection timeout (ms)
#define WIFI_CONNECT_TIMEOUT 30000

// Network check interval (ms)
#define NETWORK_CHECK_INTERVAL 5000

// Configuration
static config_t g_config;

// System state
static bool wifi_connected = false;

/**
 * @brief Initialize all system modules
 */
static void initialize_system(void) {
    printf("\n");
    printf("========================================\n");
    printf("  Pico Firewall Monitor v2.0\n");
    printf("========================================\n");
    printf("\n");
    
    // Initialize standard I/O
    stdio_init_all();
    sleep_ms(100);
    
    // Initialize configuration
    printf("[INIT] Loading configuration...\n");
    config_init();
    config_load(&g_config);
    
    // Initialize network monitor
    printf("[INIT] Initializing network monitor...\n");
    network_monitor_init();
    
    // Initialize system monitor
    printf("[INIT] Initializing system monitor...\n");
    system_monitor_init();
    
    // Initialize firewall
    printf("[INIT] Initializing firewall...\n");
    firewall_init(&g_config);
    
    // Initialize HTTP server
    printf("[INIT] Initializing HTTP server...\n");
    http_server_init();
    http_server_set_config(&g_config);
    
    printf("\n[INIT] All modules initialized successfully!\n\n");
}

/**
 * @brief Connect to Wi-Fi network
 * @return true if connected successfully
 */
static bool connect_to_wifi(void) {
    printf("[WIFI] Initializing Wi-Fi...\n");
    
    // Initialize CYW43
    if (cyw43_arch_init()) {
        printf("[WIFI] ERROR: Failed to initialize CYW43\n");
        system_monitor_set_led(LED_ERROR);
        return false;
    }
    
    // Set to station mode
    cyw43_arch_enable_sta_mode();
    
    // Set hostname
    netif_set_hostname(&cyw43_state.netif[0], DEVICE_HOSTNAME);
    
    printf("[WIFI] Connecting to '%s'...\n", g_config.ssid);
    system_monitor_set_led(LED_WIFI_CONNECTING);
    
    // Connect to Wi-Fi with timeout
    int status = cyw43_arch_wifi_connect_timeout_ms(
        g_config.ssid,
        g_config.password,
        CYW43_AUTH_WPA2_AES_PSK,
        WIFI_CONNECT_TIMEOUT
    );
    
    if (status != 0) {
        printf("[WIFI] ERROR: Failed to connect (code: %d)\n", status);
        system_monitor_set_led(LED_ERROR);
        return false;
    }
    
    // Get and display IP information
    uint8_t *ip = (uint8_t *)&cyw43_state.netif[0].ip_addr.addr;
    uint8_t *nm = (uint8_t *)&cyw43_state.netif[0].netmask.addr;
    uint8_t *gw = (uint8_t *)&cyw43_state.netif[0].gw.addr;
    
    printf("[WIFI] Successfully connected!\n");
    printf("[WIFI] IP Address: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
    printf("[WIFI] Netmask:   %d.%d.%d.%d\n", nm[0], nm[1], nm[2], nm[3]);
    printf("[WIFI] Gateway:   %d.%d.%d.%d\n", gw[0], gw[1], gw[2], gw[3]);
    printf("[WIFI] Hostname:  %s\n", DEVICE_HOSTNAME);
    
    system_monitor_set_led(LED_WIFI_CONNECTED);
    return true;
}

/**
 * @brief Monitor network status
 */
static void monitor_network_status(void) {
    static uint32_t last_check = 0;
    uint32_t current_time = time_us_32() / 1000;
    
    if (current_time - last_check >= NETWORK_CHECK_INTERVAL) {
        last_check = current_time;
        
        // Check Wi-Fi connection
        bool was_connected = wifi_connected;
        wifi_connected = netif_is_up(&cyw43_state.netif[0]) && 
                        netif_is_link_up(&cyw43_state.netif[0]);
        
        if (was_connected && !wifi_connected) {
            printf("[MONITOR] Wi-Fi connection lost!\n");
            system_monitor_set_led(LED_ERROR);
        } else if (!was_connected && wifi_connected) {
            printf("[MONITOR] Wi-Fi reconnected!\n");
            system_monitor_set_led(LED_WIFI_CONNECTED);
        }
        
        // Print status periodically
        network_stats_t stats;
        network_monitor_update_stats(&stats);
        
        printf("[MONITOR] Uptime: %lus | Connections: %lu | Blocked: %lu | RX: %luB | TX: %luB\n",
               stats.uptime_seconds,
               stats.total_connections,
               stats.blocked_connections,
               stats.bytes_received,
               stats.bytes_sent);
    }
}

/**
 * @brief Main application entry point
 */
int main(void) {
    // Initialize system
    initialize_system();
    
    // Connect to Wi-Fi
    wifi_connected = connect_to_wifi();
    
    if (!wifi_connected) {
        printf("\n[ERROR] Cannot start without Wi-Fi connection!\n");
        printf("[ERROR] Please check your Wi-Fi credentials in the configuration.\n");
        printf("[HINT] Access the web dashboard to update Wi-Fi settings after reboot.\n");
        
        // Continue anyway to allow configuration via AP mode in future
        // For now, blink error LED
        while (true) {
            system_monitor_set_led(LED_ERROR);
            sleep_ms(500);
        }
    }
    
    // Start HTTP server
    printf("\n[SERVER] Starting HTTP server...\n");
    http_server_start();
    
    printf("\n========================================\n");
    printf("  System Ready!\n");
    printf("  Access: http://%d.%d.%d.%d\n", 
           ((uint8_t*)&cyw43_state.netif[0].ip_addr.addr)[0],
           ((uint8_t*)&cyw43_state.netif[0].ip_addr.addr)[1],
           ((uint8_t*)&cyw43_state.netif[0].ip_addr.addr)[2],
           ((uint8_t*)&cyw43_state.netif[0].ip_addr.addr)[3]);
    printf("========================================\n\n");
    
    // Main loop
    uint32_t last_poll = 0;
    
    while (true) {
        // Poll CYW43 for Wi-Fi activity
        cyw43_arch_poll();
        
        // Poll HTTP server
        http_server_poll();
        
        // Monitor network status
        monitor_network_status();
        
        // Update LED based on activity
        if (g_led_state == LED_WIFI_CONNECTED) {
            // Keep LED on when connected
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        }
        
        // Small delay to prevent busy-waiting
        sleep_ms(10);
    }
    
    // Cleanup (never reached in normal operation)
    cyw43_arch_deinit();
    return 0;
}
