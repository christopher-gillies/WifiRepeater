#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"

#include "wifi_manager.h"
#include "dhcp_server.h"
#include "dns_forwarding.h"
#include "ip_forward.h"
#include "wifi_config.h"

static const char *TAG = LOG_TAG;

/**
 * LED blinking task - blinks every 500ms (250ms on, 250ms off)
 */
void led_blink_task(void *pvParameters)
{
    while (1) {
        gpio_set_level(LED_GPIO_PIN, 1);  // LED on
        vTaskDelay(pdMS_TO_TICKS(LED_BLINK_PERIOD_MS / 2));

        gpio_set_level(LED_GPIO_PIN, 0);  // LED off
        vTaskDelay(pdMS_TO_TICKS(LED_BLINK_PERIOD_MS / 2));
    }
}

/**
 * Initialize LED GPIO and start blinking task
 */
void led_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GPIO_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    gpio_config(&io_conf);
    gpio_set_level(LED_GPIO_PIN, 0);  // Start with LED off

    ESP_LOGI(TAG, "LED initialized on GPIO %d", LED_GPIO_PIN);

    xTaskCreate(led_blink_task, "led_blink", 1024, NULL, 1, NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "WiFi Repeater with NAT - Initializing");
    ESP_LOGI(TAG, "========================================");

    /* Initialize LED blinking */
    ESP_LOGI(TAG, "Step 0: Initializing LED");
    led_init();

    /* Initialize WiFi in STA+AP mode */
    ESP_LOGI(TAG, "Step 1: Initializing WiFi (STA+AP mode)");
    esp_err_t ret = wifi_init_sta_ap();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize WiFi");
        return;
    }

    /* Wait for STA to connect to upstream WiFi */
    ESP_LOGI(TAG, "Step 2: Waiting for upstream WiFi connection");
    ret = wifi_wait_connected(pdMS_TO_TICKS(30000));  // 30 second timeout
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to connect to upstream WiFi, but AP is still running");
    }

    /* Initialize DHCP server for AP clients */
    ESP_LOGI(TAG, "Step 3: Initializing DHCP server");
    ret = dhcp_server_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize DHCP server");
        return;
    }

    /* Apply DNS servers to DHCP configuration */
    ESP_LOGI(TAG, "Step 4: Configuring DNS forwarding");
    ret = dhcp_server_set_dns();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "DNS: Could not configure DNS forwarding (clients may not resolve domains)");
    }

    /* Enable IP forwarding for WiFi repeater functionality */
    ESP_LOGI(TAG, "Step 5: Enabling IP forwarding");
    ret = ip_forward_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable IP forwarding");
        return;
    }

    /* Print initial status */
    vTaskDelay(pdMS_TO_TICKS(1000));
    wifi_print_status();
    dhcp_server_print_status();

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "WiFi Repeater Ready!");
    ESP_LOGI(TAG, "Upstream SSID: %s", STA_SSID);
    ESP_LOGI(TAG, "Repeater SSID: %s", AP_SSID);
    ESP_LOGI(TAG, "AP IP Address: %s", AP_IP_ADDR);
    ESP_LOGI(TAG, "========================================");

    /* Main loop */
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));  // Print status every 10 seconds
        wifi_print_status();
    }
}