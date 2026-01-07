#include "stm32f1xx.h"

void _init(void) {}

void delay(volatile uint32_t n) {
    while (n--) {
        __asm__("nop");
    }
}

void SystemClock_Config(void) {
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY));

    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2;

    RCC->CFGR &= ~RCC_CFGR_PLLSRC & ~RCC_CFGR_PLLMULL;
    RCC->CFGR |= RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9;

    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;

    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}

int main(void) {
    SystemClock_Config();

    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13);
    GPIOC->CRH |= GPIO_CRH_MODE13_1;

    while (1) {
        GPIOC->BSRR = GPIO_BSRR_BR13;
        delay(5000000);
        GPIOC->BSRR = GPIO_BSRR_BS13;
        delay(1000000);
    }
}
