# HW1 — Driver Dio theo AUTOSAR MCAL cho STM32F103C8

Board: Yahboom YB-MSV01 (STM32F103C8T6, thạch anh HSE 8 MHz, nạp qua ST-Link SWD).
Build bằng `arm-none-eabi-gcc` + Makefile, bare-metal, **không dùng HAL**, chỉ thanh ghi CMSIS.

## Cấu trúc

```
hw1-dio-f103/
├── Makefile                      SRCS/INCS, include stm32f103.mk, target flash
├── app/main.c                    chạy TC1–TC8, kết quả trong biến g_*
├── drivers/
│   ├── dio/
│   │   ├── Dio.h                 giao diện Dio: SID, mã lỗi, kiểu, khai báo hàm
│   │   ├── Dio.c                 thân 8 hàm API
│   │   ├── Dio_Cfg.h             công tắc STD_ON/OFF, tên cổng, kênh, nhóm
│   │   └── Dio_Cfg.c             bảng nhóm kênh (const)
│   ├── port/
│   │   ├── Port.h, Port.c        Port TỐI GIẢN: chỉ Port_Init
│   │   └── Port_Cfg.h, Port_Cfg.c   bảng chân: PC13, PA0, PA4–PA7
│   └── mcu/stm32f103/            ┐ chép nguyên từ can-ecu-network
│       ├── clock_f103.c / .h     │ (xem mục "Nguồn các file chép")
│       ├── STM32F103C8_FLASH.ld  │
│       └── stm32f103.mk          ┘
├── services/det/Det.h, Det.c     Det tối giản: lưu lỗi cuối + đếm số lỗi
├── tests/host/                   chạy thử Dio/Port/Det trên PC với GPIO giả lập (g++), tách khỏi build firmware
├── general/
│   ├── Platform_Types.h          uint8/16/32/64, sint*, boolean, TRUE/FALSE, CPU_TYPE...
│   ├── Compiler.h                bản giản lược: NULL_PTR, INLINE, LOCAL_INLINE
│   └── Std_Types.h               Std_ReturnType, E_OK/E_NOT_OK, STD_HIGH/LOW, STD_ON/OFF, Std_VersionInfoType
└── third_party/cmsis_f1/         CMSIS Core + Device F1, startup, system_stm32f1xx.c (chép nguyên)
```

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

`make flash` gọi `STM32_Programmer_CLI -c port=SWD -w build/dio_f103.elf -v -rst`: chỉ ghi các trang Flash mà ELF chiếm,
đọc lại để so, rồi reset. Không xoá toàn chip (`-e`), không ghi option byte (`-ob`).

Cờ biên dịch (trong `stm32f103.mk`): `-mcpu=cortex-m3 -mthumb -DSTM32F103xB -Og -g3 -Wall -Wextra -ffunction-sections -fdata-sections`,
link với `--specs=nano.specs -Wl,--gc-sections`. Build hiện tại: **0 cảnh báo**.

## Thiết kế

### Mã kênh, cổng, nhóm (`Dio_Cfg.h`, `Dio_Cfg.c`)

- Mã kênh = `(cổng << 4) | chân`, cổng A=0 … E=4, chân 0…15. Hợp lệ khi mã ≤ `0x4F`.
- Mã cổng hợp lệ: 0…4.

| Tên | Giá trị | Chân | Chế độ (đặt bởi Port_Init) |
|---|---|---|---|
| `DioConf_DioChannel_LedBoard` | `0x2D` | PC13 | output push-pull 2 MHz, mức đầu HIGH (LED tắt; LED sáng khi LOW) |
| `DioConf_DioChannel_Button` | `0x00` | PA0 | input pull-up (ODR bit 0 = 1) |
| `DioConf_DioChannelGroup_Test` | `&Dio_ChannelGroupConfig[0]` | PA4–PA7 | output push-pull 2 MHz, mức đầu LOW; mask `0x00F0`, offset 4, cổng A |

Ba công tắc `DIO_DEV_ERROR_DETECT`, `DIO_FLIP_CHANNEL_API`, `DIO_VERSION_INFO_API` đều `STD_ON`.
Đã thử biên dịch `Dio.c` với cả ba = `STD_OFF`: không lỗi, không cảnh báo.

### Ba quy tắc khi ghi chân

