#include "sensor-task.h"
#include "bme280-driver.h"
#include "pico/stdlib.h"

static bool is_running = false;
static uint32_t period_ms = 1000; // по умолчанию 1 секунда
static uint64_t next_measure_us = 0;

// Текущие значения
static float current_temp = 0, current_pres = 0, current_hum = 0;

// История
static sensor_history_t history = {0};
static uint32_t start_time = 0; // время старта в секундах (относительное)

void sensor_task_init(void) {
    // Инициализация BME280 уже должна быть выполнена в main
    start_time = to_ms_since_boot(get_absolute_time()) / 1000;
    next_measure_us = time_us_64();
}

void sensor_task_start(void) {
    is_running = true;
    next_measure_us = time_us_64(); // сброс таймера
}

void sensor_task_stop(void) {
    is_running = false;
}

void sensor_task_set_period(uint32_t period_ms) {
    period_ms = period_ms;
}

static void take_measurement(void) {
    current_temp = bme280_read_temperature(); // предположим, такие функции есть
    current_pres = bme280_read_pressure();
    current_hum = bme280_read_humidity();

    // Добавляем в историю
    uint32_t now_sec = to_ms_since_boot(get_absolute_time()) / 1000 - start_time;
    history.timestamps[history.head] = now_sec;
    history.temperature[history.head] = current_temp;
    history.pressure[history.head] = current_pres;
    history.humidity[history.head] = current_hum;
    history.head = (history.head + 1) % HISTORY_SIZE;
}

void sensor_task_handle(void) {
    if (!is_running) return;

    uint64_t now = time_us_64();
    if (now >= next_measure_us) {
        take_measurement();
        next_measure_us = now + period_ms * 1000;
    }
}

// Геттеры
float sensor_task_get_temperature(void) { return current_temp; }
float sensor_task_get_pressure(void) { return current_pres; }
float sensor_task_get_humidity(void) { return current_hum; }
const sensor_history_t* sensor_task_get_history(void) { return &history; }