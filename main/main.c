#include "main.h"

// #define DEBUG_CODE

#define AP_ssid "ESP32_SETUP"   // This SSID used for the setup the AP
#define AP_password "123456789" // This Password used for the setup the AP

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

#ifdef DEBUG_CODE
    ESP_LOGI(TAG, "SSID - %s", wifi_credentials_read_from_NVS.WIFI_SSID);
    ESP_LOGI(TAG, "SSID size - %d", strlen(wifi_credentials_read_from_NVS.WIFI_SSID));
    ESP_LOGI(TAG, "PASSWORD - %s", wifi_credentials_read_from_NVS.WIFI_PASSWORD);
    ESP_LOGI(TAG, "Password size - %d ", strlen(wifi_credentials_read_from_NVS.WIFI_PASSWORD));
#endif
}