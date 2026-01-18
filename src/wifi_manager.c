#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"

#include "wifi_config.h"
#include "dns_forwarding.h"

#define DEFAULT_SCAN_LIST_SIZE 20

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static EventGroupHandle_t s_wifi_event_group = NULL;
static int s_retry_num = 0;

static const char *TAG = LOG_TAG;

/* WiFi Event Handler */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "STA started, attempting to connect to upstream WiFi");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        ESP_LOGI(TAG, "Connected to upstream WiFi");
        s_retry_num = 0;
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "STA disconnected from upstream WiFi");
        if (s_retry_num < STA_MAX_RETRIES) {
            s_retry_num++;
            ESP_LOGI(TAG, "Attempting reconnect (%d/%d)", s_retry_num, STA_MAX_RETRIES);
            vTaskDelay(pdMS_TO_TICKS(STA_RETRY_INTERVAL));
            esp_wifi_connect();
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "STA got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;

        /* Extract DNS servers from upstream network */
        esp_netif_t *sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        if (sta_netif) {
            esp_netif_dns_info_t dns_info_main;
            esp_netif_dns_info_t dns_info_backup;

            esp_netif_get_dns_info(sta_netif, ESP_NETIF_DNS_MAIN, &dns_info_main);
            esp_netif_get_dns_info(sta_netif, ESP_NETIF_DNS_BACKUP, &dns_info_backup);

            /* Configure DNS forwarding with upstream DNS servers */
            dns_forwarding_set_servers(dns_info_main.ip.u_addr.ip4,
                                     dns_info_backup.ip.u_addr.ip4);
        }

        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_START) {
        ESP_LOGI(TAG, "AP started");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "Station connected to AP (AID=%d)", event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "Station disconnected from AP (AID=%d)", event->aid);
    }
}

/* Initialize WiFi in STA+AP mode */
esp_err_t wifi_init_sta_ap(void)
{
    /* Initialize Event Group */
    if (s_wifi_event_group == NULL) {
        s_wifi_event_group = xEventGroupCreate();
    }

    /* Initialize NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* Initialize Network Interface */
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Create network interfaces for both STA and AP */
    esp_netif_create_default_wifi_sta();
    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();

    /* WiFi configuration */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Register event handlers */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    /* Configure STA mode */
    wifi_config_t sta_config = {
        .sta = {
            .ssid = STA_SSID,
            .password = STA_PASSWORD,
            .scan_method = WIFI_FAST_SCAN,
            .bssid_set = 0,
            .channel = 0,
            .listen_interval = 0,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold.rssi = -127,
            .threshold.authmode = WIFI_AUTH_OPEN,
        },
    };
    strcpy((char *)sta_config.sta.ssid, STA_SSID);
    strcpy((char *)sta_config.sta.password, STA_PASSWORD);

    /* Configure AP mode */
    wifi_config_t ap_config = {
        .ap = {
            .ssid = AP_SSID,
            .password = AP_PASSWORD,
            .ssid_len = strlen(AP_SSID),
            .channel = AP_CHANNEL,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .max_connection = AP_MAX_CLIENTS,
            .beacon_interval = 100,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };

    /* Set WiFi mode to STA+AP */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));

    /* Configure AP IP address */
    esp_netif_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip, 192, 168, 4, 1);
    IP4_ADDR(&ip_info.gw, 192, 168, 4, 1);
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
    esp_netif_dhcps_stop(ap_netif);
    esp_netif_set_ip_info(ap_netif, &ip_info);
    esp_netif_dhcps_start(ap_netif);

    /* Start WiFi */
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi initialization complete (STA+AP mode)");
    return ESP_OK;
}

/* Wait for WiFi connection */
esp_err_t wifi_wait_connected(TickType_t timeout)
{
    if (!s_wifi_event_group) {
        return ESP_ERR_INVALID_STATE;
    }

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                          WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                          pdFALSE,
                                          pdFALSE,
                                          timeout);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "STA connected to upstream WiFi");
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGW(TAG, "Failed to connect to upstream WiFi after %d attempts", STA_MAX_RETRIES);
        return ESP_FAIL;
    } else {
        ESP_LOGW(TAG, "WiFi connection timeout");
        return ESP_ERR_TIMEOUT;
    }
}

/* Get WiFi status */
void wifi_print_status(void)
{
    wifi_ap_record_t ap_info;
    esp_wifi_sta_get_ap_info(&ap_info);

    wifi_sta_list_t sta_list;
    esp_wifi_ap_get_sta_list(&sta_list);

    ESP_LOGI(TAG, "========== WiFi Status ==========");
    ESP_LOGI(TAG, "AP SSID: %s", ap_info.ssid);
    ESP_LOGI(TAG, "AP RSSI: %d", ap_info.rssi);
    ESP_LOGI(TAG, "Connected Clients: %d/%d", sta_list.num, AP_MAX_CLIENTS);
    ESP_LOGI(TAG, "AP SSID: %s", AP_SSID);
    ESP_LOGI(TAG, "================================");
}
