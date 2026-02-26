#include <stdio.h>
#include "pico/stdlib.h"

#ifndef LED_PIN
#define LED_PIN PICO_DEFAULT_LED_PIN
#endif

int main() {

    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);

    sleep_ms(1000);

    printf("\n--- Pico USB Terminal Ready ---\n");
    printf("Type any character and press Enter:\n");

    while (true) {
        char symbol = getchar();

        printf("received char: '%c' [ ASCII code: %d ]\n", symbol, symbol);

        switch(symbol) {
            case 'e':
	            gpio_put(LED_PIN, true);
	            printf("led enable done\n");
	            break;

            case 'd':
                gpio_put(LED_PIN, false);
                printf("led disable done\n");
                break;
    
            case 'v':
                printf("Device: Pico USB Echo\nVersion: 1.0\n");
                break;

            default:
                break;
        }
    }

    return 0;
}