/*
 * sim.cpp — Chạy thử Dio/Port/Det và các hàm TC của app/main.c trên PC, với thanh ghi giả lập
 *
 * Dùng nguyên Dio.c, Dio_Cfg.c, Port.c, Port_Cfg.c, Det.c, main.c, không sửa dòng nào
 * (main.c được include để gọi các hàm tc*() static; hàm main của nó đổi tên và không gọi).
 * Không thay được thử trên board: không kiểm thời gian, clock, SysTick, điện thật trên chân.
 *
 * Chạy: make -C tests/host   (cần g++ trong PATH, ví dụ MinGW). Trả về mã 0 khi đạt hết.
 * Thông báo in ra không dấu để cmd hiển thị đúng ở mọi code page.
 */
#include <stdio.h>
#include <string.h>
#define main app_main_khong_goi
#include "main.c"          /* lấy nguyên các hàm tc*() của main.c */
#undef main

GPIO_TypeDef sim_gpio[5];
RCC_TypeDef sim_rcc;
unsigned long sim_bsrr_writes, sim_ext_mask[5], sim_ext_level[5];
unsigned long SystemCoreClock = 72000000UL;
uint8_t clock_init(void) { return 0; }

void BsrrReg::operator=(unsigned long v) {
    GPIO_TypeDef *g = &sim_gpio[port];
    g->ODR = (g->ODR & ~(v >> 16)) | (v & 0xFFFFUL);   /* bit đặt thắng bit xoá */
    sim_bsrr_writes++;
}
IdrReg::operator unsigned long() const {
    const GPIO_TypeDef *g = &sim_gpio[port];
    unsigned long r = 0;
    for (unsigned p = 0; p < 16; p++) {
        unsigned long cr = (p < 8) ? g->CRL : g->CRH, nib = (cr >> ((p & 7) * 4)) & 0xF, b = 1UL << p;
        unsigned long lvl;
        if ((nib & 3) != 0)                lvl = g->ODR & b;                 /* output: chân = ODR */
        else if (sim_ext_mask[port] & b)   lvl = sim_ext_level[port] & b;    /* dây nối ngoài */
        else if ((nib >> 2) == 2)          lvl = g->ODR & b;                 /* input pull: ODR chọn pull */
        else                               lvl = 0;                          /* floating */
        r |= lvl;
    }
    return r;
}

static int fails = 0;
#define CHECK(cond, msg) do { if (cond) printf("  OK    %s\n", msg); else { printf("  FAIL  %s\n", msg); fails++; } } while (0)

static void snap(unsigned long *s) { for (int i = 0; i < 5; i++) { s[3*i] = sim_gpio[i].CRL; s[3*i+1] = sim_gpio[i].CRH; s[3*i+2] = sim_gpio[i].ODR; } }

