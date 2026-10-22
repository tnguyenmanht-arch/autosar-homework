/*
 * stm32f1xx.h (BẢN GIẢ LẬP) — thay header CMSIS khi chạy thử Dio/Port/Det trên PC
 *
 * Chỉ dùng cho tests/host, biên dịch bằng g++ (C++). Firmware không bao giờ thấy file này
 * vì Makefile firmware không có -Itests/host.
 *
 * Giả lập:
 *   BSRR : ghi vào là cập nhật ODR ngay (bit đặt thắng bit xoá) và đếm số lần ghi.
 *   IDR  : đọc ra mức chân tính từ CRL/CRH, ODR và dây nối ngoài (sim_ext_mask/sim_ext_level):
 *          output → bằng ODR; input có dây ngoài → mức dây; input pull → ODR chọn pull; floating → 0.
 */
#ifndef SIM_STM32F1XX_H
#define SIM_STM32F1XX_H

struct BsrrReg { int port; void operator=(unsigned long v); };
struct IdrReg  { int port; operator unsigned long() const; };

struct GPIO_TypeDef
{
    volatile unsigned long CRL, CRH;   /* unsigned long để khớp uint32 của Platform_Types.h */
    IdrReg IDR;
    volatile unsigned long ODR;
    BsrrReg BSRR;
};

struct RCC_TypeDef { volatile unsigned long APB2ENR; };

extern GPIO_TypeDef  sim_gpio[5];
extern RCC_TypeDef   sim_rcc;
extern unsigned long sim_bsrr_writes;                 /* tổng số lần ghi BSRR */
extern unsigned long sim_ext_mask[5], sim_ext_level[5];   /* chân nào có dây ngoài, mức của dây */

#define GPIOA (&sim_gpio[0])
#define GPIOB (&sim_gpio[1])
#define GPIOC (&sim_gpio[2])
#define GPIOD (&sim_gpio[3])
#define GPIOE (&sim_gpio[4])
#define RCC   (&sim_rcc)
#define RCC_APB2ENR_IOPAEN (1UL << 2)

/* Phần main.c dùng tới */
extern unsigned long SystemCoreClock;
inline void SystemCoreClockUpdate(void) {}
inline unsigned long SysTick_Config(unsigned long) { return 0; }

#endif /* SIM_STM32F1XX_H */
