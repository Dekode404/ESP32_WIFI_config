#include <stdio.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_http_server.h"

#include "connect.h"
#include "main.h"

static const char *TAG = "SERVER";

// This struct is for the storing the WIFI credentials
typedef struct
{
    char WIFI_SSID[10];

    char WIFI_PASSWORD[20];

} WIFI_CREDENTIALS_t;

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

    WIFI_CREDENTIALS_t wifi_credentials_received_from_WEB_page;

    httpd_query_key_value(Buffer_for_received_the_query, "ssid", wifi_credentials_received_from_WEB_page.WIFI_SSID, 10);
    httpd_query_key_value(Buffer_for_received_the_query, "password", wifi_credentials_received_from_WEB_page.WIFI_PASSWORD, 20);

    printf("SSID is - %s \n SSID size of the string is %d \n PASSWORD - %s \n Password size of the string is %d \n", wifi_credentials_received_from_WEB_page.WIFI_SSID, strlen(wifi_credentials_received_from_WEB_page.WIFI_SSID), wifi_credentials_received_from_WEB_page.WIFI_PASSWORD, strlen(wifi_credentials_received_from_WEB_page.WIFI_PASSWORD));

    /////////////////////////////////////////////////////////
    nvs_handle NVS_handler;

    ESP_ERROR_CHECK(nvs_open("my_credentials", NVS_READWRITE, &NVS_handler));

    ESP_ERROR_CHECK(nvs_set_blob(NVS_handler, "Credentials", &wifi_credentials_received_from_WEB_page, sizeof(WIFI_CREDENTIALS_t)));

    ESP_ERROR_CHECK(nvs_commit(NVS_handler));

    nvs_close(NVS_handler);
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

    nvs_handle NVS_handler;
    nvs_open("my_credentials", NVS_READWRITE, &NVS_handler);

    WIFI_CREDENTIALS_t wifi_credentials_read_from_NVS;

    size_t i = sizeof(WIFI_CREDENTIALS_t);

    esp_err_t result = nvs_get_blob(NVS_handler, "Credentials", &wifi_credentials_read_from_NVS, &i);

    switch (result)
    {
    case ESP_ERR_NVS_NOT_FOUND:
    case ESP_ERR_NOT_FOUND:
        ESP_LOGE(TAG, "Value not set yet");
        break;
    case ESP_OK:
        ESP_LOGI(TAG, "Value is set");
        break;
    default:
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(result));
        break;
    }
    nvs_close(NVS_handler);

    printf("SSID is - %s \n SSID size of the string is %d \n PASSWORD - %s \n Password size of the string is %d \n", wifi_credentials_read_from_NVS.WIFI_SSID, strlen(wifi_credentials_read_from_NVS.WIFI_SSID), wifi_credentials_read_from_NVS.WIFI_PASSWORD, strlen(wifi_credentials_read_from_NVS.WIFI_PASSWORD));

    wifi_init();
    wifi_connect_ap("POCO", "password");

    init_server();
}