int main(void) {
    for (int i = 0; i < 5; i++) { sim_gpio[i].CRL = sim_gpio[i].CRH = 0x44444444UL; sim_gpio[i].IDR.port = sim_gpio[i].BSRR.port = i; }
    /* Trạng thái sau reset đọc được trên board 22/10/2026: PA13–PA15 (debug) là input pull,
       PA13, PA15 pull-up (ODR = 1), PA14 pull-down (ODR = 0) */
    GPIOA->CRH = 0x88844444UL;
    GPIOA->ODR = 0xA000UL;

    printf("Port_Init\n");
    Port_Init(&Port_Config);
    CHECK((sim_rcc.APB2ENR & 0x14UL) == 0x14UL, "RCC bat IOPA + IOPC");
    CHECK(((GPIOC->CRH >> 20) & 0xF) == 0x2, "PC13 = output PP 2 MHz");
    CHECK((GPIOC->ODR >> 13 & 1) == 1, "PC13 ODR = 1 (LED tat)");
    CHECK((GPIOA->CRL & 0xF) == 0x8 && (GPIOA->ODR & 1) == 1, "PA0 = input pull-up");
    CHECK((GPIOA->CRL & 0xFFFF0000UL) == 0x22220000UL, "PA4-PA7 = output PP 2 MHz");
    CHECK((GPIOA->CRH) == 0x88844444UL, "CRH cong A (PA8-PA15) khong doi");

    printf("TC cua main.c\n");
    tc1_write_channel(); tc4_write_input_channel(); tc5_write_port(); tc6_write_group(); tc7_det(); tc8_version_info();
    for (int i = 0; i < 3; i++) tc3_flip_led();
    tc2_read_button();
    sim_ext_mask[0] = 1; sim_ext_level[0] = 0;        /* noi PA0 xuong GND */
    tc2_read_button();
    unsigned exp[8] = {1,1,1,1,1,1,1,1};
    char buf[80];
    for (int i = 0; i < 8; i++) { sprintf(buf, "g_tc[%d] (TC%d) = %u", i, i + 1, g_tc[i]); CHECK(g_tc[i] == exp[i], buf); }
    printf("  g_tc5_odr 0x%04lX->0x%04lX  g_tc6_odr 0x%04lX->0x%04lX  g_tc6_read=0x%X  g_tc7_fail_mask=0x%02X  Det_ErrorCount=%lu\n",
           g_tc5_odr_before, g_tc5_odr_after, g_tc6_odr_before, g_tc6_odr_after, g_tc6_read, g_tc7_fail_mask, Det_ErrorCount);
    CHECK(g_tc6_read == 0xA, "ReadChannelGroup sau TC6 = 0xA");
    CHECK(g_tc5_odr_before == 0xA001UL && g_tc5_odr_after == 0xA051UL
          && g_tc6_odr_before == 0xA051UL && g_tc6_odr_after == 0xA0A1UL,
          "ODR TC5/TC6 khop board: 0xA001->0xA051, 0xA051->0xA0A1 (PA13/PA15 giu pull-up)");
    CHECK(Det_ErrorCount == 5 && Det_LastError.ModuleId == 120 && Det_LastError.InstanceId == 0
          && Det_LastError.ApiId == 0x12 && Det_LastError.ErrorId == 0x20,
          "sau TC7+TC8: Det_ErrorCount = 5, Det_LastError = {120,0,0x12,0x20} (buoc 5 GetVersionInfo(NULL) chay cuoi)");
    CHECK(g_tc2_level == STD_LOW, "TC2 doc LOW khi noi GND");
    sim_ext_mask[0] = 0;

    printf("Kiem them\n");
    unsigned long a[15], b[15];
    unsigned long n = Det_ErrorCount, w = sim_bsrr_writes;
    snap(a);
    Dio_WriteChannel(0x50U, STD_HIGH); Dio_WritePort(5U, 0xFFFFU);
    Dio_WriteChannelGroup(NULL_PTR, 0xFU); Dio_FlipChannel(0xFFU);
    { Dio_ChannelGroupType gia = *DioConf_DioChannelGroup_Test; Dio_WriteChannelGroup(&gia, 0xFU); }
    snap(b);
    CHECK(memcmp(a, b, sizeof a) == 0 && sim_bsrr_writes == w, "5 lan goi sai: khong cham thanh ghi nao");
    CHECK(Det_ErrorCount == n + 5, "5 lan goi sai: Det dem them 5");
    CHECK(Det_LastError.ApiId == DIO_WRITECHANNELGROUP_SID && Det_LastError.ErrorId == DIO_E_PARAM_INVALID_GROUP, "loi cuoi = WriteChannelGroup / INVALID_GROUP");

    unsigned long odrA = GPIOA->ODR;
    CHECK(Dio_FlipChannel(DioConf_DioChannel_Button) == STD_HIGH && GPIOA->ODR == odrA, "Flip chan input PA0: tra muc IDR, ODR khong doi");

    GPIOA->CRL = (GPIOA->CRL & ~0xF0UL) | 0x20UL;  /* PA1 thanh output de thu */
    GPIOA->ODR |= 2UL;
    w = sim_bsrr_writes;
    Dio_WriteChannelGroup(DioConf_DioChannelGroup_Test, 0xFFU);   /* bit thua ngoai nhom */
    CHECK((GPIOA->ODR & 0xF0) == 0xF0 && (GPIOA->ODR & 2) == 2 && (GPIOA->ODR & 1) == 1, "Nhom 0xFF: chi PA4-7 doi, PA1 (output ngoai nhom) giu 1");
    CHECK(sim_bsrr_writes == w + 1, "WriteChannelGroup = dung 1 lan ghi BSRR");
    w = sim_bsrr_writes;
    Dio_WritePort(DioConf_DioPort_A, 0x0000U);
    CHECK((GPIOA->ODR & 0xF2) == 0 && (GPIOA->ODR & 1) == 1, "WritePort 0: PA1, PA4-7 ve 0; PA0 (input) giu pull-up");
    CHECK(sim_bsrr_writes == w + 1, "WritePort = dung 1 lan ghi BSRR");
    CHECK(Dio_ReadPort(DioConf_DioPort_A) == 0xA001U, "ReadPort A = 0xA001 (PA0, PA13, PA15 pull-up len 1)");
    Dio_WriteChannel(DioConf_DioChannel_LedBoard, STD_LOW);
    CHECK(Dio_ReadChannel(DioConf_DioChannel_LedBoard) == STD_LOW, "ReadChannel LED sau khi ghi LOW");
    CHECK(Det_ErrorCount == n + 5, "cac lan goi dung khong bao loi Det");

    printf("\n%s: %d loi\n", fails ? "KHONG DAT" : "DAT", fails);
    return fails;
}
