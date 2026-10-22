/*
 * Dio.c — Module Dio (AUTOSAR MCAL, SWS_DIO) cho STM32F103, dùng thanh ghi CMSIS, không HAL
 *
 * Thanh ghi GPIO của F103 dùng ở đây:
 *   CRL/CRH : cấu hình chân 0–7 / 8–15, mỗi chân 4 bit = CNF[1:0] MODE[1:0]. MODE = 00 là input.
 *   IDR     : mức thực tế trên chân (đọc).
 *   ODR     : mức muốn xuất. Với chân input pull, bit ODR chọn pull-up (1) hay pull-down (0).
 *   BSRR    : ghi 1 vào bit i (0–15) để đặt ODR[i] = 1, bit i+16 để xoá ODR[i] = 0.
 *             Một lần ghi là xong, không đọc–sửa–ghi ODR nên không bị ngắt chen giữa làm hỏng.
 *
 * Quy tắc chung của các hàm ghi: chân đang là input (MODE = 00) thì KHÔNG ghi gì,
 * vì ghi ODR của chân input sẽ đổi pull-up thành pull-down.
 */
#include "Dio.h"
#include "stm32f1xx.h"

#if (DIO_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif

#define DIO_INSTANCE_ID       (0U)      /* chỉ có một bản Dio */
#define DIO_NUM_PORTS         (5U)      /* A..E */
#define DIO_MAX_CHANNEL_ID    (0x4FU)   /* cổng E (4), chân 15 */

/* Tách mã kênh thành cổng và chân */
#define DIO_PORT_OF(ch)       ((Dio_PortType)((ch) >> 4U))
#define DIO_PIN_OF(ch)        ((uint8)((ch) & 0x0FU))

/* Địa chỉ khối thanh ghi của từng cổng, đánh số theo Dio_PortType */
static GPIO_TypeDef * const Dio_GpioBase[DIO_NUM_PORTS] =
{
    GPIOA, GPIOB, GPIOC, GPIOD, GPIOE
};

/* ===================== Hàm nội bộ ===================== */

/* TRUE nếu chân đang là output: 2 bit MODE trong CRL (chân 0–7) hoặc CRH (chân 8–15) khác 00 */
static boolean Dio_IsOutputPin(const GPIO_TypeDef *gpio, uint8 pin)
{
    uint32 cr    = (pin < 8U) ? gpio->CRL : gpio->CRH;
    uint32 shift = ((uint32)pin & 0x7U) * 4U;

    return (((cr >> shift) & 0x3U) != 0U) ? TRUE : FALSE;
}

/* Mặt nạ chân output của cả cổng: bit i = 1 nếu chân i là output */
static uint32 Dio_OutputMask(const GPIO_TypeDef *gpio)
{
    uint32 mask = 0U;
    uint8  pin;

    for (pin = 0U; pin < 16U; pin++)
    {
        if (Dio_IsOutputPin(gpio, pin) == TRUE)
        {
            mask |= ((uint32)1U << pin);
        }
    }
    return mask;
}

#if (DIO_DEV_ERROR_DETECT == STD_ON)
/* Nhóm hợp lệ: khác NULL và trùng địa chỉ một phần tử trong bảng Dio_ChannelGroupConfig */
static boolean Dio_IsValidGroup(const Dio_ChannelGroupType *group)
{
    uint8 i;

    if (group == NULL_PTR)
    {
        return FALSE;
    }
    for (i = 0U; i < DIO_NUM_CHANNEL_GROUPS; i++)
    {
        if (group == &Dio_ChannelGroupConfig[i])
        {
            return TRUE;
        }
    }
    return FALSE;
}

/* Gửi lỗi của module Dio về Det */
static void Dio_ReportError(uint8 ApiId, uint8 ErrorId)
{
    (void)Det_ReportError(DIO_MODULE_ID, DIO_INSTANCE_ID, ApiId, ErrorId);
}
#endif

/* ===================== API ===================== */

Dio_LevelType Dio_ReadChannel(Dio_ChannelType ChannelId)
{
#if (DIO_DEV_ERROR_DETECT == STD_ON)
    if (ChannelId > DIO_MAX_CHANNEL_ID)
    {
        Dio_ReportError(DIO_READCHANNEL_SID, DIO_E_PARAM_INVALID_CHANNEL_ID);
        return STD_LOW;
    }
#endif

    /* Đọc IDR: mức thật trên chân, đúng cho cả chân input lẫn output */
    if ((Dio_GpioBase[DIO_PORT_OF(ChannelId)]->IDR & ((uint32)1U << DIO_PIN_OF(ChannelId))) != 0U)
    {
        return STD_HIGH;
    }
    return STD_LOW;
}

void Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level)
{
    GPIO_TypeDef *gpio;
    uint8 pin;

#if (DIO_DEV_ERROR_DETECT == STD_ON)
    if (ChannelId > DIO_MAX_CHANNEL_ID)
    {
        Dio_ReportError(DIO_WRITECHANNEL_SID, DIO_E_PARAM_INVALID_CHANNEL_ID);
        return;
    }
#endif

    gpio = Dio_GpioBase[DIO_PORT_OF(ChannelId)];
    pin  = DIO_PIN_OF(ChannelId);

    /* Chân input: không ghi, giữ nguyên pull-up/pull-down */
    if (Dio_IsOutputPin(gpio, pin) == FALSE)
    {
        return;
    }

    /* STD_HIGH → bit đặt (nửa thấp BSRR); giá trị khác → bit xoá (nửa cao) */
    if (Level == STD_HIGH)
    {
        gpio->BSRR = (uint32)1U << pin;
    }
    else
    {
        gpio->BSRR = (uint32)1U << (pin + 16U);
    }
}

