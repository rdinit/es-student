#include "led-task.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>

led_state_t led_state = LED_STATE_OFF;
uint64_t last_toggle_us = 0;
uint32_t led_period_ms = 1000;
int LED_PIN = 25;

bool period_elapsed(uint64_t *last_us, uint64_t period_us)
{
    uint64_t now_us = time_us_64();

    if (now_us - *last_us < period_us) {
        return false;
    }

    // Следующее срабатывание отсчитывается от прошлого, а не от «сейчас»
    *last_us += period_us;

    // Отстали больше чем на период: не догонять, а начать отсчёт заново
    if (now_us - *last_us >= period_us) {
        *last_us = now_us;
    }

    return true;
}

void led_task_init(void) {
  // настраиваем вывод светодиода, период 1000 мс, состояние «мигает»
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  led_period_ms = 1000;
  led_task_set_state(LED_STATE_BLINK);
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
      gpio_put(LED_PIN, false);
      break;
    case LED_STATE_ON:
      gpio_put(LED_PIN, true);
      break;
    case LED_STATE_BLINK:
      if (period_elapsed(&last_toggle_us, led_period_ms * 1000)) {
        gpio_put(LED_PIN, !gpio_get(LED_PIN));
      }
  }
}

led_state_t led_task_get_state(void) {
    return led_state;
}

// false — период не принят
bool led_task_set_period_ms(uint32_t period_ms) {
    if (period_ms <= 0) {
        return false;
    }
    led_period_ms = period_ms;
    return true;
}
uint32_t led_task_get_period_ms(void) {
    return led_period_ms;
}

void print_led_state(void) {
    printf("led: %s, period %u ms\n", led_state == LED_STATE_OFF ? "off" : led_state == LED_STATE_ON ? "on" : "blink", led_period_ms);
}