#ifndef DHCP_SERVER_H
#define DHCP_SERVER_H

#include "esp_err.h"

/**
 * Initialize and configure DHCP server for AP mode
 * Distributes IPs to clients connecting to the repeater
 *
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t dhcp_server_init(void);

/**
 * Print current DHCP server status
 */
void dhcp_server_print_status(void);

/**
 * Apply DNS servers to DHCP configuration
 * Should be called after DNS servers are configured via dns_forwarding module
 *
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t dhcp_server_set_dns(void);

#endif // DHCP_SERVER_H
