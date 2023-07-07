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
