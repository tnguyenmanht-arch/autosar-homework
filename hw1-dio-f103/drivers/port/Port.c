/*
 * Port.c — Module Port TỐI GIẢN cho STM32F103 (chỉ đủ để Dio chạy, không đầy đủ theo SWS_Port)
 */
#include "Port.h"
#include "stm32f1xx.h"

#define PORT_NUM_PORTS   (5U)   /* A..E */

/* Địa chỉ khối thanh ghi của từng cổng, đánh số theo PORT_ID_x */
static GPIO_TypeDef * const Port_GpioBase[PORT_NUM_PORTS] =
{
    GPIOA, GPIOB, GPIOC, GPIOD, GPIOE
};

void Port_Init(const Port_ConfigType *ConfigPtr)
{
    uint8 i;

    if (ConfigPtr == NULL_PTR)
    {
        return;                                  /* bản tối giản: không báo Det */
    }

    for (i = 0U; i < ConfigPtr->numPins; i++)
    {
        const Port_PinConfigType *cfg = &ConfigPtr->pins[i];
        GPIO_TypeDef *gpio;
        volatile uint32 *cr;
        uint32 shift;

        if ((cfg->port >= PORT_NUM_PORTS) || (cfg->pin > 15U))
        {
            continue;                            /* dòng cấu hình sai: bỏ qua */
        }
        gpio = Port_GpioBase[cfg->port];

        /* ① Bật clock cổng: IOPAEN..IOPEEN là 5 bit liền nhau (bit 2..6) trong RCC_APB2ENR.
           Đọc lại một lần để chắc clock đã bật trước khi ghi thanh ghi GPIO. */
        RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN << cfg->port);
        (void)RCC->APB2ENR;

        /* ② Mức ban đầu trong ODR TRƯỚC khi đổi chế độ chân:
           - output: chân ra đúng mức ngay khi thành output, không bị chớp mức sai;
           - input pull: ODR chọn pull-up (1) hay pull-down (0). */
        if (cfg->level == STD_HIGH)
        {
            gpio->BSRR = (uint32)1U << cfg->pin;
        }
        else
        {
            gpio->BSRR = (uint32)1U << (cfg->pin + 16U);
        }

        /* ③ Ghi 4 bit CNF/MODE: chân 0–7 ở CRL, chân 8–15 ở CRH */
        cr    = (cfg->pin < 8U) ? &gpio->CRL : &gpio->CRH;
        shift = ((uint32)cfg->pin & 0x7U) * 4U;
        *cr   = (*cr & ~((uint32)0xFU << shift)) | ((uint32)cfg->mode << shift);
    }
}
