#include "platform/platform.h"

#include "hardware/gpio.h"

int LED_PIN = 25;
uint8_t BUTTON_PIN = 15;

void platform_init(void) {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  gpio_init(BUTTON_PIN);
  gpio_set_dir(BUTTON_PIN, GPIO_IN);
  gpio_pull_up(BUTTON_PIN);
}

void platform_led_set(bool on){
  gpio_put(LED_PIN, on);
}

// нажата ли кнопка сейчас, без подавления дребезга
bool platform_button_read(void){
  return !gpio_get(BUTTON_PIN);
}