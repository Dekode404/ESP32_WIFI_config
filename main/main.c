#include "connect.h"      // This is manually added file for the wifi related functionality
#include "wifi_manager.h" // This is manullay added file for the WIFI manager functionality

/* Uncomment the below line to debug the wifi manager functionality*/
// #define DEBUG_CODE

#define AP_ssid "ESP32_SETUP"   // This SSID used for the setup the AP
#define AP_password "123456789" // This Password used for the setup the AP

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init()); // Initialize the NVS flash used for the WIFI functionality.

    wifi_init(); // Initialize the WIFI.

    check_for_on_demand_condition(); // This function is used for the creat the on demand portal for the ESP32

    WIFI_CREDENTIALS_t wifi_credentials_read_from_NVS; // Creat the variable for the wifi credentials

    // Read the credentials of the WIFI from the NVS flash
    if (read_the_wifi_credentials_from_NVS(&wifi_credentials_read_from_NVS) != ESP_OK)
    {
        /*
         * When the no credentials found in the NVS turn on the WIFI in access point mode so that
         * user can set the wifi over the local web server.
         */
        wifi_connect_ap(AP_ssid, AP_password);

        init_web_server(); // Initialize the server so user can set the wifi parameters over the WEB
    }

    wifi_connect_sta(wifi_credentials_read_from_NVS.WIFI_SSID, wifi_credentials_read_from_NVS.WIFI_PASSWORD, 10000);

#ifdef DEBUG_CODE
    printf(" SSID is - %s \n SSID size of the string is %d \n PASSWORD - %s \n Password size of the string is %d \n", wifi_credentials_read_from_NVS.WIFI_SSID, strlen(wifi_credentials_read_from_NVS.WIFI_SSID), wifi_credentials_read_from_NVS.WIFI_PASSWORD, strlen(wifi_credentials_read_from_NVS.WIFI_PASSWORD));
#endif
}