#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

// Configuration version for flash storage
#define CONFIG_VERSION 1

// Maximum number of blocked IPs
#define MAX_BLOCKED_IPS 20

// Maximum SSID and password length
#define MAX_SSID_LENGTH 32
#define MAX_PASSWORD_LENGTH 64

// Device hostname
#define DEVICE_HOSTNAME "pico-firewall"

// Magic number for flash validation
#define CONFIG_MAGIC 0xDEADBEEF

// Flash storage address
#define FLASH_TARGET_OFFSET (1024 * 1024)  // 1MB offset

// Configuration structure stored in flash
typedef struct {
    uint32_t magic;              // Magic number for validation
    uint32_t version;            // Configuration version
    char ssid[MAX_SSID_LENGTH];  // Wi-Fi SSID
    char password[MAX_PASSWORD_LENGTH];  // Wi-Fi password
    uint8_t blocked_ips_count;   // Number of blocked IPs
    char blocked_ips[MAX_BLOCKED_IPS][16];  // Blocked IP addresses
    uint32_t checksum;           // Simple checksum for validation
} config_t;

// Initialize configuration
void config_init(void);

// Load configuration from flash
bool config_load(config_t *config);

// Save configuration to flash
bool config_save(const config_t *config);

// Reset configuration to defaults
void config_reset(config_t *config);

// Add IP to blocklist
bool config_add_blocked_ip(config_t *config, const char *ip);

// Remove IP from blocklist
bool config_remove_blocked_ip(config_t *config, const char *ip);

// Check if IP is blocked
bool config_is_ip_blocked(const config_t *config, const char *ip);

// Calculate checksum
uint32_t config_calculate_checksum(const config_t *config);

#endif // CONFIG_H
