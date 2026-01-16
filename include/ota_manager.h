#pragma once

#include <Arduino.h>

/**
 * Initialize OTA (Over-The-Air) update capability
 * Attempts to connect to WiFi and setup ArduinoOTA
 * If WiFi connection fails, OTA is disabled and normal operation continues
 * This is safe to call even if WiFi credentials are incorrect
 */
void initOTA();

/**
 * Handle OTA updates in main loop
 * Must be called regularly for OTA to function
 * Safe to call even if OTA is disabled
 */
void handleOTA();

/**
 * Check if OTA is enabled and ready
 * @return true if WiFi connected and OTA is active
 */
bool isOTAEnabled();

/**
 * Get OTA status string for debugging
 * @return Status message with IP address if connected
 */
String getOTAStatus();
