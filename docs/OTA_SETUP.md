# Over-The-Air (OTA) Updates

## Overview

The Twinkling Tree supports OTA (Over-The-Air) firmware updates via WiFi, allowing you to update the code wirelessly without connecting a USB cable. This works **alongside** the BLE RemoteXY interface without conflicts.

## Configuration

1. **Edit WiFi credentials** in [`include/ota_config.h`](../include/ota_config.h):
   ```cpp
   #define WIFI_SSID "YourWiFiSSID"
   #define WIFI_PASSWORD "YourWiFiPassword"
   ```

2. **Optional**: Change the OTA password (default: `tree2025`):
   ```cpp
   #define OTA_PASSWORD "tree2025"
   ```

3. **Optional**: Change the hostname (default: `TwinklingTree`):
   ```cpp
   #define OTA_HOSTNAME "TwinklingTree"
   ```

## How It Works

- **WiFi + BLE coexist**: The ESP32-S3 runs WiFi (for OTA) and BLE (for RemoteXY) simultaneously
- **Non-blocking**: If WiFi fails to connect (15 second timeout), OTA is disabled and the device continues normal operation
- **Automatic discovery**: Once connected, the device advertises itself on your network as `TwinklingTree.local`
- **mDNS support**: The device uses mDNS to allow discovery via hostname instead of IP address

## Uploading via OTA

### Understanding the Environment Setup

The `platformio.ini` file is configured with **three environments**:

1. **`seeed_xiao_esp32s3`** - Base configuration (shared settings)
2. **`usb`** - USB upload via serial connection (extends base)
3. **`ota`** - OTA upload via WiFi (extends base)

**Why separate environments?**

PlatformIO selects the upload protocol **before** running any pre-scripts, so automatic discovery cannot dynamically switch between USB and OTA. Instead, you manually select the appropriate environment based on how you want to upload.

### Method 1: Using the Environment Selector (Recommended)

**For initial upload or when WiFi is unavailable:**
```bash
pio run -e usb --target upload
```

**For wireless updates after OTA firmware is installed:**
```bash
pio run -e ota --target upload
```

The `ota` environment is configured to use:
```ini
upload_protocol = espota
upload_port = TwinklingTree.local
upload_flags =
    --port=3232
    --auth=tree2025
```

### Method 2: Manual Port Override

You can override the upload port for either environment:

```bash
# Using hostname (requires mDNS)
pio run -e ota --target upload --upload-port TwinklingTree.local

# Using IP address (more reliable if mDNS fails)
pio run -e ota --target upload --upload-port 192.168.1.100
```

To find the device IP address:
- Check the Serial Monitor output after device boots
- Look in your router's DHCP client list for "TwinklingTree"
- Use `pio device list --mdns` to scan for mDNS devices
- Use a network scanner app

1. After flashing the OTA-enabled firmware once via USB
2. In Arduino IDE, go to **Tools > Port**
3. Select **TwinklingTree at [IP address]** from the network ports
4. Upload as normal

## Typical Workflow

### First Time Setup
1. **Configure WiFi** in `include/ota_config.h`
2. **Upload via USB** to install OTA-capable firmware:
   ```bashusing `-e usb` - OTA firmware needs to be flashed first
- Verify WiFi credentials are correct in `include/ota_config.h`
- Check that device and computer are on the same network (not guest network)
- Some routers block mDNS - try using the IP address instead of `TwinklingTree.local`:
  ```bash
  pio run -e ota --target upload --upload-port 192.168.1.100
  ```
- Wait 15-20 seconds after power-up for WiFi to connect (15 second timeout)
- Check Serial Monitor for WiFi connection status and IP address
- Run `pio device list --mdns` to see if device is discovered
- On Windows, ensure **Bonjour service is running** (installed with iTunes or standalone)
- Check firewall - allow port 3232 and mDNS port 5353 (UDP)

### OTA timeout/failed
- Make sure the OTA password matches in `include/ota_config.h` (default: `tree2025`)
- Verify the device is powered and connected to WiFi (check Serial Monitor)
