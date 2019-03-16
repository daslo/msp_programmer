/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f10x.h"

int main(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;


	/* PC13 (built-in LED) = out, pp, 2MHz */
	GPIOC->CRH |= GPIO_CRH_MODE13_1;
	GPIOC->CRH &= ~GPIO_CRH_CNF13_0;
	GPIOC->ODR |= GPIO_ODR_ODR0; //initially HIGH

	/* PA0 (nRST) = out, pp, 2MHz */
	GPIOA->CRL |= GPIO_CRL_MODE0_1;
	GPIOA->CRL &= ~GPIO_CRL_CNF0_0;
	GPIOA->ODR &= ~GPIO_ODR_ODR0; //initially LOW
	/* PA1 (TEST) = out, pp, 2MHz */
	GPIOA->CRL |= GPIO_CRL_MODE1_1;
	GPIOA->CRL &= ~GPIO_CRL_CNF1_0;
	GPIOA->ODR &= ~GPIO_ODR_ODR1; //initially LOW
	/* PA9 (USART1 TX) = out, alt, 2MHz */
	GPIOA->CRH |= GPIO_CRH_CNF9_1;
	GPIOA->CRH &= ~GPIO_CRH_CNF9_0;
	GPIOA->CRH |= GPIO_CRH_MODE9_1;
	GPIOA->CRH &= ~GPIO_CRH_MODE9_0;

	/* PA2 (USART2 TX) = out, alt, 2MHz */
	GPIOA->CRL |= GPIO_CRL_CNF2_1;
	GPIOA->CRL &= ~GPIO_CRL_CNF2_0;
	GPIOA->CRL |= GPIO_CRL_MODE2_1;
	GPIOA->CRL &= ~GPIO_CRL_MODE2_0;

	/* To PC */
	USART1->BRR = 8000000/9600;
	USART1->CR1 = USART_CR1_UE | USART_CR1_RXNEIE| USART_CR1_TE | USART_CR1_RE;

	/* To MSP */
	USART2->BRR = 8000000/9600;
	USART2->CR1 = USART_CR1_PCE | USART_CR1_M | USART_CR1_UE | USART_CR1_RXNEIE| USART_CR1_TE | USART_CR1_RE;


		while(!(USART1->SR & USART_SR_TXE)); // wait for empty transmit register
		USART1->DR = 'H';


		SysTick_Config(8000000 * 0.1);
	NVIC_EnableIRQ(USART1_IRQn);


	for(;;);
}

void TEST_HIGH(){
	GPIOA->BSRR |= GPIO_BSRR_BS1;
}
void TEST_LOW(){
	GPIOA->BSRR |= GPIO_BSRR_BR1;
}
void nRST_HIGH(){
	GPIOA->BSRR |= GPIO_BSRR_BS0;
}
void nRST_LOW(){
	GPIOA->BSRR |= GPIO_BSRR_BR0;
}
void DELAY(){
	for(volatile int i=0; i<5; ++i); /* DONT FUGING DARE OPTIMIZING IT*/
}

__attribute__((interrupt)) void SysTick_Handler(void){
	GPIOC->BSRR |= ((GPIOC->ODR & GPIO_ODR_ODR13) ? GPIO_BSRR_BR13 : GPIO_BSRR_BS13);
}

static int CONNECTION=0;
char PCin;
char PCout;
char MSPin;
char MSPout;

__attribute__((interrupt)) void USART1_IRQHandler(void){
if(USART1->SR & USART_SR_RXNE){
	PCin=USART1->DR;
	if(CONNECTION){
		while(!(USART2->SR & USART_SR_TXE)); // wait for empty transmit register
		USART2->DR = PCin;
	}
	else {
		/* Connect to MSP */
		if(PCin == 0x80){
			CONNECTION=1;


			nRST_LOW();
			TEST_LOW();
			DELAY();
			TEST_HIGH();
			DELAY();
			DELAY();
			TEST_LOW();
			DELAY();
			DELAY();
			TEST_HIGH();
			DELAY();
			nRST_HIGH();
			DELAY();
			TEST_LOW();

			USART1->DR ='C';

			NVIC_EnableIRQ(USART2_IRQn);
		}

	}
}
}
__attribute__((interrupt)) void USART2_IRQHandler(void){
	if(USART2->SR & USART_SR_RXNE){
		MSPin=USART2->DR;
		while(!(USART1->SR & USART_SR_TXE));
		USART1->DR = (MSPin);
	}
}
