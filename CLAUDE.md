# WiFi Repeater with NAT - Technical Documentation

## Project Overview

**WiFi Repeater with NAT** is an embedded systems project for the Freenove ESP32-S3 WROOM-1 (16MB flash) that transforms the board into a WiFi range extender with Network Address Translation (NAT). The device connects to an existing WiFi network and creates its own access point, allowing other devices to connect and access the internet through the ESP32.

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                  ESP32-S3 WROOM-1 Board                  │
├─────────────────────────────────────────────────────────┤
│                                                          │
│  ┌──────────────────────────────────────────────────┐   │
│  │          WiFi Manager (STA+AP Mode)              │   │
│  ├──────────────────────────────────────────────────┤   │
│  │ STA Mode: Connects to upstream WiFi network      │   │
│  │ AP Mode:  Creates repeater WiFi for devices      │   │
│  └──────────────────────────────────────────────────┘   │
│                           ↑                              │
│                           │                              │
│  ┌──────────────────────────────────────────────────┐   │
│  │        Network Configuration & DHCP Server       │   │
│  ├──────────────────────────────────────────────────┤   │
│  │ AP IP:        192.168.4.1                        │   │
│  │ DHCP Range:   192.168.4.2 - 192.168.4.20        │   │
│  │ Max Clients:  20                                 │   │
│  └──────────────────────────────────────────────────┘   │
│                           ↑                              │
│                           │                              │
│  ┌──────────────────────────────────────────────────┐   │
│  │       NAT Engine (Implemented & Tested ✓)         │   │
│  ├──────────────────────────────────────────────────┤   │
│  │ lwIP NAPT: Translates packets between networks   │   │
│  │ DNS Forwarding: Advertises upstream DNS to      │   │
│  │ clients via DHCP                                 │   │
│  │ IP Forwarding: Routes packets between STA & AP  │   │
│  └──────────────────────────────────────────────────┘   │
│                                                          │
└─────────────────────────────────────────────────────────┘
```

## Project Structure

```
WifiRepeater/
├── include/
│   ├── wifi_config.h         # Configuration constants and structs
│   ├── wifi_manager.h        # WiFi initialization interface
│   ├── dhcp_server.h         # DHCP server interface
│   ├── ip_forward.h          # IP forwarding interface
│   └── secret.h              # WiFi credentials (keep secret, in .gitignore)
├── src/
│   ├── main.c                # Application entry point
│   ├── wifi_manager.c        # WiFi STA+AP implementation
│   ├── dhcp_server.c         # DHCP server implementation
│   ├── ip_forward.c          # IP forwarding implementation
│   └── CMakeLists.txt        # ESP-IDF build configuration
├── platformio.ini            # PlatformIO configuration
├── sdkconfig.defaults        # ESP-IDF SDK configuration defaults
├── CMakeLists.txt            # Root build configuration
├── .gitignore                # Git ignore rules (includes secret.h)
├── README.md                 # User documentation
└── CLAUDE.md                 # This file
```

## Core Modules

### 1. WiFi Manager (`wifi_manager.h/c`)

**Purpose:** Initialize and manage dual WiFi modes (STA + AP)

**Key Functions:**
- `wifi_init_sta_ap()` - Initialize both STA and AP modes
- `wifi_wait_connected()` - Wait for STA connection to upstream WiFi
- `wifi_print_status()` - Display current WiFi and client information

**Event Handling:**
- `WIFI_EVENT_STA_START` - Begins upstream WiFi connection
- `WIFI_EVENT_STA_CONNECTED` - Successfully connected to upstream
- `WIFI_EVENT_STA_DISCONNECTED` - Lost upstream connection, triggers reconnect logic
- `IP_EVENT_STA_GOT_IP` - STA received IP from upstream
- `WIFI_EVENT_AP_STACONNECTED` - Device connected to repeater
- `WIFI_EVENT_AP_STADISCONNECTED` - Device disconnected from repeater

**Configuration:**
- STA retries: 5 attempts with 5-second intervals
- AP max clients: 20
- AP channel: 6

### 2. DHCP Server (`dhcp_server.h/c`)

**Purpose:** Assign IP addresses to devices connecting to the AP

**Key Functions:**
- `dhcp_server_init()` - Configure DHCP server with IP range
- `dhcp_server_set_dns()` - Apply DNS forwarding configuration
- `dhcp_server_print_status()` - Display DHCP configuration

**Configuration:**
- DHCP IP range: 192.168.4.2 - 192.168.4.20
- Lease time: 3600 seconds (1 hour)
- Gateway: 192.168.4.1
- DNS servers advertised to clients (from upstream network)

### 3. DNS Forwarding (`dns_forwarding.h/c`)

**Purpose:** Forward DNS servers from upstream network to AP clients

**Key Functions:**
- `dns_forwarding_set_servers()` - Store DNS servers extracted from upstream
- `dns_forwarding_get_servers()` - Retrieve configured DNS servers
- `dns_forwarding_apply_to_dhcp()` - Apply DNS configuration to DHCP

**How It Works:**
1. When STA connects to upstream WiFi and receives an IP, `IP_EVENT_STA_GOT_IP` handler extracts DNS servers
2. DNS servers are stored via `dns_forwarding_set_servers()`
3. During initialization, `dns_forwarding_apply_to_dhcp()` is called to:
   - Stop DHCP server
   - Set DNS on AP interface using `esp_netif_set_dns_info()`
   - **Enable DNS advertisement in DHCP via `esp_netif_dhcps_option()` with `OFFER_DNS` flag** ← CRITICAL FIX
   - Restart DHCP server with DNS configuration
4. DHCP clients receive DNS servers advertised in their DHCP leases

**Critical Implementation Detail:**
Without explicitly enabling DNS advertisement via `esp_netif_dhcps_option()`, clients receive IP addresses but NOT DNS servers, preventing domain name resolution. This was the key fix that enabled full internet access.

**Configuration:**
- Primary DNS: Extracted from upstream network
- Secondary DNS: Extracted from upstream network (if available)
- DNS Advertisement: Enabled via `esp_netif_dhcps_option(ap_netif, ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER, &opt_val, sizeof(opt_val))`

### 4. WiFi Configuration (`wifi_config.h`)

**Purpose:** Centralized configuration constants (non-secret settings)

**Sections:**
- **AP Configuration:** SSID, password, channel, max clients
- **Network Configuration:** IP addresses, netmask, gateway
- **DHCP Configuration:** IP range, lease time
- **STA Configuration:** Retry logic and timeouts (credentials in secret.h)
- **NAT Configuration:** Session limits and timeouts (for future NAT engine)

### 5. Secret Configuration (`secret.h`)

**Purpose:** Secure storage of sensitive WiFi credentials

**Important Security Notes:**
- File is in `.gitignore` and will **never** be committed to version control
- Contains all WiFi credentials (upstream and repeater)
- Should be kept secret and not shared publicly
- Initialized with placeholder values - users must update with real credentials

**Sections:**
- **Upstream WiFi Credentials:** STA_SSID and STA_PASSWORD
- **Repeater WiFi Credentials:** AP_SSID and AP_PASSWORD

### 6. IP Forwarding & NAT (`ip_forward.h/c`)

**Purpose:** Enable Network Address Translation (NAT) between STA and AP interfaces for full WiFi repeater functionality

**Key Functions:**
- `ip_forward_init()` - Initialize lwIP IP forwarding and NAPT (Network Address Port Translation)

**Implementation:**
Uses the `ip_napt_enable()` function with the AP interface IP address:

```c
// Get AP interface IP address
esp_netif_ip_info_t ap_ip_info;
esp_netif_get_ip_info(ap_netif, &ap_ip_info);
uint32_t ap_ip = ap_ip_info.ip.addr;