1. **Chỉ ghi bằng BSRR, mỗi lần gọi đúng một lần ghi.** Bit 0–15 đặt ODR lên 1, bit 16–31 xoá ODR về 0.
   Không dùng `ODR |=` (đọc–sửa–ghi, có thể bị ngắt chen giữa làm mất bit).
2. **Chân input thì không ghi.** Trên F103, chân input pull dùng bit ODR để chọn pull-up (1) hay pull-down (0).
   Ghi LOW vào chân input pull-up sẽ biến nó thành pull-down. Vì vậy mọi hàm ghi đều đọc 2 bit MODE trong CRL/CRH trước,
   gặp MODE = 00 (input) thì bỏ qua chân đó.
3. **Tham số sai thì báo Det rồi thoát, không chạm thanh ghi nào.** Hàm đọc trả `STD_LOW` / `0`.
   Det nhận `(ModuleId = 120, InstanceId = 0, ApiId = SID của hàm, ErrorId = mã lỗi)`.

### Port tối giản

`Port_Init(&Port_Config)` chỉ đủ để Dio chạy, **không đầy đủ theo SWS_Port** (không có SetPinDirection, SetPinMode,
RefreshPortDirection, GetVersionInfo, không báo Det). Với từng chân trong bảng: bật clock cổng trong `RCC_APB2ENR`
→ đặt mức ban đầu trong ODR **trước** → rồi mới ghi 4 bit CNF/MODE trong CRL/CRH. Chân không có trong bảng giữ nguyên trạng thái sau reset
(input floating), nên Dio coi là input và không bao giờ ghi vào.

Bảng chân của Port (`Port_Cfg.c`) và bảng kênh/nhóm của Dio (`Dio_Cfg.h`, `Dio_Cfg.c`) khai báo riêng: đổi chân ở một bên thì sửa cả bên kia.

## Kiểm thử TC1–TC8

`g_tc[i]` là kết quả của TC(i+1): **0 = chưa chạy, 1 = ĐẠT, 2 = KHÔNG ĐẠT**.
TC1, TC4–TC8 chạy một lần sau khởi động (theo thứ tự TC1, TC4, TC5, TC6, TC7, TC8). TC3 và TC2 chạy liên tục trong vòng lặp chính.

**Trước khi reset/nạp: KHÔNG nối gì vào PA0** (TC4 mong đọc HIGH lúc khởi động).

| TC | Hàm thử | Làm gì | Đạt khi | Biến chẩn đoán |
|---|---|---|---|---|
| TC1 | `Dio_WriteChannel` | Ghi LED LOW rồi HIGH | ODR bit 13 = 0 sau LOW, = 1 sau HIGH | `g_tc1_odr_low` (0), `g_tc1_odr_high` (1) |
| TC2 | `Dio_ReadChannel` | Đọc PA0 liên tục vào `g_tc2_level` | Đã thấy cả HIGH (hở) lẫn LOW (nối PA0–GND). **Cần thao tác tay**: chưa nối dây thì `g_tc[1]` = 0 | `g_tc2_level`, `g_tc2_seen_high`, `g_tc2_seen_low` |
| TC3 | `Dio_FlipChannel` | Đảo LED mỗi 500 ms (LED nháy 1 Hz) | Mức trả về khớp ODR bit 13 sau ≥ 2 lần đảo; sai 1 lần là KHÔNG ĐẠT | `g_tc3_flips` (tăng 2/s), `g_tc3_errors` (0), `g_tc3_level` |
| TC4 | `Dio_WriteChannel` trên chân input | Ghi LOW vào PA0 | ODR bit 0 vẫn 1, CRL của PA0 vẫn `0x8`, đọc PA0 vẫn HIGH | `g_tc4_level` (1) |
| TC5 | `Dio_WritePort` | Ghi cổng A = `0x0050` (bit 0 = 0) | ODR bit 7..4 = `0101`; mọi bit ODR khác (gồm PA0) không đổi; PA0 vẫn input pull-up | `g_tc5_odr_before` (`0xA001`), `g_tc5_odr_after` (`0xA051`) |
| TC6 | `Dio_WriteChannelGroup` | Ghi nhóm Test = `0b1010` | ODR bit 7..4 = `1010` (`0xA0`); bit ngoài nhóm không đổi | `g_tc6_odr_before` (`0xA051`), `g_tc6_odr_after` (`0xA0A1`), `g_tc6_read` (`0xA`, không tính điểm) |
| TC7 | DET | 5 lần gọi sai, xem bảng dưới | Mỗi lần Det tăng đúng 1 lỗi, đúng SID và mã lỗi; hàm đọc trả 0; chương trình chạy tiếp | `g_tc7_fail_mask` (0), `Det_ErrorCount` (5), `Det_LastError` |
| TC8 | `Dio_GetVersionInfo` | Gọi với con trỏ hợp lệ | 5 trường = 29, 120, 1, 0, 0; Det không tăng | `g_tc8_info` |

