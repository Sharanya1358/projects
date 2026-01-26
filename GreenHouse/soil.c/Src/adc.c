#include "adc.h"
#include "uart.h"
#include "stm32f4xx.h"
#include<stdio.h>
 // ADC Initialization
void AdcInit(void) {

    /* GPIOA clock enable */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* PA0 as Analog */
    GPIOA->MODER |= (3U << (0 * 2));

    /* ADC1 clock enable */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    /* ADC config: 12-bit, single conversion */
    ADC1->CR1 &= ~(ADC_CR1_RES);
    ADC1->CR2 &= ~ADC_CR2_CONT;
    ADC1->SQR1 &= ~(ADC_SQR1_L);
    ADC1->SQR3  = 0;

    ADC1->CR2 |= ADC_CR2_ADON;

    /* PA5 as output (Relay) */
    GPIOA->MODER &= ~(3U << (5 * 2));
    GPIOA->MODER |=  (1U << (5 * 2));

    GPIOA->OTYPER &= ~(1U << 5);
    GPIOA->OSPEEDR |=  (3U << (5 * 2));
    GPIOA->PUPDR   &= ~(3U << (5 * 2));
}


// Read ADC value
uint16_t AdcRead(void) {

    ADC1->CR2 |= ADC_CR2_SWSTART;
    while (!(ADC1->SR & ADC_SR_EOC));
    return ADC1->DR;
}

   //MoistureControl()
   // Controls relay (PA5)
   // Uses thresholds lower=40%, upper=50%
void MoistureControl(void)
{
    uint16_t adc_val = AdcRead();

    // Moisture percentage
    int moisturePercent = (adc_val / 4095.0f) * 100.0f;

    // Relay control thresholds
    float lower = 40.0f;
    float upper = 50.0f;

    // Control relay
    if (moisturePercent < lower) {
        GPIOA->ODR &= ~(1U << 5);    // Pump ON
    } else if (moisturePercent > upper) {
        GPIOA->ODR |= (1U << 5);     // Pump OFF
    }

    // Prepare output string
    char output[60];
    if (moisturePercent < lower)
        sprintf(output, "ADC=%u  Moisture=%d%%  PUMP ON\r\n", adc_val, moisturePercent);
    else
        sprintf(output, "ADC=%u  Moisture=%d%%  PUMP OFF\r\n", adc_val, moisturePercent);

    // Send to UART
    UartPuts(output);

    // Small software delay
    for (volatile int i = 0; i < 300000; i++);
}

