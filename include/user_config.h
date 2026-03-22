#ifndef USER_CONFIG_H
#define USER_CONFIG_H

/**
 * User Configuration File
 * 
 * Edit this file to set your Wi-Fi credentials and other settings.
 * This file is meant to be customized and can be added to .gitignore.
 * 
 * Usage:
 * 1. Copy this file to user_config.h.local
 * 2. Edit the values in user_config.h.local
 * 3. Add user_config.h.local to .gitignore
 * 4. Build the project
 */

// Wi-Fi Credentials
#define DEFAULT_WIFI_SSID     "YourWiFiSSID"
#define DEFAULT_WIFI_PASSWORD "YourWiFiPassword"

// Device Settings
#define DEFAULT_HOSTNAME      "pico-firewall"

// Network Settings (leave empty for DHCP)
#define STATIC_IP_ENABLED     0
#define STATIC_IP             ""
#define STATIC_NETMASK        ""
#define STATIC_GATEWAY        ""
#define STATIC_DNS            ""

// Firewall Settings
#define FIREWALL_ENABLED      1
#define MAX_BLOCKED_IPS       20

// HTTP Server Settings
#define HTTP_PORT             80
#define MAX_CONNECTIONS       8

// LED Settings
#define LED_ENABLED           1

// Debug Settings
#define DEBUG_ENABLED         0
#define DEBUG_SERIAL_BAUD     115200

#endif // USER_CONFIG_H
