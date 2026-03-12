#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    ADC_TASK_STATE_IDLE = 0,
    ADC_TASK_STATE_RUN = 1,
} adc_task_state_t;

void adc_task_init(void);

float adc_task_read_voltage(void);

float adc_task_read_temperature(void);

void adc_task_set_state(adc_task_state_t state);

void adc_task_handle(void);