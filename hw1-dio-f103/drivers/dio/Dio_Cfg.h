/*
 * Dio_Cfg.h — Cấu hình module Dio cho board Yahboom YB-MSV01 (STM32F103C8)
 *
 * File này được Dio.h include SAU phần khai báo kiểu, nên dùng được Dio_ChannelGroupType.
 * Không include trực tiếp file này: luôn include "Dio.h".
 *
 * Mã kênh = (cổng << 4) | chân, cổng A=0 ... E=4, chân 0..15.
 * Chân cấm dùng (xem CLAUDE.md): PA9..PA15, PB2, PB3, PB4, PB8, PB9.
 */
#ifndef DIO_CFG_H
#define DIO_CFG_H

#include "Std_Types.h"

/* ---------- 1. Công tắc biên dịch ---------- */
#define DIO_DEV_ERROR_DETECT     STD_ON    /* kiểm tham số, báo lỗi về Det */
#define DIO_FLIP_CHANNEL_API     STD_ON    /* có hàm Dio_FlipChannel */
#define DIO_VERSION_INFO_API     STD_ON    /* có hàm Dio_GetVersionInfo */

/* ---------- 2. Tên cổng (Dio_PortType) ---------- */
#define DioConf_DioPort_A        (0U)
#define DioConf_DioPort_B        (1U)
#define DioConf_DioPort_C        (2U)
#define DioConf_DioPort_D        (3U)
#define DioConf_DioPort_E        (4U)

/* ---------- 3. Tên kênh (Dio_ChannelType) ---------- */
#define DioConf_DioChannel_LedBoard   (0x2DU)   /* PC13 = (2 << 4) | 13, ra, LED sáng khi LOW */
#define DioConf_DioChannel_Button     (0x00U)   /* PA0  = (0 << 4) | 0,  vào, pull-up; thử bằng dây PA0–GND */

/* ---------- 4. Nhóm kênh ---------- */
/* Bảng nhóm nằm trong Dio_Cfg.c. Tên nhóm là con trỏ tới phần tử của bảng,
   nên Dio.c kiểm được nhóm hợp lệ bằng cách so địa chỉ. */
#define DIO_NUM_CHANNEL_GROUPS   (1U)

extern const Dio_ChannelGroupType Dio_ChannelGroupConfig[DIO_NUM_CHANNEL_GROUPS];

#define DioConf_DioChannelGroup_Test  (&Dio_ChannelGroupConfig[0])   /* PA4–PA7, ra */

#endif /* DIO_CFG_H */
