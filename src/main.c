#include "stm32f1xx.h"

void _init(void) { /* empty */ }

#define ENABLE_GPIOB_CLOCK() (RCC->APB2ENR |= RCC_APB2ENR_IOPBEN)
#define ENABLE_GPIOC_CLOCK() (RCC->APB2ENR |= RCC_APB2ENR_IOPCEN)
#define ENABLE_AFIOEN_CLOCK() (RCC->APB2ENR |= RCC_APB2ENR_AFIOEN)

// Preparse CRH for writing configuration values
#define RESET_CRH(pin) CRH &= ~(GPIO_CRH_MODE##pin | GPIO_CRH_CNF##pin)

// Output speeds (MODE bits)
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

void ResetBlinkTime(void) {
    blink_1.time = 0;
    blink_1.delay += blink_1.delay_step;
    if (blink_1.delay > MAX_BLINK_DELAY) blink_1.delay_step = -DELAY_BLINK_STEP;
    if (blink_1.delay < 0) blink_1.delay_step = DELAY_BLINK_STEP;
}

struct DebounceConfig {
    int32_t time;
    int32_t debounce;
};

volatile struct DebounceConfig debounce = {0, 200};

void UpdateBlinkTime(void) {
    ++blink_1.time;
    if (debounce.time > 0) --debounce.time;

    if (blink_1.time > blink_1.delay && READ_BIT(GPIOC->ODR, GPIO_ODR_ODR13)) {
        GPIOC->RESET(13);
        ResetBlinkTime();
    }
    if (blink_1.time > blink_1.delay && !READ_BIT(GPIOC->ODR, GPIO_ODR_ODR13)) {
        GPIOC->SET(13);
        ResetBlinkTime();
    }
}

void BlinkSecondLED(void) {
    if (debounce.time == 0) {
        GPIOB->TOGGLE(8);
        debounce.time = debounce.debounce;
    }
}

void SysTick_Handler(void) {
    UpdateBlinkTime();
}

void EXTI9_5_IRQHandler(void) {
    if (EXTI->PR & (1U << 9)) {
        EXTI->PR = (1U << 9);
        BlinkSecondLED();
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
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000);

    ENABLE_GPIOC_CLOCK();
    ENABLE_GPIOB_CLOCK();
    ENABLE_AFIOEN_CLOCK();

    GPIOC->RESET_CRH(13);
    GPIOC->CRH |= CRH_OUT_2_MHZ(13);

    GPIOB->RESET_CRH(8);
    GPIOB->CRH |= CRH_OUT_2_MHZ(8);

    GPIOB->RESET_CRH(9);
    GPIOB->CRH |= CRH_IN_PULL(9);
    GPIOB->PULL_UP(9);

    AFIO->EXTICR[2] &= ~(0xFU << 4);
    AFIO->EXTICR[2] |=  (0x1U << 4);
    EXTI->IMR  |=  (1U << 9);
    EXTI->RTSR &= ~(1U << 9);
    EXTI->FTSR |=  (1U << 9);
    EXTI->PR    =  (1U << 9);

    NVIC_SetPriority(EXTI9_5_IRQn, 5);
    NVIC_EnableIRQ(EXTI9_5_IRQn);


    while (1) { /* empty */ }
}
