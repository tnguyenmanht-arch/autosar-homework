/*
 * Platform_Types.h — Kiểu dữ liệu nền tảng theo AUTOSAR (SWS Platform Types)
 *
 * Nền tảng: STM32F103 (ARM Cortex-M3, 32 bit, little-endian), trình biên dịch arm-none-eabi-gcc.
 * Kích thước kiểu khớp với <stdint.h> của arm-none-eabi-gcc
 * (uint32_t là "unsigned long"), để truyền con trỏ qua lại giữa uint32 và uint32_t không bị cảnh báo.
 */
#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

/* ---- Phiên bản ---- */
#define PLATFORM_AR_RELEASE_MAJOR_VERSION     4U
#define PLATFORM_AR_RELEASE_MINOR_VERSION     4U
#define PLATFORM_AR_RELEASE_REVISION_VERSION  0U
#define PLATFORM_SW_MAJOR_VERSION             1U
#define PLATFORM_SW_MINOR_VERSION             0U
#define PLATFORM_SW_PATCH_VERSION             0U

/* ---- Đặc điểm CPU ---- */
#define CPU_TYPE_8        8U
#define CPU_TYPE_16       16U
#define CPU_TYPE_32       32U
#define CPU_TYPE_64       64U

#define MSB_FIRST         0U    /* thứ tự bit: bit cao nhất đánh số 0 */
#define LSB_FIRST         1U    /* thứ tự bit: bit thấp nhất đánh số 0 */

#define HIGH_BYTE_FIRST   0U    /* big-endian */
#define LOW_BYTE_FIRST    1U    /* little-endian */

#define CPU_TYPE          CPU_TYPE_32        /* Cortex-M3 là CPU 32 bit */
#define CPU_BIT_ORDER     LSB_FIRST
#define CPU_BYTE_ORDER    LOW_BYTE_FIRST     /* STM32F1 chạy little-endian */

/* ---- Giá trị logic ---- */
/* Có #ifndef vì thư viện khác (ví dụ OSEK) có thể đã định nghĩa TRUE/FALSE */
#ifndef TRUE
#define TRUE              1U
#endif

#ifndef FALSE
#define FALSE             0U
#endif

/* ---- Kiểu cơ bản ---- */
typedef unsigned char         boolean;   /* chỉ nhận TRUE hoặc FALSE */

typedef unsigned char         uint8;     /* 0 .. 255 */
typedef unsigned short        uint16;    /* 0 .. 65535 */
typedef unsigned long         uint32;    /* 0 .. 4294967295 */
typedef unsigned long long    uint64;

typedef signed char           sint8;     /* -128 .. 127 */
typedef signed short          sint16;    /* -32768 .. 32767 */
typedef signed long           sint32;    /* -2147483648 .. 2147483647 */
typedef signed long long      sint64;

/* Kiểu "_least": ít nhất N bit, chọn độ rộng CPU xử lý nhanh nhất (32 bit trên Cortex-M3) */
typedef unsigned long         uint8_least;
typedef unsigned long         uint16_least;
typedef unsigned long         uint32_least;
typedef signed long           sint8_least;
typedef signed long           sint16_least;
typedef signed long           sint32_least;

typedef float                 float32;
typedef double                float64;

#endif /* PLATFORM_TYPES_H */
