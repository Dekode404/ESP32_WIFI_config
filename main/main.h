#ifndef main_h
#define main_h

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_http_server.h"
#include "nvs_flash.h"

#include "esp_log.h"
#include "esp_err.h"

#include "esp_netif.h"
#include "esp_wifi.h"

#include "connect.h"
#include "web_server.h"

#endif