Các bước của TC7 chạy **đúng thứ tự từ trên xuống** trong `tc7_det()` (bit k trong `g_tc7_fail_mask` = 1 nghĩa là bước k+1 không đạt):

| Bit | Lời gọi | SID mong đợi | Mã lỗi mong đợi |
|---|---|---|---|
| 0x01 | `Dio_ReadChannel(0xFF)` | `0x00` | `0x0A` INVALID_CHANNEL_ID |
| 0x02 | `Dio_ReadPort(9)` | `0x02` | `0x14` INVALID_PORT_ID |
| 0x04 | `Dio_ReadChannelGroup(NULL_PTR)` | `0x04` | `0x1F` INVALID_GROUP |
| 0x08 | `Dio_ReadChannelGroup(&bản_sao_nhóm_Test)`: nội dung giống, địa chỉ không nằm trong bảng | `0x04` | `0x1F` INVALID_GROUP |
| 0x10 | `Dio_GetVersionInfo(NULL_PTR)` | `0x12` | `0x20` PARAM_POINTER |

Sau TC7, `Det_LastError` = `{ModuleId 120, InstanceId 0, ApiId 0x12, ErrorId 0x20}`: lỗi cuối là của bước 5
`GetVersionInfo(NULL_PTR)`, vì bước nhóm sao chép (INVALID_GROUP, `{120, 0, 0x04, 0x1F}`) là bước 4, chạy trước.
TC8 chạy sau TC7 nhưng gọi với con trỏ hợp lệ nên không sinh lỗi, `Det_LastError` và `Det_ErrorCount` (= 5) giữ nguyên.

## Chạy thử trên PC trước khi nạp (`tests/host/`)

```bat
make -C tests/host          REM biên dịch bằng g++ (MinGW) rồi chạy; mã thoát 0 = đạt hết
make -C tests/host clean
```

`tests/host/stm32f1xx.h` thay header CMSIS bằng thanh ghi giả lập: ghi BSRR thì ODR đổi ngay, đọc IDR thì ra mức chân
tính từ CRL/CRH, ODR và "dây nối ngoài". `sim.cpp` dùng nguyên `Dio.c`, `Port.c`, `Det.c`, các file cấu hình và các hàm `tc*()`
của `app/main.c` (không sửa dòng nào), chạy Port_Init → TC1, TC4–TC8 → TC3 ×3 → TC2 (hở rồi nối PA0–GND), sau đó kiểm thêm:
gọi sai không chạm thanh ghi, mỗi hàm ghi đúng một lần BSRR, bit ngoài nhóm không đổi, Flip chân input...
Bản giả lập không thay được thử trên board: không kiểm clock, SysTick, thời gian hay điện thật trên chân.
Build firmware (`make` ở `hw1-dio-f103/`) không đụng tới thư mục này.

## Nạp và đọc kết quả bằng CLI

Chỉ nạp khi đã xác nhận. Nạp: `make flash` (hoặc lệnh `STM32_Programmer_CLI` ở trên).

Đọc khi chip đang chạy: **BẮT BUỘC `mode=HOTPLUG`** (mode mặc định dừng core, đọc ra số sai):

```bat
STM32_Programmer_CLI -c port=SWD mode=HOTPLUG -r8 0x20000050 8 -r32 0x20000058 8 -r8 0x20000060 6 -r8 0x20000020 0x30 -r32 0x20000000 4 -r32 0x40010800 0x10
```

Địa chỉ của bản build ngày 22/10/2026. **Sửa code thì địa chỉ có thể đổi**: lấy lại bằng

