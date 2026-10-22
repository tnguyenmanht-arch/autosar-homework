/* ============================================================
 * Dio.h — Giao diện module Dio (AUTOSAR MCAL) cho STM32F103
 * Vị trí: hw1-dio-f103/drivers/dio/Dio.h
 * Được include bởi: Dio.c (định nghĩa hàm), main.c (gọi hàm)
 * ============================================================ */
#ifndef DIO_H
#define DIO_H

#include "Std_Types.h"   /* uint8, uint16, STD_ON, Std_VersionInfoType */

/* ---------- 1. Thông tin phiên bản (Dio_GetVersionInfo trả về) ---------- */
#define DIO_VENDOR_ID           (29U)    /* ai viết module: tự chọn */
#define DIO_MODULE_ID           (120U)   /* loại module: số AUTOSAR cấp cho Dio */
#define DIO_SW_MAJOR_VERSION    (1U)
#define DIO_SW_MINOR_VERSION    (0U)
#define DIO_SW_PATCH_VERSION    (0U)

/* ---------- 2. Service ID: hàm nào báo lỗi (gửi cho Det_ReportError) ---------- */
#define DIO_READCHANNEL_SID           (0x00U)
#define DIO_WRITECHANNEL_SID          (0x01U)
#define DIO_READPORT_SID              (0x02U)
#define DIO_WRITEPORT_SID             (0x03U)
#define DIO_READCHANNELGROUP_SID      (0x04U)
#define DIO_WRITECHANNELGROUP_SID     (0x05U)
#define DIO_FLIPCHANNEL_SID           (0x11U)
#define DIO_GETVERSIONINFO_SID        (0x12U)

/* ---------- 3. Mã lỗi DET: lỗi gì (gửi cho Det_ReportError) ---------- */
#define DIO_E_PARAM_INVALID_CHANNEL_ID   (0x0AU)   /* mã kênh sai */
#define DIO_E_PARAM_INVALID_PORT_ID      (0x14U)   /* mã cổng sai */
#define DIO_E_PARAM_INVALID_GROUP        (0x1FU)   /* nhóm NULL hoặc không có trong cấu hình */
#define DIO_E_PARAM_POINTER              (0x20U)   /* con trỏ kết quả là NULL */

/* ---------- 4. Kiểu dữ liệu ---------- */
typedef uint8  Dio_ChannelType;     /* mã 1 chân = (cổng << 4) | chân, tối đa 0x4F */
typedef uint8  Dio_PortType;        /* A=0 ... E=4 */
typedef uint8  Dio_LevelType;       /* STD_LOW / STD_HIGH */
typedef uint16 Dio_PortLevelType;   /* mức cả cổng: bit i = mức chân i */

typedef struct
{
    Dio_PortLevelType mask;     /* bit nào thuộc nhóm */
    uint8             offset;   /* vị trí bit thấp nhất của nhóm */
    Dio_PortType      port;     /* cổng chứa nhóm */
} Dio_ChannelGroupType;

/* Đặt SAU phần 4: Dio_Cfg.h khai báo nhóm kênh, cần kiểu Dio_ChannelGroupType */
#include "Dio_Cfg.h"

/* ---------- 5. Khai báo hàm (thân hàm nằm ở Dio.c) ---------- */
Dio_LevelType     Dio_ReadChannel(Dio_ChannelType ChannelId);
void              Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level);
Dio_PortLevelType Dio_ReadPort(Dio_PortType PortId);
void              Dio_WritePort(Dio_PortType PortId, Dio_PortLevelType Level);
Dio_PortLevelType Dio_ReadChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr);
void              Dio_WriteChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr,
                                        Dio_PortLevelType Level);

#if (DIO_FLIP_CHANNEL_API == STD_ON)
Dio_LevelType     Dio_FlipChannel(Dio_ChannelType ChannelId);   /* trả về mức mới */
#endif

#if (DIO_VERSION_INFO_API == STD_ON)
void              Dio_GetVersionInfo(Std_VersionInfoType* versioninfo);  /* tham số đầu ra, không const */
#endif

#endif /* DIO_H */
