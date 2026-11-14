#include "main.h"
#include "lpuart.h"

int main(void) {
    if (!clock_init()) {
        for (;;) {
        }
    }
    SystemCoreClockUpdate();
    systick_init();
    // led_init();
    lpuart_init(115200UL);
    mux_init();

    for (;;) {
        uint16_t val = mux_read_raw();
    }
}