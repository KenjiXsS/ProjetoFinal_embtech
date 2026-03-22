# 🛡️ Pico Firewall Monitor

A powerful, feature-rich network monitoring and firewall management system for the Raspberry Pi Pico W with a modern web-based dashboard.

![Version](https://img.shields.io/badge/version-2.0.0-blue)
![License](https://img.shields.io/badge/license-BSD--3-green)
![Platform](https://img.shields.io/badge/platform-Raspberry%20Pi%20Pico%20W-orange)

## 🌟 Features

### Core Functionality
- **🔒 Dynamic IP Blocking** - Block/unblock IPs in real-time via web interface
- **📊 Real-time Monitoring** - Live traffic statistics and connection tracking
- **🌐 Web Dashboard** - Modern, responsive UI accessible from any browser
- **🔌 JSON API** - RESTful API for integration with other systems
- **💾 Persistent Storage** - Configuration saved to flash memory
- **📈 Traffic Analytics** - Track bytes sent/received and connection counts
- **🔔 System Health** - Monitor memory, uptime, and connection status
- **💡 LED Indicators** - Visual status feedback via onboard LED

### Technical Highlights
- Modular architecture with clean code separation
- Optimized lwIP configuration for web server performance
- Flash-based configuration storage with checksum validation
- Watchdog timer for system reliability
- CORS-enabled API for cross-origin requests
- Auto-refresh dashboard (3-second intervals)
- Connection backlog for handling multiple clients

## 📋 Prerequisites

### Hardware
- Raspberry Pi Pico W
- USB cable for programming
- Wi-Fi network (2.4GHz)

### Software
- **Raspberry Pi Pico SDK** (v2.0.0 or later)
- **CMake** (v3.13 or later)
- **GCC ARM Embedded** toolchain
- **picotool** (optional, for flashing)

### Environment Setup

#### Linux
```bash
# Install Pico SDK
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init
export PICO_SDK_PATH=$(pwd)

# Install toolchain (Ubuntu/Debian)
sudo apt install gcc-arm-none-eabi cmake build-essential
```

#### Windows
```powershell
# Use the Pico VS Code Extension or:
# 1. Download and install Raspberry Pi Pico SDK
# 2. Set environment variable: PICO_SDK_PATH
# 3. Install GNU ARM Embedded Toolchain
```

#### macOS
```bash
# Install via Homebrew
brew install --cask gcc-arm-embedded
brew install cmake

# Clone and setup SDK
git clone https://github.com/raspberrypi/pico-sdk.git
export PICO_SDK_PATH=$(pwd)/pico-sdk
```

## 🚀 Quick Start

### 1. Configure Wi-Fi Credentials

Edit the Wi-Fi settings in `src/main.c` or use the web dashboard after first flash:

```c
// In config.c, update default credentials
strncpy(config->ssid, "YourWiFiSSID", MAX_SSID_LENGTH - 1);
strncpy(config->password, "YourWiFiPassword", MAX_PASSWORD_LENGTH - 1);
```

### 2. Build the Project

```bash
# Navigate to project directory
cd ProjetoFinal_embtech

# Run build script
./scripts/build.sh build

# Or manually
mkdir build && cd build
cmake .. -DPICO_SDK_PATH=$PICO_SDK_PATH
cmake --build . -j4
```

### 3. Flash to Pico W

**Method 1: Drag and Drop (Recommended)**
1. Hold the **BOOTSEL** button on the Pico W
2. Connect to USB while holding BOOTSEL
3. Release BOOTSEL when the drive appears
4. Copy `build/pico_firewall_monitor.uf2` to the drive
5. Device will reboot automatically

**Method 2: Using picotool**
```bash
picotool load build/pico_firewall_monitor.uf2
picotool reboot
```

### 4. Access the Dashboard

1. Open serial monitor to get the IP address (optional)
2. Connect to the same Wi-Fi network
3. Open browser and navigate to the displayed IP address
4. Example: `http://192.168.1.100`

## 🎛️ Web Dashboard

### Dashboard Page
- **Network Status** - WiFi status, IP address, subnet, gateway
- **Traffic Statistics** - Total/active connections, blocked attempts, data transfer
- **System Health** - Uptime, memory usage, firewall status

### Blocklist Page
- View all blocked IPs
- Add new IPs to blocklist
- Remove IPs from blocklist
- Changes persist across reboots

### Logs Page
- View firewall action logs
- See blocked connection attempts
- Clear log history
- Real-time log updates

### Settings Page
- Update Wi-Fi credentials
- Save configuration to flash
- Reboot device remotely

## 🔌 API Reference

### GET /api/status
Returns system status and statistics.

**Response:**
```json
{
  "wifi_status": "Connected",
  "ip_address": "192.168.1.100",
  "netmask": "255.255.255.0",
  "gateway": "192.168.1.1",
  "total_connections": 42,
  "active_connections": 3,
  "blocked_connections": 5,
  "bytes_received": 1024000,
  "bytes_sent": 512000,
  "uptime": "00:15:32",
  "memory_usage": 15.5,
  "blocked_ips_count": 3,
  "firewall_enabled": true
}
```

### GET /api/blocklist
Returns list of blocked IPs.

**Response:**
```json
{
  "ips": ["192.168.1.100", "10.0.0.50"]
}
```

### POST /api/blocklist
Add an IP to the blocklist.

**Request:** `ip=192.168.1.100`

**Response:**
```json
{
  "status": "ok",
  "ip": "192.168.1.100"
}
```

### DELETE /api/blocklist
Remove an IP from the blocklist.

**Request:** `ip=192.168.1.100`

**Response:**
```json
{
  "status": "ok",
  "ip": "192.168.1.100"
}
```

### GET /api/logs
Returns firewall logs.

**Response:**
```json
{
  "logs": [
    {
      "uptime": 123,
      "ip": "192.168.1.100",
      "action": "Blocked by IP filter",
      "blocked": true
    }
  ]
}
```

### DELETE /api/logs
Clear all firewall logs.

### GET /api/config
Returns current configuration (password masked).

### POST /api/config
Update Wi-Fi configuration.

**Request:** `ssid=NewSSID&password=NewPassword`

### POST /api/reboot
Reboot the device.

## 📁 Project Structure

```
ProjetoFinal_embtech/
├── include/                    # Header files
│   ├── config.h               # Configuration management
│   ├── firewall.h             # Firewall logic
│   ├── http_server.h          # HTTP server
│   ├── network_monitor.h      # Network statistics
│   ├── system_monitor.h       # System health
│   └── web_ui.h               # Web interface
├── src/                       # Source files
│   ├── main.c                 # Application entry point
│   ├── config.c               # Flash storage
│   ├── firewall.c             # IP blocking
│   ├── http_server.c          # Web server
│   ├── network_monitor.c      # Statistics
│   ├── system_monitor.c       # Health monitoring
│   └── web_ui.c               # HTML/JSON generation
├── scripts/                   # Build utilities
│   └── build.sh               # Build automation
├── CMakeLists.txt             # Build configuration
├── lwipopts.h                 # lwIP settings
└── Readme.md                  # This file
```

## ⚙️ Configuration

### Memory Optimization
The lwIP stack is configured in `lwipopts.h`:
- `MEM_SIZE`: Heap size for network operations (8KB)
- `TCP_WND`: TCP window size (16 * MSS)
- `MEMP_NUM_TCP_SEG`: Max TCP segments (64)

### Flash Storage
Configuration is stored at 1MB offset in flash:
- Wi-Fi credentials (encrypted in production)
- Blocked IP list (up to 20 IPs)
- Device hostname
- Configuration version and checksum

### LED Status Codes
| Pattern | Status |
|---------|--------|
| Off | Idle/Off |
| Slow blink | Connecting to Wi-Fi |
| Solid on | Connected |
| Fast blink | Error |
| Quick pulse | Network activity |

## 🔧 Troubleshooting

### Device won't connect to Wi-Fi
1. Check Wi-Fi credentials in configuration
2. Ensure 2.4GHz network (Pico W doesn't support 5GHz)
3. Check Wi-Fi signal strength
4. Verify router MAC filtering settings

### Can't access web dashboard
1. Check serial output for IP address
2. Ensure device is on same network
3. Try pinging the device IP
4. Check firewall on your computer

### Build errors
1. Verify `PICO_SDK_PATH` is set correctly
2. Update Pico SDK to latest version
3. Clean build: `./scripts/build.sh clean`
4. Check toolchain installation

### Configuration not saving
1. Flash storage may be corrupted
2. Try resetting configuration
3. Check flash integrity

## 🛠️ Development

### Building in Debug Mode
```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DPICO_SDK_PATH=$PICO_SDK_PATH
cmake --build . -j4
```

### Enable Debug Output
Uncomment in `lwipopts.h`:
```c
#define LWIP_DEBUG  1
#define TCP_DEBUG   LWIP_DBG_ON
```

### Serial Output
The device outputs debug info via USB serial at 115200 baud.

## 📊 Performance

- **Boot time**: ~5 seconds
- **HTTP response**: <100ms
- **Max concurrent connections**: 4-8
- **Memory usage**: ~15-20KB
- **Flash usage**: ~150KB

## 🔐 Security Considerations

⚠️ **This is a demonstration project**. For production use:

1. **Encrypt stored passwords** - Currently stored in plaintext
2. **Add authentication** - No login required for dashboard
3. **Enable HTTPS** - Currently HTTP only
4. **Rate limiting** - No request throttling
5. **Input validation** - Limited sanitization
6. **Firmware updates** - No secure update mechanism

## 📄 License

This project is licensed under the BSD-3 License - see the LICENSE file for details.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📧 Support

- **Issues**: Open an issue on GitHub
- **Discussions**: GitHub Discussions tab
- **Documentation**: Check the Wiki

## 🙏 Acknowledgments

- Raspberry Pi Foundation for the Pico W
- lwIP TCP/IP stack contributors
- Pico SDK team
- Community contributors and testers

## 📝 Changelog

### v2.0.0 (2026)
- ✨ Complete modular architecture
- ✨ Modern web dashboard with real-time updates
- ✨ JSON API for integration
- ✨ Persistent flash storage
- ✨ System health monitoring
- ✨ LED status indicators
- 🔧 Optimized lwIP configuration
- 🔧 Improved error handling

### v1.0.0 (Previous)
- Basic HTTP server
- Static IP blocking
- Simple network monitoring

---

**Made with ❤️ for the embedded tech community**

*Last updated: March 2026*
