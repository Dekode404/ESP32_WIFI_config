#include "wifi_manager.h"

QueueHandle_t interruptQueue; // Creat the global handler for the WIFI on demand switch.

static void IRAM_ATTR gpio_isr_handler(void *args)
{
    int pinNumber = (int)args;
    xQueueSendFromISR(interruptQueue, &pinNumber, NULL);
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
 * This function is used for the Initialize the ondemand switch GPIO pin.
 */
esp_err_t Initialize_GPIO_for_on_demand_portal(void)
{
    /* Initialize GPIO configuration variable for the on demand WIFI switch */
    gpio_set_direction(ON_DEMAND_SWITCH_GPIO, GPIO_MODE_INPUT);
    gpio_pulldown_en(ON_DEMAND_SWITCH_GPIO);
    gpio_pullup_dis(ON_DEMAND_SWITCH_GPIO);
    gpio_set_intr_type(ON_DEMAND_SWITCH_GPIO, GPIO_INTR_POSEDGE);

    interruptQueue = xQueueCreate(10, sizeof(int));
    xTaskCreate(buttonPushedTask, "buttonPushedTask", 2048, NULL, 1, NULL);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(ON_DEMAND_SWITCH_GPIO, gpio_isr_handler, (void *)ON_DEMAND_SWITCH_GPIO);

    return ESP_OK;
}

void buttonPushedTask(void *params)
{
    int pinNumber, count = 0;
    while (true)
    {
        if (xQueueReceive(interruptQueue, &pinNumber, portMAX_DELAY))
        {
            // disable the interrupt
            gpio_isr_handler_remove(pinNumber);

            // wait some time while we check for the button to be released
            do
            {
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            } while (gpio_get_level(pinNumber) == 1);

            // do some work
            printf("GPIO %d was pressed %d times. The state is %d\n", pinNumber, count++, gpio_get_level(ON_DEMAND_SWITCH_GPIO));

            // re-enable the interrupt
            gpio_isr_handler_add(pinNumber, gpio_isr_handler, (void *)pinNumber);
        }
    }
}

/*
 * This function is for the checking the on demand wifi switch.
 * If the switch is press then this will initiate the server to set the wifi parameters.
 */
esp_err_t check_for_on_demand_condition(void)
{
    ESP_ERROR_CHECK(Initialize_GPIO_for_on_demand_portal()); // Initialize the GPIO pin which is used as the on demand pin.
}

/*
 * Function for starting the web server. URI handlers can be registered in real time as long as the
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