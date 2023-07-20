#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_netif.h"
#include "esp_wifi.h"

const static char *TAG = "WIFI";
esp_netif_t *esp_netif;

static EventGroupHandle_t wifi_events;
static const int CONNECTED_GOT_IP = BIT0;
static const int DISCONNECTED = BIT1;

const char *get_error(uint8_t code)
{
    switch (code)
    {
    case WIFI_REASON_UNSPECIFIED:
        return "WIFI_REASON_UNSPECIFIED";
    case WIFI_REASON_AUTH_EXPIRE:
        return "WIFI_REASON_AUTH_EXPIRE";
    case WIFI_REASON_AUTH_LEAVE:
        return "WIFI_REASON_AUTH_LEAVE";
    case WIFI_REASON_ASSOC_EXPIRE:
        return "WIFI_REASON_ASSOC_EXPIRE";
    case WIFI_REASON_ASSOC_TOOMANY:
        return "WIFI_REASON_ASSOC_TOOMANY";
    case WIFI_REASON_NOT_AUTHED:
        return "WIFI_REASON_NOT_AUTHED";
    case WIFI_REASON_NOT_ASSOCED:
        return "WIFI_REASON_NOT_ASSOCED";
    case WIFI_REASON_ASSOC_LEAVE:
        return "WIFI_REASON_ASSOC_LEAVE";
    case WIFI_REASON_ASSOC_NOT_AUTHED:
        return "WIFI_REASON_ASSOC_NOT_AUTHED";
    case WIFI_REASON_DISASSOC_PWRCAP_BAD:
        return "WIFI_REASON_DISASSOC_PWRCAP_BAD";
    case WIFI_REASON_DISASSOC_SUPCHAN_BAD:
        return "WIFI_REASON_DISASSOC_SUPCHAN_BAD";
    case WIFI_REASON_IE_INVALID:
        return "WIFI_REASON_IE_INVALID";
    case WIFI_REASON_MIC_FAILURE:
        return "WIFI_REASON_MIC_FAILURE";
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
        return "WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT";
    case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT:
        return "WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT";
    case WIFI_REASON_IE_IN_4WAY_DIFFERS:
        return "WIFI_REASON_IE_IN_4WAY_DIFFERS";
    case WIFI_REASON_GROUP_CIPHER_INVALID:
        return "WIFI_REASON_GROUP_CIPHER_INVALID";
    case WIFI_REASON_PAIRWISE_CIPHER_INVALID:
        return "WIFI_REASON_PAIRWISE_CIPHER_INVALID";
    case WIFI_REASON_AKMP_INVALID:
        return "WIFI_REASON_AKMP_INVALID";
    case WIFI_REASON_UNSUPP_RSN_IE_VERSION:
        return "WIFI_REASON_UNSUPP_RSN_IE_VERSION";
    case WIFI_REASON_INVALID_RSN_IE_CAP:
        return "WIFI_REASON_INVALID_RSN_IE_CAP";
    case WIFI_REASON_802_1X_AUTH_FAILED:
        return "WIFI_REASON_802_1X_AUTH_FAILED";
    case WIFI_REASON_CIPHER_SUITE_REJECTED:
        return "WIFI_REASON_CIPHER_SUITE_REJECTED";
    case WIFI_REASON_INVALID_PMKID:
        return "WIFI_REASON_INVALID_PMKID";
    case WIFI_REASON_BEACON_TIMEOUT:
        return "WIFI_REASON_BEACON_TIMEOUT";
    case WIFI_REASON_NO_AP_FOUND:
        return "WIFI_REASON_NO_AP_FOUND";
    case WIFI_REASON_AUTH_FAIL:
        return "WIFI_REASON_AUTH_FAIL";
    case WIFI_REASON_ASSOC_FAIL:
        return "WIFI_REASON_ASSOC_FAIL";
    case WIFI_REASON_HANDSHAKE_TIMEOUT:
        return "WIFI_REASON_HANDSHAKE_TIMEOUT";
    case WIFI_REASON_CONNECTION_FAIL:
        return "WIFI_REASON_CONNECTION_FAIL";
    case WIFI_REASON_AP_TSF_RESET:
        return "WIFI_REASON_AP_TSF_RESET";
    }
    return "WIFI_REASON_UNSPECIFIED";
}

void event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case SYSTEM_EVENT_STA_START:
        ESP_LOGI(TAG, "connecting...");
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG, "connected");
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
    {
        wifi_event_sta_disconnected_t *wifi_event_sta_disconnected = event_data;
        if (wifi_event_sta_disconnected->reason == WIFI_REASON_ASSOC_LEAVE)
        {
            ESP_LOGI(TAG, "disconnected");
            xEventGroupSetBits(wifi_events, DISCONNECTED);
            break;
        }
        const char *err = get_error(wifi_event_sta_disconnected->reason);
        ESP_LOGE(TAG, "disconnected: %s", err);
        esp_wifi_connect();
        // xEventGroupSetBits(wifi_events, DISCONNECTED);
    }
    break;
    case IP_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "GOT IP");
        xEventGroupSetBits(wifi_events, CONNECTED_GOT_IP);
        break;
    case WIFI_EVENT_AP_START:
        ESP_LOGI(TAG, "AP started");
        break;
    case WIFI_EVENT_AP_STOP:
        ESP_LOGI(TAG, "AP stopped");
        break;

    default:
        break;
    }
}