Dio_PortLevelType Dio_ReadPort(Dio_PortType PortId)
{
#if (DIO_DEV_ERROR_DETECT == STD_ON)
    if (PortId >= DIO_NUM_PORTS)
    {
        Dio_ReportError(DIO_READPORT_SID, DIO_E_PARAM_INVALID_PORT_ID);
        return 0U;
    }
#endif

    /* IDR chỉ có 16 bit thấp là dữ liệu */
    return (Dio_PortLevelType)(Dio_GpioBase[PortId]->IDR & 0xFFFFU);
}

void Dio_WritePort(Dio_PortType PortId, Dio_PortLevelType Level)
{
    GPIO_TypeDef *gpio;
    uint32 outMask;
    uint32 setBits;
    uint32 clrBits;

#if (DIO_DEV_ERROR_DETECT == STD_ON)
    if (PortId >= DIO_NUM_PORTS)
    {
        Dio_ReportError(DIO_WRITEPORT_SID, DIO_E_PARAM_INVALID_PORT_ID);
        return;
    }
#endif

    gpio    = Dio_GpioBase[PortId];
    outMask = Dio_OutputMask(gpio);                 /* chỉ đụng chân output */
    setBits = (uint32)Level & outMask;              /* chân cần lên 1 */
    clrBits = (~(uint32)Level) & outMask;           /* chân cần xuống 0 */

    gpio->BSRR = setBits | (clrBits << 16U);        /* một lần ghi cho cả cổng */
}

Dio_PortLevelType Dio_ReadChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr)
{
#if (DIO_DEV_ERROR_DETECT == STD_ON)
    if (Dio_IsValidGroup(ChannelGroupIdPtr) == FALSE)
    {
        Dio_ReportError(DIO_READCHANNELGROUP_SID, DIO_E_PARAM_INVALID_GROUP);
        return 0U;
    }
#endif

    /* Lấy các bit của nhóm rồi dồn về bit 0 */
    return (Dio_PortLevelType)((Dio_GpioBase[ChannelGroupIdPtr->port]->IDR & ChannelGroupIdPtr->mask)
                               >> ChannelGroupIdPtr->offset);
}

void Dio_WriteChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr,
                           Dio_PortLevelType Level)
{
    GPIO_TypeDef *gpio;
    uint32 mask;
    uint32 setBits;
    uint32 clrBits;

#if (DIO_DEV_ERROR_DETECT == STD_ON)
    if (Dio_IsValidGroup(ChannelGroupIdPtr) == FALSE)
    {
        Dio_ReportError(DIO_WRITECHANNELGROUP_SID, DIO_E_PARAM_INVALID_GROUP);
        return;
    }
#endif

    gpio    = Dio_GpioBase[ChannelGroupIdPtr->port];
    mask    = (uint32)ChannelGroupIdPtr->mask & Dio_OutputMask(gpio);   /* bit của nhóm, chỉ chân output */
    setBits = ((uint32)Level << ChannelGroupIdPtr->offset) & mask;
    clrBits = (~((uint32)Level << ChannelGroupIdPtr->offset)) & mask;

    /* Bit ngoài nhóm = 0 ở cả hai nửa BSRR nên không đổi */
    gpio->BSRR = setBits | (clrBits << 16U);
}

#if (DIO_FLIP_CHANNEL_API == STD_ON)
Dio_LevelType Dio_FlipChannel(Dio_ChannelType ChannelId)
{
    GPIO_TypeDef *gpio;
    uint8 pin;
    uint32 bit;

#if (DIO_DEV_ERROR_DETECT == STD_ON)
    if (ChannelId > DIO_MAX_CHANNEL_ID)
    {
        Dio_ReportError(DIO_FLIPCHANNEL_SID, DIO_E_PARAM_INVALID_CHANNEL_ID);
        return STD_LOW;
    }
#endif

    gpio = Dio_GpioBase[DIO_PORT_OF(ChannelId)];
    pin  = DIO_PIN_OF(ChannelId);
    bit  = (uint32)1U << pin;

    /* Chân input: không đổi gì, trả mức thật đang đọc được */
    if (Dio_IsOutputPin(gpio, pin) == FALSE)
    {
        return ((gpio->IDR & bit) != 0U) ? STD_HIGH : STD_LOW;
    }

    /* Chân output: xem ODR đang xuất gì rồi ghi mức ngược lại */
    if ((gpio->ODR & bit) != 0U)
    {
        gpio->BSRR = bit << 16U;        /* đang 1 → xoá về 0 */
        return STD_LOW;
    }
    gpio->BSRR = bit;                   /* đang 0 → đặt lên 1 */
    return STD_HIGH;
}
#endif

#if (DIO_VERSION_INFO_API == STD_ON)
void Dio_GetVersionInfo(Std_VersionInfoType* versioninfo)
{
#if (DIO_DEV_ERROR_DETECT == STD_ON)
    if (versioninfo == NULL_PTR)
    {
        Dio_ReportError(DIO_GETVERSIONINFO_SID, DIO_E_PARAM_POINTER);
        return;
    }
#endif

    versioninfo->vendorID         = DIO_VENDOR_ID;
    versioninfo->moduleID         = DIO_MODULE_ID;
    versioninfo->sw_major_version = DIO_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = DIO_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = DIO_SW_PATCH_VERSION;
}
#endif
