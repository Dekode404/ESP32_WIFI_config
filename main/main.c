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

httpd_handle_t server = NULL;
typedef struct
{
    httpd_handle_t hd;
    int fd;

} async_resp_arg;

int led_state = 0;
#define LED_PIN 2

static const char *TAG = "WebSocket Server"; // TAG for debug

char response_data[4096];

httpd_handle_t setup_websocket_server(void);
esp_err_t handle_ws_req(httpd_req_t *req);
esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req);
void ws_async_send(void *arg);
esp_err_t get_req_handler(httpd_req_t *req);
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

    Initialize_GPIO_PIN();

    initialize_nvs(); // Initiate NVS which is used for the wifi

    wifi_init(); // Initiate wifi

    uint8_t wifi_status = 0x00;

    if (gpio_get_level(WIFI_MODE_SWITCH))
    {
        if (ESP_OK == wifi_connect_sta(SSID, PASS, TIMEOUT_FOR_WIFI)) // This function is not needed
            printf("connected to the WIFI \n");
        else
            printf("Not connected to the WIFI \n");
    }
    else
    {
        wifi_connect_ap(SSID, PASS); // This function is not needed

        gpio_pad_select_gpio(LED_PIN);
        gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

        led_state = 0;

        ESP_LOGI(TAG, "ESP32 ESP-IDF WebSocket Web Server is running ... ...\n");

        setup_websocket_server();
    }
}

esp_err_t get_req_handler(httpd_req_t *req)
{
    int response;
    if (led_state)
    {
        sprintf(response_data, index_html, "ON");
    }
    else
    {
        sprintf(response_data, index_html, "OFF");
    }
    response = httpd_resp_send(req, response_data, HTTPD_RESP_USE_STRLEN);

    return response;
}

// Error due to the old file is used in this code.

void ws_async_send(void *arg)
{
    httpd_ws_frame_t ws_pkt;
    async_resp_arg *resp_arg = arg;
    httpd_handle_t hd = resp_arg->hd;
    int fd = resp_arg->fd;

    led_state = !led_state;
    gpio_set_level(LED_PIN, led_state);

    char buff[4];
    memset(buff, 0, sizeof(buff));
    sprintf(buff, "%d", led_state);

    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *)buff;
    ws_pkt.len = strlen(buff);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    static size_t max_clients = CONFIG_LWIP_MAX_LISTENING_TCP;
    size_t fds = max_clients;
    int client_fds[max_clients];

    esp_err_t ret = httpd_get_client_list(server, &fds, client_fds);

    if (ret != ESP_OK)
    {
        return;
    }

    for (int i = 0; i < fds; i++)
    {
        int client_info = httpd_ws_get_fd_info(server, client_fds[i]);

        if (client_info == HTTPD_WS_CLIENT_WEBSOCKET)
        {
            httpd_ws_send_frame_async(hd, client_fds[i], &ws_pkt);
        }
    }
    free(resp_arg);
}

esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req)
{
    async_resp_arg *resp_arg = malloc(sizeof(async_resp_arg));
    resp_arg->hd = req->handle;
    resp_arg->fd = httpd_req_to_sockfd(req);
    return httpd_queue_work(handle, ws_async_send, resp_arg);
}

esp_err_t handle_ws_req(httpd_req_t *req)
{
    if (req->method == HTTP_GET)
    {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }

    if (ws_pkt.len)
    {
        buf = calloc(1, ws_pkt.len + 1);
        if (buf == NULL)
        {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        ESP_LOGI(TAG, "Got packet with message: %s", ws_pkt.payload);
    }

    ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);

    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT &&
        strcmp((char *)ws_pkt.payload, "toggle") == 0)
    {
        free(buf);
        return trigger_async_send(req->handle, req);
    }
    return ESP_OK;
}

httpd_handle_t setup_websocket_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t uri_get = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = get_req_handler,
        .user_ctx = NULL};

    httpd_uri_t ws = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = handle_ws_req,
        .user_ctx = NULL,
        .is_websocket = true};

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &ws);
    }

    return server;
}
