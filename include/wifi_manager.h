#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"

/**
 * Initialize WiFi in STA+AP mode
 * STA: Connects to upstream WiFi network
 * AP: Creates repeater WiFi network
 *
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t wifi_init_sta_ap(void);

/**
 * Wait for STA to connect to upstream WiFi
 *
 * @param timeout Maximum time to wait (in ticks)
 * @return ESP_OK if connected, ESP_FAIL if connection failed, ESP_ERR_TIMEOUT if timeout
 */
esp_err_t wifi_wait_connected(TickType_t timeout);

/**
 * Print current WiFi status (STA and AP information)
 */
void wifi_print_status(void);

#endif // WIFI_MANAGER_H