```bat
arm-none-eabi-nm -S -n build/dio_f103.elf | findstr "g_ Det_ SystemCoreClock"
```

| Địa chỉ | Cỡ | Biến | Mong đợi |
|---|---|---|---|
| 0x20000000 | 4 | `SystemCoreClock` | 72000000 |
| 0x20000020 | 8 | `g_tc8_info`: vendorID (2 B), moduleID (2 B), major, minor, patch, đệm | `1D 00 78 00 01 00 00 xx` |
| 0x20000028 | 1 | `g_tc7_fail_mask` | 0 |
| 0x2000002A | 2 | `g_tc6_read` | `0x000A` |
| 0x2000002C | 4 | `g_tc6_odr_after` | `0x0000A0A1` |
| 0x20000030 | 4 | `g_tc6_odr_before` | `0x0000A051` |
| 0x20000034 | 4 | `g_tc5_odr_after` | `0x0000A051` |
| 0x20000038 | 4 | `g_tc5_odr_before` | `0x0000A001` |
| 0x2000003C | 1 | `g_tc4_level` | 1 |
| 0x2000003D | 1 | `g_tc3_level` | 0 hoặc 1, đổi mỗi 500 ms |
| 0x20000040 | 4 | `g_tc3_errors` | 0 |
| 0x20000044 | 4 | `g_tc3_flips` | tăng ≈ 2/s |
| 0x20000048 | 1 | `g_tc2_seen_low` | 0 khi chưa nối dây, 1 sau khi nối PA0–GND |
| 0x20000049 | 1 | `g_tc2_seen_high` | 1 |
| 0x2000004A | 1 | `g_tc2_level` | 1 khi hở, 0 khi nối GND |
| 0x2000004B | 1 | `g_tc1_odr_high` | 1 |
| 0x2000004C | 1 | `g_tc1_odr_low` | 0 |
| 0x2000004D | 1 | `g_clk_err` | 0 |
| 0x20000050 | 8 | `g_tc[0..7]` | `01 00 01 01 01 01 01 01` trước khi nối dây; `01 01 01 01 01 01 01 01` sau |
| 0x20000058 | 4 | `g_tick_ms` | tăng ≈ 1000/s |
| 0x2000005C | 4 | `Det_ErrorCount` | 5 |
| 0x20000060 | 6 | `Det_LastError`: ModuleId (2 B), InstanceId, ApiId, ErrorId, đệm | `78 00 00 12 20 xx` |
| 0x40010800 | 16 | GPIOA: CRL, CRH, IDR, ODR (địa chỉ phần cứng, không đổi khi sửa code) | CRL `0x22224448`, CRH `0x88844444`, ODR `0x0000A0A1` |

Số nhiều byte lưu little-endian: đọc `-r8` thì byte thấp đứng trước (ví dụ ModuleId 120 = `78 00`).

**Vì sao ODR cổng A có sẵn `0xA000`:** sau reset, PA13 (SWDIO), PA14 (SWCLK), PA15 (JTDI) là chân debug, cấu hình
input pull (GPIOA_CRH = `0x88844444`, nibble 13–15 = `8`), với PA13 và PA15 pull-up (ODR bit 13, 15 = 1), PA14 pull-down (bit 14 = 0).
Các biến `g_tc5_odr_*`, `g_tc6_odr_*` vì thế mang thêm `0xA000`. Bit này giữ nguyên qua `Dio_WritePort(A, 0x0050)` dù
`Level` có bit 13, 15 = 0: chứng tỏ trên chip thật Dio không ghi vào chân input (nếu ghi thì PA13 thành pull-down, có thể mất SWD).

Trình tự đọc gợi ý:
1. Nạp, chờ ≥ 2 s, đọc lần A (PA0 để hở): `g_tc` = `01 00 01 01 01 01 01 01`, LED nháy 1 Hz.
2. Nối PA0 xuống GND bằng dây, đọc lần B: `g_tc2_level` = 0, `g_tc[1]` = 1.
3. Rút dây, đọc lần C: `g_tc2_level` = 1, `g_tc[1]` vẫn 1. `g_tc3_flips` tăng ≈ 2 × số giây giữa hai lần đọc.

### Bảng chẩn đoán

