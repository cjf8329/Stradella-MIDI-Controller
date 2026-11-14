#pragma once
#include "stm32g4xx.h"
#include <stdint.h>

bool mux_init(void);
uint16_t mux_read_raw(void);

// Update s0-3 simultaneously
static inline void mux_set_channel(uint8_t ch) {
    // clear and set new address
    GPIOB->BSRR = (0x0FU << (16 + 6) | ((ch & 0xFU) << 6));
}