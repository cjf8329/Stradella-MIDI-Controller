#include "lpuart.h"
#include "stm32g474xx.h"
#include "stm32g4xx.h"

bool lpuart_init(uint32_t baud) {

    // Enable HSI16 and wait for HSIRDY
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY)) {
    }

    // RCC enable lpuart
    RCC->APB1ENR2 |= RCC_APB1ENR2_LPUART1EN;

    // RCC enable GPIOA
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    // Select HSI16 as LPUART1 clock
    RCC->CCIPR = (RCC->CCIPR & ~RCC_CCIPR_LPUART1SEL) | (2U << RCC_CCIPR_LPUART1SEL_Pos);

    // Configure PA2 (TX) and PA3 (RX) as Alternate Function
    GPIOA->MODER &= ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
    GPIOA->MODER |= (2U << GPIO_MODER_MODE2_Pos) | (2U << GPIO_MODER_MODE3_Pos);

    // Select AF12 for PA2 and PA3 in AFRL (AFR[0])
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2 | GPIO_AFRL_AFSEL3);
    GPIOA->AFR[0] |= (12U << GPIO_AFRL_AFSEL2_Pos) | (12U << GPIO_AFRL_AFSEL3_Pos);

    // Disable LPUART1 for config (reset value of 0)
    LPUART1->CR1 = 0;

    // BRR = (256 * 16,000,000) / baud
    LPUART1->BRR = (uint32_t)((256ULL * 16000000ULL) / baud);

    // Enable transmitter, receiver, and LPUART1
    LPUART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void lpuart1_write_byte(uint8_t data) {
    while (!(LPUART1->ISR & USART_ISR_TXE_TXFNF)) {
    } // Wait until TX register/FIFO not full
    LPUART1->TDR = data;
}

uint8_t lpuart1_read_byte(void) {
    while (!(LPUART1->ISR & USART_ISR_RXNE_RXFNE)) {
    } // Wait until RX data available
    return (uint8_t)LPUART1->RDR;
}