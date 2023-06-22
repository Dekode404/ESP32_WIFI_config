/*
 * @author:- Saurabh kadam
 * @date:- 19/06/2023
 * @brief:- This file is main function file for the WIFI config functionality.
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h" // Used for the GPIO
#include "nvs_flash.h"   // This used for the WIFI

#include "connect.h" // Manually added file for the wifi related oprations

/* Micros define for string used in the wifi opreations */
#define SSID "SSK"      // Change SSID according to the nearby wifi station
#define PASS "12345678" // Change PASSWORD according to the nearby wifi station

#define HIGH_GPIO GPIO_NUM_21
#define SET_GPIO 1

/*
 * @brief - Function to initialize the NVS flash used by the WIFI components
 * @note - Befor start the WIFI as the component initialize the NVS.
 */
void initialize_nvs(void)
{
    esp_err_t err = nvs_flash_init(); // initialize the default NVS partition

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    ESP_ERROR_CHECK(err);
}

void app_main(void)
{
    initialize_nvs(); // Initiate NVS which is used for the wifi
    wifi_init();      // Initiate wifi

    if (ESP_OK == wifi_connect_sta(SSID, PASS, 5000)) // This function is not needed
        printf("connected to the WIFI \n");
    else
        printf("Not connected to the WIFI \n");

    /* Set the GPIO as a push/pull output */

    printf("HELLO WORLD \n");

    vTaskDelay(500 / portTICK_PERIOD_MS);

    gpio_reset_pin(HIGH_GPIO);

    gpio_set_direction(HIGH_GPIO, GPIO_MODE_OUTPUT);

    gpio_set_level(HIGH_GPIO, SET_GPIO);
}
