/*
 * @author:- Saurabh kadam
 * @date:- 19/06/2023
 * @brief:- This file is main function file for the WIFI config functionality.
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define HIGH_GPIO GPIO_NUM_21
#define SET_GPIO 1

void app_main(void)
{
    /* Set the GPIO as a push/pull output */

    printf("HELLO WORLD \n");

    vTaskDelay(500 / portTICK_PERIOD_MS);

    gpio_reset_pin(HIGH_GPIO);

    gpio_set_direction(HIGH_GPIO, GPIO_MODE_OUTPUT);

    gpio_set_level(HIGH_GPIO, SET_GPIO);
}
