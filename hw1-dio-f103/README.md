# HW1 — Driver Dio theo AUTOSAR MCAL cho STM32F103C8

Board: Yahboom YB-MSV01 (STM32F103C8T6, thạch anh HSE 8 MHz, nạp qua ST-Link SWD).
Build bằng `arm-none-eabi-gcc` + Makefile, bare-metal, **không dùng HAL**.

## Cấu trúc

```
hw1-dio-f103/
├── Makefile                      khai báo SRCS/INCS, include stm32f103.mk, target flash
├── app/main.c                    chương trình tạm: clock_init() rồi lặp vô hạn
├── general/                      header AUTOSAR dùng chung
│   ├── Platform_Types.h          uint8/16/32/64, sint*, boolean, TRUE/FALSE, CPU_TYPE...
│   └── Std_Types.h               Std_ReturnType, E_OK/E_NOT_OK, STD_HIGH/LOW, STD_ON/OFF, Std_VersionInfoType
├── drivers/mcu/stm32f103/        ┐
│   ├── clock_f103.c / .h         │ CHÉP NGUYÊN từ can-ecu-network (xem bảng dưới)
│   ├── STM32F103C8_FLASH.ld      │
│   └── stm32f103.mk              │
└── third_party/cmsis_f1/         ┘ CMSIS Core + Device F1, startup, system_stm32f1xx.c
```

Dio.h/Dio.c, Port, Det do Thắng tự viết (dự kiến đặt trong `mcal/`), rồi thêm vào `SRCS`/`INCS` của Makefile.

## Build và nạp

Mở **cmd** ở gốc repo, chạy `env.bat` (thêm arm-none-eabi-gcc, make, STM32_Programmer_CLI của STM32CubeIDE vào PATH của phiên đó):

```bat
cd %USERPROFILE%\Documents\autosar-homework
env.bat
cd hw1-dio-f103
make            REM ra build\dio_f103.elf/.hex/.bin, in kích thước
make clean
make flash      REM CHỈ chạy khi đã cắm ST-Link và đồng ý nạp
```

`make flash` gọi `STM32_Programmer_CLI -c port=SWD -w build/dio_f103.elf -v -rst` (ghi, đọc lại để so sánh, reset).

Cờ biên dịch (trong `stm32f103.mk`): `-mcpu=cortex-m3 -mthumb -DSTM32F103xB -Og -g3 -Wall -Wextra -ffunction-sections -fdata-sections`,
link với `--specs=nano.specs -Wl,--gc-sections`. Mục tiêu: build luôn đạt 0 cảnh báo.

## Nguồn các file chép

Chép từ repo đồ án `~/Documents/can-ecu-network`, commit `c10f790`, ngày 21/10/2026.
Nội dung **giữ nguyên từng byte** (đã so bằng `cmp`); muốn đổi thì sửa trong `Makefile` của HW1, không sửa các file này.

| File trong hw1-dio-f103 | Nguồn trong can-ecu-network |
|---|---|
| `drivers/mcu/stm32f103/clock_f103.c`, `clock_f103.h` | `drivers/mcu/stm32f103/` (HSE 8 MHz → PLL ×9 → 72 MHz, APB1 36 MHz) |
| `drivers/mcu/stm32f103/STM32F103C8_FLASH.ld` | `drivers/mcu/stm32f103/` (Flash 64 KB, RAM 20 KB, do STM32CubeIDE sinh) |
| `drivers/mcu/stm32f103/stm32f103.mk` | `drivers/mcu/stm32f103/` (cờ build, quy tắc make) |
| `third_party/cmsis_f1/` (cả thư mục, kèm README và LICENSE) | `third_party/cmsis_f1/` (gốc: STM32Cube_FW_F1 V1.8.7, Apache-2.0) |

Ghi chú:
- Cây thư mục `drivers/mcu/stm32f103/` và `third_party/cmsis_f1/` giữ đúng như bên đồ án, vì `stm32f103.mk` tìm file theo `$(REPO_ROOT)/...`.
  Makefile của HW1 đặt `REPO_ROOT := .` nên dùng được file .mk mà không phải sửa.
- Chú thích đầu `stm32f103.mk` nhắc `scripts/f103_build.sh`: script đó chỉ có ở đồ án. Ở đây dùng `env.bat` thay thế.
