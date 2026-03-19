#include "display-gui.h"
#include <stdio.h>
#include <string.h>
#include "pico/time.h"
#include "ili9341-display.h"
#include "ili9341-font.h"
#include "sensor-task.h"

// Определяем макрос RGB, если его нет в заголовках
#ifndef RGB
#define RGB(r,g,b) (((r)<<16) | ((g)<<8) | (b))
#endif

// Цвета интерфейса
#define COLOR_BG      RGB(0,0,0)
#define COLOR_TEMP    RGB(255,100,100)
#define COLOR_PRESS   RGB(100,255,100)
#define COLOR_HUM     RGB(100,100,255)

static ili9341_display_t* disp = NULL;
static uint32_t last_update = 0;

void display_gui_init(ili9341_display_t* display) {
    disp = display;
}

static void draw_values(void) {
    float t = sensor_task_get_temperature();
    float p = sensor_task_get_pressure();
    float h = sensor_task_get_humidity();

    char buf[32];

    snprintf(buf, sizeof(buf), "T: %.1f C", t);
    ili9341_draw_text(disp, 10, 30, buf, &jetbrains_font, COLOR_TEMP, COLOR_BG);

    snprintf(buf, sizeof(buf), "P: %.1f hPa", p / 100.0f);
    ili9341_draw_text(disp, 10, 50, buf, &jetbrains_font, COLOR_PRESS, COLOR_BG);

    snprintf(buf, sizeof(buf), "H: %.1f %%", h);
    ili9341_draw_text(disp, 10, 70, buf, &jetbrains_font, COLOR_HUM, COLOR_BG);
}

static void draw_bars(void) {
    float t = sensor_task_get_temperature();
    int bar_height = (int)((t + 40) * 100 / 125); // -40..85 -> 0..100
    if (bar_height < 0) bar_height = 0;
    if (bar_height > 100) bar_height = 100;
    ili9341_draw_filled_rect(disp, 200, 140 - bar_height, 30, bar_height, COLOR_TEMP);
}

void display_gui_update(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - last_update < 200) return; // обновление 5 раз/сек
    last_update = now;

    draw_values();
    draw_bars();
}