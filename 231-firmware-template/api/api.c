
#include "api.h"

#include <ctype.h>
#include <stdlib.h>

#include "button-task.h"
#include "device/device.h"
#include "errno.h"
#include "hardware/clocks.h"
#include "led-task.h"
#include "logging/log.h"
#include "pi-task.h"
#include "pico/stdlib.h"
#include "profiling/profiling.h"
#include "stdio.h"
#include "string.h"
#include "firmware.h"
#include "systime/systime.h"

// Число без знака из десятичной записи. false — в записи не только цифры
// или число не помещается в uint32_t.
static bool parse_u32(const char* text, uint32_t* value) {
  // strtoul приняла бы и знак: из "-1" получилось бы 4294967295
  if (!isdigit((unsigned char)text[0])) {
    return false;
  }

  char* end;
  errno = 0;
  unsigned long number = strtoul(text, &end, 10);

  // после числа остались символы или число не поместилось
  if (*end != '\0' || errno == ERANGE || number > UINT32_MAX) {
    return false;
  }

  *value = (uint32_t)number;
  return true;
}

// обработчик команды: получает имя и аргументы
typedef void (*api_callback_t)(const command_t* command);

// команда прибора: имя, справка для help и обработчик
typedef struct {
  const char* name;
  const char* help;
  api_callback_t callback;
} api_command_t;

void cmd_pi_start(const command_t* command) {
  uint32_t terms = PI_TERMS_DEFAULT;

  if (command->argc > 1) {
    printf("error: usage pi_start [<terms>]\n");
    return;
  }
  if (command->argc == 1 && !parse_u32(command->argv[0], &terms)) {
    printf("error: can't parse uint32_t from '%s'\n", command->argv[0]);
    return;
  }
  if (!pi_task_start(terms)) {
    printf("error: pi is busy\n");
    return;
  }

  printf("pi: started, %u terms\n", (unsigned)terms);
}

void cmd_pi(const command_t* command) { print_pi_state(); }

void cmd_info(const command_t* command) {
  device_info_t info;
  device_get_info(&info);
  printf("name: %s\n", FIRMWARE_NAME);
  printf("version: %s\n", FIRMWARE_VERSION);
  printf("project: %s\n", FIRMWARE_PROJECT);
  printf("repo: %s\n", FIRMWARE_REPO);
  printf("board: %s\n", info.device_board);
  printf("serial: %s\n", info.board_id);
  printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\n",
         (unsigned)info.manufacturer, (unsigned)info.part,
         (unsigned)info.chip_revision);
  printf("pico-sdk: %s\n", info.sdk_version);
}

void cmd_uptime(const command_t* command) {
  printf("uptime: %llu ms\n", systime_ms);
}

void cmd_help(const command_t* command);

void cmd_led_period(const command_t* command) {
  uint32_t period_ms;

  if (command->argc != 1 || !parse_u32(command->argv[0], &period_ms)) {
    printf("error: usage led_period <period_ms>\n");
    return;
  }

  // годится ли такой период, решает задача светодиода
  if (!led_task_set_period_ms(period_ms)) {
    printf("error: period_ms must be greater than 0\n");
    return;
  }

  // ответ: состояние и период светодиода
  print_led_state();
}

void cmd_led_enable(const command_t* command) {
  led_task_set_state(LED_STATE_ON);
  print_led_state();
}
void cmd_led_disable(const command_t* command) {
  led_task_set_state(LED_STATE_OFF);
  print_led_state();
}
void cmd_led_blink(const command_t* command) {
  led_task_set_state(LED_STATE_BLINK);
  print_led_state();
}

void cmd_button(const command_t* command) {
  printf("button: %s, presses %u\n",
         button_task_is_pressed() ? "pressed" : "released",
         button_task_get_press_count());
}

void cmd_profiling(const command_t* command) {
  if (PROFILING_STOPWATCHES_COUNT == 0) {
    printf("profiling is disabled in this build\n");
    return;
  }

  profiling_result_t r;
  
  for (uint32_t id = 0; id < PROFILING_STOPWATCHES_COUNT; id++) {
    if (profiling_get(id, &r)) {
      //      loop       count 812345     mean 5.12 us  max 498 us
      printf("%-10s count %-8u mean %.2f us  max %u us\n", r.name, r.count,
             r.mean_us, r.max_us);
    }

  }
}

void cmd_profiling_reset(const command_t* command) { profiling_reset(); }

const api_command_t commands[] = {
    {"help", "list of commands", cmd_help},
    {"info", "device passport", cmd_info},
    {"uptime", "time since reset", cmd_uptime},
    {"pi_start", "start pi calculation: pi_start [terms]", cmd_pi_start},
    {"pi", "pi calculation result or state", cmd_pi},
    {"led_enable", "enable led", cmd_led_enable},
    {"led_disable", "disable led", cmd_led_disable},
    {"led_blink", "toggle led blinking", cmd_led_blink},
    {"led_period", "set led period in ms: led_period <period_ms>",
     cmd_led_period},
    {"button", "show button state and press count", cmd_button},
    {"profiling", "stopwatch results", cmd_profiling},
    {"profiling_reset", "reset stopwatch results", cmd_profiling_reset},
};

const uint command_count = sizeof(commands) / sizeof(commands[0]);

void cmd_help(const command_t* command) {
  printf("list of commands:\n");
  for (uint i = 0; i < command_count; i++) {
    printf("  %s: %s\n", commands[i].name, commands[i].help);
  }
}

void api_handle(const command_t* command) {
  if (command->truncated) {
    LOG_ERR("error: command longer than 63 characters or 4 arguments\n");
    // printf("",
    // command->name);
    return;
  }
  for (uint i = 0; i < command_count; i++) {
    if (strcmp(command->name, commands[i].name) == 0) {
      if (commands[i].callback != NULL) {
        commands[i].callback(command);
      }

      return;
    }
  }

  printf("error: unknown command '%s', try help\n", command->name);
}