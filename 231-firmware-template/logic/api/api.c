
#include "api.h"

#include <stdio.h>
#include <string.h>

#include "api-commands.h"
#include "api.h"
#include "logging/log.h"



void api_handle(const command_t* command) {
  if (command->truncated) {
    LOG_ERR("error: command longer than 63 characters or 4 arguments\n");
    // printf("",
    // command->name);
    return;
  }
  for (uint32_t i = 0; i < api_command_count; i++) {
    if (strcmp(command->name, api_commands[i].name) == 0) {
      if (api_commands[i].callback != NULL) {
        api_commands[i].callback(command);
      }

      return;
    }
  }

  printf("error: unknown command '%s', try help\n", command->name);
}