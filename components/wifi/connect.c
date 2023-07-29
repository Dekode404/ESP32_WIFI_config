#include "connect.h"

const static char *TAG = "WIFI";
esp_netif_t *esp_netif;

static EventGroupHandle_t wifi_events; // event handler for the WIFI

/*
 * This function is used for the debug purpose. According to the error this return the char string pointer that contain the error message.
 */
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

/*
 * This function is for handling the all the WIFI related events.
 */
void WIFI_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case SYSTEM_EVENT_STA_START: // Connecting to forcefully to the AP network.
    {
        ESP_LOGI(TAG, "connecting...");
        esp_wifi_connect(); // Connect the ESP32 WiFi station to the AP network. [configured while start the STA mode]
    }
    break;

    case SYSTEM_EVENT_STA_CONNECTED: // ESP32 connected successfully to the WIFI network.
    {
        ESP_LOGI(TAG, "connected");
    }
    break;

    case SYSTEM_EVENT_STA_DISCONNECTED: // ESP32 Disconnected from the AP network
    {
        /* Initialize argument structure for WIFI_EVENT_STA_DISCONNECTED event*/
        wifi_event_sta_disconnected_t *wifi_event_sta_disconnected = event_data;

        // If the disconnection reason is due to AP network is not alliable.
        if (wifi_event_sta_disconnected->reason == WIFI_REASON_ASSOC_LEAVE)
        {
            ESP_LOGI(TAG, "disconnected");
            xEventGroupSetBits(wifi_events, ESP32_DISCONNECTED);
            break;
        }

        /*
         * To debug the WIFI disconnection pass the disconnection code to the get_error function so the we can get the disconnection message,
         */
        const char *err = get_error(wifi_event_sta_disconnected->reason);
        ESP_LOGE(TAG, "disconnected: %s", err);

        esp_wifi_connect();
        // xEventGroupSetBits(wifi_events, ESP32_DISCONNECTED);
    }
    break;

    case IP_EVENT_STA_GOT_IP:
    {
        ESP_LOGI(TAG, "GOT IP");
        xEventGroupSetBits(wifi_events, ESP32_GOT_IP); // Set the BIT of connection is successful.
    }
    break;

    case WIFI_EVENT_AP_START: // ESP32 start in the AP mode
    {
        ESP_LOGI(TAG, "AP started");
    }
    break;

    case WIFI_EVENT_AP_STOP: // AP mode is stop
    {
        ESP_LOGI(TAG, "AP stopped");
    }
    break;

    default:
        break;
    }
}

/*
 * This function is for start the WIFI peripheral in the ESP
 */
void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());                // Initialize the underlying TCP/IP stack
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // Create default event loop

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT(); //  WiFi stack configuration parameters to the default

    /*
     *Initialize WiFi Allocate resource for WiFi driver, such as WiFi control structure,
     * RX/TX buffer, WiFi NVS structure etc. This WiFi also starts WiFi task
     */
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, WIFI_event_handler, NULL));  // Register an WIFI event handler to the system event loop (legacy).
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, WIFI_event_handler, NULL)); // Register an IP event handler to the system event loop (legacy).

    /*
     * Set the WiFi API configuration storage type.
     * For this we have the two options - WIFI_STORAGE_RAM & WIFI_STORAGE_RAM
     */
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
}

/*
 * @brief  function is used for the connect the WIFI to the station network.

 * @param[in] ssid - SSID of the WIFI network over which 2we have to th3e connect.
 * @param[in] pass - credentials of the wifi network over which we have to the connect.
 * @param[in] timeout - Time out if the ESP is not able to connect to the WIFI network.
 *
 * @return
 *   - ESP_OK: succeed
 *   - ESP_FAIL: WiFi is not able to connect with the WIFI network
 */
esp_err_t wifi_connect_sta(const char *ssid, const char *pass, int timeout)
{
    wifi_events = xEventGroupCreate(); // Creat the event group for the WIFI sta mode.

    esp_netif = esp_netif_create_default_wifi_sta(); // Creates default WIFI STA. In case of any init error this API aborts.

    wifi_config_t wifi_config;                      // Initialize the variable to hold the credential parameters of the WIFI network.
    memset(&wifi_config, 0, sizeof(wifi_config_t)); // Set to zero

    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);         // copy the SSID
    strncpy((char *)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password) - 1); // copy the password

    esp_wifi_set_mode(WIFI_MODE_STA);                   // Set the WiFi operating mode to the station.
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config); // Pass the credential parameters of the WIFI network
    esp_wifi_start();                                   // Start WiFi according to current configuration

    /*
     * block to wait for one bits to be set within a previously created event group.
     */
    EventBits_t result = xEventGroupWaitBits(wifi_events, ESP32_GOT_IP | ESP32_DISCONNECTED, pdTRUE, pdFALSE, pdMS_TO_TICKS(timeout));

    esp_err_t function_status = ESP_FAIL; // Initialize the variable to return the function status of the WIFI is connected or not.

    /*
     * If the WIFI is connected successfully to the station network change the function_status to the successful.
     */
    if (result == ESP32_GOT_IP)
    {
        function_status = ESP_OK;
    }

    return function_status;
}

void wifi_connect_ap(const char *ssid, const char *pass)
{
    esp_netif = esp_netif_create_default_wifi_ap(); // Creates default WIFI STA. In case of any init error this API aborts.

    wifi_config_t wifi_config;                      // Initialize the variable to hold the credential parameters of the WIFI network.
    memset(&wifi_config, 0, sizeof(wifi_config_t)); // Clear the memory to avoid the garbage value.

    strncpy((char *)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid) - 1);         // copy the SSID
    strncpy((char *)wifi_config.ap.password, pass, sizeof(wifi_config.ap.password) - 1); // copy the PASS

    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK; // Set the wifi authentication mode to the WPA2
    wifi_config.ap.max_connection = 2;                // St the max connection the can connected to the ESP32 when ESP is in AP mode is set to the 2.

    esp_wifi_set_mode(WIFI_MODE_AP);                   // Set the WiFi operating mode to the station.
    esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config); // Pass the credential parameters of the AP WIFI network
    esp_wifi_start();                                  // Start WiFi according to current configuration
}

/*
 * This function is used for the disconnect the WIFI from the current access-point network
 */
void wifi_disconnect(void)
{
    esp_wifi_disconnect(); // Disconnect the ESP32 WiFi station from the AP.

    /*
     * Stop WiFi (mode is WIFI_MODE_STA) stop station and free station control block.
     */
    esp_wifi_stop();
}

/*
 * This function is used for the Destroys the esp_netif object.
 * @brief - while change the WIFI mode from the STA to the AP or vise varas then need to Destroys the esp_netif object to free the control block.
 */
void wifi_destroy_netif(void)
{
    esp_netif_destroy(esp_netif); // Destroys the esp_netif object
}