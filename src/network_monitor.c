#include "network_monitor.h"
#include "pico/stdlib.h"
#include <string.h>
#include <stdio.h>

static network_stats_t g_stats;
static connection_info_t g_connections[MAX_CONNECTIONS];
static uint32_t g_start_time;

void network_monitor_init(void) {
    memset(&g_stats, 0, sizeof(network_stats_t));
    memset(g_connections, 0, sizeof(g_connections));
    g_start_time = time_us_32() / 1000;
    printf("Network Monitor: Initialized\n");
}

void network_monitor_update_stats(network_stats_t *stats) {
    stats->total_connections = g_stats.total_connections;
    stats->active_connections = g_stats.active_connections;
    stats->blocked_connections = g_stats.blocked_connections;
    stats->bytes_received = g_stats.bytes_received;
    stats->bytes_sent = g_stats.bytes_sent;
    stats->uptime_seconds = network_monitor_get_uptime();
}

int network_monitor_get_connections(connection_info_t *connections, int max_connections) {
    int count = (max_connections < MAX_CONNECTIONS) ? max_connections : MAX_CONNECTIONS;
    memcpy(connections, g_connections, sizeof(connection_info_t) * count);
    return count;
}

void network_monitor_add_bytes_received(uint32_t bytes) {
    g_stats.bytes_received += bytes;
}

void network_monitor_add_bytes_sent(uint32_t bytes) {
    g_stats.bytes_sent += bytes;
}

void network_monitor_increment_connections(void) {
    g_stats.total_connections++;
    g_stats.active_connections++;
}

void network_monitor_increment_blocked(void) {
    g_stats.blocked_connections++;
}

uint32_t network_monitor_get_uptime(void) {
    uint32_t current_time = time_us_32() / 1000;
    return current_time - g_start_time;
}

void network_monitor_format_ip(const ip_addr_t *addr, char *buffer, size_t buffer_size) {
    if (buffer_size < 16) return;
    
    const uint8_t *ip = (const uint8_t *)&addr->addr;
    snprintf(buffer, buffer_size, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
}

const char* network_monitor_get_status_string(void) {
    if (netif_is_up(&cyw43_state.netif[0])) {
        if (netif_is_link_up(&cyw43_state.netif[0])) {
            return "Connected";
        } else {
            return "Link Down";
        }
    } else {
        return "Disconnected";
    }
}
