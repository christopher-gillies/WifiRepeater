#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "dhcpserver/dhcpserver.h"

#include "wifi_config.h"
#include "dns_forwarding.h"

static const char *TAG = LOG_TAG;

/* Global DNS server storage */
static esp_ip4_addr_t s_dns1 = {0};
static esp_ip4_addr_t s_dns2 = {0};
static bool s_dns_configured = false;

/**
 * Set DNS servers for forwarding to AP clients
 */
esp_err_t dns_forwarding_set_servers(esp_ip4_addr_t dns1, esp_ip4_addr_t dns2)
{
    s_dns1 = dns1;
    s_dns2 = dns2;
    s_dns_configured = true;

    ESP_LOGI(TAG, "DNS Servers configured:");
    ESP_LOGI(TAG, "  Primary DNS:   " IPSTR, IP2STR(&s_dns1));
    if (s_dns2.addr != 0) {
        ESP_LOGI(TAG, "  Secondary DNS: " IPSTR, IP2STR(&s_dns2));
    } else {
        ESP_LOGI(TAG, "  Secondary DNS: (not configured)");
    }

    return ESP_OK;
}

/**
 * Get currently configured DNS servers
 */
esp_err_t dns_forwarding_get_servers(esp_ip4_addr_t *dns1, esp_ip4_addr_t *dns2)
{
    if (!s_dns_configured) {
        return ESP_FAIL;
    }

    if (dns1) {
        *dns1 = s_dns1;
    }
    if (dns2) {
        *dns2 = s_dns2;
    }

    return ESP_OK;
}

/**
 * Apply DNS servers to DHCP configuration
 * Configures the AP interface DNS servers which are advertised to DHCP clients
 */
esp_err_t dns_forwarding_apply_to_dhcp(void)
{
    if (!s_dns_configured) {
        ESP_LOGW(TAG, "DNS: No DNS servers configured, using default gateway");
        return ESP_FAIL;
    }

    esp_netif_t *ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    if (!ap_netif) {
        ESP_LOGE(TAG, "DNS: Failed to get AP netif");
        return ESP_FAIL;
    }

    /* Stop DHCP server to allow configuration changes */
    esp_netif_dhcps_stop(ap_netif);
    ESP_LOGI(TAG, "DNS: Stopped DHCP server for DNS configuration");

    /* Configure primary DNS server on AP interface
     * This sets the DNS for the interface itself */
    esp_netif_dns_info_t dns_info;
    dns_info.ip.u_addr.ip4 = s_dns1;
    dns_info.ip.type = ESP_IPADDR_TYPE_V4;

    esp_err_t err = esp_netif_set_dns_info(ap_netif, ESP_NETIF_DNS_MAIN, &dns_info);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "DNS: Could not set primary DNS server (0x%X)", err);
    } else {
        ESP_LOGI(TAG, "DNS: Primary DNS server set to " IPSTR, IP2STR(&s_dns1));
    }

    /* Configure secondary DNS server if available */
    if (s_dns2.addr != 0) {
        dns_info.ip.u_addr.ip4 = s_dns2;
        dns_info.ip.type = ESP_IPADDR_TYPE_V4;

        err = esp_netif_set_dns_info(ap_netif, ESP_NETIF_DNS_BACKUP, &dns_info);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "DNS: Could not set secondary DNS server (0x%X)", err);
        } else {
            ESP_LOGI(TAG, "DNS: Secondary DNS server set to " IPSTR, IP2STR(&s_dns2));
        }
    }

    /* Enable DNS advertisement in DHCP responses */
    dhcps_offer_t opt_val = OFFER_DNS;
    esp_netif_dhcps_option(ap_netif, ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER, &opt_val, sizeof(opt_val));

    /* Restart DHCP server - it will advertise the configured DNS servers to clients */
    err = esp_netif_dhcps_start(ap_netif);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "DNS: Failed to restart DHCP server (0x%X)", err);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "DNS: DHCP server restarted with DNS advertisement enabled");
    return ESP_OK;
}
