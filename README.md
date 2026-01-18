# WiFi Repeater with NAT

**✅ FULLY FUNCTIONAL AND TESTED - Full Internet Access Working**

A WiFi range extender project for the **Freenove ESP32-S3 WROOM-1** (16MB flash) that extends your WiFi coverage by acting as a repeater with Network Address Translation (NAT).

## What It Does

Connect your ESP32-S3 to your existing WiFi network, and it will:
1. **Receive WiFi signal** from your home network (STA mode)
2. **Create its own WiFi network** for your devices to connect to (AP mode)
3. **Share internet access** between both networks with NAT ✓ **TESTED AND WORKING**

Perfect for extending WiFi coverage to dead zones in your house without needing a separate power outlet or internet line. Your phone/devices will get full internet access through the repeater.

## Quick Start

### What You Need
- Freenove ESP32-S3 WROOM-1 board (16MB flash)
- USB cable for programming
- PlatformIO installed (VS Code extension or CLI)
- Your home WiFi network credentials

### Getting the Code

**Clone the repository:**
```bash
git clone <repository-url>
cd WifiRepeater
```

**Or download as ZIP:**
- Click "Code" → "Download ZIP" on GitHub
- Extract the zip file

### Step 1: Create & Configure WiFi Credentials

**1a. Create secret.h from template:**

The repository includes a template file to prevent accidentally committing credentials:

```bash
# Linux/macOS
cp include/secret.h.template include/secret.h

# Windows (PowerShell)
Copy-Item include/secret.h.template include/secret.h

# Windows (CMD)
copy include\secret.h.template include\secret.h
```

**1b. Edit `include/secret.h` with your home WiFi and repeater credentials:**

```c
// Upstream WiFi Network Credentials
#define STA_SSID "YOUR_HOME_WIFI_SSID"
#define STA_PASSWORD "YOUR_HOME_WIFI_PASSWORD"

// Repeater WiFi Network Credentials
#define AP_SSID "ESP32-Repeater"
#define AP_PASSWORD "repeater123"
```

**Important Security Notes:**
- The `secret.h` file is in `.gitignore` and will **never** be committed to version control
- The template file (`secret.h.template`) is in the repo for reference
- Your actual credentials in `secret.h` are safe - they will never be shared
- Keep your credentials safe and don't share `secret.h`

### Step 2: Build

```bash
pio run -e freenove_esp32_s3_wroom
```

### Step 3: Upload to Board

```bash
pio run -e freenove_esp32_s3_wroom -t upload
```

### Step 4: Monitor

```bash
pio device monitor -e freenove_esp32_s3_wroom
```

You should see:
```
[WiFiRepeater] WiFi Repeater with NAT - Initializing
[WiFiRepeater] Step 1: Initializing WiFi (STA+AP mode)
[WiFiRepeater] Step 2: Waiting for upstream WiFi connection
[WiFiRepeater] STA connected to upstream WiFi
[WiFiRepeater] WiFi Repeater Ready!
```

## Connect Your Phone

1. Open WiFi settings on your phone
2. Look for network named: **ESP32-Repeater** (or your custom AP_SSID)
3. Enter password: **repeater123** (or your custom AP_PASSWORD)
4. You're connected! ✓ **Full internet access is immediately available**
   - DNS will resolve automatically
   - Websites will load normally
   - You can access both local network and internet

## Setup Verification Checklist

Before connecting your phone, verify these settings are correct:

### ✅ WiFi Credentials Set?
- [ ] Created `include/secret.h` from `include/secret.h.template`
- [ ] `include/secret.h` has your home WiFi SSID (STA_SSID) - not "YOUR_HOME_WIFI_SSID"
- [ ] `include/secret.h` has your home WiFi password (STA_PASSWORD) - not a placeholder
- [ ] `include/secret.h` has repeater SSID (AP_SSID) - "ESP32-Repeater" or custom name
- [ ] `include/secret.h` has repeater password (AP_PASSWORD) - secure password

