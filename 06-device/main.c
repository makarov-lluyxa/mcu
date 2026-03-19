#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task.h"
#include "led-task/led-task.h"
#include "bme280-driver.h"
#include "ili9341-driver.h"
#include "ili9341-display.h"
#include "ili9341-font.h"
#include "sensor-task.h"
#include "display-gui.h"

#define DEVICE_NAME "AtmoMonitor"
#define DEVICE_VRSN "1.0"

// ---------- Пины и константы ----------
#define BME280_I2C_ADDR 0x76

#define ILI9341_PIN_MISO  4
#define ILI9341_PIN_CS    10
#define ILI9341_PIN_SCK   6
#define ILI9341_PIN_MOSI  7
#define ILI9341_PIN_DC    8
#define ILI9341_PIN_RESET 9

// ---------- HAL для BME280 (I2C) ----------
void rp2040_i2c_read(uint8_t* buffer, uint16_t length) {
    i2c_read_timeout_us(i2c1, BME280_I2C_ADDR, buffer, length, false, 100000);
}

void rp2040_i2c_write(uint8_t* data, uint16_t size) {
    i2c_write_timeout_us(i2c1, BME280_I2C_ADDR, data, size, false, 100000);
}

// ---------- HAL для ILI9341 (SPI) ----------
static ili9341_display_t ili9341_display = {0};
static ili9341_hal_t ili9341_hal = {0};

void rp2040_spi_write(const uint8_t *data, uint32_t size) {
    spi_write_blocking(spi0, data, size);
}

void rp2040_spi_read(uint8_t *buffer, uint32_t length) {
    spi_read_blocking(spi0, 0, buffer, length);
}

void rp2040_gpio_cs_write(bool level) {
    gpio_put(ILI9341_PIN_CS, level);
}

void rp2040_gpio_dc_write(bool level) {
    gpio_put(ILI9341_PIN_DC, level);
}

void rp2040_gpio_reset_write(bool level) {
    gpio_put(ILI9341_PIN_RESET, level);
}

void rp2040_delay_ms(uint32_t ms) {
    sleep_ms(ms);
}

