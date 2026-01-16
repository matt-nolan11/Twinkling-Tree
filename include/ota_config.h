#pragma once

// ===========================
// OTA Configuration
// ===========================

// WiFi credentials for OTA updates
// NOTE: Change these to match your network
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASSWORD"

// OTA settings
#define OTA_HOSTNAME "TwinklingTree"
#define OTA_PASSWORD "tree2025"  // Password for OTA updates (optional, can be empty)

// OTA timeout settings
#define WIFI_CONNECT_TIMEOUT 15000  // 15 seconds to connect to WiFi
#define OTA_PORT 3232                // Default ArduinoOTA port
