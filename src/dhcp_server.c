#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "dhcpserver/dhcpserver.h"
#include "lwip/ip4_addr.h"

#include "wifi_config.h"
#include "dns_forwarding.h"

static const char *TAG = LOG_TAG;

/**
 * Configure DHCP server for AP clients (but don't start it yet)
 * Assigns IPs from DHCP_START_IP to DHCP_END_IP
 * DHCP will be started after DNS servers are configured
 */
esp_err_t dhcp_server_init(void)
{
    esp_netif_t *ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");

    if (!ap_netif) {
        ESP_LOGE(TAG, "DHCP: Failed to get AP netif");
        return ESP_FAIL;
    }

    /* Stop DHCP server if it's already running */
    esp_netif_dhcps_stop(ap_netif);

    /* Note: DHCP lease range is configured when AP interface is set up
       in wifi_manager.c (IP range 192.168.4.2 - 192.168.4.20)
       DHCP server will be started in dns_forwarding_apply_to_dhcp()
       after DNS servers are configured */

    ESP_LOGI(TAG, "DHCP server configured");
    ESP_LOGI(TAG, "DHCP IP range: %s - %s", DHCP_START_IP, DHCP_END_IP);
    ESP_LOGI(TAG, "DHCP will start after DNS configuration");

    return ESP_OK;
}

/**
 * Get current DHCP server status
 */
void dhcp_server_print_status(void)
{
    esp_netif_t *ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");

    if (!ap_netif) {
        ESP_LOGE(TAG, "DHCP: Failed to get AP netif");
        return;
    }

    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(ap_netif, &ip_info);

    ESP_LOGI(TAG, "========== DHCP Status ==========");
    ESP_LOGI(TAG, "AP IP Address: " IPSTR, IP2STR(&ip_info.ip));
    ESP_LOGI(TAG, "AP Gateway: " IPSTR, IP2STR(&ip_info.gw));
    ESP_LOGI(TAG, "AP Netmask: " IPSTR, IP2STR(&ip_info.netmask));
    ESP_LOGI(TAG, "================================");
}

/**
 * Apply DNS servers to DHCP configuration
 */
esp_err_t dhcp_server_set_dns(void)
{
    return dns_forwarding_apply_to_dhcp();
}
