#ifndef connect_h
#define connect_h

// This used for the WIFI
#include "nvs_flash.h"
#include "nvs_flash.h"
#include "esp_partition.h"
#include "sdkconfig.h"
#include "esp_err.h"

void initialize_nvs(void);
void wifi_init(void);
esp_err_t wifi_connect_sta(const char *ssid, const char *pass, int timeout);
void wifi_connect_ap(const char *ssid, const char *pass);
void wifi_disconnect(void);

#endif