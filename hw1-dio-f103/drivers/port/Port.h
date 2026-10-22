/*
 * Port.h — Module Port TỐI GIẢN cho STM32F103 (chỉ đủ để Dio chạy)
 *
 * KHÔNG đầy đủ theo SWS_Port: chỉ có Port_Init. Không có Port_SetPinDirection,
 * Port_SetPinMode, Port_RefreshPortDirection, Port_GetVersionInfo, không báo lỗi về Det.
 * Port_Init chỉnh từng chân có trong bảng cấu hình, chân khác giữ nguyên trạng thái sau reset.
 */
#ifndef PORT_H
#define PORT_H

#include "Std_Types.h"

/* Mã cổng, cùng cách đánh số với Dio_PortType */
#define PORT_ID_A    (0U)
#define PORT_ID_B    (1U)
#define PORT_ID_C    (2U)
#define PORT_ID_D    (3U)
#define PORT_ID_E    (4U)

/* Giá trị 4 bit CNF[1:0] MODE[1:0] ghi vào CRL/CRH */
#define PORT_MODE_OUTPUT_PP_2MHZ   (0x2U)   /* CNF = 00 push-pull, MODE = 10 output 2 MHz */
#define PORT_MODE_INPUT_PULL       (0x8U)   /* CNF = 10 pull-up/pull-down, MODE = 00 input */

/* Cấu hình một chân */
typedef struct
{
    uint8 port;     /* PORT_ID_A .. PORT_ID_E */
    uint8 pin;      /* 0..15 */
    uint8 mode;     /* PORT_MODE_... */
    uint8 level;    /* output: mức ban đầu; input pull: STD_HIGH = pull-up, STD_LOW = pull-down */
} Port_PinConfigType;

/* Cấu hình cả module: danh sách chân */
typedef struct
{
    const Port_PinConfigType *pins;
    uint8                     numPins;
} Port_ConfigType;

/* Đặt SAU phần kiểu: Port_Cfg.h khai báo Port_Config, cần kiểu Port_ConfigType */
#include "Port_Cfg.h"

void Port_Init(const Port_ConfigType *ConfigPtr);

#endif /* PORT_H */
