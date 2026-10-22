/*
 * Det.h — Default Error Tracer (bản tối giản cho HW1)
 *
 * Module khác gọi Det_ReportError khi phát hiện tham số sai.
 * Det chỉ ghi lại lỗi cuối cùng và đếm số lỗi, để đọc qua SWD. Không dừng chương trình.
 */
#ifndef DET_H
#define DET_H

#include "Std_Types.h"

/* Bộ 4 thông tin của một lỗi */
typedef struct
{
    uint16 ModuleId;     /* module báo lỗi (Dio = 120) */
    uint8  InstanceId;   /* số thứ tự bản module, Dio chỉ có 0 */
    uint8  ApiId;        /* Service ID của hàm báo lỗi */
    uint8  ErrorId;      /* mã lỗi */
} Det_ErrorInfoType;

/* volatile: chương trình không đọc lại nhưng debugger/CLI đọc qua SWD */
extern volatile Det_ErrorInfoType Det_LastError;   /* lỗi gần nhất */
extern volatile uint32            Det_ErrorCount;  /* tổng số lỗi từ lúc reset */

/* Ghi lại một lỗi. Luôn trả E_OK. */
Std_ReturnType Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId);

#endif /* DET_H */
