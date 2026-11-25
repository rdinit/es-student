#include "button-task/button-task.h"

#include <stdbool.h>
#include <stdint.h>

#include "platform/platform.h"
#include "systime/systime.h"

typedef enum {
  RELEASED,
  PREPRESSED,
  PRESSED,
  PRERELEASED,

} button_state_t;

static button_state_t button_state[BUTTON_COUNT] = {0};
static uint64_t last_toggle_us[BUTTON_COUNT] = {0};
static uint32_t button_press_count[BUTTON_COUNT] = {0};

// static button_state_t button_state = RELEASED;
// static uint64_t last_toggle_us = 0;
// uint32_t button_press_count = 0;

static void (*next_state)(button_id) = 0;

const uint64_t DEBOUNCE_US = 20000;

static void (*callbacks[BUTTON_COUNT])(void) = {0};

void button_task_init(void (*const handlers[BUTTON_COUNT])(void)) {
    for (int i = 0; i < BUTTON_COUNT; ++i) {
        callbacks[i] = handlers[i];
    }
}

void button_task_handle(void) {
  // читаем уровень: нажатая кнопка замыкает вывод на землю и даёт ноль
  // читаем часы

  uint64_t now_us = systime_us();
  for (int i = 0; i < BUTTON_COUNT; ++i) {
    uint32_t button_input = platform_button_read(i);
    switch (button_state[i]) {
      // переходы по таблице; при переходе в «нажата»
      // увеличиваем счётчик и вызываем led_task_next_state()
      case RELEASED:
        if (button_input == 1) {
          button_state[i] = PREPRESSED;
          last_toggle_us[i] = now_us;
        }
        break;
      case PREPRESSED:
        if (now_us - last_toggle_us[i] >= DEBOUNCE_US) {
          if (button_input == 1) {
            button_press_count[i]++;
            next_state(i);
            button_state[i] = PRESSED;
          } else {
            button_state[i] = RELEASED;
          }
        }
        break;
      case PRESSED:
        if (button_input == 0) {
          button_state[i] = PRERELEASED;
          last_toggle_us[i] = now_us;
        }
        break;
      case PRERELEASED:
        if (now_us - last_toggle_us[i] >= DEBOUNCE_US) {
          if (button_input == 0) {
            button_state[i] = RELEASED;
          } else {
            button_state[i] = PRESSED;
          }
        }
    }
  }
}

bool button_task_is_pressed(button_id btn) {
  return button_state[btn] == PRESSED;
}
uint32_t button_task_get_press_count(button_id btn) {
  return button_press_count[btn];
}