void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
}

esp_err_t wifi_connect_sta(const char *ssid, const char *pass, int timeout)
{
    wifi_events = xEventGroupCreate();

    esp_netif = esp_netif_create_default_wifi_sta();

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password) - 1);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();

    EventBits_t result = xEventGroupWaitBits(wifi_events, CONNECTED_GOT_IP | DISCONNECTED, pdTRUE, pdFALSE, pdMS_TO_TICKS(timeout));
    if (result == CONNECTED_GOT_IP)
    {
        return ESP_OK;
    }
    return ESP_FAIL;
}

void wifi_connect_ap(const char *ssid, const char *pass)
{
    esp_netif = esp_netif_create_default_wifi_ap();

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    strncpy((char *)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid) - 1);
    strncpy((char *)wifi_config.ap.password, pass, sizeof(wifi_config.ap.password) - 1);
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    wifi_config.ap.max_connection = 4;

    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
    esp_wifi_start();
}

void wifi_disconnect(void)
{
    esp_wifi_disconnect();
    esp_wifi_stop();
}

void wifi_destroy_netif(void)
{
    esp_netif_destroy(esp_netif);
}

/*
 * This fuction is for the save the wifi credentials into the NVS
 */
esp_err_t save_the_wifi_credentials_into_NVS(WIFI_CREDENTIALS_t *wifi_credentials)
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
esp_err_t read_the_wifi_credentials_from_NVS(WIFI_CREDENTIALS_t *wifi_credentials)
{
    nvs_handle NVS_handler; // NVS handler initialize

    // Open non-volatile storage with a given namespace from the default NVS partition
    nvs_open("my_credentials", NVS_READWRITE, &NVS_handler);

    size_t size_of_the_wifi_credentials = sizeof(WIFI_CREDENTIALS_t);

    // get blob value for given key
    esp_err_t result = nvs_get_blob(NVS_handler, "Credentials", wifi_credentials, &size_of_the_wifi_credentials);

#ifdef DEBUG_CODE

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

#endif

    // Close the storage handle and free any allocated resources
    nvs_close(NVS_handler);

    return result;
}

/*
 * This fuction is handler function for the home page URL
 */
esp_err_t set_wifi_credentials_url(httpd_req_t *req)
{
#ifdef DEBUG_CODE
    ESP_LOGI(TAG, "URL: %s", req->uri);
#endif

    httpd_resp_sendstr(req, STATIC_HOME_PAGE);
    return ESP_OK;
}

/*
 * This fuction is handler function for the WIFI credentials save
 */
esp_err_t save_wifi_credentials_url(httpd_req_t *req)
{
#ifdef DEBUG_CODE
    ESP_LOGI(TAG, "URL: %s", req->uri);
#endif

    size_t size_of_the_query = 1 + httpd_req_get_url_query_len(req); // Get the size of the query

#ifdef DEBUG_CODE
    ESP_LOGI(TAG, "size of the received query is %d ", size_of_the_query);
#endif

    char Buffer_for_received_the_query[size_of_the_query + 1];

    httpd_req_get_url_query_str(req, Buffer_for_received_the_query, size_of_the_query);

#ifdef DEBUG_CODE
    ESP_LOGI(TAG, "The query string received from the url - %s ", Buffer_for_received_the_query);
#endif

    WIFI_CREDENTIALS_t wifi_credentials_received_from_WEB_page;

    httpd_query_key_value(Buffer_for_received_the_query, "ssid", wifi_credentials_received_from_WEB_page.WIFI_SSID, 10);
    httpd_query_key_value(Buffer_for_received_the_query, "password", wifi_credentials_received_from_WEB_page.WIFI_PASSWORD, 20);

#ifdef DEBUG_CODE
    ESP_LOGI(TAG, "SSID - %s", wifi_credentials_received_from_WEB_page.WIFI_SSID);
    ESP_LOGI(TAG, "SSID size - %d", strlen(wifi_credentials_received_from_WEB_page.WIFI_SSID));
    ESP_LOGI(TAG, "PASSWORD - %s", wifi_credentials_received_from_WEB_page.WIFI_PASSWORD);
    ESP_LOGI(TAG, "Password size - %d ", strlen(wifi_credentials_received_from_WEB_page.WIFI_PASSWORD));
#endif

    save_the_wifi_credentials_into_NVS(&wifi_credentials_received_from_WEB_page);

    httpd_resp_sendstr(req, STATIC_SAVE_PAGE);
    return ESP_OK;
}

/*
 * Function for starting the webserver. URI handlers can be registered in real time as long as the
 * server handle is valid.
 */
void init_web_server(void)
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

    while (true)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}