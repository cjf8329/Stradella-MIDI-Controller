#include "mux.h"
#include "stm32g474xx.h"
#include "stm32g4xx.h"
#include "system_stm32g4xx.h"
#include <stddef.h>

typedef struct {
    GPIO_TypeDef *port;
    uint8_t pin;
    uint8_t adc_ch;
} mux_ch_cfg_t;

// PB0 ADC1_IN15
// PB1 ADC1_IN12
// PB11 ADC12_IN14
// PB12 ADC1_IN11
// PB14 ADC1_IN5
// PA1 ADC12_IN2
// PA0 ADC12_IN1
// PC3 ADC12_IN9
static const mux_ch_cfg_t mux_cfg[] = {{GPIOB, 0, 15},  {GPIOB, 1, 12}, {GPIOB, 11, 14},
                                       {GPIOB, 12, 11}, {GPIOB, 14, 5}, {GPIOA, 1, 2},
                                       {GPIOA, 0, 1},   {GPIOC, 3, 9}};

#define NUM_MUXES (sizeof(mux_cfg) / sizeof(mux_cfg[0]))
const uint8_t MUX_COUNT = NUM_MUXES;

static volatile uint16_t adc_buf[NUM_MUXES];

// Set sample time for channel in smp, automatically handles channels 0-18
static void adc_set_smp(ADC_TypeDef *adc, uint8_t ch, uint8_t smp) {
    if (ch <= 9) {
        uint8_t pos = ch * 3U; // smp for each channel is 3 bits wide
        adc->SMPR1 = (adc->SMPR1 & ~(7U << pos)) | ((uint32_t)smp << pos);
    } else {
        uint8_t pos = (ch - 10U) * 3U; // need offset for upper 10 channels
        adc->SMPR2 = (adc->SMPR2 & ~(7U << pos)) | ((uint32_t)smp << pos);
    }
}

// Place channel into sequence slot idx
static void adc_set_sqr(ADC_TypeDef *adc, uint8_t mux_id, uint8_t ch) {
    volatile uint32_t *sqr = &adc->SQR1 + (mux_id / 6U); // SQR1-4 are contiguous
    uint8_t pos;
    // SQR1 starts at bit 6 while SQR2-4 start at bit 0,
    // if chain handles all muxes
    if (mux_id < 4U) {
        sqr = &adc->SQR1;
        pos = (mux_id + 1U) * 6U;
    } else if (mux_id < 9U) {
        sqr = &adc->SQR2;
        pos = (mux_id - 4U) * 6U;
    } else if (mux_id < 14U) {
        sqr = &adc->SQR3;
        pos = (mux_id - 9U) * 6U;
    } else {
        sqr = &adc->SQR4;
        pos = (mux_id - 14U) * 6U;
    }

    // read-modify-write channel into appropriate sequence field
    *sqr = (*sqr & ~(0x1FU << pos)) | ((uint32_t)ch << pos);
}

uint8_t mux_init(void) {
    // Enable GPIOABC, ADC1
    RCC->AHB2ENR |=
        RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOCEN | RCC_AHB2ENR_ADC12EN;

    // Enable DMA1 and DMAMUX1
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMAMUX1EN;

    // Configure sequence length L = num muxes - 1
    ADC1->SQR1 = ((NUM_MUXES - 1U) << ADC_SQR1_L_Pos);

    // Configure mux pins, sample times, sequence slots
    for (size_t i = 0; i < NUM_MUXES; i++) {
        uint8_t pin = mux_cfg[i].pin;
        mux_cfg[i].port->MODER |= (3U << (pin * 2U)); // analog mode

        adc_set_smp(ADC1, mux_cfg[i].adc_ch, 4U); // 47.5 adc cycles
        adc_set_sqr(ADC1, (uint8_t)i, mux_cfg[i].adc_ch);
    }

    // Set select pins B6-9 as output
    GPIOB->MODER &= ~(GPIO_MODER_MODE6 | GPIO_MODER_MODE7 | GPIO_MODER_MODE8 | GPIO_MODER_MODE9);
    GPIOB->MODER |= (1U << GPIO_MODER_MODE6_Pos) | (1U << GPIO_MODER_MODE7_Pos) |
                    (1U << GPIO_MODER_MODE8_Pos) | (1U << GPIO_MODER_MODE9_Pos);

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

    // DMAMUX reqid ADC1 = ID5, dmamux ch 0-7 on dma1 ch 1-8 on cat 3 devices
    // DMAMUX1_Channel0 corresponds with DMA1_Channel1 per rm0440
    DMAMUX1_Channel0->CCR = (5U << DMAMUX_CxCR_DMAREQ_ID_Pos);

    // Set peripheral address register to adc1 data register
    DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;

    // set memory address register and number of data to transfer register
    DMA1_Channel1->CMAR = (uint32_t)adc_buf;
    DMA1_Channel1->CNDTR = NUM_MUXES;

    // Set peripheral size and memory size to 16 bits (8 bits is next step down, too little),
    // memory increment mode, circular mode, and enable dma1)channel1
    DMA1_Channel1->CCR = (1U << DMA_CCR_PSIZE_Pos) | (1U << DMA_CCR_MSIZE_Pos) | DMA_CCR_MINC |
                         DMA_CCR_CIRC | DMA_CCR_EN;

    // Enable circular DMA requests (on ADC), ADRDY interrupt, enable ADC
    ADC1->CFGR |= ADC_CFGR_DMACFG | ADC_CFGR_DMAEN;
    ADC1->ISR |= ADC_ISR_ADRDY;
    ADC1->CR |= ADC_CR_ADEN;

    // wait until ready
    while (!(ADC1->ISR & ADC_ISR_ADRDY)) {
    };

    return MUX_COUNT;
}

uint16_t mux_read_raw(void) {
    ADC1->CR |= ADC_CR_ADSTART;
    while (!(ADC1->ISR & ADC_ISR_EOC)) {
    };
    return (uint16_t)ADC1->DR; // reading DR clears EOC
}

// >3400 cycles at 170 MHz for 20 microsecond delay
void mux_settle_delay() {
    for (size_t i = 0; i < 3400; i++) {
    }
}

void mux_scan_all(uint16_t out[][MUX_CH_PER]) {
    for (uint8_t ch = 0; ch < MUX_CH_PER; ch++) {
        mux_set_channel(ch);
        mux_settle_delay();

        // transfer complete flag
        DMA1->IFCR = DMA_IFCR_CTCIF1;

        // start adc, wait for transfer to complete
        ADC1->CR |= ADC_CR_ADSTART;
        while (!(DMA1->ISR & DMA_ISR_TCIF1)) {
        }

        for (uint8_t m = 0U; m < NUM_MUXES; m++) {
            out[m][ch] = adc_buf[m];
        }
    }
}