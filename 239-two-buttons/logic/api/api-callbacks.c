#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "api/api-commands.h"
#include "button-task/button-task.h"
#include "device/device.h"
#include "errno.h"
#include "firmware.h"
#include "led-task/led-task.h"
#include "logging/log.h"
#include "pi-task/pi-task.h"
#include "platform/platform.h"
#include "profiling/profiling.h"
#include "stdio-text-protocol/stdio-text-protocol.h"
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

static void print_led_state(void) {
  led_state_t state = led_task_get_state();
  printf("led: %s, period %u ms\n",
         state == LED_STATE_OFF  ? "off"
         : state == LED_STATE_ON ? "on"
                                 : "blink",
         (unsigned)led_task_get_period_ms());
}

static void cmd_pi_start(const command_t* command) {
  uint32_t terms = PI_TERMS_DEFAULT;

  if (command->argc > 1) {
    printf("error: usage pi_start [<terms>]\n");
    return;
  }
  if (command->argc == 1 && !parse_u32(command->argv[0], &terms)) {
    printf("error: usage pi_start [<terms>]\n");
    return;
  }
  if (!pi_task_start(terms)) {
    printf("error: pi is busy\n");
    return;
  }
  if (terms == 0) {
    printf("error: terms must be greater than 0\n");
    return;
  }
  printf("pi: started, %u terms\n", (unsigned)terms);
}

static void cmd_pi(const command_t* command) {
  pi_state_t state = pi_task_get_state();
  /*
  Ответ	pi: not started — расчёт не запускался;
pi: running — расчёт идёт;
pi: <значение> in <время> ms — расчёт закончен; значение — с 9 знаками после
точки, время — десятичное число миллисекунд
*/
  switch (state) {
    case PI_STATE_INIT:
      printf("pi: not started\n");
      break;
    case PI_STATE_CALC:
      printf("pi: running\n");
      break;
    case PI_STATE_DONE:
      double result;
      uint64_t time_ms;
      pi_task_get_result(&result, &time_ms);
      printf("pi: %f in %f ms\n", result, time_ms);
      break;
    default:
      break;
  }
}

static void cmd_info(const command_t* command) {
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

static void cmd_uptime(const command_t* command) {
  printf("uptime: %llu ms\n", systime_ms());
}

static void cmd_help(const command_t* command);

static void cmd_led_period(const command_t* command) {
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

static void cmd_led_enable(const command_t* command) {
  led_task_set_state(LED_STATE_ON);
  print_led_state();
}
static void cmd_led_disable(const command_t* command) {
  led_task_set_state(LED_STATE_OFF);
  print_led_state();
}
static void cmd_led_blink(const command_t* command) {
  led_task_set_state(LED_STATE_BLINK);
  print_led_state();
}

static void cmd_button(const command_t* command) {
  for (int i = 0; i < BUTTON_COUNT; ++i) {
    printf("button %d: %s, presses %u\n", i + 1,
           button_task_is_pressed((button_id)i) ? "pressed" : "released",
           (unsigned)button_task_get_press_count((button_id)i));
  }
}

static void cmd_profiling(const command_t* command) {
  if (PROFILING_STOPWATCHES_COUNT == 0) {
    printf("profiling is disabled in this build\n");
    return;
  }

  profiling_result_t r;

  for (uint32_t id = 0; id < PROFILING_STOPWATCHES_COUNT; id++) {
    if (profiling_get(id, &r)) {
      //      loop       count 812345     mean 5.12 us  max 498 us
      printf("%-10s count %-8u mean %.2f us  max %u us\n", r.name,
             (unsigned)r.count, r.mean_us, (unsigned)r.max_us);
    }
  }
}

static void cmd_profiling_reset(const command_t* command) {
  profiling_reset();
  printf("profiling reset\n");
}

const api_command_t api_commands[] = {
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

const uint32_t api_command_count =
    sizeof(api_commands) / sizeof(api_commands[0]);

static void cmd_help(const command_t* command) {
  for (uint32_t i = 0; i < api_command_count; i++) {
    printf("%-16s %s\n", api_commands[i].name, api_commands[i].help);
  }
}
