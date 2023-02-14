#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

#include "cJson.h"

#include "mqtt.h"
#include "dht11.h"
#include "dht11_manager.h"

#define DHT11_SENSOR GPIO_NUM_5
#define DHT11_LABEL "DHT11"
#define DHT11_MAX_SAMPLES 5

#define AVG     0
#define LAST    1

static xSemaphoreHandle avg_data_semphr;

static struct dht11_reading sensor_data;
static float avg_temp = 0;
static float avg_hum = 0;
static int samples = 0;

static void refresh_data()
{
    sensor_data = DHT11_read();
    if (sensor_data.status == DHT11_OK) {
        if (xSemaphoreTake(avg_data_semphr, 100 / portTICK_PERIOD_MS)) {
            samples++;

            avg_temp += sensor_data.temperature;
            avg_hum += sensor_data.humidity;

            xSemaphoreGive(avg_data_semphr);
        }

        ESP_LOGI(DHT11_LABEL, "Temp: %d, Hum: %d", sensor_data.temperature, sensor_data.humidity);
    }
    else {
        ESP_LOGE(DHT11_LABEL, "Error while getting sensor information.");
    }
}

static void push_data(int type)
{
    if (sensor_data.status != DHT11_OK)
        return;

    cJSON * json = cJSON_CreateObject();

    if (type == AVG) {
        if (xSemaphoreTake(avg_data_semphr, portMAX_DELAY)) {
            samples = samples <= 0 ? 1 : samples;

            cJSON_AddItemToObject(json, "avg_temperature", cJSON_CreateNumber(avg_temp / samples));
            cJSON_AddItemToObject(json, "avg_humidity", cJSON_CreateNumber(avg_hum / samples));

            avg_temp = 0.0f;
            avg_hum = 0.0f;
            samples = 0;

            xSemaphoreGive(avg_data_semphr);
        }
    }
    else {
        cJSON_AddItemToObject(json, "temperature", cJSON_CreateNumber(sensor_data.temperature));
        cJSON_AddItemToObject(json, "humidity", cJSON_CreateNumber(sensor_data.humidity));
    }

    mqtt_publish("v1/devices/me/telemetry", cJSON_Print(json));

    cJSON_Delete(json);
}

static void main_task(void * params)
{
    while (true) {
        refresh_data();
        push_data(LAST);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static void avg_task(void * params)
{
    while (true) {
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        push_data(AVG);
    }
}

void dht11_start_up()
{
    avg_data_semphr = xSemaphoreCreateBinary();
    xSemaphoreGive(avg_data_semphr);

    DHT11_init(DHT11_SENSOR);
    xTaskCreate(main_task, "DHT11 sensor task", 4096, NULL, 1, NULL);
    xTaskCreate(avg_task, "DHT11 avg task", 4096, NULL, 1, NULL);
}
