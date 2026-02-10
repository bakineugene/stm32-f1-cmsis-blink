#include "stm32f1xx.h"

void _init(void) { /* empty */ }

// Settings for High Speed External (HSE) oscillator
#define ENABLE_HSE() (RCC->CR |= RCC_CR_HSEON)
#define HSE_READY() (RCC->CR & RCC_CR_HSERDY)

#define ENABLE_GPIOA_CLOCK() (RCC->APB2ENR |= RCC_APB2ENR_IOPAEN)
#define ENABLE_GPIOB_CLOCK() (RCC->APB2ENR |= RCC_APB2ENR_IOPBEN)
#define ENABLE_GPIOC_CLOCK() (RCC->APB2ENR |= RCC_APB2ENR_IOPCEN)
#define ENABLE_AFIOEN_CLOCK() (RCC->APB2ENR |= RCC_APB2ENR_AFIOEN)

#define RESET_EXTICR(exti, pin) AFIO->EXTICR[exti - 1] &= ~AFIO_EXTICR##exti##_EXTI##pin
#define SET_EXTICR(exti, pin, port) AFIO->EXTICR[exti - 1] |= AFIO_EXTICR##exti##_EXTI##pin##_##port

#define ENABLE_EXTI(pin) EXTI->IMR |= EXTI_IMR_MR##pin
#define ENABLE_EXTI_FALLING(pin) EXTI->FTSR |= EXTI_FTSR_TR##pin
#define DISABLE_EXTI_FALLING(pin) EXTI->FTSR &= ~EXTI_FTSR_TR##pin
#define ENABLE_EXTI_RISING(pin) EXTI->RTSR |= EXTI_RTSR_TR##pin
#define DISABLE_EXTI_RISING(pin) EXTI->RTSR &= ~EXTI_RTSR_TR##pin
#define RESET_EXTI_PENDING(pin) EXTI->PR = EXTI_PR_PR##pin
#define EXTI_PENDING(pin) EXTI->PR & EXTI_PR_PR##pin

#define RESET_CRL(pin) CRL &= ~(GPIO_CRL_MODE##pin | GPIO_CRL_CNF##pin)
#define RESET_CRH(pin) CRH &= ~(GPIO_CRH_MODE##pin | GPIO_CRH_CNF##pin)

#define CRL_OUT_10_MHZ(pin) (GPIO_CRL_MODE##pin##_0)                           // MODE = 01
#define CRL_OUT_2_MHZ(pin)  (GPIO_CRL_MODE##pin##_1)                           // MODE = 10
#define CRL_OUT_50_MHZ(pin) (GPIO_CRL_MODE##pin##_1 | GPIO_CRL_MODE##pin##_0)  // MODE = 11

#define CRH_OUT_10_MHZ(pin) (GPIO_CRH_MODE##pin##_0)                           // MODE = 01
#define CRH_OUT_2_MHZ(pin)  (GPIO_CRH_MODE##pin##_1)                           // MODE = 10
#define CRH_OUT_50_MHZ(pin) (GPIO_CRH_MODE##pin##_1 | GPIO_CRH_MODE##pin##_0)  // MODE = 11

// GPIO input mode with pull-up/down
#define CRH_IN_PULL(pin)   (GPIO_CRH_CNF##pin##_1)

// Pull direction for input-pull mode
#define PULL_UP(pin)   ODR |=  GPIO_ODR_ODR##pin
#define PULL_DOWN(pin) ODR &= ~GPIO_ODR_ODR##pin
#define TOGGLE(pin) ODR ^= GPIO_ODR_ODR##pin

#define SET(pin)   BSRR = GPIO_BSRR_BS##pin
#define RESET(pin) BSRR = GPIO_BSRR_BR##pin

#define MAX_BLINK_DELAY 300
#define DELAY_BLINK_STEP 5

struct BlinkConfig {
    int32_t time;
    int32_t delay;
    int8_t delay_step;
};

volatile struct BlinkConfig blink_1 = {0, MAX_BLINK_DELAY, -DELAY_BLINK_STEP};

void UpdateBlinkTime(void) {
    ++blink_1.time;
}

void BlinkLED(void) {
    if (blink_1.time > blink_1.delay) {
        GPIOA->TOGGLE(5);
        blink_1.time = 0;
        blink_1.delay += blink_1.delay_step;
        if (blink_1.delay > MAX_BLINK_DELAY) blink_1.delay_step = -DELAY_BLINK_STEP;
        if (blink_1.delay < 0) blink_1.delay_step = DELAY_BLINK_STEP;
    }
}

void SysTick_Handler(void) {
    UpdateBlinkTime();
}

void EXTI15_10_IRQHandler(void) {
    if (EXTI_PENDING(13)) {
        RESET_EXTI_PENDING(13);
        GPIOA->TOGGLE(5);
        // BlinkSecondLED();
    }
}

void SystemClock_Config(void) {
    ENABLE_HSE(); while (!HSE_READY());

    /**
     * Bits 2:0 LATENCY: Latency
     * These bits represent the ratio of the SYSCLK (system clock) period to the Flash access
     * time.
     *
     * 000 Zero wait state        0 MHz < SYSCLK ≤ 24 MHz   : default
     * 001 One wait state        24 MHz < SYSCLK ≤ 48 MHz   : FLASH_ACR_LATENCY_0
     * 010 Two wait states       48 MHz < SYSCLK ≤ 72 MHz   : FLASH_ACR_LATENCY_1
     *
     * See PM0075 Flash Programming Tutorial
     */
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

    SystemCoreClockUpdate();
}

int main(void) {
    SystemClock_Config();
    if(SysTick_Config(SystemCoreClock / 1000)) {
        while(1); // error
    }

    ENABLE_GPIOC_CLOCK();
    ENABLE_GPIOA_CLOCK();
    ENABLE_AFIOEN_CLOCK();

    GPIOA->RESET_CRL(5);
    GPIOA->CRL |= CRL_OUT_2_MHZ(5);

    GPIOC->RESET_CRH(13);
    GPIOC->CRH |= CRH_IN_PULL(13);
    GPIOC->PULL_UP(13);

    RESET_EXTICR(4, 13);
    SET_EXTICR(4, 13, PC);

    ENABLE_EXTI(13);
    ENABLE_EXTI_FALLING(13);
    DISABLE_EXTI_RISING(13);
    RESET_EXTI_PENDING(13);

    // NVIC_SetPriority(EXTI9_5_IRQn, 5);
    NVIC_EnableIRQ(EXTI15_10_IRQn);

    while (1) {
        BlinkLED();
    }
}
