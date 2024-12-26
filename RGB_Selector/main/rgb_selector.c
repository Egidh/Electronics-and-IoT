#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#include "esp_adc/adc_oneshot.h"

#define LEDR_PIN 23
#define LEDG_PIN 22
#define LEDB_PIN 21

#define POTR ADC_CHANNEL_6 // gpio 34
#define POTG ADC_CHANNEL_7 // gpio 35
#define POTB ADC_CHANNEL_4 // gpio 32

typedef struct Color
{
    int r;
    int g;
    int b;
} Color;

adc_oneshot_unit_handle_t setup()
{
    printf("Setting up..\n");

    // Setting up the leds
    gpio_set_direction(LEDR_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDG_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDB_PIN, GPIO_MODE_OUTPUT);

    // setting up the ledc timer
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer_config);

    // Setting up the red led
    ledc_channel_config_t red_led_config = {
        .gpio_num = LEDR_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&red_led_config);

    // Setting up the green led
    ledc_channel_config_t green_led_config = {
        .gpio_num = LEDG_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&green_led_config);

    // Setting up the blue led
    ledc_channel_config_t blue_led_config = {
        .gpio_num = LEDB_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_2,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&blue_led_config);

    // Setting up the ADC readings
    adc_oneshot_unit_init_cfg_t adc_config = {
        .ulp_mode = ADC_ULP_MODE_DISABLE,
        .unit_id = ADC_UNIT_1};

    adc_oneshot_unit_handle_t adc_handle;
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_config, &adc_handle));

    // Setting up the potentiometers
    adc_oneshot_chan_cfg_t pot_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12};

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, POTR, &pot_cfg));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, POTG, &pot_cfg));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, POTB, &pot_cfg));

    return adc_handle;
}

void Led_setColor(Color color)
{
    printf("Setting color: R=%d, G=%d, B=%d\n", 255 - color.r, 255 - color.g, 255 - color.b);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, color.r);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, color.g);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, color.b);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
}

void get_color(Color *color, adc_oneshot_unit_handle_t handle)
{
    int raw_r = 0, raw_g = 0, raw_b = 0;

    // Reading the values
    adc_oneshot_read(handle, POTR, &raw_r);
    adc_oneshot_read(handle, POTG, &raw_g);
    adc_oneshot_read(handle, POTB, &raw_b);

    // Convert 12 bits to 8 bits
    color->r = raw_r * 255 / 4095;
    color->g = raw_g * 255 / 4095;
    color->b = raw_b * 255 / 4095;

    // Inverting the value since I use a common cathode LED
    color->r = 255 - color->r;
    color->g = 255 - color->g;
    color->b = 255 - color->b;
}

void app_main(void)
{
    Color color = {0, 0, 0};
    adc_oneshot_unit_handle_t adc_handle = setup();

    while (true)
    {
        get_color(&color, adc_handle);
        Led_setColor(color);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
