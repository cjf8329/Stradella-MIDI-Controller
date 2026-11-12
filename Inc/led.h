#pragma once
// gotta make sure to switch to pragma once everywhere
#include "stm32g4xx.h"

void led_init(void);
static inline void led_on(void) { GPIOA->BSRR = GPIO_BSRR_BS5; }
static inline void led_off(void) { GPIOA->BSRR = GPIO_BSRR_BR5; }