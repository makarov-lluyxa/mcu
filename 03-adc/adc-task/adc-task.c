#include "adc-task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define ADC_GPIO_PIN  26
#define ADC_CHANNEL   0

#define TEMP_SENSOR_CHANNEL 4

#define ADC_REF_VOLTAGE 3.3f
#define ADC_RESOLUTION  (1 << 12)

#define ADC_TASK_MEAS_PERIOD_US 100000

static adc_task_state_t adc_state = ADC_TASK_STATE_IDLE;
static uint64_t next_measurement_us = 0;

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

void adc_task_set_state(adc_task_state_t state) {
    adc_state = state;
    if (state == ADC_TASK_STATE_RUN) {
        next_measurement_us = time_us_64();
    }
}

void adc_task_handle(void) {
    if (adc_state != ADC_TASK_STATE_RUN) {
        return;
    }

    uint64_t now = time_us_64();
    if (now >= next_measurement_us) {
        float voltage = adc_task_read_voltage();
        float temp = adc_task_read_temperature();

        printf("%f %f\n", voltage, temp);

        next_measurement_us = now + ADC_TASK_MEAS_PERIOD_US;
    }
}