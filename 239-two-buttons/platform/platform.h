#pragma once

#include <stdbool.h>

typedef enum {
  BUTTON_1,
  BUTTON_2,
  BUTTON_COUNT 
} button_id;

// запускает стандартный ввод-вывод и настраивает выводы
void platform_init(void);

void platform_led_set(bool on);

// нажата ли кнопка сейчас, без подавления дребезга
bool platform_button_read(button_id btn);