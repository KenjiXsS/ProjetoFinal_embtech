#include "system_monitor.h"
#include "network_monitor.h"
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "pico/malloc.h"
#include <string.h>
#include <stdio.h>

// Include network monitor for WiFi status
#include "pico/cyw43_arch.h"
#include "lwip/netif.h"

static led_state_t g_led_state = LED_OFF;
static uint32_t g_last_activity_time = 0;
static bool g_initialized = false;

// External HTTP server status
extern bool http_server_is_running(void);
extern bool firewall_is_enabled(void);

void system_monitor_init(void) {
    g_led_state = LED_OFF;
    g_last_activity_time = time_us_32() / 1000;
    g_initialized = true;
    printf("System Monitor: Initialized\n");
}

void system_monitor_get_health(system_health_t *health) {
    if (!health) return;
    
    // Uptime
    uint32_t current_time = time_us_32() / 1000;
    health->uptime_seconds = current_time - g_last_activity_time;  // Approximate
    
    // Memory usage (approximate for pico)
    struct mallinfo mi = mallinfo();
    health->free_memory = ciGetFreeHeapSize ? ciGetFreeHeapSize() : 100000;
    health->total_memory = 256 * 1024;  // Pico W has 256KB SRAM
    
    // WiFi status
    health->wifi_connected = netif_is_up(&cyw43_state.netif[0]) && 
                             netif_is_link_up(&cyw43_state.netif[0]);
    
    // HTTP server status
    health->http_server_running = http_server_is_running();
    
    // Firewall status
    health->firewall_enabled = firewall_is_enabled();
    
    // LED state
    health->led_state = (uint8_t)g_led_state;
    
    // CPU temperature (not directly available on RP2040 without ADC reading)
    health->cpu_temperature = 0.0f;
}

void system_monitor_set_led(led_state_t state) {
    g_led_state = state;
    
    switch (state) {
        case LED_OFF:
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            break;
        case LED_WIFI_CONNECTING:
            // Blinking slow
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, (time_us_32() / 500000) % 2);
            break;
        case LED_WIFI_CONNECTED:
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            break;
        case LED_ERROR:
            // Blinking fast
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, (time_us_32() / 100000) % 2);
            break;
        case LED_ACTIVITY:
            // Quick blink
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            sleep_ms(50);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            break;
    }
}

led_state_t system_monitor_get_led_state(void) {
    return g_led_state;
}

void system_monitor_led_activity(void) {
    if (!g_initialized) return;
    
    uint32_t current_time = time_us_32() / 1000;
    
    // Only blink if not already blinking for activity (debounce)
    if (current_time - g_last_activity_time > 50) {
        system_monitor_set_led(LED_ACTIVITY);
        g_last_activity_time = current_time;
    }
}

void system_monitor_get_uptime_string(char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 20) return;
    
    uint32_t total_seconds = network_monitor_get_uptime();
    
    uint32_t hours = total_seconds / 3600;
    uint32_t minutes = (total_seconds % 3600) / 60;
    uint32_t seconds = total_seconds % 60;
    
    snprintf(buffer, buffer_size, "%02lu:%02lu:%02lu", hours, minutes, seconds);
}

float system_monitor_get_memory_usage_percent(void) {
    // Approximate memory usage
    // In real implementation, you'd use proper heap monitoring
    return 15.0f;  // Placeholder
}

void system_monitor_reset(void) {
    printf("System Monitor: Initiating reset...\n");
    
    // Use watchdog for soft reset
    watchdog_enable(1, 1);  // 1ms timeout, pause on debug
    
    while (1) {
        // Wait for watchdog to reset
    }
}
