#include "platform/platform.h"

#include "hardware/gpio.h"

int LED_PIN = 25;

static const uint8_t BUTTON_PINS[BUTTON_COUNT] = {
    [BUTTON_1] = 15,
    [BUTTON_2] = 14,
};

void platform_init(void) {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  for (int i = 0; i < BUTTON_COUNT; i++) {
    gpio_init(BUTTON_PINS[i]);
    gpio_set_dir(BUTTON_PINS[i], GPIO_IN);
    gpio_pull_up(BUTTON_PINS[i]);
  }
}

void platform_led_set(bool on) { gpio_put(LED_PIN, on); }

// нажата ли кнопка сейчас, без подавления дребезга
bool platform_button_read(button_id btn) {
    return !gpio_get(BUTTON_PINS[btn]);
}