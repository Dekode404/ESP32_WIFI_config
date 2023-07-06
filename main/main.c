/*
 * @author:- Saurabh kadam
 * @date:- 19/06/2023
 * @brief:- This file is main function file for the WIFI config functionality.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include <esp_http_server.h>

#include "driver/gpio.h" // Used for the GPIO
#include "connect.h"     // Manually added file for the wifi related oprations
#include "main.h"

/* Micros define for string used in the wifi opreations */
#define SSID "SSK"            // Change SSID according to the nearby wifi station
#define PASS "12345678"       // Change PASSWORD according to the nearby wifi station
#define TIMEOUT_FOR_WIFI 5000 // Connection timeout for the WIFI

#define INDICATION_LED GPIO_NUM_2
#define WIFI_MODE_SWITCH GPIO_NUM_13

#define GPIO_HIGH 0x01
#define GPIO_LOW 0x00

static httpd_handle_t server = NULL;

esp_err_t home_web_handler(httpd_req_t *req);
httpd_handle_t start_webserver(void);
void Initialize_GPIO_PIN(void);
void initialize_nvs(void);

/*
 * @brief - Function to initialize the NVS flash used by the WIFI components
 * @note - Befor start the WIFI as the component initialize the NVS.
 */
void initialize_nvs(void)
{
    esp_err_t err = nvs_flash_init(); // initialize the default NVS partition

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    ESP_ERROR_CHECK(err);
}

void Initialize_GPIO_PIN(void)
{
    /* Set the GPIO as a push/pull output */
    gpio_reset_pin(INDICATION_LED);
    gpio_set_direction(INDICATION_LED, GPIO_MODE_OUTPUT);
    gpio_set_level(INDICATION_LED, GPIO_LOW);

    gpio_reset_pin(WIFI_MODE_SWITCH);
    gpio_set_direction(WIFI_MODE_SWITCH, GPIO_MODE_INPUT);
}

void app_main(void)
{

    // Initialize_GPIO_PIN();

    initialize_nvs(); // Initiate NVS which is used for the wifi

    wifi_init(); // Initiate wifi

    wifi_connect_ap(SSID, PASS); // This function is not needed

    if (server == NULL)
    {
        server = start_webserver();
    }
}

/* An HTTP GET handler */
esp_err_t home_web_handler(httpd_req_t *req)
{
    /* Send a simple response */
    const char *resp_str = "Hello world";
    httpd_resp_send(req, resp_str, strlen(resp_str));
    return ESP_OK;
}

/* Start the server */
httpd_handle_t start_webserver(void)
{
    httpd_uri_t home_page_get = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = home_web_handler,
        .user_ctx = NULL};

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &home_page_get);
        httpd_start(&server, &config);
    }

    return NULL;
}

void stop_webserver(httpd_handle_t server)
{
    /* Stop the httpd server */
    httpd_stop(server);
}