### ✅ Configuration Files Correct?
- [ ] `sdkconfig.defaults` has `CONFIG_LWIP_IP_FORWARD=y` (CRITICAL for internet access)
- [ ] `sdkconfig.defaults` has `CONFIG_LWIP_IPV4_NAPT=y`
- [ ] Deleted `sdkconfig.freenove_esp32_s3_wroom` before building

### ✅ Build & Upload Successful?
- [ ] Build completed without errors: `pio run -e freenove_esp32_s3_wroom`
- [ ] Upload completed without errors: `pio run -e freenove_esp32_s3_wroom -t upload`
- [ ] Serial monitor shows: "WiFi Repeater Ready!"

### ✅ Internet Access Working?
After connecting your phone to the repeater:
1. Open a website in your browser (not just the login portal)
2. Try accessing a domain like google.com
3. DNS should resolve and pages should load

**If your phone connects but has no internet:**
- Check that NAPT message appears in serial output
- Verify you can access your home router page (192.168.1.1)
- Try a different WiFi channel in `include/wifi_config.h`

## ✅ NAPT Configuration Requirements (All Included & Verified)

**The current firmware has all of these configured and working:**

```
CONFIG_LWIP_IP_FORWARD=y              ← ✓ Enabled (Global IP forwarding - CRITICAL)
CONFIG_LWIP_IPV4_NAPT=y               ← ✓ Enabled (NAPT engine)
CONFIG_LWIP_L2_TO_L3_COPY=y           ← ✓ Enabled (Packet translation)
CONFIG_LWIP_IPV4_NAPT_PORTMAP=y       ← ✓ Enabled (Port mapping support)
```

