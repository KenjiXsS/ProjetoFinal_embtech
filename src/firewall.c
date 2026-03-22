#include "firewall.h"
#include "network_monitor.h"
#include <string.h>
#include <stdio.h>

static config_t g_config;
static bool g_firewall_enabled = true;
static firewall_log_t g_logs[MAX_FIREWALL_LOGS];
static int g_log_index = 0;
static uint32_t g_log_count = 0;

void firewall_init(const config_t *config) {
    memcpy(&g_config, config, sizeof(config_t));
    memset(g_logs, 0, sizeof(g_logs));
    g_log_index = 0;
    g_log_count = 0;
    printf("Firewall: Initialized with %d blocked IPs\n", g_config.blocked_ips_count);
}

bool firewall_check_ip(const char *ip) {
    if (!g_firewall_enabled) {
        return false;  // Not blocked if firewall is disabled
    }
    
    bool blocked = config_is_ip_blocked(&g_config, ip);
    
    if (blocked) {
        firewall_add_log(ip, 0, true, "Blocked by IP filter");
        network_monitor_increment_blocked();
    }
    
    return blocked;
}

bool firewall_block_ip(const char *ip) {
    if (config_add_blocked_ip(&g_config, ip)) {
        firewall_add_log(ip, 0, true, "IP added to blocklist");
        return true;
    }
    return false;
}

bool firewall_unblock_ip(const char *ip) {
    if (config_remove_blocked_ip(&g_config, ip)) {
        firewall_add_log(ip, 0, false, "IP removed from blocklist");
        return true;
    }
    return false;
}

int firewall_get_blocked_ips_count(void) {
    return g_config.blocked_ips_count;
}

const char* firewall_get_blocked_ip(int index) {
    if (index < 0 || index >= g_config.blocked_ips_count) {
        return NULL;
    }
    return g_config.blocked_ips[index];
}

int firewall_get_logs(firewall_log_t *logs, int max_logs) {
    int count = (max_logs < MAX_FIREWALL_LOGS) ? max_logs : MAX_FIREWALL_LOGS;
    
    // Return logs in reverse order (newest first)
    for (int i = 0; i < count; i++) {
        int log_index = (g_log_index - 1 - i + MAX_FIREWALL_LOGS) % MAX_FIREWALL_LOGS;
        if (g_logs[log_index].timestamp != 0) {
            memcpy(&logs[i], &g_logs[log_index], sizeof(firewall_log_t));
        } else {
            memset(&logs[i], 0, sizeof(firewall_log_t));
        }
    }
    
    return count;
}

void firewall_add_log(const char *ip, uint16_t port, bool blocked, const char *action) {
    firewall_log_t *log = &g_logs[g_log_index];
    
    log->timestamp = network_monitor_get_uptime();
    strncpy(log->ip_address, ip, 15);
    log->ip_address[15] = '\0';
    log->port = port;
    log->blocked = blocked;
    strncpy(log->action, action, 31);
    log->action[31] = '\0';
    
    g_log_index = (g_log_index + 1) % MAX_FIREWALL_LOGS;
    g_log_count++;
}

void firewall_clear_logs(void) {
    memset(g_logs, 0, sizeof(g_logs));
    g_log_index = 0;
    g_log_count = 0;
    printf("Firewall: Logs cleared\n");
}

bool firewall_is_enabled(void) {
    return g_firewall_enabled;
}

void firewall_set_enabled(bool enabled) {
    g_firewall_enabled = enabled;
    printf("Firewall: %s\n", enabled ? "enabled" : "disabled");
}
