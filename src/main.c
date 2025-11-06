#include "stm32f1xx.h"

// Need that for some reason
void _init(){}

void main(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    GPIOC->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13);
    GPIOC->CRH |= GPIO_CRH_MODE13_1;

    while (1)
    {
        GPIOC->ODR |= GPIO_ODR_ODR13;
        for (int i = 0; i < 500000; i++)
            ; // arbitrary delay
        GPIOC->ODR &= ~GPIO_ODR_ODR13;
        for (int i = 0; i < 100000; i++)
            ; // arbitrary delay
    }
}
