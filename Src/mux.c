#include "mux.h"
#include "stm32g474xx.h"
#include "stm32g4xx.h"

// PB0 ADC1_IN15

bool mux_init(void) {
    // Enable GPIOB
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    // Enable ADC 1 and 2
    RCC->AHB2ENR |= RCC_AHB2ENR_ADC12EN;

    // Set PB0 port mode to analog
    GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE0_Msk) | GPIO_MODER_MODE0;

    // HCLK /4 division (42.5 MHz @170 MHz system clock, below 52 MHz max for all ADCs enabled)
    ADC12_COMMON->CCR = (ADC12_COMMON->CCR & ~ADC_CCR_CKMODE_Msk) | ADC_CCR_CKMODE;

    // Exit deep power down
    ADC1->CR &= ~ADC_CR_DEEPPWD;
    ADC1->CR |= ADC_CR_ADVREGEN;
    for (volatile int i = 0; i < 50000; i++) {
    }; // >20 microsecond regulator startup

    // Single ended calibration before enabling
    ADC1->CR |= ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL) {
    };

    // Enable ADC & wait until ready
    ADC1->ISR |= ADC_ISR_ADRDY;
    ADC1->CR |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY)) {
    };

    // 47.5 samples + 12.5 conversion cycles at 42.5 MHz ADC clock -> ~1.4 microseconds per channel
    // yielding around 350 microseconds to scan 128 sensors. 12.5 cycles minimum, 47.5 is safe
    ADC1->SMPR2 = (ADC1->SMPR2 & ~ADC_SMPR2_SMP15_Msk) | (4U << ADC_SMPR2_SMP15_Pos);
    ADC1->SQR1 = (15U << ADC_SQR1_SQ1_Pos); // L = 0 for 1 conversion

    return true;
}

uint16_t mux_read_raw(void) {
    ADC1->CR |= ADC_CR_ADSTART;
    while (!(ADC1->ISR & ADC_ISR_EOC)) {
    };
    return (uint16_t)ADC1->DR; // reading DR clears EOC
}