| Triệu chứng | Nghi ngờ |
|---|---|
| Mọi biến = 0, `g_tick_ms` không tăng | Chương trình không chạy / SysTick không chạy; kiểm đã nạp đúng ELF, đã reset |
| `g_clk_err` ≠ 0 | Clock 72 MHz không lên (xem `clock_f103.h`). Chip vẫn chạy ở HSI 8 MHz, các TC vẫn có nghĩa |
| TC1 = 2 | WriteChannel không ghi BSRR, hoặc PC13 chưa là output (Port_Init) |
| TC3 = 2 | FlipChannel trả mức khác ODR (đọc nhầm IDR thay vì ODR) |
| TC4 = 2, `g_tc4_level` = 0 | Có thứ gì kéo PA0 xuống lúc khởi động (dây nối sẵn?), hoặc WriteChannel ghi vào chân input |
| TC5 = 2, bit 0 của `g_tc5_odr_after` = 0 | WritePort ghi vào chân input (PA0 bị đổi thành pull-down) |
| TC6 = 2, bit ngoài `0xF0` đổi | WriteChannelGroup ghi ra ngoài mask |
| `g_tc6_read` ≠ `0xA` | Có mạch ngoài kéo PA4–PA7 (xem sơ đồ board) |
| TC7 = 2 | Xem `g_tc7_fail_mask` để biết bước nào sai, rồi so `Det_LastError` |

## Kết quả trên board (22/10/2026)

Board Yahboom YB-MSV01: ST-Link V2 (FW V2J46S7) nối SWD, không cắm USB-C, FPC và hàng chân để trống, PA0 hở lúc reset.
`STM32_Programmer_CLI` nhận chip: Device ID `0x410` (STM32F101/F102/F103 medium-density), Flash 64 KB, Cortex-M3, điện áp 3,33 V.
Nạp `build/dio_f103.elf` (2,86 KB, xoá trang 0–2), verify đạt, reset phần mềm lúc 10:55:36.
Đọc 3 lần bằng `mode=HOTPLUG`, không reset giữa các lần:

- Lần A, 10:55:47.70: PA0 hở.
- Lần B, 11:22:59.56: PA0 nối GND bằng dây.
- Lần C, 11:23:46.09: đã tháo dây.

| TC | `g_tc` A / B / C | Biến chẩn đoán (A / B / C) | Mong đợi | Đạt? |
|---|---|---|---|---|
| TC1 WriteChannel | 01 / 01 / 01 | `g_tc1_odr_low` = 0, `g_tc1_odr_high` = 1 | 0, 1 | ✅ |
| TC2 ReadChannel (nút) | 00 / 01 / 01 | `g_tc2_level` 1 / 0 / 1; `seen_high` 1/1/1, `seen_low` 0/1/1 | HIGH khi hở, LOW khi nối GND | ✅ |
| TC3 FlipChannel | 01 / 01 / 01 | `g_tc3_flips` 22 / 3286 / 3379; `g_tc3_errors` 0 / 0 / 0 | 2 lần/s, 0 lỗi | ✅ (`g_tick_ms`/500 = 22,6 / 3286,4 / 3379,5) |
| TC4 WriteChannel chân input | 01 / 01 / 01 | `g_tc4_level` = 1 | 1 | ✅ |
| TC5 WritePort | 01 / 01 / 01 | ODR `0xA001` → `0xA051` | bit 7..4 = 0101, bit khác giữ nguyên | ✅ |
| TC6 WriteChannelGroup | 01 / 01 / 01 | ODR `0xA051` → `0xA0A1`; `g_tc6_read` = `0xA` | bit 7..4 = 1010, bit khác giữ nguyên | ✅ |
| TC7 DET | 01 / 01 / 01 | `g_tc7_fail_mask` = 0; `Det_ErrorCount` = 5; `Det_LastError` = {120, 0, 0x12, 0x20} | 0; 5; {120, 0, 0x12, 0x20} | ✅ |
| TC8 GetVersionInfo | 01 / 01 / 01 | `g_tc8_info` = 29, 120, 1.0.0 | 29, 120, 1.0.0 | ✅ |

