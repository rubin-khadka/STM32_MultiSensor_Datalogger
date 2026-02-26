/*
 * spi1.c
 *
 *  Created on: Feb 25, 2026
 *      Author: Rubin Khadka
 */

#include "stm32f103xb.h"

void SPI1_Init(void)
{
  // Enable clock and spi peripheral
  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_SPI1EN | RCC_APB2ENR_IOPAEN;

  // Configure pins for connection with shift registers
  // PA7 (MOSI)
  GPIOA->CRL &= ~(GPIO_CRL_CNF7 | GPIO_CRL_MODE7);
  GPIOA->CRL |= GPIO_CRL_MODE7_0 | GPIO_CRL_MODE7_1; // 50 MHz
  GPIOA->CRL |= GPIO_CRL_CNF7_1;  // Alternate function output push-pull

  // PA6 (MISO)
  GPIOA->CRL &= ~(GPIO_CRL_CNF6 | GPIO_CRL_MODE6);
  GPIOA->CRL |= GPIO_CRL_CNF6_0;  // Input floating

  // PA5 (SCK)
  GPIOA->CRL &= ~(GPIO_CRL_CNF5 | GPIO_CRL_MODE5);
  GPIOA->CRL |= GPIO_CRL_MODE5_0 | GPIO_CRL_MODE5_1; // 50 MHz
  GPIOA->CRL |= GPIO_CRL_CNF5_1;  // Alternate function output push-pull

  // PA3 (CS)
  GPIOA->CRL &= ~(GPIO_CRL_CNF3 | GPIO_CRL_MODE3);
  GPIOA->CRL |= GPIO_CRL_MODE3_0 | GPIO_CRL_MODE3_1; // 50 MHz output
  GPIOA->CRL |= GPIO_CRL_CNF3_0;  // Push-pull
  GPIOA->BSRR = GPIO_BSRR_BS3;    // CS high

  // SPI1 Configurations
  SPI1->CR1 = 0;
  SPI1->CR1 |= SPI_CR1_SSM;
  SPI1->CR1 |= SPI_CR1_SSI;
  SPI1->CR1 |= SPI_CR1_MSTR;

  // 72MHz/8 - 9 MHZ
  SPI1->CR1 |= SPI_CR1_BR_1;

  // CPOL and CPHA for W25Q64
  SPI1->CR1 &= ~SPI_CR1_CPOL; // Clock idle low (0)
  SPI1->CR1 &= ~SPI_CR1_CPHA; // Data capture on first edge

  // MSB first
  SPI1->CR1 &= ~SPI_CR1_LSBFIRST;

  // Enable SPI
  SPI1->CR1 |= SPI_CR1_SPE;
}

void SPI1_CS_Low(void)
{
  GPIOA->BRR = GPIO_BRR_BR3;
}

void SPI1_CS_High(void)
{
  GPIOA->BSRR = GPIO_BSRR_BS3;
}

uint8_t SPI1_Transfer(uint8_t data)
{
  // Wait until TX buffer is empty
  while(!(SPI1->SR & SPI_SR_TXE));

  // Send data
  SPI1->DR = data;

  // Wait until RX buffer is not empty
  while(!(SPI1->SR & SPI_SR_RXNE));

  // Return received data
  return SPI1->DR;
}
