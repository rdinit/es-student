#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"


const uint LED_PIN = 25;

// добавляем заголовочный файл функций ввода-вывода
// добавляем заголовочный файл функций работы с GPIO

// объявляем константу вывода светодиода
const uint BUTTON_PIN = 15;


const uint DEBOUNCE_MS = 20;

bool get_button_debounce(uint pin)
{
    bool state = gpio_get(pin);
    sleep_ms(DEBOUNCE_MS);
    return state && gpio_get(pin);
}

void set_led(bool on)
{
    gpio_put(LED_PIN, on);
    printf("led %s\n", on ? "on" : "off");
}

int main()
{
    // инициализируем пин светодиода
    // настраиваем пин светодиода на выход
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    bool led = false;
    bool previous = false;

    while (1)
    {
        bool current = get_button_debounce(BUTTON_PIN);;

        if (previous == true && current == false)
        {
            led = !led;
            set_led(led);
        }

        previous = current;
    }
}