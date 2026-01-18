#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include <stdint.h>
#include "secret.h"  // Contains STA_SSID, STA_PASSWORD, AP_SSID, AP_PASSWORD

/* WiFi AP (Access Point) Configuration */
/* AP_SSID and AP_PASSWORD are defined in secret.h */
#define AP_CHANNEL 6
#define AP_HIDDEN 0
#define AP_MAX_CLIENTS 20

/* Network Configuration */
#define AP_IP_ADDR "192.168.4.1"
#define AP_NETMASK "255.255.255.0"
#define AP_GATEWAY "192.168.4.1"

/* DHCP Server Configuration */
#define DHCP_START_IP "192.168.4.2"
#define DHCP_END_IP "192.168.4.20"
#define DHCP_LEASE_TIME 3600  // 1 hour in seconds

/* STA (Station) Configuration - Upstream WiFi */
/* STA_SSID and STA_PASSWORD are defined in secret.h */
#define STA_MAX_RETRIES 5
#define STA_RETRY_INTERVAL 5000  // 5 seconds

/* NAT Configuration */
#define NAT_MAX_SESSIONS 50
#define NAT_SESSION_TIMEOUT 120  // 2 minutes

/* LED Configuration */
#define LED_GPIO_PIN 2                        // GPIO2 - built-in LED on Freenove ESP32-S3 WROOM-1
#define LED_BLINK_PERIOD_MS 500               // Total blink period in milliseconds

/* Logging */
#define LOG_TAG "WiFiRepeater"

typedef struct {
    char ssid[32];
    char password[64];
} wifi_credentials_t;

#endif // WIFI_CONFIG_H
