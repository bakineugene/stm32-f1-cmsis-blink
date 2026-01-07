#include "stm32f1xx.h"

void _init(void) {}

#define MAX_BLINK_DELAY 300
#define DELAY_BLINK_STEP 5

struct BlinkConfig {
    int32_t time;
    int32_t delay;
    int8_t delay_step;
};

volatile struct BlinkConfig blink = {0, MAX_BLINK_DELAY, -DELAY_BLINK_STEP};

void ResetBlinkTime(void) {
    blink.time = 0;
    blink.delay += blink.delay_step;
    if (blink.delay > MAX_BLINK_DELAY) blink.delay_step = -DELAY_BLINK_STEP;
    if (blink.delay < 0) blink.delay_step = DELAY_BLINK_STEP;
}

void UpdateBlinkTime(void) {
    ++blink.time;

    if (blink.time > blink.delay && READ_BIT(GPIOC->ODR, GPIO_ODR_ODR13)) {
        GPIOC->BSRR = GPIO_BSRR_BR13;
        ResetBlinkTime();
    }
    if (blink.time > blink.delay && !READ_BIT(GPIOC->ODR, GPIO_ODR_ODR13)) {
        GPIOC->BSRR = GPIO_BSRR_BS13;
        ResetBlinkTime();
    }
}

void SysTick_Handler(void) {
    UpdateBlinkTime();
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
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000);

    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13);
    GPIOC->CRH |= GPIO_CRH_MODE13_1;

    while (1) { /* empty */ }
}
