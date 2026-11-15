#pragma once
#include "stm32g4xx.h"
#include <stdint.h>

#define MUX_CH_PER 16U

extern const uint8_t MUX_COUNT;

uint8_t mux_init(void);
uint16_t mux_read_raw(void);

// Update s0-3 simultaneously
static inline void mux_set_channel(uint8_t ch) {
    // clear and set new address (PB6-9)
    GPIOB->BSRR = (0x0FU << (16 + 6) | ((ch & 0xFU) << 6));
}

void mux_scan_all(uint16_t out[][MUX_CH_PER]);