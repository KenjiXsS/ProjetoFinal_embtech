#ifndef FIREWALL_H
#define FIREWALL_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

// Maximum log entries
#define MAX_FIREWALL_LOGS 50

// Firewall log entry
typedef struct {
    uint32_t timestamp;              // Time when event occurred
    char ip_address[16];             // IP address involved
    uint16_t port;                   // Port number
    bool blocked;                    // Whether action was blocked
    char action[32];                 // Action description
} firewall_log_t;

// Initialize firewall
void firewall_init(const config_t *config);

// Check if IP should be blocked
bool firewall_check_ip(const char *ip);

// Add IP to blocklist (runtime only)
bool firewall_block_ip(const char *ip);

// Remove IP from blocklist (runtime only)
bool firewall_unblock_ip(const char *ip);

// Get blocked IPs count
int firewall_get_blocked_ips_count(void);

// Get blocked IP at index
const char* firewall_get_blocked_ip(int index);

// Get firewall logs
int firewall_get_logs(firewall_log_t *logs, int max_logs);

// Add log entry
void firewall_add_log(const char *ip, uint16_t port, bool blocked, const char *action);

// Clear firewall logs
void firewall_clear_logs(void);

// Get firewall status
bool firewall_is_enabled(void);

// Enable/disable firewall
void firewall_set_enabled(bool enabled);

#endif // FIREWALL_H
