#include <stdio.h>
#include <string.h>
#include "api.h"
#include "device.h"
#include "log.h"
#include "pico/stdlib.h"
#include "profiling.h"
#include "stdio-text-protocol.h"
#include "led-task.h"


int main() {
  // инициализируем пин светодиода
  // настраиваем пин светодиода на выход
  led_task_init();
  profiling_init();
  stdio_text_protocol_init();

  

  while (1) {
    profiling_iteration();
    led_task_handle();
    const command_t *command = stdio_text_protocol_handle();
    if (command != NULL) {
      api_handle(command);
    }
  }
}