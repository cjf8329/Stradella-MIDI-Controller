#include "main.h"
#include "mux.h"
#include "stm32g474xx.h"
#include <stddef.h>
#include <stdio.h>

int main(void) {
    if (!clock_init()) {
        for (;;) {
        }
    }
    SystemCoreClockUpdate();
    systick_init();
    // led_init();
    lpuart_init(115200UL);
    uint8_t mux_count = mux_init();

    uint16_t keys[mux_count][MUX_CH_PER];

    // Set PC9 as output for timing test
    GPIOC->MODER *= ~GPIO_MODER_MODE9;
    GPIOC->MODER |= (1U << GPIO_MODER_MODE9_Pos);
    bool toggle = false;

    for (;;) {
        mux_scan_all(keys);

        // for (size_t i = 0U; i < mux_count; i++) {
        //     for (size_t j = 0U; j < MUX_CH_PER; j++) {
        //         volatile uint16_t num = keys[i][j];
        //     }
        // }
        if (toggle) {
            GPIOC->BSRR = (1U << GPIO_BSRR_BR9_Pos);
        } else {
            GPIOC->BSRR = (1U << GPIO_BSRR_BS9_Pos);
        }
    }
}