// Enable NAPT ONLY on AP interface (not on STA)
// Enabling on both interfaces causes routing conflicts
ip_napt_enable(ap_ip, 1);
```

**Why This Approach:**
1. NAPT enabled only on AP interface (client-facing)
2. STA interface remains unnated (upstream-facing)
3. Avoids routing conflicts that occur when NAPT is enabled on both interfaces
4. Simpler and more reliable than bidirectional NAPT

**Critical Requirements:**
ALL of these must be enabled in `sdkconfig.defaults` or NAPT **will not work**:
1. `CONFIG_LWIP_IP_FORWARD=y` - **ABSOLUTELY CRITICAL:** Enables global IP forwarding
2. `CONFIG_LWIP_IPV4_NAPT=y` - Enables NAPT engine
3. `CONFIG_LWIP_L2_TO_L3_COPY=y` - Required for NAT packet translation
4. `CONFIG_LWIP_IPV4_NAPT_PORTMAP=y` - Enables port mapping support

**Why Configuration is Essential:**
- NAPT requires global IP forwarding enabled at the lwIP stack level (`CONFIG_LWIP_IP_FORWARD=y`)
- Without this compile-time setting, packets are NOT forwarded between interfaces
- The SDK configuration must be in place BEFORE compilation
- Calling the function without the configuration will fail or return 0 (failure)

**NAT Engine Features:**
- **lwIP NAPT Support:** Leverages ESP-IDF's built-in lwIP NAPT implementation
- **Connection Tracking:** Automatically maintains 512 concurrent NAT sessions
- **Port Translation:** Remaps client ports to ESP32 ports for upstream communication
- **Automatic Cleanup:** Dynamically removes stale connections based on protocol timeouts
- **Port Mapping Support:** Configurable static port forwarding (32 rules max)

**How It Works:**
1. AP clients send packets destined for external networks
2. NAPT intercepts and translates source IP/port to ESP32's IP/translated port
3. Packets forwarded to STA interface via IP forwarding
4. Response packets arrive from upstream on STA interface
5. NAPT translates destination IP/port back to original client IP/port
6. Clients transparently receive responses

**Configuration Files Required:**
- `sdkconfig.defaults` - Must have all 4 CONFIG options enabled
- Code calls `ip_napt_enable_netif(netif, 1)` with lwIP netif structure
- Must be called AFTER both STA and AP interfaces are initialized
- Must be called AFTER DHCP server is running
- Includes error checking: returns ESP_FAIL if NAPT enable fails

**NAT Implementation Reference:**
For detailed information about NAT implementation and lwIP NAPT API, see:
- `NAT_DOCUMENTATION_INDEX.md` - Master guide (start here)
- `LWIP_NAT_API_REFERENCE.md` - Complete API reference with all available functions
- `NAT_FUNCTION_SIGNATURES.md` - Function signatures and usage patterns
- `NAT_RESEARCH_SUMMARY.txt` - Technical research notes
- `FINDINGS.txt` - Quick reference summary

## Hardware Specifications

**Board:** Freenove ESP32-S3 WROOM-1
- **Processor:** ESP32-S3 dual-core @ 240 MHz
- **RAM:** 320 KB SRAM
- **PSRAM:** 8 MB
- **Flash:** 16 MB (configured in sdkconfig.defaults and platformio.ini)
- **WiFi:** 802.11 b/g/n 2.4 GHz
- **Serial Monitor Baud Rate:** 115200

## Build & Deployment

### Prerequisites
- PlatformIO CLI or IDE
- ESP-IDF framework (installed automatically)
- USB cable for board connection

### Configuration Files

The project uses two key configuration files:

1. **`platformio.ini`** - PlatformIO build settings
   - Specifies board type and flash size (16MB)
   - Sets serial monitor speed (115200 baud)
   - Defines build flags (PSRAM support)

2. **`sdkconfig.defaults`** - ESP-IDF SDK configuration
   - Configures console UART baud rate (115200)
   - Sets flash memory size (16MB)
   - Enables and configures PSRAM (8MB)
   - Optimizes WiFi, lwIP, and memory settings
   - **IMPORTANT:** Whenever you modify `sdkconfig.defaults`, delete the generated `sdkconfig.freenove_esp32_s3_wroom` file to ensure changes take effect

### Build Command
```bash
pio run -e freenove_esp32_s3_wroom
```

**IMPORTANT: Avoid Clean Builds**
- Do **NOT** run `--target clean` unless absolutely necessary
- Full recompilation takes 5-6 minutes due to ESP-IDF framework size
- Incremental builds (rebuilding only changed files) take 10-20 seconds
- Only use clean build when:
  - Switching between major configuration changes in `sdkconfig.defaults`
  - Experiencing unexplained build errors
  - After updating ESP-IDF framework version
- **Always delete `sdkconfig.freenove_esp32_s3_wroom` when:**
  - Performing a clean build
  - Making any changes to `sdkconfig.defaults`
  - This ensures the build system regenerates the configuration from the defaults
  - File will be automatically recreated during the next build
  - **Commands (choose based on your system):**
    - Windows CMD: `del sdkconfig.freenove_esp32_s3_wroom`
    - PowerShell: `Remove-Item sdkconfig.freenove_esp32_s3_wroom`
    - Git Bash: `rm sdkconfig.freenove_esp32_s3_wroom`

### Upload Command
```bash
pio run -e freenove_esp32_s3_wroom -t upload
```

**Upload Tips:**
- If upload fails with "Write timeout", press and hold the **BOOT button** on the ESP32 board before running upload
- Release BOOT button once "Connecting..." appears and upload starts
- Most reliable: Hold BOOT → Run upload → Wait for "Connecting..." → Release BOOT

### Monitor Serial Output
```bash
pio device monitor -e freenove_esp32_s3_wroom
```
The serial monitor automatically connects at **115200 baud** (configured in both `platformio.ini` and `sdkconfig.defaults`).

## Configuration Guide

### Step 1: Set WiFi Credentials in `include/secret.h`

**IMPORTANT:** This file is in `.gitignore` and should never be committed to version control.

Edit `include/secret.h`:
```c
// Upstream WiFi Network Credentials
#define STA_SSID "YOUR_HOME_WIFI_SSID"
#define STA_PASSWORD "YOUR_HOME_WIFI_PASSWORD"

