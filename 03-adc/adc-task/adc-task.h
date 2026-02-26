#pragma once

#include <stdint.h>

void adc_task_init(void);

float adc_task_read_voltage(void);

float adc_task_read_temperature(void);