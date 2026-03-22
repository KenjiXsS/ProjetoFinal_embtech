#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <stdint.h>
#include <stdbool.h>

// System health status
typedef struct {
    uint32_t uptime_seconds;         // System uptime
    uint32_t free_memory;            // Free heap memory
    uint32_t total_memory;           // Total heap memory
    float cpu_temperature;           // CPU temperature (if available)
    bool wifi_connected;             // Wi-Fi connection status
    bool http_server_running;        // HTTP server status
    bool firewall_enabled;           // Firewall status
    uint8_t led_state;               // Current LED state
} system_health_t;

// LED states
typedef enum {
    LED_OFF = 0,
    LED_WIFI_CONNECTING = 1,
    LED_WIFI_CONNECTED = 2,
    LED_ERROR = 3,
    LED_ACTIVITY = 4
} led_state_t;

// Initialize system monitor
void system_monitor_init(void);

// Get system health status
void system_monitor_get_health(system_health_t *health);

// Update LED state
void system_monitor_set_led(led_state_t state);

// Get current LED state
led_state_t system_monitor_get_led_state(void);

// Blink LED for activity
void system_monitor_led_activity(void);

// Get uptime formatted string
void system_monitor_get_uptime_string(char *buffer, size_t buffer_size);

// Get memory usage percentage
float system_monitor_get_memory_usage_percent(void);

// Reset system (soft reset)
void system_monitor_reset(void);

#endif // SYSTEM_MONITOR_H
