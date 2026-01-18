#include "esp_log.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "lwip/lwip_napt.h"

#include "wifi_config.h"

static const char *TAG = LOG_TAG;

/**
 * Enable IP forwarding and NAPT (Network Address Port Translation) between STA and AP
 * Allows packets from AP clients to reach the upstream network via STA
 *
 * NAPT Configuration:
 * - CONFIG_LWIP_IP_FORWARD=y        - Enable IP forwarding
 * - CONFIG_LWIP_IPV4_NAPT=y         - Enable NAPT
 * - CONFIG_LWIP_L2_TO_L3_COPY=y     - Enable L2 to L3 copy for NAT
 */
esp_err_t ip_forward_init(void)
{
    ESP_LOGI(TAG, "Configuring IP routing for WiFi Repeater");

    /* Get both interfaces */
    esp_netif_t *sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_t *ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");

    if (!sta_netif) {
        ESP_LOGE(TAG, "STA interface not found");
        return ESP_FAIL;
    }

    if (!ap_netif) {
        ESP_LOGE(TAG, "AP interface not found");
        return ESP_FAIL;
    }

    /* Enable NAPT on AP interface (WiFi SoftAP)
     * This allows clients connected to AP to access upstream network through NAT
     * Requires: CONFIG_LWIP_IP_FORWARD=y, CONFIG_LWIP_IPV4_NAPT=y in sdkconfig
     */
#ifdef CONFIG_LWIP_IPV4_NAPT
    /* Get AP interface IP address and enable NAPT by IP address
     * This is the most direct method and works reliably
     */
    esp_netif_ip_info_t ap_ip_info;
    esp_netif_get_ip_info(ap_netif, &ap_ip_info);
    uint32_t ap_ip = ap_ip_info.ip.addr;

    ESP_LOGI(TAG, "Enabling NAPT on AP interface (IP: " IPSTR ")", IP2STR(&ap_ip_info.ip));

    /* Enable NAPT using AP IP address (void return, no error checking)
     * Requires CONFIG_LWIP_IP_FORWARD=y to be enabled in sdkconfig
     * Only enable on AP interface - enabling on both can cause conflicts
     */
    ip_napt_enable(ap_ip, 1);
    ESP_LOGI(TAG, "NAPT enabled on AP interface (192.168.4.1)");

#else
    ESP_LOGW(TAG, "CONFIG_LWIP_IPV4_NAPT not enabled - NAT functionality disabled");
#endif

    ESP_LOGI(TAG, "IP routing configured - packets will be forwarded between STA and AP");

    return ESP_OK;
}
