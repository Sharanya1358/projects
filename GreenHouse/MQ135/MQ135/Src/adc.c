/*
 * adc.c
 *
 *  Created on: Oct 1, 2025
 *      Author: sunbeam
 */
#include "adc.h"
//ldr sensor channel PA0
void AdcInit(void)
{   //ENABLE GPIO PERIPHERAL CLOCK
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	// CONFIGURE GPIO AS ANALOG MODE 11
	GPIOA->MODER |= BV(0)|BV(1);
	//ENABLE ADC PERIPHERAL CLOCK
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    //SET ADC RESOLUTION BIT 12
    ADC1->CR1 &= ~(ADC_CR1_RES_0 |ADC_CR1_RES_1);
    //ENABLE SINGLE CONVERSION
    ADC1->CR2 &= ~ADC_CR2_CONT;
    // SET A NUMBER FOR WANT CONV OF 1 PERIPHERAL
    ADC1->SQR1 |= (0<< ADC_SQR1_L_Pos);
    // SET A CHANNEL FOR PERIPHERAL
    ADC1->SQR3 |= (0 << ADC_SQR3_SQ1_Pos);
    //ENABLE ADC
    ADC1->CR2 |= ADC_CR2_ADON;

}
uint16_t AdcRead(void)
{
	// start the conversion
	ADC1->CR2 |= ADC_CR2_SWSTART;
	//WAIT FOR END OF CONVERSION
	while(!(ADC1->SR & ADC_SR_EOC));
	uint16_t val = ADC1->DR;
	return val;
	}

