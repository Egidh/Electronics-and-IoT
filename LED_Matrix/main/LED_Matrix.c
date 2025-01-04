#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "rom/ets_sys.h"

#include "esp_task_wdt.h"

#define DATA 13  // GPIO connected to the SER pin
#define CLOCK 14 // GPIO connected to the SRCLK pin
#define LATCH 15 // GPIO connected to the RCLK pin

// Not used here
#define ERR_BAD_GPIO -1
#define ERR_BAD_DURATION -2

/// @brief Set a pulse on a GPIO
/// @param gpio The GPIO
/// @param duration The duration in us while the GPIO will be high
/// @return 0 if OK, else ERR_BAD_GPIO or ERR_BAD_DURATION
int gpio_pulse(int gpio, int duration)
{
    if (!gpio)
        return ERR_BAD_GPIO;
    if (duration < 1)
        return ERR_BAD_DURATION;

    gpio_set_level(gpio, 1);
    ets_delay_us(duration); // The delay ensures that the component gets the data
    gpio_set_level(gpio, 0);

    return 0;
}

/// @brief Send a pulse to the LATCH pin
/// @return 0 if OK, else ERR_BAD_GPIO
static inline int matrix_Update()
{
    return gpio_pulse(LATCH, 1);
}

void setRegister(uint8_t col, uint8_t row)
{
    col = ~col; // I'm using a common anode LED matrix here

    // Setting up rows
    for (int i = 0; i < 8; i++)
    {
        gpio_set_level(DATA, (col >> i) & 1);

        gpio_pulse(CLOCK, 1);
    }

    // Setting up columns
    for (int i = 7; i >= 0; i--)
    {
        gpio_set_level(DATA, (row >> i) & 1);

        gpio_pulse(CLOCK, 1);
    }

    // Sending the data to the matrix
    matrix_Update();
}

// Do NOT use gpio_set_direction with the same pin I used
// I recommend you to use gpio_config as shown instead
void setup()
{
    gpio_config_t config = {
        .pin_bit_mask = (1ULL << DATA) | (1ULL << LATCH) | (1ULL << CLOCK),
        .mode = GPIO_MODE_OUTPUT,
    };

    gpio_config(&config);

    esp_task_wdt_config_t wdt = {
        .timeout_ms = 5000,
        .trigger_panic = true,
    };
    esp_task_wdt_reconfigure(&wdt); // Timeout set to 5 seconds, reset after 5 seconds of inactivity
}

// Frames for a Pacman ghost animation
uint8_t frame1[8] = {
    0b00111100,
    0b01111110,
    0b10011001,
    0b10011001,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11110101,
};

uint8_t frame2[8] = {
    0b00111100,
    0b01111110,
    0b10011001,
    0b10011001,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11101101,
};

uint8_t frame3[8] = {
    0b00111100,
    0b01111110,
    0b10011001,
    0b10011001,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11011011,
};

uint8_t frame4[8] = {
    0b00111100,
    0b01111110,
    0b10011001,
    0b10011001,
    0b11111111,
    0b11111111,
    0b11111111,
    0b10110111,
};

int currentFrame = 0;
TimerHandle_t xTimer = NULL;

// Each time the timer reaches the specified value, this
// function will be called, allowing the actual frame to change
void timerCallback(TimerHandle_t xTimer)
{
    currentFrame++;
    if (currentFrame > 3)
        currentFrame = 0;
}

// Setting up the timer for the callback function,
// feel free to modify the pdMS_TO_TICKS value as you prefer
void setup_timer()
{
    xTimer = xTimerCreate("MatrixRefreshTimer", pdMS_TO_TICKS(200), pdTRUE, (void *)0, timerCallback);
    if (xTimer != NULL)
    {
        xTimerStart(xTimer, 0);
    }
}

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void app_main(void)
{
    // Calling the setup functions
    setup();
    setup_timer();
    printf("%s\n", (esp_task_wdt_add(NULL) == ESP_OK) ? "OK" : "not OK"); // Add the WDT (WatchDog Timer) to the current task (here the main task)

    // This array groups all of the frames needed
    uint8_t *frames[] = {frame1, frame2, frame3, frame4};

    while (true)
    {
        // Setting up the frame to be displayed
        uint8_t *frame = frames[currentFrame];

        // Displaying the frame row by row
        for (int i = 0; i < 8; i++)
        {
            setRegister(frame[i], 1 << i);
            // Feed the WDT to avoid a reset
            esp_task_wdt_reset();
        }
    }
}
