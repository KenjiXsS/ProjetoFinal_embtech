# System Architecture

## Overview

The Pico Firewall Monitor is built with a modular architecture that separates concerns and promotes maintainability.

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                        Application Layer                        │
│                          (main.c)                               │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │  System Initialization & Main Event Loop                  │  │
│  └───────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                              │
        ┌─────────────────────┼─────────────────────┐
        │                     │                     │
        ▼                     ▼                     ▼
┌──────────────────┐ ┌──────────────────┐ ┌──────────────────┐
│  System Monitor  │ │ Network Monitor  │ │     Firewall     │
│                  │ │                  │ │                  │
│ • LED Control    │ │ • Statistics     │ │ • IP Blocking    │
│ • Health Check   │ │ • Traffic Stats  │ │ • Log Tracking   │
│ • Watchdog       │ │ • Connection     │ │ • Runtime Filter │
│ • Reset          │ │   Monitoring     │ │ • Config Sync    │
└──────────────────┘ └──────────────────┘ └──────────────────┘
        │                     │                     │
        └─────────────────────┼─────────────────────┘
                              │
                    ┌─────────▼──────────┐
                    │    HTTP Server     │
                    │                    │
                    │ • Request Router   │
                    │ • Response Handler │
                    │ • API Endpoints    │
                    │ • CORS Support     │
                    └─────────┬──────────┘
                              │
                    ┌─────────▼──────────┐
                    │      Web UI        │
                    │                    │
                    │ • HTML Generator   │
                    │ • JSON Builder     │
                    │ • Form Parser      │
                    │ • Dashboard        │
                    └─────────┬──────────┘
                              │
                    ┌─────────▼──────────┐
                    │  Configuration     │
                    │                    │
                    │ • Flash Storage    │
                    │ • Validation       │
                    │ • Checksum         │
                    │ • Persistence      │
                    └─────────┬──────────┘
                              │
        ┌─────────────────────┼─────────────────────┐
        │                     │                     │
        ▼                     ▼                     ▼
┌──────────────────┐ ┌──────────────────┐ ┌──────────────────┐
│  Hardware Abstraction Layer (Pico SDK)                   │
│                                                          │
│  • CYW43 (WiFi)  • Flash Memory  • GPIO (LED)           │
│  • Watchdog      • USB Serial    • LwIP Stack           │
└──────────────────────────────────────────────────────────┘
```

## Module Responsibilities

### 1. Main Application (`main.c`)
- System initialization orchestration
- WiFi connection management
- Main event loop
- Module coordination

### 2. Configuration Module (`config.h/c`)
**Purpose**: Persistent storage management

**Features**:
- Flash memory read/write operations
- Configuration validation with checksums
- Version control for migrations
- IP blocklist persistence

**Data Structure**:
```c
typedef struct {
    uint32_t magic;              // Validation magic number
    uint32_t version;            // Config version
    char ssid[32];               // WiFi SSID
    char password[64];           // WiFi password
    uint8_t blocked_ips_count;   // Number of blocked IPs
    char blocked_ips[20][16];    // Blocked IP addresses
    uint32_t checksum;           // Validation checksum
} config_t;
```

### 3. Network Monitor (`network_monitor.h/c`)
**Purpose**: Network statistics and connection tracking

**Features**:
- Connection counting
- Traffic measurement (bytes sent/received)
- Uptime tracking
- IP address formatting
- Status string generation

**Statistics Tracked**:
- Total connections
- Active connections
- Blocked connections
- Bytes received
- Bytes sent
- System uptime

### 4. Firewall Module (`firewall.h/c`)
**Purpose**: IP filtering and security logging

**Features**:
- Runtime IP blocking/unblocking
- Blocklist management
- Action logging (circular buffer)
- Integration with config module
- Enable/disable toggle

**Log Structure**:
```c
typedef struct {
    uint32_t timestamp;    // Uptime when event occurred
    char ip_address[16];   // Client IP
    uint16_t port;         // Port number
    bool blocked;          // Was action blocked
    char action[32];       // Action description
} firewall_log_t;
```

### 5. HTTP Server (`http_server.h/c`)
**Purpose**: Web server and API handling

**Features**:
- TCP connection management
- HTTP request parsing
- Request routing (static vs API)
- Response generation
- CORS support
- Multiple HTTP methods (GET, POST, DELETE, OPTIONS)

**API Endpoints**:
```
GET    /api/status     - System status
GET    /api/blocklist  - Get blocked IPs
POST   /api/blocklist  - Block an IP
DELETE /api/blocklist  - Unblock an IP
GET    /api/logs       - Get firewall logs
DELETE /api/logs       - Clear logs
GET    /api/config     - Get configuration
POST   /api/config     - Update config
POST   /api/reboot     - Reboot device
```

### 6. Web UI (`web_ui.h/c`)
**Purpose**: User interface generation

**Features**:
- HTML dashboard (embedded)
- JSON response generation
- Form data parsing
- Modern responsive design
- Auto-refresh functionality

**Pages Generated**:
- Dashboard (real-time stats)
- Blocklist management
- Firewall logs
- System settings

### 7. System Monitor (`system_monitor.h/c`)
**Purpose**: System health and hardware control

**Features**:
- LED status indication
- Memory monitoring
- Uptime calculation
- Watchdog reset
- Health status aggregation

**LED States**:
- `LED_OFF` - LED off
- `LED_WIFI_CONNECTING` - Slow blink
- `LED_WIFI_CONNECTED` - Solid on
- `LED_ERROR` - Fast blink
- `LED_ACTIVITY` - Quick pulse

## Data Flow

### Request Processing Flow
```
1. TCP Connection Established
         │
