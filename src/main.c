#include "stm32f1xx.h"

// Нужно для компиляции с newlib
void _init(void) {}

// Простая задержка (volatile, чтобы компилятор не оптимизировал)
void delay(volatile uint32_t n) {
    while (n--) {
        __asm__("nop");
    }
}

void SystemClock_Config(void) {
    // 1. Включаем HSE
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY));

    // 2. Настройка Flash latency
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2; // 2 wait states для 72MHz

    // 3. Настройка PLL (HSE 8 MHz * 9 = 72 MHz)
    RCC->CFGR &= ~RCC_CFGR_PLLSRC & ~RCC_CFGR_PLLMULL;  // сброс
    RCC->CFGR |= RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9;

    // 4. Делители шины
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;   // AHB = SYSCLK
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;  // APB1 = 36 MHz (таймеры x2)
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;  // APB2 = 72 MHz

    // 5. Включаем PLL и ждём готовности
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    // 6. Переключаем SYSCLK на PLL
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}

int main(void) {
    // Настраиваем тактирование на 72 MHz
    SystemClock_Config();

    // Включаем тактирование GPIOC
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13); // обнуляем
    GIOC->CRH |= GPIO_CRH_MODE13_1;

    while (1) {
        GPIOC->BSRR = GPIO_BSRR_BR13;
        // GPIOC->BSRR &= ~GPIO_BSRR_BR13;  // LED ON (PC13 низкий активный)
        delay(5000000);
        GPIOC->BSRR = GPIO_BSRR_BS13;
        // GPIOC->BSRR |= GPIO_BSRR_BR13;   // LED OFF
        delay(1000000);
    }
}
