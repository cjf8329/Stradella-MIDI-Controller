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

    for (;;) {
        lpuart1_write_byte('A');
    }
}