#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "cJson.h"

#include "mqtt.h"

#include "hall_manager.h"

#define HALL_PIN GPIO_NUM_4
#define HALL_LABEL "MAG_HALL"
#define HALL_MAX_SAMPLES 10

static int samples_count = 0;
static int samples_high = 0;
static int filtered_sensor_data = 0;

static int data_is_ready()
{
    return samples_count == 0;
}

static void refresh_data()
{
    int sensor_data = gpio_get_level(HALL_PIN);

    if (sensor_data)
        samples_high++;

    samples_count++;
    if (samples_count >= HALL_MAX_SAMPLES) {
        filtered_sensor_data = (samples_high > 0);
        samples_count = 0;
        samples_high = 0;
        // ESP_LOGI(HALL_LABEL, "Magnect field: %d", filtered_sensor_data);
    }
}

static void push_data()
{
    if (!data_is_ready())
        return;

    cJSON * json = cJSON_CreateObject();

    cJSON_AddItemToObject(json, "hall", cJSON_CreateNumber(filtered_sensor_data));
    mqtt_publish("v1/devices/me/telemetry", cJSON_Print(json));

    cJSON_Delete(json);
}

static void main_task(void * params)
{
    while (true)
    {
        refresh_data();
        push_data();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void hall_start_up()
{
    gpio_pad_select_gpio(HALL_PIN);
    gpio_set_direction(HALL_PIN, GPIO_MODE_INPUT);

    xTaskCreate(main_task, "Hall sensor task", 4096, NULL, 1, NULL);

    ESP_LOGI(HALL_LABEL, "Hall initialized");
}