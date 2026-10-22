/*
 * Det.c — Default Error Tracer (bản tối giản cho HW1)
 */
#include "Det.h"

volatile Det_ErrorInfoType Det_LastError;    /* = 0 sau reset (vùng .bss) */
volatile uint32            Det_ErrorCount;

Std_ReturnType Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId)
{
    /* Ghi đủ 4 trường trước rồi mới tăng bộ đếm:
       thấy bộ đếm tăng thì 4 trường đã là của lỗi mới */
    Det_LastError.ModuleId   = ModuleId;
    Det_LastError.InstanceId = InstanceId;
    Det_LastError.ApiId      = ApiId;
    Det_LastError.ErrorId    = ErrorId;
    Det_ErrorCount++;

    return E_OK;
}
