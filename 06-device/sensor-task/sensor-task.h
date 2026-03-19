#pragma once

#include <stdint.h>
#include <stdbool.h>

// Инициализация задачи датчика
void sensor_task_init(void);

// Запуск/остановка периодических измерений
void sensor_task_start(void);
void sensor_task_stop(void);

void sensor_task_handle(void);

// Установка периода измерений в миллисекундах
void sensor_task_set_period(uint32_t period_ms);

// Получение последних измеренных значений
float sensor_task_get_temperature(void);
float sensor_task_get_pressure(void);
float sensor_task_get_humidity(void);

// Доступ к истории измерений для графиков (размер HISTORY_SIZE)
#define HISTORY_SIZE 60
typedef struct {
    float temperature[HISTORY_SIZE];
    float pressure[HISTORY_SIZE];
    float humidity[HISTORY_SIZE];
    uint32_t timestamps[HISTORY_SIZE]; // секунды от старта
    uint8_t head; // индекс для следующей записи
} sensor_history_t;

const sensor_history_t* sensor_task_get_history(void);