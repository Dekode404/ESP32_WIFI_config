/**
 * @file main.c
 *
 * @brief Wi-Fi Manager for ESP32
 *
 * This application code manages Wi-Fi connections on an ESP32 device. It provides the following functionality:
 * - Initializes the ESP32's non-volatile storage (NVS) for storing Wi-Fi credentials.
 * - Handles an on-demand portal for configuring Wi-Fi credentials.
 * - Reads stored Wi-Fi credentials from NVS or sets up an access point (AP) and a local web server for configuring credentials if none are found.
 * - Attempts to connect to a Wi-Fi network using the stored credentials (station mode).
 * - Optionally, prints debug information about the stored SSID and password.
 *
 * @note Before using this code, ensure that you have included the required header files for Wi-Fi management (connect.h)
 *       and the Wi-Fi manager's functionality (wifi_manager.h).
 *
 * @note To enable debug mode, uncomment the line "#define DEBUG_CODE."
 *
 * @warning This code snippet assumes that some functions (e.g., check_for_on_demand_condition(), wifi_init(),
 *          and read_the_wifi_credentials_from_NVS()) are defined elsewhere and may not provide complete functionality.
 *
 * @see "connect.h" for Wi-Fi management functions and definitions.
 * @see "wifi_manager.h" for the Wi-Fi manager's functions and structures.
 *
 * @author Saurabh kadam
 * @date 21 Jun 2023
 *
 */
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

    wifi_connect_sta(wifi_credentials_read_from_NVS.WIFI_SSID, wifi_credentials_read_from_NVS.WIFI_PASSWORD, WIFI_CONNECTION_TIMEOUT_10_SEC);

#ifdef DEBUG_CODE
    printf(" SSID is - %s \n SSID size of the string is %d \n PASSWORD - %s \n Password size of the string is %d \n", wifi_credentials_read_from_NVS.WIFI_SSID, strlen(wifi_credentials_read_from_NVS.WIFI_SSID), wifi_credentials_read_from_NVS.WIFI_PASSWORD, strlen(wifi_credentials_read_from_NVS.WIFI_PASSWORD));
#endif
}