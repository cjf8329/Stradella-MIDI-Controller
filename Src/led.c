#include "led.h"

void led_init(void) {
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; // Enable GPIOA clock
  (void)RCC->AHB2ENR;                  // To prevent clock enable race condition

  GPIOA->OTYPER &= ~GPIO_OTYPER_OT5;       // Set PA5 to push-pull
  GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED5; // Low speed is fine
  GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD5;       // No pull-up or pull-down
  GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE5_Msk) |
                 GPIO_MODER_MODE5_0; // Set PA5 to output mode
}
