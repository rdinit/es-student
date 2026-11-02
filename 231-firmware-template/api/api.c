
#include "api.h"
#include "pico/stdlib.h"
#include "stdio.h"
#include "string.h"
#include "profiling.h"
#include "device.h"
#include "hardware/clocks.h"
// обработчик команды: получает имя и аргументы
typedef void (*api_callback_t)(const command_t *command);

// команда прибора: имя, справка для help и обработчик
typedef struct
{
    const char *name;
    const char *help;
    api_callback_t callback;
} api_command_t;

// прикидка: за член ряда 4 операции с double, 175 + 110 + 190 + 110 = 585
// тактов; 1 000 000 членов по 585 тактов при 125 МГц — около 4,7 с
const uint CALC_PI_TERMS = 1000000;

double calc_pi(uint terms) {
  double sum = 0.0;
  double sign = 1.0;

  for (int k = 0; k < terms; k++) {
    // Прибавить очередной член ряда: ±1 / (2k + 1)
    sum += sign / (2.0 * k + 1.0);
    sign = -sign;
  }

  // Сумма ряда равна π/4
  return 4.0 * sum;
}

volatile double pi_result;

void cmd_calc_pi(const command_t *command) {
  uint64_t start_us = time_us_64();
  pi_result = calc_pi(CALC_PI_TERMS);
  uint64_t spent_us = time_us_64() - start_us;

  printf("pi: %.8f\n", pi_result);
  printf("time: %llu ms\n", spent_us / 1000);
}


void cmd_info(const command_t *command) {
  device_info();
}

void cmd_uptime(const command_t *command) { printf("uptime: %llu ms\n", time_us_64() / 1000); }

void cmd_main_time_reset(const command_t *command) { main_time_reset(); }

void cmd_main_time_exec(const command_t *command) { main_time_exec(); }

void cmd_help(const command_t *command);

const api_command_t commands[] = {
    {"help", "list of commands", cmd_help},
    {"info", "device passport", cmd_info},
    {"uptime", "time since reset", cmd_uptime},
    {"calc_pi", "calculate pi: calc_pi [terms]", cmd_calc_pi},
    {"main_time_reset", "reset maximum iteration", cmd_main_time_reset},
    {"main_time_exec", "superloop iteration: average and maximum", cmd_main_time_exec},
};

const uint command_count = sizeof(commands) / sizeof(commands[0]);

void cmd_help(const command_t *command) {
  printf("list of commands:\n");
  for (uint i = 0; i < command_count; i++) {
    printf("  %s: %s\n", commands[i].name, commands[i].help);
  }
}


void api_handle(const command_t *command) {
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