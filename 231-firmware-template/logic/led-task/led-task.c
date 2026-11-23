#include "led-task.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "platform/platform.h"
#include "systime/systime.h"

led_state_t led_state = LED_STATE_OFF;
static uint64_t last_toggle_us = 0;
uint32_t led_period_ms = 1000;

bool LED_ON = false;

void led_task_init(void) {
  // настраиваем вывод светодиода, период 1000 мс, состояние «мигает»

  led_period_ms = 1000;
  led_task_set_state(LED_STATE_BLINK);
}

void led_task_next_state() {
  switch (led_state) {
    case LED_STATE_OFF:
      led_task_set_state(LED_STATE_ON);
      break;
    case LED_STATE_ON:
      led_task_set_state(LED_STATE_BLINK);
      break;
    case LED_STATE_BLINK:
      led_task_set_state(LED_STATE_OFF);
      break;
  }
}

void led_task_set_state(led_state_t state) {
  // запоминаем состояние и выполняем действие при входе:
  // «выключен» — гасим, «горит» — зажигаем, «мигает» — первое переключение
  // сразу
  led_state = state;
  led_task_handle();
}

void led_task_handle(void) {
  switch (led_state) {
    // «выключен» и «горит» — ничего не делаем, светодиод установлен при входе
    // «мигает» — каждые полпериода переключаем светодиод
    case LED_STATE_OFF:
      LED_ON = false;
      platform_led_set(LED_ON);
      break;
    case LED_STATE_ON:
      LED_ON = true;
      platform_led_set(LED_ON);
      break;
    case LED_STATE_BLINK:
      if (systime_period_elapsed(&last_toggle_us, led_period_ms * 1000)) {
        LED_ON = !LED_ON;
        platform_led_set(LED_ON);
      }
  }
}

led_state_t led_task_get_state(void) { return led_state; }

// false — период не принят
bool led_task_set_period_ms(uint32_t period_ms) {
  if (period_ms <= 0) {
    return false;
  }
  led_period_ms = period_ms;
  return true;
}
uint32_t led_task_get_period_ms(void) { return led_period_ms; }

void print_led_state(void) {
  printf("led: %s, period %u ms\n",
         led_state == LED_STATE_OFF  ? "off"
         : led_state == LED_STATE_ON ? "on"
                                     : "blink",
         (unsigned)led_period_ms);
}