| Kiểm chung | A | B | C | Mong đợi | Đạt? |
|---|---|---|---|---|---|
| `g_clk_err` | 0 | 0 | 0 | 0 | ✅ |
| `SystemCoreClock` | 72000000 | 72000000 | 72000000 | 72000000 | ✅ |
| `g_tick_ms` | 11319 | 1643209 | 1689734 | ≈ 1000/s | ✅ A→B: 1631890 ms / 1631,86 s = 1000,02 /s; B→C: 46525 ms / 46,53 s = 999,9 /s |
| GPIOA CRL / CRH | — | `0x22224448` / `0x88844444` | `0x22224448` / `0x88844444` | PA0 = 8, PA4–7 = 2; PA13–15 giữ cấu hình debug | ✅ |
| GPIOA IDR bit 0 (PA0) | — | 0 (`0xDFA0`) | 1 (`0xDFAF`) | LOW khi nối, HIGH khi tháo | ✅ |

**8/8 TC ĐẠT.** Thời gian lấy theo `%TIME%` của cmd (độ phân giải 10 ms). LED nháy được suy ra từ ODR bit 13 và `g_tc3_flips`,
không quan sát bằng mắt qua CLI. Bit `0xA000` trong ODR cổng A: xem giải thích ở mục đọc kết quả.

## Giải thích từng hàm

### Dio (`drivers/dio/Dio.c`)

**`Dio_IsOutputPin(gpio, pin)`** (nội bộ): lấy CRL nếu chân 0–7, CRH nếu chân 8–15, dịch phải `(pin & 7) × 4` bit rồi lấy 2 bit MODE.
MODE ≠ 00 là output (2/10/50 MHz) → TRUE; MODE = 00 là input → FALSE. Mọi hàm ghi dựa vào hàm này để không ghi chân input.

**`Dio_OutputMask(gpio)`** (nội bộ): gọi `Dio_IsOutputPin` cho 16 chân của một cổng, ghép thành mặt nạ 16 bit (bit i = 1 nếu chân i là output).
WritePort và WriteChannelGroup AND mức muốn ghi với mặt nạ này, nên chân input không bao giờ bị ghi.

**`Dio_IsValidGroup(group)`** (nội bộ, chỉ có khi DET bật): con trỏ NULL → không hợp lệ. Ngoài ra con trỏ phải trùng địa chỉ một phần tử
của `Dio_ChannelGroupConfig[]`. So địa chỉ chứ không so nội dung, nên một bản sao giống hệt nhưng nằm chỗ khác vẫn bị từ chối (TC7 bước 4).

**`Dio_ReadChannel(ChannelId)`**: kiểm `ChannelId ≤ 0x4F`, sai thì báo Det `0x0A` (SID `0x00`) và trả `STD_LOW`.
Tách cổng = `ChannelId >> 4`, chân = `ChannelId & 0xF`, đọc bit tương ứng trong **IDR** (mức thật trên chân, đúng cho cả input lẫn output) rồi trả `STD_HIGH`/`STD_LOW`.

**`Dio_WriteChannel(ChannelId, Level)`**: kiểm mã kênh như trên (SID `0x01`). Nếu chân là input thì thoát, không ghi gì.
Nếu là output: `Level == STD_HIGH` → ghi `1 << pin` vào **BSRR** (đặt), giá trị khác → ghi `1 << (pin + 16)` (xoá). Một lần ghi, không đọc–sửa–ghi ODR.

**`Dio_ReadPort(PortId)`**: kiểm `PortId ≤ 4`, sai thì báo Det `0x14` (SID `0x02`) và trả 0.
Trả 16 bit thấp của **IDR** của cổng đó: bit i = mức chân i.

**`Dio_WritePort(PortId, Level)`**: kiểm mã cổng (SID `0x03`). Lấy mặt nạ output của cổng; bit cần đặt = `Level & mask`,
bit cần xoá = `~Level & mask`; ghi một lần `BSRR = đặt | (xoá << 16)`. Chân input có bit = 0 ở cả hai nửa nên giữ nguyên ODR, giữ nguyên pull-up/pull-down (TC5).

**`Dio_ReadChannelGroup(ChannelGroupIdPtr)`**: kiểm nhóm bằng `Dio_IsValidGroup`, sai thì báo Det `0x1F` (SID `0x04`) và trả 0.
Trả `(IDR & mask) >> offset`: lấy các bit của nhóm trong IDR rồi dồn về bit 0, ví dụ nhóm PA4–PA7 trả giá trị 0…15.

