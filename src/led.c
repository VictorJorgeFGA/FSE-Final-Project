#include "freertos/FreeRTOS.h"
#include "driver/ledc.h"
#include "esp_log.h"

#include "led.h"

#define LED GPIO_NUM_2
#define LED_LABEL "LED"

void led_start_up() {
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_timer_config(&timer_config);
    ledc_channel_config_t channel_config = {
        .gpio_num = LED,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&channel_config);
}

int led_get_intensity() {
    return ledc_get_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void led_set_intensity(int value) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, (uint32_t) (((float) value) * 2.5f));

    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    ESP_LOGI(LED_LABEL, "Led intensity: to %d", value);
}