2. HTTP Request Received
         │
3. Client IP Extracted
         │
4. Firewall Check ────[Blocked]───→ Close Connection
         │ [Allowed]
5. Parse Request (Method, Path, Body)
         │
6. Route Request
         │
    ┌────┴────┐
    │         │
[API]     [Static]
    │         │
    │         └──→ Serve HTML Dashboard
    │
    └──→ Process API Call
             │
         Update State
             │
         Generate JSON
             │
         Send Response
```

### Configuration Save Flow
```
1. User Updates Settings (Web UI)
         │
2. HTTP POST /api/config
         │
3. Parse Form Data
         │
4. Update config_t Structure
         │
5. Calculate Checksum
         │
6. Disable Interrupts
         │
7. Erase Flash Sector
         │
8. Program Flash
         │
9. Restore Interrupts
         │
10. Send Confirmation
```

## Memory Layout

### Flash Memory (2MB Total)
```
┌─────────────────────────────────┐
│ 0x00000000 - Program Code       │ (~150KB)
│         ...                     │
│ 0x00020000 - Program Data       │
│         ...                     │
│ 0x000F0000 - Configuration      │ (1KB at 1MB offset)
│         ...                     │
│ 0x00100000 - End                │
└─────────────────────────────────┘
```

### SRAM (256KB Total)
```
┌─────────────────────────────────┐
│ Stack (~8KB)                    │
│ Heap/Network Buffer (~20KB)     │
│ lwIP Stack (~8KB)               │
│ Application Data (~5KB)         │
│ Free Space                      │
└─────────────────────────────────┘
```

## Concurrency Model

The application uses a **cooperative multitasking** model:

1. **Main Loop**: Continuously polls for events
2. **cyw43_poll**: Handles WiFi events
3. **TCP Callbacks**: Event-driven network handling
4. **No Preemption**: All tasks yield control

### Timing Considerations
```
├─ WiFi Poll: Every loop iteration
├─ Network Monitor: Every 5 seconds
├─ LED Updates: On state changes
├─ HTTP Poll: Every loop iteration
└─ Dashboard Refresh: Every 3 seconds (client-side)
```

## Error Handling Strategy

### Levels of Error Handling
1. **Hardware Level**: Retry operations, timeout detection
2. **Driver Level**: Error codes from SDK functions
3. **Application Level**: Graceful degradation, logging
4. **User Level**: Status messages, LED indicators

### Recovery Mechanisms
- **WiFi Disconnect**: Auto-reconnect on next poll
- **HTTP Error**: Close connection, log error
- **Config Corruption**: Reset to defaults
- **System Hang**: Watchdog reset (1ms timeout)

## Security Considerations

### Current Implementation
- ✅ IP-based access control
- ✅ Input validation (basic)
- ✅ Checksum validation for config
- ⚠️ No authentication required
- ⚠️ HTTP only (no HTTPS)
- ⚠️ Passwords stored in plaintext

### Recommended Improvements
- Add HTTP Basic/Digest authentication
- Implement HTTPS with TLS
- Encrypt stored credentials
- Add rate limiting
- Implement CSRF protection
- Add input sanitization

## Performance Optimizations

### Implemented
- TCP_NODELAY (Nagle's algorithm disabled)
- Increased TCP window size (16 * MSS)
- Optimized lwIP memory pools
- Single PBUF transmission
- Connection backlog (4 connections)

### Potential Improvements
- HTTP keep-alive support
- GZIP compression for HTML
- Caching headers
- WebSocket for real-time updates
- DNS-SD/mDNS for discovery

## Testing Strategy

### Manual Testing
1. WiFi connection stability
2. HTTP server responsiveness
3. IP blocking functionality
4. Configuration persistence
5. LED status accuracy

### Automated Testing (Future)
- Unit tests for modules
- API endpoint tests
- Integration tests
- Stress tests

## Future Enhancements

### Planned Features
- [ ] WPA3 support
- [ ] IPv6 support
- [ ] Port-based filtering
- [ ] Scheduled rules
- [ ] Email alerts
- [ ] SNMP monitoring
- [ ] MQTT integration
- [ ] OTA updates
- [ ] Multiple SSID support
- [ ] AP mode fallback

### Architecture Improvements
- [ ] FreeRTOS integration
- [ ] Message queue system
- [ ] Plugin architecture
- [ ] Configuration UI wizard
- [ ] Backup/restore config

---

*This architecture document is maintained alongside the codebase.*
