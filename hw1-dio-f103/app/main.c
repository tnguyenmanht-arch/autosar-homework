/*
 * main.c — Chạy thử module Dio trên board Yahboom YB-MSV01 (STM32F103C8)
 *
 * Kết quả nằm trong các biến g_* và Det_*, đọc qua SWD bằng STM32_Programmer_CLI (mode=HOTPLUG)
 * hoặc Live Expressions trong CubeIDE. Bảng TC1–TC8 và cách đọc: xem README.md.
 *
 * g_tc[i] là kết quả của TC(i+1): 0 = chưa chạy, 1 = ĐẠT, 2 = KHÔNG ĐẠT.
 *   TC1, TC4..TC8 chạy một lần ngay sau khởi động.
 *   TC3 (nháy LED) và TC2 (đọc nút) chạy liên tục trong vòng lặp chính.
 */
#include "stm32f1xx.h"
#include "clock_f103.h"
#include "Port.h"
#include "Dio.h"
#include "Det.h"

#define TC_NOT_RUN   (0U)
#define TC_PASS      (1U)
#define TC_FAIL      (2U)

#define LED_PIN      (13U)   /* PC13 */
#define BUTTON_PIN   (0U)    /* PA0 */
#define TC3_PERIOD_MS (500U)

/* Bit thứ pin của ODR, và 4 bit CNF/MODE của chân 0–7 trong CRL */
#define ODR_BIT(gpio, pin)      (((gpio)->ODR >> (pin)) & 1U)
#define CRL_NIBBLE(gpio, pin)   (((gpio)->CRL >> ((pin) * 4U)) & 0xFU)

/* ===== Đồng hồ mili-giây (giống GĐ 4 của đồ án) ===== */
volatile uint32 g_tick_ms = 0;

void SysTick_Handler(void)
{
    g_tick_ms++;                       /* ngắt mỗi 1 ms */
}

/* ===== Kết quả, đọc qua SWD ===== */
volatile uint8  g_tc[8];               /* g_tc[0] = TC1 ... g_tc[7] = TC8 */
volatile uint8  g_clk_err;             /* clock_init(): 0 OK, 1 HSE, 2 PLL, 3 chuyển SW */

/* Biến phụ để chẩn đoán khi một TC không đạt */
volatile uint8  g_tc1_odr_low;         /* ODR bit 13 sau khi ghi LOW, mong đợi 0 */
volatile uint8  g_tc1_odr_high;        /* ODR bit 13 sau khi ghi HIGH, mong đợi 1 */
volatile uint8  g_tc2_level;           /* mức PA0 đọc liên tục: HIGH khi hở, LOW khi nối GND */
volatile uint8  g_tc2_seen_high;       /* đã từng đọc được HIGH */
volatile uint8  g_tc2_seen_low;        /* đã từng đọc được LOW */
volatile uint32 g_tc3_flips;           /* số lần đã gọi FlipChannel */
volatile uint32 g_tc3_errors;          /* số lần giá trị trả về khác ODR */
volatile uint8  g_tc3_level;           /* mức mới lần flip gần nhất */
volatile uint8  g_tc4_level;           /* đọc PA0 sau WriteChannel(Button, LOW), mong đợi HIGH */
volatile uint32 g_tc5_odr_before;      /* GPIOA->ODR trước/sau WritePort */
volatile uint32 g_tc5_odr_after;
volatile uint32 g_tc6_odr_before;      /* GPIOA->ODR trước/sau WriteChannelGroup */
volatile uint32 g_tc6_odr_after;
volatile uint16 g_tc6_read;            /* Dio_ReadChannelGroup sau khi ghi, mong đợi 0xA */
volatile uint8  g_tc7_fail_mask;       /* bit k = 1: bước k+1 của TC7 không đạt */
Std_VersionInfoType g_tc8_info;        /* Dio_GetVersionInfo điền vào đây */

/* Đổi điều kiện đúng/sai thành mã kết quả */
static uint8 tc_result(boolean ok)
{
    return (ok == TRUE) ? TC_PASS : TC_FAIL;
}

/* TC7: sau một lần gọi sai, Det phải nhận đúng 1 lỗi mới với đúng module, instance, SID, mã lỗi */
static boolean det_expect(uint32 count_before, uint8 api_id, uint8 error_id)
{
    if ((Det_ErrorCount == (count_before + 1U))
        && (Det_LastError.ModuleId == DIO_MODULE_ID)
        && (Det_LastError.InstanceId == 0U)
        && (Det_LastError.ApiId == api_id)
        && (Det_LastError.ErrorId == error_id))
    {
        return TRUE;
    }
    return FALSE;
}

