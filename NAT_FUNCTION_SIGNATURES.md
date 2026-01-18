# NAT/NAPT Function Signatures - Quick Reference

## All Available ESP-IDF lwIP NAT Functions

### Header File
```c
#include "lwip/lwip_napt.h"
```

### Function List

#### 1. NAPT Enable/Disable (3 variants)

```c
// Enable/disable by IP address
void ip_napt_enable(u32_t addr, int enable);

// Enable/disable by interface number
void ip_napt_enable_no(u8_t number, int enable);

// Enable/disable by netif pointer (RECOMMENDED)
int ip_napt_enable_netif(struct netif *netif, int enable);
// Returns: 1 = success, 0 = failure
```

#### 2. Port Mapping Functions

```c
// Add a port mapping rule
u8_t ip_portmap_add(u8_t proto, u32_t maddr, u16_t mport, u32_t daddr, u16_t dport);
// proto: IPPROTO_TCP or IPPROTO_UDP
// maddr: mapped (external) IP address
// mport: mapped (external) port (host byte order)
// daddr: destination (internal) IP address
// dport: destination (internal) port (host byte order)
// Returns: 1 = success, 0 = failure

// Get port mapping details
u8_t ip_portmap_get(u8_t proto, u16_t mport, u32_t *maddr, u32_t *daddr, u16_t *dport);
// proto: IPPROTO_TCP or IPPROTO_UDP
// mport: mapped (external) port to look up
// maddr: [output] pointer to store mapped IP
// daddr: [output] pointer to store destination IP
// dport: [output] pointer to store destination port
// Returns: 1 = found, 0 = not found

// Remove a port mapping rule
u8_t ip_portmap_remove(u8_t proto, u16_t mport);
// proto: IPPROTO_TCP or IPPROTO_UDP
// mport: mapped (external) port to remove
// Returns: 1 = success, 0 = not found
```

#### 3. Statistics (if enabled)

```c
// Get NAPT statistics (requires LWIP_STATS=1 and IP_NAPT_STATS=1)
void ip_napt_get_stats(struct stats_ip_napt *stats);
// stats: pointer to stats_ip_napt structure
// Note: Use within #if LWIP_STATS guard
```

### Type Definitions

```c
typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;

// Protocol constants
#define IPPROTO_TCP  6
#define IPPROTO_UDP  17
#define IPPROTO_ICMP 1
```

### Internal Functions (Used by lwIP, not directly by applications)

```c
// Private function: called during IP packet forwarding
err_t ip_napt_forward(struct pbuf *p, struct ip_hdr *iphdr, struct netif *inp, struct netif *outp);

// Private function: called for incoming packets
void ip_napt_recv(struct pbuf *p, struct ip_hdr *iphdr);
```

## Configuration Compilation Requirements

For NAPT to work, these must be enabled in sdkconfig or menuconfig:

```
CONFIG_LWIP_IPV4_NAPT=y
CONFIG_LWIP_IPV4_NAPT_PORTMAP=y  
CONFIG_LWIP_IP_FORWARD=y
CONFIG_LWIP_L2_TO_L3_COPY=y
```

Equivalent lwIP options:
```
IP_NAPT = 1
IP_NAPT_PORTMAP = 1
IP_FORWARD = 1
L2_TO_L3_COPY = 1
```

## Recommended Initialization Pattern

```c
#include "lwip/lwip_napt.h"
#include "esp_netif.h"

int enable_nat_repeater(esp_netif_t *ap_netif_handle)
{
    // Get the underlying lwIP netif structure
    struct netif *ap_netif = esp_netif_get_netif(ap_netif_handle);
    
    if (!ap_netif) {
        ESP_LOGE(TAG, "Failed to get netif from esp_netif handle");
        return 0;
    }
    
    // Enable NAPT on the AP interface
    int result = ip_napt_enable_netif(ap_netif, 1);
    
    if (result) {
        ESP_LOGI(TAG, "NAPT enabled successfully on AP interface");
        return 1;
    } else {
        ESP_LOGE(TAG, "Failed to enable NAPT");
        return 0;
    }
}

int disable_nat_repeater(esp_netif_t *ap_netif_handle)
{
    struct netif *ap_netif = esp_netif_get_netif(ap_netif_handle);
    
    if (!ap_netif) {
        ESP_LOGE(TAG, "Failed to get netif from esp_netif handle");
        return 0;
    }
    
    int result = ip_napt_enable_netif(ap_netif, 0);
    
    if (result) {
        ESP_LOGI(TAG, "NAPT disabled");
        return 1;
    } else {
        ESP_LOGE(TAG, "Failed to disable NAPT");
        return 0;
    }
}
```

## Default Limits and Configuration

```c
#define IP_NAPT_MAX              512    // Max concurrent translation sessions
#define IP_PORTMAP_MAX           32     // Max static port mapping rules

#define IP_NAPT_TIMEOUT_MS_TCP   1800000  // 30 minutes (TCP persistent)
#define IP_NAPT_TIMEOUT_MS_TCP_DISCON 60000  // ~1 minute (TCP disconnected)
#define IP_NAPT_TIMEOUT_MS_UDP   2000   // 2 seconds
#define IP_NAPT_TIMEOUT_MS_ICMP  2000   // 2 seconds

#define IP_NAPT_PORT_RANGE_START 49152  // Dynamic port range start
#define IP_NAPT_PORT_RANGE_END   61439  // Dynamic port range end (~12k ports)

#define NAPT_TMR_INTERVAL        2000   // Cleanup timer: runs every 2 seconds
```

## Memory Estimation

With default configuration:
- 512 NAPT entries × ~40 bytes = ~20 KB
- 32 port mappings × ~13 bytes = ~0.4 KB
- Total: ~20.4 KB heap memory

## Files in ESP-IDF Source

```
components/lwip/lwip/src/include/lwip/lwip_napt.h      (Public API)
components/lwip/lwip/src/include/lwip/ip4_napt.h       (Private interface)
components/lwip/lwip/src/core/ipv4/ip4_napt.c          (Implementation)
components/lwip/lwip/src/include/lwip/opt.h            (Configuration options)
```

## Return Value Summary

| Function | Return | Meaning |
|----------|--------|---------|
| ip_napt_enable | void | Always succeeds (or initializes internally) |
| ip_napt_enable_no | void | Always succeeds (or initializes internally) |
| ip_napt_enable_netif | 1 | Success |
| ip_napt_enable_netif | 0 | Failure |
| ip_portmap_add | 1 | Success |
| ip_portmap_add | 0 | Failure |
| ip_portmap_get | 1 | Mapping found |
| ip_portmap_get | 0 | Mapping not found |
| ip_portmap_remove | 1 | Success |
| ip_portmap_remove | 0 | Mapping not found |

