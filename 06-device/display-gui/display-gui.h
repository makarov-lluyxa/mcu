#pragma once

#include "ili9341-display.h"
#include "sensor-task.h"

void display_gui_init(ili9341_display_t* disp);
void display_gui_update(void); // вызывать в главном цикле