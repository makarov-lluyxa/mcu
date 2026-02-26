#include "adc-task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

// GPIO 26 соответствует каналу ADC0
#define ADC_GPIO_PIN  26
#define ADC_CHANNEL   0

// Канал для встроенного датчика температуры
#define TEMP_SENSOR_CHANNEL 4

// Опорное напряжение и разрядность
#define ADC_REF_VOLTAGE 3.3f
#define ADC_RESOLUTION  (1 << 12)  // 4096

void adc_task_init(void) {
    adc_init();
    adc_gpio_init(ADC_GPIO_PIN);
    adc_set_temp_sensor_enabled(true);
}

float adc_task_read_voltage(void) {
    adc_select_input(ADC_CHANNEL);
    uint16_t raw = adc_read();
    float voltage = raw * ADC_REF_VOLTAGE / ADC_RESOLUTION;
    return voltage;
}

float adc_task_read_temperature(void) {
    adc_select_input(TEMP_SENSOR_CHANNEL);
    uint16_t raw = adc_read();
    float voltage = raw * ADC_REF_VOLTAGE / ADC_RESOLUTION;
    float temp_c = 27.0f - (voltage - 0.706f) / 0.001721f;
    return temp_c;
}