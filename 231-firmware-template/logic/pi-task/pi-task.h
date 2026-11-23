#pragma once

#include <stdbool.h>
#include <stdint.h>

#define PI_TERMS_DEFAULT 1000000

bool pi_task_start(uint32_t terms);
void pi_task_handle(void);

bool pi_task_get_result(double* result, uint64_t* time_ms);

void print_pi_state(void);
