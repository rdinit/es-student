#include "button-task.h"

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

static button_state_t button_state = RELEASED;
static uint64_t last_toggle_us = 0;
uint32_t button_press_count = 0;

static void (*next_state)(void) = 0;

const uint64_t DEBOUNCE_US = 20000;

void button_task_init(void (*next_state_)(void)) {
  next_state = next_state_;
}

void button_task_handle(void) {
  // читаем уровень: нажатая кнопка замыкает вывод на землю и даёт ноль
  // читаем часы
  uint32_t button_input = platform_button_read();
  uint64_t now_us = systime_us();
  switch (button_state) {
    // переходы по таблице; при переходе в «нажата»
    // увеличиваем счётчик и вызываем led_task_next_state()
    case RELEASED:
      if (button_input == 1) {
        button_state = PREPRESSED;
        last_toggle_us = now_us;
      }
      break;
    case PREPRESSED:
      if (now_us - last_toggle_us >= DEBOUNCE_US) {
        if (button_input == 1) {
          button_press_count++;
          next_state();
          button_state = PRESSED;
        } else {
          button_state = RELEASED;
        }
      }
      break;
    case PRESSED:
      if (button_input == 0) {
        button_state = PRERELEASED;
        last_toggle_us = now_us;
      }
      break;
    case PRERELEASED:
      if (now_us - last_toggle_us >= DEBOUNCE_US) {
        if (button_input == 0) {
          button_state = RELEASED;
        } else {
          button_state = PRESSED;
        }
      }
  }
}

bool button_task_is_pressed() { return button_state == PRESSED; }

uint32_t button_task_get_press_count() { return button_press_count; }