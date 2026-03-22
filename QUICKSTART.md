# Quick Reference Guide

## 🚀 Quick Start Commands

### Build Project
```bash
./scripts/build.sh build
```

### Full Rebuild
```bash
./scripts/build.sh rebuild
```

### Clean Build Directory
```bash
./scripts/build.sh clean
```

### Create Config Template
```bash
./scripts/build.sh config
```

## 📡 Default Network Information

| Parameter | Value |
|-----------|-------|
| HTTP Port | 80 |
| Protocol | HTTP |
| Hostname | pico-firewall |

## 🔌 API Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/status` | GET | Get system status |
| `/api/blocklist` | GET | Get blocked IPs |
| `/api/blocklist` | POST | Block an IP |
| `/api/blocklist` | DELETE | Unblock an IP |
| `/api/logs` | GET | Get firewall logs |
| `/api/logs` | DELETE | Clear logs |
| `/api/config` | GET | Get config |
| `/api/config` | POST | Update config |
| `/api/reboot` | POST | Reboot device |

## 💡 LED Status Codes

| Pattern | Meaning |
|---------|---------|
| ⚫ Off | Idle |
| 🟡 Slow blink | Connecting to WiFi |
| 🟢 Solid | Connected |
| 🔴 Fast blink | Error |
| ⚡ Quick pulse | Activity |

## 📁 Key Files

| File | Purpose |
|------|---------|
| `src/main.c` | Main application |
| `include/config.h` | Configuration header |
| `src/config.c` | Flash storage |
| `lwipopts.h` | Network settings |
| `CMakeLists.txt` | Build config |

## 🔧 Common Issues

### Can't Build
```bash
# Check SDK path
echo $PICO_SDK_PATH

# Clean and rebuild
./scripts/build.sh rebuild
```

### Can't Connect to WiFi
- Check SSID and password
- Ensure 2.4GHz network
- Check signal strength

### Can't Access Dashboard
- Check device IP in serial output
- Ensure same network
- Ping the device IP

## 📊 Memory Map

| Section | Size |
|---------|------|
| Flash | 2MB |
| SRAM | 256KB |
| Network Heap | 8KB |
| Config Storage | 1KB |

## 🔧 Development Tips

### Enable Debug Output
Edit `lwipopts.h`:
```c
#define LWIP_DEBUG  1
#define TCP_DEBUG   LWIP_DBG_ON
```

### Monitor Serial Output
```bash
# Linux
screen /dev/ttyACM0 115200

# Or use picotool
picotool monitor
```

### Flash Manually
1. Hold BOOTSEL
2. Connect USB
3. Copy .uf2 file
4. Auto-reboot

## 📈 Performance Metrics

| Metric | Target |
|--------|--------|
| Boot time | <10s |
| HTTP response | <100ms |
| Max connections | 4-8 |
| Memory usage | <20KB |

## 🛠️ Module Overview

| Module | Purpose |
|--------|---------|
| `config` | Flash storage |
| `firewall` | IP blocking |
| `http_server` | Web server |
| `network_monitor` | Statistics |
| `system_monitor` | Health |
| `web_ui` | HTML/JSON |

## 📝 Example API Usage

### Get Status
```bash
curl http://192.168.1.100/api/status
```

### Block IP
```bash
curl -X POST -d "ip=192.168.1.100" \
  http://192.168.1.100/api/blocklist
```

### Unblock IP
```bash
curl -X DELETE -d "ip=192.168.1.100" \
  http://192.168.1.100/api/blocklist
```

### Get Logs
```bash
curl http://192.168.1.100/api/logs
```

## 🎯 Next Steps

1. **Configure WiFi** - Edit credentials in `src/config.c`
2. **Build** - Run `./scripts/build.sh build`
3. **Flash** - Copy .uf2 to Pico W
4. **Access** - Open browser to device IP
5. **Monitor** - Use dashboard for real-time stats

---

For detailed documentation, see [Readme.md](Readme.md)
