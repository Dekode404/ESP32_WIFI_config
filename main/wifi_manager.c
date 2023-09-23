/**
 * @file wifi_manager.c
 *
 * @brief WiFi Credential Management System source
 *
 * This file defines functions and data structures for managing WiFi credentials on an ESP32 device.
 * The system provides a web-based configuration interface, supports on-demand configuration initiation via a physical switch,
 * and ensures that WiFi credentials persist across device restarts.
 *
 * @note Before using this module, ensure that you have included the required header files and initialized necessary components,
 *       such as ESP32 NVS (Non-Volatile Storage) and WiFi, in your main application code.
 *
 * @note This header also includes references to ESP32-specific components, such as NVS and HTTP server functions.
 *
 * @warning This header assumes that certain functions, like NVS initialization and HTTP server setup, are defined elsewhere in the code.
 *          Make sure to include the required components and functions in your application for complete functionality.
 *
 * @author Saurabh Kadam
 * @date 21 Jun 2023
 * @version 1.0
 *
 * @see "connect.h" for WiFi management functions and definitions.
 *
 */

#include "wifi_manager.h"

/*
 * This function effectively stores the WiFi credentials in the NVS storage under the "my_credentials" namespace with the key "Credentials."
 * Storing these credentials in NVS allows for persistence, so they can be retrieved and used even after a power cycle or reboot of the ESP32 device.
 * It's important to note that error checking is done at various stages to ensure that the operations complete successfully and handle any potential
 * errors that may occur during NVS operations.
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
 * This function effectively reads WiFi credentials from the NVS storage and populates the wifi_credentials structure with the retrieved data.
 * It also includes error checking and debug logging to provide feedback on the status of the NVS retrieval operation.
 * If the credentials have been previously stored in NVS, they will be loaded into wifi_credentials. If not found, an appropriate error message will be logged.
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
 * This function is designed to handle requests for the home page of a web interface used for configuring WiFi credentials.
 * When a user accesses the home page URL, the server responds with the HTML content provided in STATIC_HOME_PAGE.
 * This HTML page likely contains a form where users can input their WiFi SSID and password for configuration.
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
 * Function appears to be an HTTP request handler for saving WiFi credentials received from a web page.
 * It processes a URL query string, extracts the SSID and password, and then saves them into non-volatile storage (NVS)
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
 * This function initializes a GPIO pin as an input with a pull-up resistor enabled and pull-down resistor disabled.
 * The pin's state will be checked periodically to detect changes, and no interrupts will be generated when the state changes.
 */
esp_err_t Initialize_GPIO_for_on_demand_portal(void)
{
    /* Initialize GPIO configuration variable for the on demand WIFI switch */

    gpio_config_t on_demand_switch_configuration;
    on_demand_switch_configuration.mode = GPIO_MODE_INPUT;
    on_demand_switch_configuration.pull_down_en = false;
    on_demand_switch_configuration.pull_up_en = true;
    on_demand_switch_configuration.intr_type = GPIO_INTR_DISABLE;
    on_demand_switch_configuration.pin_bit_mask = (1ULL << ON_DEMAND_SWITCH_GPIO);

    return gpio_config(&on_demand_switch_configuration);
}

/*
 * After this initialization, the system wait for some external trigger (physical switch press) to initiate the WiFi configuration process.
 * This function does the setup part, and the actual handling of the on-demand condition, possibly in an interrupt service routine or a task that monitors the GPIO pin's state.
 */
esp_err_t check_for_on_demand_condition(void)
{
    ESP_ERROR_CHECK(Initialize_GPIO_for_on_demand_portal()); // Initialize the GPIO pin which is used as the on demand pin.

    return ESP_OK;
}

/*
 * Function is responsible for initializing and starting a web server on an ESP32. It sets up the server configuration, registers URI handlers for specific URLs, and then starts the server.
 * This function sets up a basic web server on the ESP32 with two URL handlers—one for serving a home page and another for saving WiFi credentials.
 * It then starts the server, and the server will continue running as long as the program is executing.
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