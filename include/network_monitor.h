#ifndef NETWORK_MONITOR_H
#define NETWORK_MONITOR_H

#include <stdint.h>
#include <stdbool.h>
#include "lwip/netif.h"

// Maximum number of tracked connections
#define MAX_CONNECTIONS 10

// Connection statistics structure
typedef struct {
    uint32_t total_connections;      // Total connections since startup
    uint32_t active_connections;     // Currently active connections
    uint32_t blocked_connections;    // Blocked connections
    uint32_t bytes_received;         // Total bytes received
    uint32_t bytes_sent;             // Total bytes sent
    uint32_t uptime_seconds;         // System uptime in seconds
} network_stats_t;

// Connection info structure
typedef struct {
    char ip_address[16];             // Client IP address
    uint16_t port;                   // Client port
    uint32_t bytes_received;         // Bytes received from this client
    uint32_t bytes_sent;             // Bytes sent to this client
    uint32_t connection_time;        // Connection start time
    bool is_blocked;                 // Whether connection was blocked
} connection_info_t;

// Initialize network monitor
void network_monitor_init(void);

// Update network statistics
void network_monitor_update_stats(network_stats_t *stats);

// Get current connection info
int network_monitor_get_connections(connection_info_t *connections, int max_connections);

// Increment bytes received
void network_monitor_add_bytes_received(uint32_t bytes);

// Increment bytes sent
void network_monitor_add_bytes_sent(uint32_t bytes);

// Increment total connections
void network_monitor_increment_connections(void);

// Increment blocked connections
void network_monitor_increment_blocked(void);

// Get uptime in seconds
uint32_t network_monitor_get_uptime(void);

// Format IP address to string
void network_monitor_format_ip(const ip_addr_t *addr, char *buffer, size_t buffer_size);

// Get network status as string
const char* network_monitor_get_status_string(void);

#endif // NETWORK_MONITOR_H