/* TC1: ghi kênh LED LOW rồi HIGH, kiểm bằng ODR */
static void tc1_write_channel(void)
{
    Dio_WriteChannel(DioConf_DioChannel_LedBoard, STD_LOW);
    g_tc1_odr_low = (uint8)ODR_BIT(GPIOC, LED_PIN);

    Dio_WriteChannel(DioConf_DioChannel_LedBoard, STD_HIGH);     /* để LED tắt trước TC3 */
    g_tc1_odr_high = (uint8)ODR_BIT(GPIOC, LED_PIN);

    g_tc[0] = tc_result(((g_tc1_odr_low == 0U) && (g_tc1_odr_high == 1U)) ? TRUE : FALSE);
}

/* TC4: ghi LOW vào chân input PA0 → không được ghi gì, pull-up giữ nguyên, đọc vẫn HIGH (khi hở) */
static void tc4_write_input_channel(void)
{
    Dio_WriteChannel(DioConf_DioChannel_Button, STD_LOW);
    g_tc4_level = Dio_ReadChannel(DioConf_DioChannel_Button);

    g_tc[3] = tc_result(((ODR_BIT(GPIOA, BUTTON_PIN) == 1U)                      /* vẫn chọn pull-up */
                         && (CRL_NIBBLE(GPIOA, BUTTON_PIN) == PORT_MODE_INPUT_PULL) /* vẫn là input pull */
                         && (g_tc4_level == STD_HIGH)) ? TRUE : FALSE);
}

/* TC5: WritePort cổng A với Level = 0x0050 (bit 0 = 0).
   PA4–PA7 (output) ra 0101; PA0 (input) và mọi chân input khác giữ nguyên ODR, PA0 vẫn input pull-up. */
static void tc5_write_port(void)
{
    g_tc5_odr_before = GPIOA->ODR;
    Dio_WritePort(DioConf_DioPort_A, 0x0050U);
    g_tc5_odr_after = GPIOA->ODR;

    g_tc[4] = tc_result((((g_tc5_odr_after & 0x00F0U) == 0x0050U)
                         && ((g_tc5_odr_after & ~0x00F0UL) == (g_tc5_odr_before & ~0x00F0UL))
                         && ((g_tc5_odr_after & 0x0001U) == 0x0001U)
                         && (CRL_NIBBLE(GPIOA, BUTTON_PIN) == PORT_MODE_INPUT_PULL)) ? TRUE : FALSE);
}

/* TC6: ghi nhóm Test = 0b1010 → PA7..PA4 = 1,0,1,0 (ODR bit 7..4 = 0xA0), bit ngoài nhóm không đổi */
static void tc6_write_group(void)
{
    g_tc6_odr_before = GPIOA->ODR;
    Dio_WriteChannelGroup(DioConf_DioChannelGroup_Test, 0x000AU);
    g_tc6_odr_after = GPIOA->ODR;
    g_tc6_read = Dio_ReadChannelGroup(DioConf_DioChannelGroup_Test);   /* chẩn đoán, không tính điểm */

    g_tc[5] = tc_result((((g_tc6_odr_after & 0x00F0U) == 0x00A0U)
                         && ((g_tc6_odr_after & ~0x00F0UL) == (g_tc6_odr_before & ~0x00F0UL))) ? TRUE : FALSE);
}

