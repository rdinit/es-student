#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "platform/platform.h"

void button_task_init(void (*const handlers[BUTTON_COUNT])(void));

void button_task_handle(void);

bool button_task_is_pressed(button_id btn);

uint32_t button_task_get_press_count(button_id btn);