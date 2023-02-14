#include <stdio.h>
#include "esp_event.h"
#include "freertos/semphr.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "wifi.h"
#include "mqtt.h"
#include "led.h"

#include "hall_manager.h"
#include "dht11_manager.h"

xSemaphoreHandle wifiSemaphore;
xSemaphoreHandle mqttSemaphore;

void init_systems()
{
    wifi_start_up();
    if (xSemaphoreTake(wifiSemaphore, portMAX_DELAY)) {
        mqtt_start();
        xSemaphoreGive(wifiSemaphore);
    }

    if (xSemaphoreTake(mqttSemaphore, portMAX_DELAY)) {
        dht11_start_up();
        hall_start_up();
        led_start_up();
    }
}

void app_main()
{
    mqttSemaphore = xSemaphoreCreateBinary();
    wifiSemaphore = xSemaphoreCreateBinary();

    nvs_flash_init();
    init_systems();
}