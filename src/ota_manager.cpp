#include "ota_manager.h"
#include "ota_config.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>

static bool otaEnabled = false;
static unsigned long wifiStartTime = 0;

void initOTA() {
  Serial.println("[OTA] Initializing...");
  Serial.print("[OTA] Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  
  // Start WiFi connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  wifiStartTime = millis();
  
  // Wait for connection with timeout
  while (WiFi.status() != WL_CONNECTED && 
         (millis() - wifiStartTime < WIFI_CONNECT_TIMEOUT)) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  
  // Check if WiFi connected successfully
  if (WiFi.status() != WL_CONNECTED) {
    // WiFi failed to connect - disable OTA but continue normal operation
    Serial.println("[OTA] WiFi connection failed - OTA disabled");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    otaEnabled = false;
    return;
  }
  
  Serial.println("[OTA] WiFi connected!");
  Serial.print("[OTA] IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("[OTA] Hostname: ");
  Serial.println(OTA_HOSTNAME);
  
  // Initialize mDNS explicitly for better reliability
  Serial.print("[OTA] Starting mDNS responder...");
  if (!MDNS.begin(OTA_HOSTNAME)) {
    Serial.println(" FAILED!");
    Serial.println("[OTA] mDNS failed to start - use IP address for OTA");
  } else {
    Serial.println(" SUCCESS!");
    Serial.print("[OTA] Accessible at: ");
    Serial.print(OTA_HOSTNAME);
    Serial.println(".local");
  }
  
  // WiFi connected - setup OTA
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPort(OTA_PORT);
  
  // Set OTA password if defined
  if (strlen(OTA_PASSWORD) > 0) {
    ArduinoOTA.setPassword(OTA_PASSWORD);
  }
  
  // Configure OTA callbacks for monitoring
  ArduinoOTA.onStart([]() {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    Serial.println("[OTA] Update started: " + type);
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\n[OTA] Update complete! Rebooting...");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    static unsigned int lastPercent = 0;
    unsigned int percent = (progress / (total / 100));
    if (percent != lastPercent && percent % 10 == 0) {
      Serial.printf("[OTA] Progress: %u%%\n", percent);
      lastPercent = percent;
    }
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[OTA] Error[%u]: ", error);
    switch (error) {
      case OTA_AUTH_ERROR: Serial.println("Auth Failed"); break;
      case OTA_BEGIN_ERROR: Serial.println("Begin Failed"); break;
      case OTA_CONNECT_ERROR: Serial.println("Connect Failed"); break;
      case OTA_RECEIVE_ERROR: Serial.println("Receive Failed"); break;
      case OTA_END_ERROR: Serial.println("End Failed"); break;
      default: Serial.println("Unknown Error"); break;
    }
  });
  
  ArduinoOTA.begin();
  otaEnabled = true;
  
  Serial.println("[OTA] Ready for updates!");
  Serial.println("[OTA] ---");
  Serial.println("[OTA] Upload with IP: pio run --target upload --upload-port " + WiFi.localIP().toString());
  Serial.println("[OTA] Or with mDNS: pio run --target upload --upload-port " + String(OTA_HOSTNAME) + ".local");
  Serial.println("[OTA] ---");
}

void handleOTA() {
  if (otaEnabled) {
    ArduinoOTA.handle();
  }
}

bool isOTAEnabled() {
  return otaEnabled;
}

String getOTAStatus() {
  if (!otaEnabled) {
    return "OTA Disabled (WiFi not connected)";
  }
  
  String status = "OTA Ready - IP: ";
  status += WiFi.localIP().toString();
  status += " (";
  status += OTA_HOSTNAME;
  status += ")";
  return status;
}