**`Dio_WriteChannelGroup(ChannelGroupIdPtr, Level)`**: kiểm nhóm (SID `0x05`). Dịch `Level` lên `offset`, AND với `mask` của nhóm và mặt nạ output của cổng;
bit 1 vào nửa thấp BSRR (đặt), bit 0 vào nửa cao (xoá), ghi một lần. Bit ngoài nhóm là 0 ở cả hai nửa nên không đổi; bit thừa của `Level` nằm ngoài mask bị bỏ.

**`Dio_FlipChannel(ChannelId)`**: kiểm mã kênh (SID `0x11`). Chân input: không đổi gì, trả mức đọc từ IDR.
Chân output: đọc bit trong **ODR** (mức đang xuất, không phải IDR), đang 1 thì ghi BSRR nửa cao để xoá, đang 0 thì ghi nửa thấp để đặt, rồi trả mức mới.

**`Dio_GetVersionInfo(versioninfo)`**: con trỏ NULL thì báo Det `0x20` (SID `0x12`) và thoát.
Ngược lại điền 5 trường từ macro trong `Dio.h`: vendorID 29, moduleID 120, phiên bản 1.0.0. Không đụng thanh ghi nào.

### Port (`drivers/port/Port.c`)

**`Port_Init(ConfigPtr)`**: duyệt bảng chân trong `Port_Cfg.c`. Mỗi chân: bật clock cổng bằng bit `IOPxEN` trong **RCC_APB2ENR** (đọc lại một lần cho chắc clock đã bật);
đặt mức ban đầu qua **BSRR** (output: mức xuất đầu tiên; input pull: chọn pull-up/down); cuối cùng ghi 4 bit CNF/MODE trong **CRL/CRH**.
Đặt ODR trước để chân không bị chớp mức sai khi vừa thành output. ConfigPtr NULL hoặc dòng cấu hình sai cổng/chân thì bỏ qua, không báo Det.

### Det (`services/det/Det.c`)

**`Det_ReportError(ModuleId, InstanceId, ApiId, ErrorId)`**: ghi 4 tham số vào biến toàn cục `volatile Det_LastError`, rồi tăng `Det_ErrorCount`, trả `E_OK`.
Không dừng chương trình, để TC7 gọi sai liên tiếp mà vẫn chạy tiếp. Hai biến `volatile` để debugger/CLI đọc được qua SWD.

## Nguồn các file chép

Chép từ repo đồ án `~/Documents/can-ecu-network`, commit `c10f790`, ngày 21/10/2026.
Nội dung **giữ nguyên từng byte** (đã so bằng `cmp`); muốn đổi thì sửa trong `Makefile` của HW1, không sửa các file này.

| File trong hw1-dio-f103 | Nguồn trong can-ecu-network |
|---|---|
| `drivers/mcu/stm32f103/clock_f103.c`, `clock_f103.h` | `drivers/mcu/stm32f103/` (HSE 8 MHz → PLL ×9 → 72 MHz, APB1 36 MHz) |
| `drivers/mcu/stm32f103/STM32F103C8_FLASH.ld` | `drivers/mcu/stm32f103/` (Flash 64 KB, RAM 20 KB, do STM32CubeIDE sinh) |
| `drivers/mcu/stm32f103/stm32f103.mk` | `drivers/mcu/stm32f103/` (cờ build, quy tắc make) |
| `third_party/cmsis_f1/` (cả thư mục, kèm README và LICENSE) | `third_party/cmsis_f1/` (gốc: STM32Cube_FW_F1 V1.8.7, Apache-2.0) |

`SysTick_Handler` và cách gọi `SysTick_Config` trong `app/main.c` viết theo `tests/target/bxcan_loopback/main.c` (GĐ 4) của đồ án.

Ghi chú:
- Cây thư mục `drivers/mcu/stm32f103/` và `third_party/cmsis_f1/` giữ đúng như bên đồ án, vì `stm32f103.mk` tìm file theo `$(REPO_ROOT)/...`.
  Makefile của HW1 đặt `REPO_ROOT := .` nên dùng được file .mk mà không phải sửa.
- Chú thích đầu `stm32f103.mk` nhắc `scripts/f103_build.sh`: script đó chỉ có ở đồ án. Ở đây dùng `env.bat` thay thế.
