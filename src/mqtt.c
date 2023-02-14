#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "mqtt.h"
#include "cJson.h"
#include "led.h"

#define MQTT_LABEL "MQTT"
#define MQTT_KEY CONFIG_ESP_MQTT_KEY
#define MQTT_URI CONFIG_ESP_MQTT_URI

extern xSemaphoreHandle mqttSemaphore;
esp_mqtt_client_handle_t client;

int mqtt_subscribe(char *topic);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
void mqtt_event_data_handler(char *data);

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(MQTT_LABEL, "MQTT_EVENT_CONNECTED");
            esp_err_t err = mqtt_subscribe("v1/devices/me/rpc/request/+");

            if (err == -1)
                ESP_LOGE(MQTT_LABEL, "Error while subscribing to topic: %d", err);
            else
                ESP_LOGI(MQTT_LABEL, "Subscription success");

            xSemaphoreGive(mqttSemaphore);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(MQTT_LABEL, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(MQTT_LABEL, "MQTT_EVENT_DATA");
            ESP_LOGI(MQTT_LABEL, "Got from topic %.*s the data: %.*s", event->topic_len, event->topic, event->data_len, event->data);
            mqtt_event_data_handler(event->data);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(MQTT_LABEL, "MQTT_EVENT_ERROR");
            break;

        default:
            ESP_LOGI(MQTT_LABEL, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(MQTT_LABEL, "Event dispatched from event loop base %s, event_id %d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

void mqtt_event_data_handler(char *data)
{
    cJSON *json = cJSON_Parse(data);
    if (json == NULL) {
        ESP_LOGE(MQTT_LABEL, "Error while trying to parse received data");
        return;
    }

    char * json_key = cJSON_GetObjectItem(json, "method")->valuestring;
    int json_value = cJSON_GetObjectItem(json, "params")->valueint;

    if(strstr(json_key, "setValue") != NULL){
        led_set_intensity(json_value);
        ESP_LOGI(MQTT_LABEL, "Received value for led intensity: %d", json_value);
    }

    cJSON_Delete(json);
}

void mqtt_start() {
    esp_mqtt_client_config_t mqtt_config = {
        .uri = MQTT_URI,
        .username = MQTT_KEY
    };
    client = esp_mqtt_client_init(&mqtt_config);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

void mqtt_publish(char *topic, char *message)
{
    int message_id = esp_mqtt_client_publish(client, topic, message, 0, 1, 0);
    ESP_LOGI(MQTT_LABEL, "Published message %d to topic: %s", message_id, topic);
}

int mqtt_subscribe(char *topic)
{
    int message_id = esp_mqtt_client_subscribe(client, topic, 0);
    return message_id;
}
