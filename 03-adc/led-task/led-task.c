#include "led-task.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;

uint LED_BLINK_PERIOD_US = 500000;

static uint64_t led_ts;
static led_state_t led_state;

void led_task_init(void) {
    led_state = LED_STATE_OFF;
    led_ts = 0;

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);
}

void led_task_state_set(led_state_t state) {
    led_state = state;
}

void led_task_handle(void) {
    switch (led_state) {
        case LED_STATE_OFF:
            gpio_put(LED_PIN, 0);
            break;

        case LED_STATE_ON:
            gpio_put(LED_PIN, 1);
            break;

        case LED_STATE_BLINK:
            if (time_us_64() > led_ts) {
                gpio_put(LED_PIN, !gpio_get(LED_PIN));
                led_ts = time_us_64() + LED_BLINK_PERIOD_US;
            }
            break;

        default:
            break;
    }
}