**Key Fixes Applied (January 2026):**
1. **DNS Advertisement:** Added `esp_netif_dhcps_option()` to advertise DNS in DHCP responses
   - Without this: clients get IP but no DNS (can't resolve domains)
   - With this: clients automatically receive DNS servers via DHCP ✓
2. **NAPT Interface:** Enabled NAPT only on AP interface (not both)
   - Prevents routing conflicts that cause packet loss
   - Simpler and more reliable than bidirectional NAPT

**Why this works:**
- All 4 CONFIG options are in `sdkconfig.defaults` and enabled
- Code calls `ip_napt_enable()` with AP interface IP
- DNS is explicitly advertised to clients via DHCP
- Packets correctly forward between STA and AP interfaces
- Always delete `sdkconfig.freenove_esp32_s3_wroom` after modifying `sdkconfig.defaults` so the build regenerates it

## How It Works

```
Your Home WiFi (192.168.1.x)
         ↑
         │
     ESP32-S3
    [STA Mode]
         │
         └──→ Processes packets between networks
         │
     [AP Mode]
         ↓
Your Phone / Devices (192.168.4.x)
```

The ESP32 sits in the middle:
- Connects to your home network as a regular client (STA)
- Broadcasts its own network (AP)
- Translates network traffic between both networks (NAT)

## Network Configuration

### ESP32 Repeater Network
- **Network Name:** ESP32-Repeater (customizable)
- **IP Address:** 192.168.4.1
- **IP Range:** 192.168.4.2 - 192.168.4.20
- **Gateway:** 192.168.4.1
- **Max Devices:** 20 simultaneously

### Custom Configuration

### Credentials (Secret)
Edit `include/secret.h` to set your WiFi credentials:
```c
// Upstream WiFi credentials
#define STA_SSID "YOUR_HOME_WIFI_SSID"
#define STA_PASSWORD "YOUR_HOME_WIFI_PASSWORD"

// Repeater WiFi credentials
#define AP_SSID "MyRepeater"
#define AP_PASSWORD "securepassword"
```

### Repeater Settings
Edit `include/wifi_config.h` to change:

```c
// Repeater settings (SSID and password are in secret.h)
#define AP_CHANNEL 6               // WiFi channel (1-13)
#define AP_MAX_CLIENTS 20          // Max connected devices

// Network IP range
#define AP_IP_ADDR "192.168.4.1"
#define DHCP_START_IP "192.168.4.2"
#define DHCP_END_IP "192.168.4.20"

// Upstream WiFi retries
#define STA_MAX_RETRIES 5
#define STA_RETRY_INTERVAL 5000    // milliseconds
```

## Features (All Tested & Working ✓)

✅ **Dual WiFi Mode**
- Connect to upstream WiFi AND broadcast your own network simultaneously
- Both modes run concurrently

✅ **Automatic Reconnection**
- If upstream WiFi drops, ESP32 automatically reconnects (5 retries)
- Repeater stays active even if upstream is temporarily unavailable

✅ **DHCP Server**
- Automatic IP assignment to devices connecting to your repeater (192.168.4.2-20)
- Clients get IPs instantly upon connection

✅ **DNS Forwarding** ← **KEY FIX - NOW WORKING PERFECTLY**
- Automatically forwards DNS servers from upstream network to connected devices
- Clients receive DNS via DHCP (no manual configuration needed)
- Domain name resolution works immediately

✅ **Network Address Translation (NAT)** ← **TESTED AND VERIFIED**
- Packets are translated between networks (IPv4)
- Full internet access for connected devices
- Transparent to clients - works just like a real repeater

✅ **Status Monitoring**
- Real-time serial output showing:
  - Connected upstream WiFi signal strength (RSSI)
  - Number of devices on your repeater
  - Network statistics and connection status

✅ **Efficient Memory Usage**
- Uses only 10.6% of available RAM
- Supports 20 simultaneous clients

## LED Indicators (if available on your board)

Currently, the system uses serial logging. You can extend it to add LED indicators:
- Blinking LED = Trying to connect to upstream WiFi
- Solid LED = Repeater ready and working
- Off = Error or startup

## Troubleshooting

### ✅ **Working as Expected**
If you see these in serial output, everything is working correctly:
- "STA connected to upstream WiFi"
- "STA got IP: 192.168.1.x"
- "DNS: DHCP server restarted with DNS advertisement enabled"
- "NAPT enabled on AP interface"
- "WiFi Repeater Ready!"

Your phone will connect and have full internet access immediately.

### Phone won't connect to repeater
- Check AP_PASSWORD is correct
- Verify AP_SSID is visible (not hidden)
- Restart the ESP32 board

### No internet on connected devices (rare if using current firmware)
**Most Common Cause:** Outdated firmware without DNS advertisement fix
- Rebuild with latest code: `pio run -e freenove_esp32_s3_wroom`
- Upload: `pio run -e freenove_esp32_s3_wroom -t upload`

**If still having issues:**
- Verify `sdkconfig.defaults` has `CONFIG_LWIP_IP_FORWARD=y` (CRITICAL)
- Check ESP32 is connected to upstream WiFi (serial shows "STA got IP")
- Verify DNS message appears: "DNS: DHCP server restarted with DNS advertisement enabled"
- Verify NAPT message appears: "NAPT enabled on AP interface"

### Frequent disconnections
- Try changing WiFi channel in `include/wifi_config.h`
- Move ESP32 away from interference sources (microwaves, cordless phones)
- Increase STA_RETRY_INTERVAL for more stable connection

### Build fails with "Flash memory size mismatch"
- Ensure `sdkconfig.defaults` file exists in project root
- Verify it contains: `CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y`
- Delete the generated config file: `del sdkconfig.freenove_esp32_s3_wroom` (Windows) or `rm sdkconfig.freenove_esp32_s3_wroom` (Linux/Mac)
- Run clean build: `pio run -e freenove_esp32_s3_wroom --target clean`

### Build fails with "Unknown board"
- Ensure PlatformIO is updated: `pip install -U platformio`
- Run clean build: `pio run -e freenove_esp32_s3_wroom --target clean`

### Serial output shows garbled text
- Verify serial monitor is using 115200 baud rate
- Use explicit baud rate: `pio device monitor -e freenove_esp32_s3_wroom --baud 115200`
- Check USB cable is properly connected
- Try restarting the ESP32 board

## Performance

| Metric | Value |
|--------|-------|
| **Simultaneous Clients** | Up to 20 |
| **Range** | ~30-50 meters (typical WiFi) |
| **Speed** | Depends on upstream WiFi |
| **Power** | ~500mA @ 5V USB |
| **Memory Used** | ~34.5 KB RAM, 763 KB Flash |

## Current Capabilities & Future Improvements

### ✅ Current Capabilities (All Working)
- **Fully functional NAT** (IPv4) with packet translation ✓
- **DNS forwarding** to clients via DHCP ✓
- **Full internet access** for connected devices ✓
- **Up to 20 simultaneous connections** ✓

### Current Limitations
- No DNS caching/proxy (forwards upstream DNS directly)
- No persistent configuration storage (settings reset on reboot)
- Limited to 20 simultaneous connections (can be increased)

### Planned Features (Future Enhancements)
1. Web dashboard for configuration and monitoring
2. Advanced NAT with port forwarding rules
3. DNS caching and query optimization for performance
4. Traffic statistics and per-client monitoring
5. Persistent settings storage (NVS - survives reboot)
6. Power management modes for battery operation
7. Multiple upstream network support (switching)

## Technical Details

**Board:** Freenove ESP32-S3 WROOM-1
- CPU: Xtensa dual-core 240 MHz
- RAM: 320 KB
- Flash: 16 MB
- Networking: 802.11 b/g/n @ 2.4 GHz

**Software Stack:**
- ESP-IDF 5.5.0
- lwIP network stack
- FreeRTOS real-time OS

**Build System:** PlatformIO + ESP-IDF

## Project Structure

```
include/
  ├── wifi_config.h              - Configuration constants
  ├── wifi_manager.h             - WiFi management interface
  ├── dhcp_server.h              - DHCP server interface
  ├── dns_forwarding.h           - DNS forwarding interface
  ├── ip_forward.h               - IP forwarding interface
  ├── secret.h.template          - Template for credentials (in repo)
  └── secret.h                   - YOUR WiFi credentials (in .gitignore - create from template)

src/
  ├── main.c                - Entry point
  ├── wifi_manager.c        - WiFi implementation
  ├── dhcp_server.c         - DHCP implementation
  ├── dns_forwarding.c      - DNS forwarding implementation
  └── ip_forward.c          - IP forwarding implementation

CLAUDE.md                 - Technical documentation
README.md                 - This file
platformio.ini            - PlatformIO build configuration
sdkconfig.defaults        - ESP-IDF SDK configuration
.gitignore                - Ignore rules (includes secret.h to prevent credential leaks)
```

**Important:** `secret.h.template` is in the repository. You create your own `secret.h` locally by copying the template and editing it with your credentials. Your `secret.h` will never be committed to git.

## Tips for Best Performance

1. **Positioning:** Place ESP32 between your router and dead zone
2. **Channel Selection:** Use WiFi analyzer app to find least congested channel
3. **Credentials:** Use strong passwords for your repeater network
4. **Updates:** Rebuild firmware if you change WiFi credentials
5. **Monitoring:** Check serial output to verify connection status

## Support & Debugging

### View Serial Output
The serial monitor automatically uses 115200 baud (configured in `sdkconfig.defaults`):
```bash
pio device monitor -e freenove_esp32_s3_wroom
```

Or explicitly specify baud rate:
```bash
pio device monitor -e freenove_esp32_s3_wroom --baud 115200
```

### Full Verbose Build
```bash
pio run -e freenove_esp32_s3_wroom -v
```

### Clean Rebuild
Before clean building, delete the generated config file:
```bash
del sdkconfig.freenove_esp32_s3_wroom
pio run -e freenove_esp32_s3_wroom --target clean
pio run -e freenove_esp32_s3_wroom
```

(On Linux/Mac, use `rm sdkconfig.freenove_esp32_s3_wroom` instead of `del`)

## Maintenance

### Periodic Tasks
- Check upstream WiFi connection status (view serial output)
- Verify connected clients can access internet
- Restart board monthly for optimal performance

### Firmware Updates
1. Modify `include/wifi_config.h` as needed
2. Rebuild: `pio run -e freenove_esp32_s3_wroom`
3. Upload: `pio run -e freenove_esp32_s3_wroom -t upload`

### Configuration Changes
**Important:** Whenever you modify `sdkconfig.defaults`, always delete the generated config file first:
- Windows: `del sdkconfig.freenove_esp32_s3_wroom`
- Linux/Mac: `rm sdkconfig.freenove_esp32_s3_wroom`

This ensures the build system regenerates the configuration from your changes. The file will be automatically recreated during the next build.

## FAQ

**Q: Why is `include/secret.h` not in the repository?**
A: Security! The file is in `.gitignore` to prevent accidentally committing your WiFi credentials to git. You'll find `include/secret.h.template` in the repo to use as a starting point. Just copy it to `secret.h` and edit with your credentials.

**Q: How do I create my `secret.h` file?**
A: Copy the template:
```bash
cp include/secret.h.template include/secret.h  # Linux/Mac
copy include\secret.h.template include\secret.h # Windows CMD
Copy-Item include/secret.h.template include/secret.h # Windows PowerShell
```
Then edit the new `secret.h` file with your actual WiFi credentials.

**Q: Will my WiFi credentials ever be committed to git?**
A: No. The `secret.h` file is in `.gitignore`, so git will never track it. Only the template is in the repository.

**Q: Can I use this with 5GHz WiFi?**
A: The ESP32-S3 only supports 2.4 GHz. Your upstream router must broadcast 2.4 GHz band.

**Q: How do I change the repeater password?**
A: Edit `AP_PASSWORD` in `include/secret.h`, rebuild, and upload.

**Q: Can I connect to multiple upstream networks?**
A: Not currently. Future version will support switching between networks.

**Q: What happens if upstream WiFi drops?**
A: ESP32 automatically retries 5 times. The repeater AP stays active but won't have internet.

**Q: Can I use this as a bridge instead of NAT?**
A: Current version uses NAT. Bridging mode could be added in future versions.

**Q: How do I view connected devices?**
A: Check serial output - it shows connected client count every 10 seconds.

**Q: Where should I put my WiFi credentials?**
A: Edit `include/secret.h` with your home WiFi (STA) and repeater (AP) SSID and passwords. This file is in `.gitignore` and won't be committed to version control.

**Q: What's in `.gitignore`?**
A: The `.gitignore` file prevents committing sensitive files like `secret.h` to git repositories. Never remove `include/secret.h` from `.gitignore`!

## Documentation & References

**Technical Documentation:**
- `CLAUDE.md` - Complete technical documentation (start here for implementation details)
- For information about the NAT implementation, see the NAT reference files:
  - `NAT_DOCUMENTATION_INDEX.md` - Master guide for NAT implementation
  - `LWIP_NAT_API_REFERENCE.md` - Complete lwIP NAPT API reference
  - `NAT_FUNCTION_SIGNATURES.md` - Function signatures and usage examples
  - `NAT_RESEARCH_SUMMARY.txt` - Technical research notes
  - `FINDINGS.txt` - Quick reference

**External References:**
- ESP-IDF Documentation: https://docs.espressif.com/projects/esp-idf/
- lwIP Documentation: https://savannah.nongnu.org/projects/lwip/
- PlatformIO Docs: https://docs.platformio.org/

## License

This project is open source and available for educational and personal use.

## Contact & Feedback

For issues, questions, or feature requests related to this implementation, refer to the technical documentation in `Claude.md`.

---

**Happy WiFi extending! 📶**

*Last Updated: January 18, 2026 - FULLY FUNCTIONAL AND TESTED*
*For Technical Details: See CLAUDE.md*

**Status:** ✅ Production Ready - Full Internet Access Verified