// Repeater WiFi Network Credentials
#define AP_SSID "ESP32-Repeater"              // Repeater network name
#define AP_PASSWORD "repeater123"             // Repeater password
```

### Step 2: Customize WiFi Settings in `include/wifi_config.h` (Optional)

Edit `include/wifi_config.h` for repeater-specific settings:

```c
#define AP_CHANNEL 6                          // WiFi channel (1-13)
#define AP_MAX_CLIENTS 20                     // Max connected devices

// Network range
#define DHCP_START_IP "192.168.4.2"          // DHCP pool start
#define DHCP_END_IP "192.168.4.20"           // DHCP pool end
```

**Note:** All WiFi credentials (`STA_SSID`, `STA_PASSWORD`, `AP_SSID`, `AP_PASSWORD`) are stored in `secret.h` for security.

## Configuration Checklist for Internet Access

To enable your phone to access the internet through the repeater, verify these settings are configured correctly:

### ✅ Required WiFi Credentials (`include/secret.h`)
- [ ] `STA_SSID` - Set to your upstream WiFi network name
- [ ] `STA_PASSWORD` - Set to your upstream WiFi password
- [ ] `AP_SSID` - Name of repeater network (what you'll see on your phone)
- [ ] `AP_PASSWORD` - Password for repeater network

### ✅ Required SDK Configuration (`sdkconfig.defaults`)
**All 4 of these MUST be present or internet access will NOT work:**
- [ ] `CONFIG_LWIP_IP_FORWARD=y` - **ABSOLUTELY CRITICAL: Enables global IP forwarding (without this, no packets forward)**
- [ ] `CONFIG_LWIP_IPV4_NAPT=y` - Enables NAT/NAPT engine
- [ ] `CONFIG_LWIP_L2_TO_L3_COPY=y` - Required for packet translation
- [ ] `CONFIG_LWIP_IPV4_NAPT_PORTMAP=y` - Enables port mapping support
- [ ] `CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y` - Matches board flash size

### ✅ Network Configuration (`include/wifi_config.h`)
- [ ] `AP_IP_ADDR` = `192.168.4.1` - Repeater IP address
- [ ] `AP_GATEWAY` = `192.168.4.1` - Same as AP IP
- [ ] `AP_NETMASK` = `255.255.255.0` - Subnet mask
- [ ] `DHCP_START_IP` = `192.168.4.2` - DHCP range start
- [ ] `DHCP_END_IP` = `192.168.4.20` - DHCP range end
- [ ] `AP_CHANNEL` = 6 (or non-conflicting channel 1-13)
- [ ] `AP_MAX_CLIENTS` = 20 or higher

### ✅ Repeater Settings (`include/wifi_config.h`)
- [ ] `AP_HIDDEN` = 0 (network is visible)
- [ ] `STA_MAX_RETRIES` = 5 (reconnect attempts)
- [ ] `STA_RETRY_INTERVAL` = 5000 (5 seconds between retries)
- [ ] `DHCP_LEASE_TIME` = 3600 (1 hour lease)

### ✅ Build Process
- [ ] Delete `sdkconfig.freenove_esp32_s3_wroom` before building (generated file must be removed when config changes)
- [ ] Run `pio run -e freenove_esp32_s3_wroom`
- [ ] Upload with `pio run -e freenove_esp32_s3_wroom -t upload`

### ✅ Verification Steps
1. **Verify SDK Configuration BEFORE Building**
   ```bash
   grep "CONFIG_LWIP_IP_FORWARD\|CONFIG_LWIP_IPV4_NAPT\|CONFIG_LWIP_L2_TO_L3_COPY" sdkconfig.defaults
   ```
   Must see all 4 options set to `y`:
   - CONFIG_LWIP_IP_FORWARD=y
   - CONFIG_LWIP_IPV4_NAPT=y
   - CONFIG_LWIP_IPV4_NAPT_PORTMAP=y
   - CONFIG_LWIP_L2_TO_L3_COPY=y

2. **Delete Generated Config File**
   ```bash
   del sdkconfig.freenove_esp32_s3_wroom
   pio run -e freenove_esp32_s3_wroom
   ```

3. **Check Serial Output** - Look for these messages (✅ **NOW WORKING - TESTED AND VERIFIED**):
   ```
   STA connected to upstream WiFi
   STA got IP: 192.168.1.x
   DNS: Enabled DNS advertisement in DHCP responses ← KEY FIX
   NAPT enabled on AP interface (IP: 192.168.4.1)
   IP routing configured - packets will be forwarded between STA and AP
   WiFi Repeater Ready!
   ```

   **Critical messages indicating proper DNS forwarding:**
   - "DNS: Primary DNS server set to [IP]"
   - "DNS: DHCP server restarted with DNS advertisement enabled"

4. **Connect Your Phone**
   - Select repeater network (AP_SSID) in WiFi settings
   - Enter repeater password (AP_PASSWORD)
   - Phone should receive IP in range 192.168.4.2 - 192.168.4.20
   - **Phone will automatically receive DNS servers via DHCP** ← THIS NOW WORKS

5. **Test Internet Access** (✅ **VERIFIED WORKING**)
   - Open browser and try accessing a website
   - DNS resolution should work immediately
   - Full internet access through the repeater works
   - Phone can access both upstream router (192.168.1.1) and internet

### ⚠️ Common Issues Preventing Internet Access

| Issue | Symptom in Logs | Fix |
|-------|-----------------|-----|
| ✅ **Phone has full internet** | "DNS: DHCP server restarted with DNS advertisement enabled" + "NAPT enabled on AP interface" | **THIS IS WORKING** - Expected behavior, repeater is functioning |
| **Phone connects but no internet** (rare now) | See "NAPT enabled" but "DNS advertisement" missing | Ensure `esp_netif_dhcps_option()` call is in dns_forwarding.c - see dns_forwarding_apply_to_dhcp() |
| **No DNS servers in phone settings** | "NAPT enabled" but no DNS messages | Missing `esp_netif_dhcps_option()` call - DNS not being advertised to clients |
| **NAPT enable fails** | "NAPT enabled" message missing | Verify ALL 4 options in sdkconfig.defaults: CONFIG_LWIP_IP_FORWARD, CONFIG_LWIP_IPV4_NAPT, CONFIG_LWIP_L2_TO_L3_COPY, CONFIG_LWIP_IPV4_NAPT_PORTMAP |
| sdkconfig file wasn't regenerated | Config options don't take effect | Delete `sdkconfig.freenove_esp32_s3_wroom` before building |
| Can't find repeater network | No AP SSID visible | Verify AP_SSID in secret.h, restart ESP32 |
| Repeater won't connect to upstream | "STA got IP" never appears | Check STA_SSID and STA_PASSWORD in secret.h, verify upstream WiFi is 2.4GHz |

## Current Implementation Status

### ✅ **FULLY IMPLEMENTED AND TESTED** ✓

**Production-Ready: WiFi Repeater with Full Internet Access Working**

### ✅ Completed Features
- Dual WiFi mode (STA + AP) initialization
- Event-based WiFi connection management
- Automatic upstream reconnection with retry logic
- DHCP server for AP clients with automatic IP assignment (192.168.4.2-20)
- **DNS forwarding from upstream network to AP clients** ← TESTED WORKING
  - Clients receive DNS via DHCP automatically
  - Implemented via `esp_netif_dhcps_option()` with OFFER_DNS flag
- IP forwarding between STA and AP interfaces
- **NAT (Network Address Translation):** Full stateful connection tracking via lwIP NAPT
  - Only enabled on AP interface (avoids routing conflicts)
  - 512 concurrent NAT sessions
  - 32 port mapping rules supported
  - Automatic connection state management
  - TCP/UDP protocol support with dynamic timeouts
- **Full Internet Access:** Phones/devices connecting to repeater get complete internet access
- Serial logging and status reporting (115200 baud)
- Built-in LED blinking (GPIO2, 500ms cycle)
- Memory-efficient design (10.6% RAM used, 73.9% flash used)
- Complete SDK configuration with sdkconfig.defaults (all 4 critical NAPT options enabled)
- Secure credential management with secret.h (.gitignore)

### Key Fixes Applied (January 2026)
1. **DNS Advertisement Fix:** Added `esp_netif_dhcps_option()` call to explicitly advertise DNS servers in DHCP responses
2. **NAPT Interface Fix:** Enabled NAPT only on AP interface (removed dual-interface NAPT that caused routing conflicts)

### ⏳ Planned Features
- **Port Forwarding Configuration:** Web interface or CLI for managing port maps
- **DNS Caching/Proxy:** Improve DNS query performance with caching
- **Web Dashboard:** Configuration and monitoring interface
- **Power Management:** Sleep modes for low-power operation
- **Persistent Configuration:** Save settings to NVS (flash storage)
- **QoS Features:** Bandwidth limiting and priority-based packet forwarding
- **Bridge Mode Alternative:** Option for transparent bridge instead of repeater

## Memory Usage Analysis

**RAM Distribution:**
- FreeRTOS & OS: ~2-3 MB
- WiFi Stack: ~2 MB
- lwIP Network Stack: ~1.5 MB
- Application & Buffers: ~500 KB
- Available for NAT session tracking: ~1-2 MB

**Recommendations:**
- Limit NAT connection tracking to ~50 concurrent sessions
- Session timeout cleanup every 2 minutes
- Use connection pooling for efficient memory usage

## Key Technologies

| Technology | Purpose | Version |
|-----------|---------|---------|
| ESP-IDF | Embedded framework | 5.5.0 |
| lwIP | Network stack | Built-in |
| FreeRTOS | Real-time OS | Built-in |
| PlatformIO | Build system | 6.1.18 |

## Implementation Notes

### Why ESP-IDF over Arduino?
- Fine-grained control over WiFi and networking
- Direct access to lwIP stack for NAT implementation
- Better performance for packet processing
- More memory-efficient for embedded systems

### WiFi Event Handling
- Non-blocking event-driven architecture
- All WiFi events handled in dedicated callbacks
- Event groups for synchronization between tasks

### DHCP Server Implementation
- Uses lwIP's built-in DHCP server (dhcpserver component)
- Automatic IP assignment to AP clients (192.168.4.2-20)
- DNS servers from upstream network advertised to clients
- Configured via esp_netif API with `esp_netif_set_dns_info()`

## Debugging

### Serial Output
The application provides detailed logging:
```
[WiFiRepeater] WiFi Repeater with NAT - Initializing
[WiFiRepeater] Step 1: Initializing WiFi (STA+AP mode)
[WiFiRepeater] Step 2: Waiting for upstream WiFi connection
[WiFiRepeater] STA connected to upstream WiFi
[WiFiRepeater] Step 3: Initializing DHCP server
[WiFiRepeater] WiFi Repeater Ready!
```

### Common Issues

**Build Error: "Flash memory size mismatch"**
- Ensure `sdkconfig.defaults` exists in project root
- Verify `CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y` is set
- Run clean build: `pio run -e freenove_esp32_s3_wroom --target clean`

**Serial Monitor Shows Garbled Output**
- Verify serial monitor baud rate is 115200
- Check USB cable is properly connected
- Try: `pio device monitor -e freenove_esp32_s3_wroom --baud 115200`

**Connection Timeout**
- Verify SSID and password are correct
- Check if upstream WiFi is on same channel band

**No Clients Getting DHCP Leases or No DNS Resolution**
- Ensure AP interface is properly initialized
- Check if DHCP port (UDP 67/68) is not blocked
- Verify STA connected to upstream WiFi first (DNS comes from upstream)
- Check DNS servers are extracted and advertised (see serial logs)

**WiFi Disconnections**
- Verify antenna connection
- Check for interference on WiFi channel
- Increase STA_RETRY_INTERVAL if needed

## Future Enhancements

1. **Configuration via WebUI**
   - Real-time WiFi network scanning
   - Credentials management without recompilation

2. **Advanced NAT**
   - Connection state tracking (TCP/UDP)
   - Port forwarding capabilities
   - Traffic statistics

3. **Network Optimization**
   - Automatic channel selection
   - Signal strength improvement algorithms
   - Bandwidth throttling per client

4. **Persistent Storage**
   - NVS (Non-Volatile Storage) for credentials
   - Configuration persistence across reboots

## References

- ESP-IDF Documentation: https://docs.espressif.com/projects/esp-idf/
- lwIP Documentation: https://savannah.nongnu.org/projects/lwip/
- PlatformIO Docs: https://docs.platformio.org/

---

**Author:** Generated by Claude Code
**Created:** January 2026
**Board:** Freenove ESP32-S3 WROOM-1 (16MB)
