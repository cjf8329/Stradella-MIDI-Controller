#include "systick.h"
#include "stm32g4xx.h"

static volatile uint32_t s_ms;

void systick_init(void) {
    if (SysTick_Config(SystemCoreClock / 1000U) != 0U) {
        while (1) {
            // Error handling: SysTick configuration failed
        }
    }
}

uint32_t systick_ms(void) {
    return s_ms;
}

void SysTick_Handler(void) {
    s_ms++;
}