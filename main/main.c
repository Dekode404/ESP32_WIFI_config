#include <stdio.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_http_server.h"

#include "connect.h"
#include "main.h"

#define AP_ssid "ESP32_SETUP"   // This SSID used for the setup the AP
#define AP_password "123456789" // This Password used for the setup the AP

static const char *TAG = "SERVER";

// This struct is for the storing the WIFI credentials
typedef struct
{
    char WIFI_SSID[10];     // 10 byte storage for the SSID
    char WIFI_PASSWORD[20]; // 20 byte storage for the password

} WIFI_CREDENTIALS_t;

/*
 * This fuction is for the save the wifi credentials into the NVS
 */
static esp_err_t save_the_wifi_credentials_into_NVS(WIFI_CREDENTIALS_t *wifi_credentials)
{
    nvs_handle NVS_handler; // NVS handler initialize

    // Open non-volatile storage with a given namespace from the default NVS partition
    ESP_ERROR_CHECK(nvs_open("my_credentials", NVS_READWRITE, &NVS_handler));

    // set variable length binary value for given key
    ESP_ERROR_CHECK(nvs_set_blob(NVS_handler, "Credentials", wifi_credentials, sizeof(WIFI_CREDENTIALS_t)));

    // Write any pending changes to non-volatile storage
    ESP_ERROR_CHECK(nvs_commit(NVS_handler));

    // Close the storage handle and free any allocated resources
    nvs_close(NVS_handler);

    return ESP_OK;
}

/*
 * This fuction is for the read the wifi credentials from the NVS
 */
static esp_err_t read_the_wifi_credentials_from_NVS(WIFI_CREDENTIALS_t *wifi_credentials)
{
    nvs_handle NVS_handler; // NVS handler initialize

    // Open non-volatile storage with a given namespace from the default NVS partition
    nvs_open("my_credentials", NVS_READWRITE, &NVS_handler);

    size_t size_of_the_wifi_credentials = sizeof(WIFI_CREDENTIALS_t);

    esp_err_t result = nvs_get_blob(NVS_handler, "Credentials", wifi_credentials, &size_of_the_wifi_credentials);

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

    // Close the storage handle and free any allocated resources
    nvs_close(NVS_handler);

    return result;
}

static esp_err_t set_wifi_credentials_url(httpd_req_t *req)
{
    ESP_LOGI(TAG, "URL: %s", req->uri);
    httpd_resp_sendstr(req, STATIC_HOME_PAGE);
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

    save_the_wifi_credentials_into_NVS(&wifi_credentials_received_from_WEB_page);

    httpd_resp_sendstr(req, STATIC_SAVE_PAGE);
    return ESP_OK;
}

/*
 * Function for starting the webserver. URI handlers can be registered in real time as long as the
 * server handle is valid.
 */
static void init_web_server()
{
    httpd_handle_t server = NULL;                   // Declination of the server handler.
    httpd_config_t config = HTTPD_DEFAULT_CONFIG(); // Declination of the server configuration.

    ESP_ERROR_CHECK(httpd_start(&server, &config)); // Start the server wi the default settting

    /* This URL is for the home page */
    httpd_uri_t set_wifi_credentials_url_handler = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = set_wifi_credentials_url};
    httpd_register_uri_handler(server, &set_wifi_credentials_url_handler);

    /* This URL is for the save the wifi setting into the NVS */
    httpd_uri_t save_wifi_credentials_url_handler = {
        .uri = "/save_credentials",
        .method = HTTP_GET,
        .handler = save_wifi_credentials_url};
    httpd_register_uri_handler(server, &save_wifi_credentials_url_handler);
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    WIFI_CREDENTIALS_t wifi_credentials_read_from_NVS;

    wifi_init(); // Initialize the WIFI.

    if (read_the_wifi_credentials_from_NVS(&wifi_credentials_read_from_NVS) != ESP_OK)
    {
        wifi_connect_ap(AP_ssid, AP_password);

        init_web_server(); // Initialize the server so user can set the wifi parameters over the WEB
    }

    wifi_connect_sta(wifi_credentials_read_from_NVS.WIFI_SSID, wifi_credentials_read_from_NVS.WIFI_PASSWORD, 10000);

    printf(" SSID is - %s \n SSID size of the string is %d \n PASSWORD - %s \n Password size of the string is %d \n", wifi_credentials_read_from_NVS.WIFI_SSID, strlen(wifi_credentials_read_from_NVS.WIFI_SSID), wifi_credentials_read_from_NVS.WIFI_PASSWORD, strlen(wifi_credentials_read_from_NVS.WIFI_PASSWORD));
}