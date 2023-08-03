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

void wifi_init(void);
esp_err_t wifi_connect_sta(const char *ssid, const char *pass, int timeout);
void wifi_connect_ap(const char *ssid, const char *pass);
void wifi_disconnect(void);
void wifi_destroy_netif(void);

#endif