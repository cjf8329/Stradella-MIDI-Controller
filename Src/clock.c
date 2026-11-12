#include "clock.h"
#include "stm32g4xx.h"

void clock_init(void) {

    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN; // Enable power interface clock
    (void)RCC->AHB2ENR; // To prevent clock enable race condition


}