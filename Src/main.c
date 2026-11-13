#include "main.h"
#include "clock.h"
#include "system_stm32g4xx.h"

void mco_init(void) {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    (void)RCC->AHB2ENR;

    GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED8; /* very high speed */
    GPIOA->AFR[1] &= ~GPIO_AFRH_AFSEL8_Msk; /* AF0 = MCO */
    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_MCOSEL_Msk | RCC_CFGR_MCOPRE_Msk)) |
                RCC_CFGR_MCOSEL_0                                               /* 0001 = SYSCLK */
                | RCC_CFGR_MCOPRE_2;                                            /* 100 = /16 */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE8_Msk) | GPIO_MODER_MODE8_1; /* AF, last */
}

int main(void) {
    if (!clock_init()) {
        for (;;) {
        }
    }
    mco_init();
    SystemCoreClockUpdate();
    systick_init();
    led_init();

    bool on = false;
    uint32_t next = systick_ms();

    while (1) {
        if ((int32_t)(systick_ms() - next) >= 0) {
            next += 500U;
            on = !on;
            if (on) {
                led_on();
            } else {
                led_off();
            }
        }
    }
}