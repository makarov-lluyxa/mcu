#include <stdio.h>
#include "pico/stdlib.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task/protocol-task.h"
#include "led-task/led-task.h"

#define DEVICE_NAME "Pico Control"
#define DEVICE_VRSN "1.0"

void version_callback(const char* args) {
    printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}

void led_on_callback(const char* args) {
    led_task_state_set(LED_STATE_ON);
    printf("LED ON\n");
}

void led_off_callback(const char* args) {
    led_task_state_set(LED_STATE_OFF);
    printf("LED OFF\n");
}

void led_blink_callback(const char* args) {
    led_task_state_set(LED_STATE_BLINK);
    printf("LED BLINK\n");
}

api_t device_api[] = {
    {"version", version_callback, "get device name and firmware version"},
    {"on",      led_on_callback,  "turn on LED"},
    {"off",     led_off_callback, "turn off LED"},
    {"blink",   led_blink_callback, "make LED blink"},
    {NULL, NULL, NULL}
};

int main() {
    stdio_init_all();
    stdio_task_init();
    protocol_task_init(device_api);
    led_task_init();

    sleep_ms(1000);

    printf("\n--- Pico Control Terminal ---\n");
    printf("Available commands:\n");
    printf("  version - get device info\n");
    printf("  on      - turn LED on\n");
    printf("  off     - turn LED off\n");
    printf("  blink   - make LED blink\n");
    printf("Type a command and press Enter:\n");

    while (true) {
        char* cmd = stdio_task_handle();

        if (cmd != NULL) {
            protocol_task_handle(cmd);
        }

        led_task_handle();

    }

    return 0;
}