/* TC7: gọi sai tham số → Det nhận đúng lỗi, hàm Read trả 0/STD_LOW, chương trình chạy tiếp */
static void tc7_det(void)
{
    /* Bản sao của nhóm Test: nội dung giống hệt nhưng địa chỉ không nằm trong bảng cấu hình */
    const Dio_ChannelGroupType fake_group = { 0x00F0U, 4U, DioConf_DioPort_A };
    uint32 n;
    uint8  fail = 0U;

    /* Bước 1: mã kênh 0xFF (cổng 15 không tồn tại) */
    n = Det_ErrorCount;
    if ((Dio_ReadChannel(0xFFU) != STD_LOW)
        || (det_expect(n, DIO_READCHANNEL_SID, DIO_E_PARAM_INVALID_CHANNEL_ID) == FALSE))
    {
        fail |= 0x01U;
    }

    /* Bước 2: mã cổng 9 */
    n = Det_ErrorCount;
    if ((Dio_ReadPort(9U) != 0U)
        || (det_expect(n, DIO_READPORT_SID, DIO_E_PARAM_INVALID_PORT_ID) == FALSE))
    {
        fail |= 0x02U;
    }

    /* Bước 3: nhóm NULL */
    n = Det_ErrorCount;
    if ((Dio_ReadChannelGroup(NULL_PTR) != 0U)
        || (det_expect(n, DIO_READCHANNELGROUP_SID, DIO_E_PARAM_INVALID_GROUP) == FALSE))
    {
        fail |= 0x04U;
    }

    /* Bước 4: nhóm không có trong bảng cấu hình */
    n = Det_ErrorCount;
    if ((Dio_ReadChannelGroup(&fake_group) != 0U)
        || (det_expect(n, DIO_READCHANNELGROUP_SID, DIO_E_PARAM_INVALID_GROUP) == FALSE))
    {
        fail |= 0x08U;
    }

    /* Bước 5: GetVersionInfo với con trỏ NULL */
    n = Det_ErrorCount;
    Dio_GetVersionInfo(NULL_PTR);
    if (det_expect(n, DIO_GETVERSIONINFO_SID, DIO_E_PARAM_POINTER) == FALSE)
    {
        fail |= 0x10U;
    }

    g_tc7_fail_mask = fail;
    g_tc[6] = tc_result((fail == 0U) ? TRUE : FALSE);   /* tới được đây = không treo */
}

/* TC8: GetVersionInfo điền đúng 5 trường, không báo lỗi Det */
static void tc8_version_info(void)
{
    uint32 n = Det_ErrorCount;

    Dio_GetVersionInfo(&g_tc8_info);

    g_tc[7] = tc_result(((g_tc8_info.vendorID == DIO_VENDOR_ID)
                         && (g_tc8_info.moduleID == DIO_MODULE_ID)
                         && (g_tc8_info.sw_major_version == DIO_SW_MAJOR_VERSION)
                         && (g_tc8_info.sw_minor_version == DIO_SW_MINOR_VERSION)
                         && (g_tc8_info.sw_patch_version == DIO_SW_PATCH_VERSION)
                         && (Det_ErrorCount == n)) ? TRUE : FALSE);
}

/* TC2: đọc nút liên tục. Đạt khi đã thấy cả HIGH (hở) lẫn LOW (nối PA0–GND) */
static void tc2_read_button(void)
{
    g_tc2_level = Dio_ReadChannel(DioConf_DioChannel_Button);

    if (g_tc2_level == STD_HIGH)
    {
        g_tc2_seen_high = 1U;
    }
    else
    {
        g_tc2_seen_low = 1U;
    }

    if ((g_tc2_seen_high == 1U) && (g_tc2_seen_low == 1U))
    {
        g_tc[1] = TC_PASS;
    }
}

/* TC3: đảo LED, giá trị trả về phải khớp ODR. Đạt sau 2 lần đảo không sai; sai 1 lần là KHÔNG ĐẠT */
static void tc3_flip_led(void)
{
    g_tc3_level = Dio_FlipChannel(DioConf_DioChannel_LedBoard);
    g_tc3_flips++;

    if (g_tc3_level != (uint8)ODR_BIT(GPIOC, LED_PIN))
    {
        g_tc3_errors++;
    }

    if (g_tc3_errors != 0U)
    {
        g_tc[2] = TC_FAIL;
    }
    else if (g_tc3_flips >= 2U)
    {
        g_tc[2] = TC_PASS;
    }
    else
    {
        /* mới đảo 1 lần: chưa kết luận */
    }
}

int main(void)
{
    uint32 t_last;

    /* 72 MHz từ HSE. Lỗi thì vẫn chạy tiếp ở HSI 8 MHz để còn đọc kết quả */
    g_clk_err = clock_init();

    SystemCoreClockUpdate();                   /* SystemCoreClock = tần số CPU (Hz) */
    SysTick_Config(SystemCoreClock / 1000U);   /* ngắt mỗi 1 ms */

    /* Bật clock cổng, đặt mức ban đầu, cấu hình PC13, PA0, PA4–PA7 */
    Port_Init(&Port_Config);

    /* Các TC chạy một lần */
    tc1_write_channel();
    tc4_write_input_channel();
    tc5_write_port();
    tc6_write_group();
    tc7_det();
    tc8_version_info();

    /* Vòng lặp chính: TC2 liên tục, TC3 mỗi 500 ms (lịch cố định, không trôi) */
    t_last = g_tick_ms;
    while (1)
    {
        tc2_read_button();

        if ((g_tick_ms - t_last) >= TC3_PERIOD_MS)
        {
            t_last += TC3_PERIOD_MS;
            tc3_flip_led();
        }
    }
}
