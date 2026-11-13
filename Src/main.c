#include "main.h"
#include "clock.h"
#include "system_stm32g4xx.h"

int main(void) {
    if (!clock_init()) {
        for (;;) {
        }
    }
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