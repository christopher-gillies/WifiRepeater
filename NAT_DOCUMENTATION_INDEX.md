# NAT/NAPT Implementation Documentation Index

## Quick Answer

The correct function to enable IP forwarding in ESP-IDF's lwIP is:

```c
int ip_napt_enable_netif(struct netif *netif, int enable);
```

**Location:** `lwip/lwip_napt.h`  
**Returns:** 1 (success) or 0 (failure)  
**Recommended:** YES - This is the most efficient method

---

## Documentation Files

This directory contains comprehensive documentation about implementing NAT in the WiFi Repeater project:

### 1. **FINDINGS.txt** (Start here)
- Quick summary of findings
- All 7 available NAT functions
- Usage examples
- Critical requirements
- ~120 lines, easy to scan

### 2. **NAT_RESEARCH_SUMMARY.txt** 
- Detailed research findings
- All 7 function signatures
- Configuration requirements
- Implementation pattern
- ~105 lines

### 3. **LWIP_NAT_API_REFERENCE.md**
- Complete API reference for all 7 functions
- Detailed parameter descriptions
- How NAPT works internally
- Memory usage analysis
- Configuration constants
- Typical usage patterns
- ~195 lines, most comprehensive

### 4. **NAT_FUNCTION_SIGNATURES.md**
- Quick function reference format
- All signatures with types
- Return value summary table
- Default limits and configuration
- Recommended initialization pattern
- ~160 lines

---

## Implementation Quick Start

### Step 1: Include Header
```c
#include "lwip/lwip_napt.h"
```

### Step 2: Enable NAT
```c
// Get lwIP netif from esp_netif handle
struct netif *ap_netif = esp_netif_get_netif(ap_netif_handle);

// Enable NAT on AP interface
int result = ip_napt_enable_netif(ap_netif, 1);

if (result) {
    printf("NAT enabled successfully\n");
} else {
    printf("Failed to enable NAT\n");
}
```

### Step 3: Configure sdkconfig
```
CONFIG_LWIP_IP_FORWARD=y              # CRITICAL!
CONFIG_LWIP_IPV4_NAPT=y
CONFIG_LWIP_L2_TO_L3_COPY=y
```

---

## All Available Functions

| Function | Purpose | Returns |
|----------|---------|---------|
| `ip_napt_enable(addr, enable)` | Enable by IP address | void |
| `ip_napt_enable_no(num, enable)` | Enable by interface # | void |
| **`ip_napt_enable_netif(netif, enable)`** | **Enable by netif ptr** | **1/0** |
| `ip_portmap_add(...)` | Add port mapping | 1/0 |
| `ip_portmap_get(...)` | Query port mapping | 1/0 |
| `ip_portmap_remove(...)` | Remove port mapping | 1/0 |
| `ip_napt_get_stats(...)` | Get statistics | void |

---

## Key Specifications

**Limits:**
- 512 concurrent sessions (configurable)
- 32 port mappings (configurable)
- 12k dynamic ports (49152-61439)

**Memory:** ~20.4 KB (default config)

**Session Timeouts:**
- TCP: 30 minutes
- UDP: 2 seconds
- ICMP: 2 seconds

**Auto-Cleanup:** Every 2 seconds

**Protocols Supported:**
- TCP
- UDP
- ICMP

---

## Critical Requirements

MUST enable in sdkconfig:
- `CONFIG_LWIP_IP_FORWARD=y` - **If this is not enabled, NAPT WILL NOT WORK**
- `CONFIG_LWIP_IPV4_NAPT=y`
- `CONFIG_LWIP_L2_TO_L3_COPY=y`

---

## Usage Scenarios

### Basic WiFi Repeater (NAT Only)
```c
ip_napt_enable_netif(ap_netif, 1);
// Done! Clients can now access external networks
```

### With Port Forwarding
```c
ip_napt_enable_netif(ap_netif, 1);

// Forward external port 8080 to internal web server
ip_portmap_add(IPPROTO_TCP, 
               ipaddr_addr("192.168.4.1"), 8080,
               ipaddr_addr("192.168.1.50"), 80);
```

### Disable NAT
```c
ip_napt_enable_netif(ap_netif, 0);
```

---

## File Locations in ESP-IDF

```
~/.platformio/.cache/tmp/pkg-installing-*/components/lwip/

├── lwip/src/include/lwip/
│   ├── lwip_napt.h          (PUBLIC API)
│   ├── ip4_napt.h           (PRIVATE)
│   └── opt.h                (CONFIG)
└── lwip/src/core/ipv4/
    └── ip4_napt.c           (IMPLEMENTATION)
```

---

## Common Questions

**Q: What's the difference between the 3 enable methods?**  
A: All do the same thing. Use `ip_napt_enable_netif()` when you have a netif pointer (most efficient).

**Q: Do I need to manually handle packets?**  
A: No, NAPT is completely transparent after enabling.

**Q: What happens if IP_FORWARD is not enabled?**  
A: NAPT will not work at all, even if all other options are enabled.

**Q: How much memory does NAPT use?**  
A: ~20 KB default (512 sessions + 32 port maps). Minimal compared to ESP32's 300+ KB available.

**Q: Is it thread-safe?**  
A: Yes, can be called from different tasks.

---

## Recommended Workflow

1. Read **FINDINGS.txt** for quick overview
2. Check **NAT_FUNCTION_SIGNATURES.md** for function details
3. Consult **LWIP_NAT_API_REFERENCE.md** for in-depth information
4. Use **NAT_RESEARCH_SUMMARY.txt** as a reference during implementation

---

## For WifiRepeater Project

Implement NAT in:
- `include/ip_forward.h` - Interface
- `src/ip_forward.c` - Implementation using `ip_napt_enable_netif()`
- `src/main.c` - Call during setup after WiFi initialization

Simple wrapper function:
```c
int ip_forward_init(esp_netif_t *ap_netif) {
    struct netif *netif = esp_netif_get_netif(ap_netif);
    return netif ? ip_napt_enable_netif(netif, 1) : 0;
}
```

---

## Documentation Maintenance

All reference files automatically generated from PlatformIO's lwIP headers.

Research completed: 2026-01-18  
Platform: Windows (git bash)  
PlatformIO Version: Latest  
ESP-IDF lwIP: 5.5.0
