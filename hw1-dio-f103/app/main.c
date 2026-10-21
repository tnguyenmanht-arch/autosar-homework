/*
 * main.c — Chương trình tạm cho HW1: chỉ cấu hình clock 72 MHz rồi đứng yên.
 * Khi có driver Dio, phần gọi Dio sẽ được thêm vào đây.
 */
#include "clock_f103.h"

int main(void)
{
    /* Bỏ qua mã lỗi: lỗi thì clock_init tự quay về HSI 8 MHz, chip vẫn chạy */
    (void)clock_init();

    while (1)
    {
    }
}