void ili9341_rp2040_init(void) {
    spi_init(spi0, 62500000);
    gpio_set_function(ILI9341_PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(ILI9341_PIN_CS);
    gpio_init(ILI9341_PIN_DC);
    gpio_init(ILI9341_PIN_RESET);
    gpio_set_dir(ILI9341_PIN_CS, GPIO_OUT);
    gpio_set_dir(ILI9341_PIN_DC, GPIO_OUT);
    gpio_set_dir(ILI9341_PIN_RESET, GPIO_OUT);
    gpio_put(ILI9341_PIN_CS, 1);
    gpio_put(ILI9341_PIN_DC, 0);
    gpio_put(ILI9341_PIN_RESET, 0);

    ili9341_hal.spi_write = rp2040_spi_write;
    ili9341_hal.spi_read = rp2040_spi_read;
    ili9341_hal.gpio_cs_write = rp2040_gpio_cs_write;
    ili9341_hal.gpio_dc_write = rp2040_gpio_dc_write;
    ili9341_hal.gpio_reset_write = rp2040_gpio_reset_write;
    ili9341_hal.delay_ms = rp2040_delay_ms;
}

// ---------- Прототипы команд API ----------
void version_callback(const char* args);
void help_callback(const char* args);
void led_on_callback(const char* args);
void led_off_callback(const char* args);
void led_blink_callback(const char* args);
void start_callback(const char* args);
void stop_callback(const char* args);
void period_callback(const char* args);
void disp_screen_callback(const char *args);
void disp_px_callback(const char *args);
void disp_line_callback(const char *args);
void disp_rect_callback(const char *args);
void disp_frect_callback(const char *args);
void disp_text_callback(const char *args);

// ---------- Массив команд API ----------
api_t device_api[] = {
    {"version",   version_callback,   "get device name and firmware version"},
    {"help",      help_callback,      "show this help"},
    {"on",        led_on_callback,    "turn on LED"},
    {"off",       led_off_callback,   "turn off LED"},
    {"blink",     led_blink_callback, "make LED blink"},
    {"start",     start_callback,     "start periodic measurements"},
    {"stop",      stop_callback,      "stop measurements"},
    {"period",    period_callback,    "set measurement period in ms"},
    {"disp_screen", disp_screen_callback, "fill screen with color (hex RGB888)"},
    {"disp_px",     disp_px_callback,     "draw pixel: x y [color]"},
    {"disp_line",   disp_line_callback,   "draw line: x0 y0 x1 y1 color"},
    {"disp_rect",   disp_rect_callback,   "draw rect outline: x y w h color"},
    {"disp_frect",  disp_frect_callback,  "draw filled rect: x y w h color"},
    {"disp_text",   disp_text_callback,   "draw text: x y text color bg"},
    {NULL, NULL, NULL}
};

// ---------- Реализации команд ----------
void version_callback(const char* args) {
    printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}

void help_callback(const char* args) {
    printf("Available commands:\n");
    for (int i = 0; device_api[i].command_name != NULL; i++) {
        printf("  %s - %s\n", device_api[i].command_name, device_api[i].command_help);
    }
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

void start_callback(const char* args) {
    sensor_task_start();
    printf("Measurements started\n");
}

void stop_callback(const char* args) {
    sensor_task_stop();
    printf("Measurements stopped\n");
}

void period_callback(const char* args) {
    uint32_t p;
    if (sscanf(args, "%lu", &p) == 1) {
        sensor_task_set_period(p);
        printf("Period set to %lu ms\n", p);
    } else {
        printf("Usage: period <ms>\n");
    }
}

// Команды дисплея (из 05-display)
void disp_screen_callback(const char *args) {
    uint32_t c = 0;
    int result = sscanf(args, "%x", &c);
    uint16_t color = COLOR_BLACK;
    if (result == 1) {
        color = RGB888_2_RGB565(c);
    }
    ili9341_fill_screen(&ili9341_display, color);
}

void disp_px_callback(const char *args) {
    int x = 0, y = 0;
    uint32_t c = 0;
    int result = sscanf(args, "%d %d %x", &x, &y, &c);
    if (result < 2) return;
    uint16_t color = (result == 3) ? RGB888_2_RGB565(c) : COLOR_WHITE;
    ili9341_draw_pixel(&ili9341_display, x, y, color);
}

void disp_line_callback(const char *args) {
    int x0, y0, x1, y1;
    uint32_t c;
    if (sscanf(args, "%d %d %d %d %x", &x0, &y0, &x1, &y1, &c) != 5) return;
    uint16_t color = RGB888_2_RGB565(c);
    ili9341_draw_line(&ili9341_display, x0, y0, x1, y1, color);
}

void disp_rect_callback(const char *args) {
    int x, y, w, h;
    uint32_t c;
    if (sscanf(args, "%d %d %d %d %x", &x, &y, &w, &h, &c) != 5) return;
    uint16_t color = RGB888_2_RGB565(c);
    ili9341_draw_rect(&ili9341_display, x, y, w, h, color);
}

void disp_frect_callback(const char *args) {
    int x, y, w, h;
    uint32_t c;
    if (sscanf(args, "%d %d %d %d %x", &x, &y, &w, &h, &c) != 5) return;
    uint16_t color = RGB888_2_RGB565(c);
    ili9341_draw_filled_rect(&ili9341_display, x, y, w, h, color);
}

void disp_text_callback(const char *args) {
    int x, y;
    char text[64];
    uint32_t c, bg;
    if (sscanf(args, "%d %d %63s %x %x", &x, &y, text, &c, &bg) != 5) return;
    uint16_t color = RGB888_2_RGB565(c);
    uint16_t bgcolor = RGB888_2_RGB565(bg);
    ili9341_draw_text(&ili9341_display, x, y, text, &jetbrains_font, color, bgcolor);
}

// ---------- main ----------
int main() {
    stdio_init_all();
    stdio_task_init();
    protocol_task_init(device_api);
    led_task_init();

    // Инициализация I2C для BME280
    i2c_init(i2c1, 100000);
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    bme280_init(rp2040_i2c_read, rp2040_i2c_write);

    // Инициализация SPI и дисплея
    ili9341_rp2040_init();
    ili9341_init(&ili9341_display, &ili9341_hal);
    ili9341_set_rotation(&ili9341_display, ILI9341_ROTATION_90);

    // Тестовая отрисовка (как в 05-display)
    ili9341_fill_screen(&ili9341_display, COLOR_BLACK);
    sleep_ms(300);
    ili9341_draw_filled_rect(&ili9341_display, 10, 10, 100, 60, COLOR_RED);
    ili9341_draw_filled_rect(&ili9341_display, 120, 10, 100, 60, COLOR_GREEN);
    ili9341_draw_filled_rect(&ili9341_display, 230, 10, 80, 60, COLOR_BLUE);
    ili9341_draw_rect(&ili9341_display, 10, 90, 300, 80, COLOR_WHITE);
    ili9341_draw_line(&ili9341_display, 0, 0, 319, 239, COLOR_YELLOW);
    ili9341_draw_line(&ili9341_display, 319, 0, 0, 239, COLOR_CYAN);
    ili9341_draw_text(&ili9341_display, 20, 100, "Hello, ILI9341!", &jetbrains_font, COLOR_WHITE, COLOR_BLACK);
    ili9341_draw_text(&ili9341_display, 20, 116, "RP2040 / Pico SDK", &jetbrains_font, COLOR_YELLOW, COLOR_BLACK);

    // Инициализация модулей
    sensor_task_init();
    display_gui_init(&ili9341_display);

    // Запуск измерений по умолчанию
    sensor_task_start();

    sleep_ms(1000);
    printf("\n--- Atmospheric Monitor ---\n");
    printf("Type 'help' for commands.\n");

    while (true) {
        char* cmd = stdio_task_handle();
        if (cmd != NULL) {
            protocol_task_handle(cmd);
        }

        led_task_handle();
        sensor_task_handle();   // обновление данных датчика
        display_gui_update();   // обновление экрана
    }

    return 0;
}