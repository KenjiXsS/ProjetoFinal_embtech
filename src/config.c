#include "config.h"
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include <string.h>
#include <stdio.h>

static config_t g_config;
static bool g_config_loaded = false;

// Static buffer for flash operations
static uint8_t flash_buffer[sizeof(config_t)];

uint32_t config_calculate_checksum(const config_t *config) {
    uint32_t checksum = 0;
    const uint8_t *data = (const uint8_t *)config;
    
    // Calculate checksum excluding the checksum field itself
    for (size_t i = 0; i < offsetof(config_t, checksum); i++) {
        checksum += data[i];
    }
    
    return checksum;
}

void config_reset(config_t *config) {
    memset(config, 0, sizeof(config_t));
    config->magic = CONFIG_MAGIC;
    config->version = CONFIG_VERSION;
    strncpy(config->ssid, "YourWiFiSSID", MAX_SSID_LENGTH - 1);
    strncpy(config->password, "YourWiFiPassword", MAX_PASSWORD_LENGTH - 1);
    config->blocked_ips_count = 0;
    config->checksum = config_calculate_checksum(config);
}

bool config_load(config_t *config) {
    // Disable interrupts during flash read
    uint32_t save = save_and_disable_interrupts();
    
    // Read from flash
    const uint8_t *flash_target = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
    memcpy(config, flash_target, sizeof(config_t));
    
    // Restore interrupts
    restore_interrupts(save);
    
    // Validate configuration
    if (config->magic != CONFIG_MAGIC) {
        printf("Config: Invalid magic number, resetting to defaults\n");
        config_reset(config);
        return false;
    }
    
    if (config->version != CONFIG_VERSION) {
        printf("Config: Version mismatch, resetting to defaults\n");
        config_reset(config);
        return false;
    }
    
    uint32_t expected_checksum = config->checksum;
    uint32_t calculated_checksum = config_calculate_checksum(config);
    
    if (expected_checksum != calculated_checksum) {
        printf("Config: Checksum mismatch, resetting to defaults\n");
        config_reset(config);
        return false;
    }
    
    printf("Config: Loaded successfully from flash\n");
    g_config_loaded = true;
    return true;
}

bool config_save(const config_t *config) {
    // Prepare buffer
    memcpy(flash_buffer, config, sizeof(config_t));
    
    // Calculate and update checksum
    config_t *temp_config = (config_t *)flash_buffer;
    temp_config->checksum = config_calculate_checksum(temp_config);
    
    // Disable interrupts and flash
    uint32_t save = save_and_disable_interrupts();
    
    // Erase flash sector
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    
    // Program flash
    flash_range_program(FLASH_TARGET_OFFSET, flash_buffer, sizeof(config_t));
    
    // Restore interrupts
    restore_interrupts(save);
    
    printf("Config: Saved to flash\n");
    return true;
}

void config_init(void) {
    if (!g_config_loaded) {
        config_load(&g_config);
    }
}

bool config_add_blocked_ip(config_t *config, const char *ip) {
    if (config->blocked_ips_count >= MAX_BLOCKED_IPS) {
        printf("Config: Cannot add more blocked IPs (max %d)\n", MAX_BLOCKED_IPS);
        return false;
    }
    
    // Check if IP already exists
    for (int i = 0; i < config->blocked_ips_count; i++) {
        if (strcmp(config->blocked_ips[i], ip) == 0) {
            printf("Config: IP %s already blocked\n", ip);
            return false;
        }
    }
    
    // Add IP to list
    strncpy(config->blocked_ips[config->blocked_ips_count], ip, 15);
    config->blocked_ips[config->blocked_ips_count][15] = '\0';
    config->blocked_ips_count++;
    
    // Save to flash
    config_save(config);
    
    printf("Config: Added %s to blocklist\n", ip);
    return true;
}

bool config_remove_blocked_ip(config_t *config, const char *ip) {
    int found_index = -1;
    
    // Find IP in list
    for (int i = 0; i < config->blocked_ips_count; i++) {
        if (strcmp(config->blocked_ips[i], ip) == 0) {
            found_index = i;
            break;
        }
    }
    
    if (found_index == -1) {
        printf("Config: IP %s not in blocklist\n", ip);
        return false;
    }
    
    // Remove IP from list (shift remaining entries)
    for (int i = found_index; i < config->blocked_ips_count - 1; i++) {
        strcpy(config->blocked_ips[i], config->blocked_ips[i + 1]);
    }
    
    config->blocked_ips_count--;
    memset(config->blocked_ips[config->blocked_ips_count], 0, 16);
    
    // Save to flash
    config_save(config);
    
    printf("Config: Removed %s from blocklist\n", ip);
    return true;
}

bool config_is_ip_blocked(const config_t *config, const char *ip) {
    for (int i = 0; i < config->blocked_ips_count; i++) {
        if (strcmp(config->blocked_ips[i], ip) == 0) {
            return true;
        }
    }
    return false;
}
