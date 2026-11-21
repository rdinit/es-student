#pragma once



#include <stdint.h>

// восемь байт номера платы по две шестнадцатеричные цифры и нулевой символ
#define DEVICE_SERIAL_SIZE (8 * 2 + 1)

typedef struct {
  const char* device_board;
  char board_id[DEVICE_SERIAL_SIZE];
  uint32_t manufacturer;
  uint32_t part;
  uint32_t chip_revision;
  const char* sdk_version;
} device_info_t;

void device_get_info(device_info_t* info);
