/**
 * @file connect.h
 * @brief ESP32 Wi-Fi Management Header
 *
 * This header file provides declarations for managing Wi-Fi connections on an ESP32 device.
 * It includes functions for connecting to Wi-Fi networks in both station (client) and access point (AP) modes,
 * event handling, error reporting, and network interface management.
 *
 * The code is designed to initialize, configure, and connect to Wi-Fi networks, as well as handle disconnections and related events.
 * It also provides functions for configuring the ESP32 to operate as an AP.
 *
 * @author Saurabh kadam
 * @date 21 Jun 2023
 * @version 1.0
 *
 */

#ifndef connect_h
#define connect_h

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_err.h"

#include "esp_netif.h"
#include "esp_wifi.h"

#include "connect.h"

#define ESP32_GOT_IP BIT0
#define ESP32_DISCONNECTED BIT1

#define WIFI_CONNECTION_TIMEOUT_10_SEC 10000

void wifi_init(void);
esp_err_t wifi_connect_sta(const char *ssid, const char *pass, int timeout);
void wifi_connect_ap(const char *ssid, const char *pass);
void wifi_disconnect(void);
void wifi_destroy_netif(void);

#endif