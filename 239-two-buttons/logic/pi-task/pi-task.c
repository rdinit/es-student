#define PI_TERMS_PER_CALL 100
#include "pi-task.h"

#include <stdbool.h>
#include <stdint.h>

#include "systime/systime.h"

static pi_state_t pi_state = PI_STATE_INIT;

static uint32_t pi_terms;
static uint32_t pi_k;
static double pi_sum;
static double pi_sign;
static double pi_result;

static uint64_t pi_start_us;
static uint64_t pi_end_us;

bool pi_task_start(uint32_t terms) {
  if (pi_state == PI_STATE_CALC) {
    return false;
  }

  pi_terms = terms;
  pi_k = 0;
  pi_sum = 0.0;
  pi_sign = 1.0;
  pi_result = 0.0;
  pi_start_us = systime_us();
  pi_end_us = 0;
  pi_state = PI_STATE_CALC;
  return true;
}

void pi_task_handle(void) {
  switch (pi_state) {
    case PI_STATE_INIT:
    case PI_STATE_DONE:
      return;
    case PI_STATE_CALC:
      break;
  }

  uint32_t end = pi_k + PI_TERMS_PER_CALL;
  if (end < pi_k || end > pi_terms) {
    end = pi_terms;
  }

  for (; pi_k < end; pi_k++) {
    pi_sum += pi_sign / (2.0 * pi_k + 1.0);
    pi_sign = -pi_sign;
  }

  if (pi_k == pi_terms) {
    pi_result = 4.0 * pi_sum;
    pi_end_us = systime_us();
    pi_state = PI_STATE_DONE;
  }
}

pi_state_t pi_task_get_state(void) { return pi_state; }

bool pi_task_get_result(double* result, uint64_t* time_ms) {
  if (pi_state != PI_STATE_DONE) {
    return false;
  }

  *result = pi_result;
  *time_ms = (pi_end_us - pi_start_us) / 1000;
  return true;
}
