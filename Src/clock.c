#include "clock.h"
#include "stm32g4xx.h"

#define WAIT_LOOPS 100000U

static bool wait_for(volatile uint32_t *reg, uint32_t mask, uint32_t value) {
    for (uint32_t i = 0; i < WAIT_LOOPS; i++) {
        if ((*reg & mask) == value)
            return true;
    }
    return false;
}

bool clock_init(void) {
    // Enable power interface clock
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;

    // To prevent clock enable race condition
    (void)RCC->APB1ENR1;

    // Set HPRE = /2
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_HPRE_Msk) | RCC_CFGR_HPRE_DIV2;

    // Set main regulator in range 1 to boost mode
    PWR->CR5 &= ~PWR_CR5_R1MODE;

    // At 170MHz, 4 wait states are needed per rm0440
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY_Msk) | FLASH_ACR_LATENCY_4WS;

    // rm0440 says to read the register so the new number of wait states is taken into account
    if (!wait_for(&FLASH->ACR, FLASH_ACR_LATENCY_Msk, FLASH_ACR_LATENCY_4WS))
        return false;

    // enable instruction cache, data cache, and prefetch
    FLASH->ACR |= FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_PRFTEN;

    // Disable PLL for configuration
    RCC->CR &= ~RCC_CR_PLLON;

    // wait for PLL to stop
    if (!wait_for(&RCC->CR, RCC_CR_PLLRDY, 0))
        return false;

    RCC->PLLCFGR = RCC_PLLCFGR_PLLSRC_HSI          // Set PLL source to HSI
                   | (3U << RCC_PLLCFGR_PLLM_Pos)  // PLLM = 4 (set to m-1 as per rm0440)
                   | (85U << RCC_PLLCFGR_PLLN_Pos) // PLLN = 85, 16 * 85/4 = 340MHz VCO frequency
                   | (0U << RCC_PLLCFGR_PLLR_Pos)  // PLLR = 2 (00 = /2)
                   | RCC_PLLCFGR_PLLREN;

    RCC->CR |= RCC_CR_PLLON;
    if (!wait_for(&RCC->CR, RCC_CR_PLLRDY, RCC_CR_PLLRDY))
        return false;

    // Switch at HCLK = SYSCLK/2, then step to /1
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_PLL;
    if (!wait_for(&RCC->CFGR, RCC_CFGR_SWS_Msk, RCC_CFGR_SWS_PLL))
        return false;

    // wait at least 1 microsecond at 85MHz
    for (volatile uint32_t i = 0; i < 100U; i++) {
    }

    // HPRE = /1 AHB prescaler for 170MHz
    RCC->CFGR &= ~RCC_CFGR_HPRE_Msk;
    (void)RCC->CFGR;

    return true;
}