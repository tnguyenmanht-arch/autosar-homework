/*
 * Compiler.h — Macro phụ thuộc trình biên dịch theo AUTOSAR (SWS Compiler Abstraction)
 *
 * BẢN GIẢN LƯỢC cho arm-none-eabi-gcc trên STM32F103 (32 bit):
 * chỉ có NULL_PTR, INLINE, LOCAL_INLINE.
 * Không có FUNC, P2VAR, P2CONST, CONSTP2VAR, VAR, CONST... vì các macro đó sinh ra để thêm
 * từ khoá near/far cho CPU 8/16 bit nhiều vùng nhớ. GCC 32 bit địa chỉ phẳng, không cần,
 * nên khai báo hàm và con trỏ viết thẳng kiểu C thường.
 */
#ifndef COMPILER_H
#define COMPILER_H

/* Con trỏ rỗng, dùng để kiểm tra tham số con trỏ (ví dụ trong Dio_GetVersionInfo) */
#ifndef NULL_PTR
#define NULL_PTR      ((void *)0)
#endif

/* Hàm inline dùng chung giữa nhiều file */
#define INLINE        inline

/* Hàm inline chỉ dùng trong một file */
#define LOCAL_INLINE  static inline

#endif /* COMPILER_H */
