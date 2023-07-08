#include <stdio.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_http_server.h"

#include "connect.h"
#include "main.h"

static const char *TAG = "SERVER";

static esp_err_t set_wifi_credentials_url(httpd_req_t *req)
{
    ESP_LOGI(TAG, "URL: %s", req->uri);
    httpd_resp_sendstr(req, set_wifi_credentials_HTML);
    return ESP_OK;
}

static esp_err_t save_wifi_credentials_url(httpd_req_t *req)
{
    ESP_LOGI(TAG, "URL: %s", req->uri);
    /////////////////////////////////////////////////////////

    size_t size_of_the_query = 1 + httpd_req_get_url_query_len(req);

    printf("size of the received query is %d \n", size_of_the_query);

    char Buffer_for_received_the_query[size_of_the_query + 1];

    httpd_req_get_url_query_str(req, Buffer_for_received_the_query, size_of_the_query);

    printf("The query string received from the url - %s \n", Buffer_for_received_the_query);

    char SSID_buffer[10];
    char PASS_buffer[10];

    httpd_query_key_value(Buffer_for_received_the_query, "ssid", SSID_buffer, 10);
    httpd_query_key_value(Buffer_for_received_the_query, "password", PASS_buffer, 10);

    printf("SSID is - %s \n SSID size of the string is %d \n PASSWORD - %s \n Password size of the string is %d \n", SSID_buffer, strlen(SSID_buffer), PASS_buffer, strlen(PASS_buffer));
    /////////////////////////////////////////////////////////
    httpd_resp_sendstr(req, save_wifi_credentials_HTML);
    return ESP_OK;
}

static void init_server()
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_ERROR_CHECK(httpd_start(&server, &config));

    httpd_uri_t set_wifi_credentials_url_handler = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = set_wifi_credentials_url};
    httpd_register_uri_handler(server, &set_wifi_credentials_url_handler);

    httpd_uri_t save_wifi_credentials_url_handler = {
        .uri = "/save_credentials",
        .method = HTTP_GET,
        .handler = save_wifi_credentials_url};
    httpd_register_uri_handler(server, &save_wifi_credentials_url_handler);
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init();
    wifi_connect_ap("POCO", "password");

    init_server();
}
