#pragma once
#include <stdint.h>

bool lpuart_init(uint32_t baud);

void lpuart1_write_byte(uint8_t data);

uint8_t lpuart1_read_byte(void);