/**
 * @file wifi_manager.h
 *
 * @brief WiFi Credential Management System header
 *
 * This header file defines functions and data structures for managing WiFi credentials on an ESP32 device.
 * The system provides a web-based configuration interface, supports on-demand configuration initiation via a physical switch,
 * and ensures that WiFi credentials persist across device restarts.
 *
 * @author Saurabh Kadam
 * @date 21 Jun 2023
 * @version 1.0
 *
 *
 */

#ifndef wifi_manager_h
#define wifi_manager_h

/* Standard C Library Includes
 * This header file includes standard C libraries such as <stdio.h> and <string.h> for
 * general input/output and string manipulation functions.
 */
#include <stdio.h>
#include <string.h>

/* The code includes various FreeRTOS and ESP-IDF libraries.
 * These libraries are used for tasks, queues, event groups, HTTP server functionality, non-volatile storage,
 * GPIO control, and error handling in the ESP-IDF environment.
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "esp_http_server.h"
#include "nvs_flash.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_err.h"

#include "esp_netif.h"

#define ON_DEMAND_SWITCH_GPIO GPIO_NUM_0 // This pin is used for the trigger the wifi manager portal

#define AP_SSID "ESP32"    // Default SSID of the ESP32 while operating into AP mode
#define AP_PASS "12345678" // Default PASS of the ESP32 while operating into AP mode

#define STATIC_HOME_PAGE "<!DOCTYPE html><html><body><h2>Set WiFi Credentials</h2><form action='/save_credentials'> <label for='ssid'>SSID:</label><br> <input type='text' id='ssid' name='ssid' required><br> <label for='password'>Password:</label><br> <input type='password' id='password' name='password' required><br><br> <input type='submit' value='Submit'> </form>  </body> </html>"
#define STATIC_SAVE_PAGE "<!DOCTYPE html><html><body><h2>WiFi Credentials saved</h2> </body> </html>"

#ifdef DEBUG_CODE
static const char *TAG = "SERVER";
#endif

// This struct is for the storing the WIFI credentials
typedef struct
{
    char WIFI_SSID[10];     // 10 byte storage for the SSID
    char WIFI_PASSWORD[20]; // 20 byte storage for the password

} WIFI_CREDENTIALS_t;

/*
 * This struct is used as the global variable to store all parameter used for the WIFI
 */
typedef struct
{
    WIFI_CREDENTIALS_t wifi_credentials_s;

    uint8_t WIFI_mode;

} WIFI_MANAGER_t;

/* This fuction is for the save the wifi credentials into the NVS */
esp_err_t save_the_wifi_credentials_into_NVS(WIFI_CREDENTIALS_t *wifi_credentials);

/* This fuction is for the read the wifi credentials from the NVS */
esp_err_t read_the_wifi_credentials_from_NVS(WIFI_CREDENTIALS_t *wifi_credentials);

/* This fuction is handler function for the home page URL */
esp_err_t set_wifi_credentials_url(httpd_req_t *req);

/* This fuction is handler function for the WIFI credentials save */
esp_err_t save_wifi_credentials_url(httpd_req_t *req);

/* This function is for the checking the on demand wifi switch. */
esp_err_t check_for_on_demand_condition(void);

/* This function is used for the Initialize the ondemand switch GPIO pin. */
esp_err_t Initialize_GPIO_for_on_demand_portal(void);

/* Function is responsible for initializing and starting a web server on an ESP32*/
void init_web_server(void);

#endif