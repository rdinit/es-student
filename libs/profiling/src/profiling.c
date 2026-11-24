#include "profiling/profiling.h"

#include <pico/time.h>
#include <stdio.h>

typedef struct {
  const char* name;
  uint32_t count;
  uint32_t sum_us;
  uint32_t max_us;
  uint32_t start_us;
} profiling_stopwatch_t;

static profiling_stopwatch_t stopwatches[PROFILING_STOPWATCHES_COUNT] = {0};

void profiling_stopwatch_init(uint32_t id, const char* name) {
  if (id < PROFILING_STOPWATCHES_COUNT) {
    stopwatches[id].name = (char*)name;
    stopwatches[id].count = 0;
    stopwatches[id].sum_us = 0;
    stopwatches[id].max_us = 0;
    stopwatches[id].start_us = 0;
  }
}
#if PROFILING_ENABLED
void profiling_start(uint32_t id) {
  if (id < PROFILING_STOPWATCHES_COUNT) {
    stopwatches[id].start_us = time_us_32();
  }
}
void profiling_stop(uint32_t id) {
  if (id < PROFILING_STOPWATCHES_COUNT) {
    uint32_t now_us = time_us_32();
    uint32_t elapsed_us = now_us - stopwatches[id].start_us;
    stopwatches[id].count++;
    stopwatches[id].sum_us += elapsed_us;
    if (elapsed_us > stopwatches[id].max_us) {
      stopwatches[id].max_us = elapsed_us;
    }
  }
}

#endif
// false — секундомер с таким номером не заведён
bool profiling_get(uint32_t id, profiling_result_t* result) {
  if (id < PROFILING_STOPWATCHES_COUNT && result != NULL &&
      stopwatches[id].name != NULL) {
    result->name = stopwatches[id].name;
    result->count = stopwatches[id].count;
    if (stopwatches[id].count > 0) {
      result->mean_us =
          (float)stopwatches[id].sum_us / (float)stopwatches[id].count;
    } else {
      result->mean_us = 0.0f;
    }
    result->max_us = stopwatches[id].max_us;
    return true;
  }
  return false;
}

void profiling_reset(void) {
  for (uint32_t i = 0; i < PROFILING_STOPWATCHES_COUNT; i++) {
    stopwatches[i].count = 0;
    stopwatches[i].sum_us = 0;
    stopwatches[i].max_us = 0;
    stopwatches[i].start_us = 0;
  }
}
