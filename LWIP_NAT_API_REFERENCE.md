# ESP-IDF lwIP NAT/NAPT API Reference

## Overview

The ESP-IDF lwIP stack provides comprehensive Network Address Translation (NAT) support through the NAPT (Network Address and Port Translation) engine. This document details all available functions and their signatures.

## Key Configuration Options

Before using NAPT functions, enable these in sdkconfig:

```
CONFIG_LWIP_IPV4_NAPT=y              # Enable NAPT engine
CONFIG_LWIP_IPV4_NAPT_PORTMAP=y      # Enable port mapping support
CONFIG_LWIP_L2_TO_L3_COPY=y          # Required for NAT packet translation
CONFIG_LWIP_IP_FORWARD=y             # Enable IP forwarding (required for NAPT)
```

IMPORTANT: IP forwarding (IP_FORWARD) must be enabled for NAPT to function.

## Header Files

Include this header to use NAPT functions:

```c
#include "lwip/lwip_napt.h"
```

## Public API Functions

### 1. Enable/Disable NAPT by IP Address

```c
void ip_napt_enable(u32_t addr, int enable);
```

Parameters:
- addr: IP address of the interface (network byte order)
- enable: Non-zero to enable NAPT, 0 to disable

### 2. Enable/Disable NAPT by Interface Number

```c
void ip_napt_enable_no(u8_t number, int enable);
```

Parameters:
- number: Interface number (0, 1, 2, etc.)
- enable: Non-zero to enable NAPT, 0 to disable

### 3. Enable/Disable NAPT by Interface Handle (RECOMMENDED)

```c
int ip_napt_enable_netif(struct netif *netif, int enable);
```

Parameters:
- netif: Pointer to network interface structure
- enable: Non-zero to enable NAPT, 0 to disable

Returns:
- 1: Success
- 0: Failure

This is the most efficient method when you have the netif structure.

## Port Mapping Functions

### 4. Add Port Mapping

```c
u8_t ip_portmap_add(u8_t proto, u32_t maddr, u16_t mport, u32_t daddr, u16_t dport);
```

Parameters:
- proto: IPPROTO_TCP or IPPROTO_UDP
- maddr: Mapped (external) IP address
- mport: Mapped (external) port in host byte order
- daddr: Destination (internal) IP address
- dport: Destination (internal) port in host byte order

Returns:
- 1: Success
- 0: Failure

Example: Forward port 8080 to internal server at 192.168.1.100:80

### 5. Get Port Mapping

```c
u8_t ip_portmap_get(u8_t proto, u16_t mport, u32_t *maddr, u32_t *daddr, u16_t *dport);
```

Retrieves existing port mapping configuration.

Returns:
- 1: Mapping found
- 0: Mapping not found

### 6. Remove Port Mapping

```c
u8_t ip_portmap_remove(u8_t proto, u16_t mport);
```

Removes a port mapping.

Returns:
- 1: Success
- 0: Failure (mapping not found)

## Statistics Functions

### 7. Get NAPT Statistics

```c
void ip_napt_get_stats(struct stats_ip_napt *stats);
```

Requires LWIP_STATS and IP_NAPT_STATS enabled.

## Configuration Constants

```c
#define IP_NAPT_MAX              512    /* Max concurrent sessions */
#define IP_PORTMAP_MAX           32     /* Max port mapping rules */
#define IP_NAPT_TIMEOUT_MS_TCP   (30*60*1000)   /* 30 minutes */
#define IP_NAPT_TIMEOUT_MS_UDP   2000           /* 2 seconds */
#define IP_NAPT_TIMEOUT_MS_ICMP  2000           /* 2 seconds */
#define IP_NAPT_PORT_RANGE_START 49152          /* Dynamic port start */
#define IP_NAPT_PORT_RANGE_END   61439          /* Dynamic port end */
#define NAPT_TMR_INTERVAL        2000           /* Cleanup: 2 seconds */
```

## Typical Usage Pattern

```c
#include "lwip/lwip_napt.h"
#include "lwip/netif.h"
#include "esp_netif.h"
#include "esp_netif_net_stack.h"

void setup_nat_repeater(esp_netif_t *ap_netif)
{
    // Get lwIP netif structure from esp_netif handle
    // esp_netif_get_netif_impl() returns void*, must cast to struct netif*
    struct netif *ap_netif_lwip = (struct netif *)esp_netif_get_netif_impl(ap_netif);

    if (!ap_netif_lwip) {
        printf("Failed to get lwIP netif\n");
        return;
    }

    // Enable NAPT on AP interface (returns 1=success, 0=failure)
    int result = ip_napt_enable_netif(ap_netif_lwip, 1);

    if (result == 1) {
        printf("NAPT enabled successfully\n");
    } else {
        printf("Failed to enable NAPT - check CONFIG_LWIP_IP_FORWARD=y in sdkconfig\n");
    }
}
```

**IMPORTANT:** This requires including `esp_netif_net_stack.h` to access `esp_netif_get_netif_impl()`

## How NAPT Works

**Automatic Initialization:**
NAPT tables are allocated when ip_napt_enable() is first called with enable=1:
- 512 NAPT entries allocated from heap (~20 KB)
- 32 port mapping entries (if enabled) (~0.4 KB)
- Periodic cleanup timer started (every 2 seconds)

**Packet Flow (Outbound):**
1. Client packet arrives on AP interface
2. NAPT intercepts and creates session entry
3. Source IP/port remapped to ESP32's IP and dynamic port
4. Packet forwarded to STA interface
5. Response returns and entry matches it
6. Destination IP/port translated back to original client
7. Packet forwarded back to client

## Important Notes

1. IP forwarding (IP_FORWARD=1) must be enabled for NAPT to work
2. Max 512 concurrent sessions (configurable)
3. Max 32 port forwarding rules
4. Stale sessions auto-cleaned every 2 seconds
5. Thread-safe for use from different tasks
6. No built-in UPnP/NAT-PMP support

## Memory Usage

- Each NAPT session: ~40 bytes
- Each port mapping: ~13 bytes
- Default 512 sessions: ~20 KB
- Default 32 mappings: ~0.4 KB

## References

- Header: lwip/lwip_napt.h
- Implementation: components/lwip/lwip/src/core/ipv4/ip4_napt.c
- Config: components/lwip/Kconfig
