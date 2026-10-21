/*
 * Std_Types.h — Kiểu và hằng số chuẩn dùng chung cho mọi module AUTOSAR (SWS Standard Types)
 *
 * Module BSW (ví dụ Dio) include file này, không include trực tiếp Platform_Types.h hay Compiler.h.
 */
#ifndef STD_TYPES_H
#define STD_TYPES_H

#include "Platform_Types.h"
#include "Compiler.h"

/* ---- Phiên bản: module khác kiểm tra các số này để bảo đảm cùng bản AUTOSAR ---- */
#define STD_AR_RELEASE_MAJOR_VERSION     4U
#define STD_AR_RELEASE_MINOR_VERSION     4U
#define STD_AR_RELEASE_REVISION_VERSION  0U
#define STD_SW_MAJOR_VERSION             1U
#define STD_SW_MINOR_VERSION             0U
#define STD_SW_PATCH_VERSION             0U

/* ---- Kết quả trả về của API ---- */
typedef uint8 Std_ReturnType;

/* E_OK nằm trong khối STATUSTYPEDEFINED để không trùng với E_OK của OSEK (StatusType) */
#ifndef STATUSTYPEDEFINED
#define STATUSTYPEDEFINED
#define E_OK          0x00U
typedef unsigned char StatusType;
#endif

#define E_NOT_OK      0x01U

/* ---- Mức điện áp của chân ---- */
#define STD_HIGH      0x01U    /* mức cao, 3.3 V trên STM32F103 */
#define STD_LOW       0x00U    /* mức thấp, 0 V */

/* ---- Trạng thái logic ---- */
#define STD_ACTIVE    0x01U
#define STD_IDLE      0x00U

/* ---- Bật/tắt tính năng khi cấu hình (ví dụ DIO_DEV_ERROR_DETECT) ---- */
#define STD_ON        0x01U
#define STD_OFF       0x00U

/* ---- Thông tin phiên bản module, dùng cho hàm <Module>_GetVersionInfo() ---- */
typedef struct
{
    uint16 vendorID;
    uint16 moduleID;
    uint8  sw_major_version;
    uint8  sw_minor_version;
    uint8  sw_patch_version;
} Std_VersionInfoType;

#endif /* STD_TYPES_H */
