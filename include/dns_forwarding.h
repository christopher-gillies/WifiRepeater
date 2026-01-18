#ifndef DNS_FORWARDING_H
#define DNS_FORWARDING_H

#include "esp_err.h"
#include "esp_netif.h"

/**
 * Set DNS servers for DHCP to advertise to AP clients
 * Should be called after STA gets IP from upstream network
 *
 * @param dns1 Primary DNS server IP address
 * @param dns2 Secondary DNS server IP address (optional, can be 0)
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t dns_forwarding_set_servers(esp_ip4_addr_t dns1, esp_ip4_addr_t dns2);

/**
 * Get the currently configured DNS servers
 *
 * @param dns1 Pointer to store primary DNS server
 * @param dns2 Pointer to store secondary DNS server
 * @return ESP_OK on success, ESP_FAIL if not configured
 */
esp_err_t dns_forwarding_get_servers(esp_ip4_addr_t *dns1, esp_ip4_addr_t *dns2);

/**
 * Apply DNS servers to DHCP configuration
 * Configures DHCP to advertise DNS servers to connected clients
 *
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t dns_forwarding_apply_to_dhcp(void);

#endif // DNS_FORWARDING_H
