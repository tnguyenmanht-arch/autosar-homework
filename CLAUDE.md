## Ngữ cảnh
Mình là Thắng, sinh viên ô tô HUST. Mình làm 2 bài tập môn AUTOSAR, hạn 26/10/2026:
- HW1: driver Dio theo chuẩn AUTOSAR MCAL cho STM32F103C8 (board Yahboom YB-MSV01), ở `hw1-dio-f103/`. Xem phần HW1 bên dưới.
- HW2: chạy ví dụ blink của Trampoline (OSEK/VDX) trên Arduino Uno thật (ATmega328P).
  Luồng: .oil → goil sinh code C cấu hình + make.py → avr-gcc/avr-g++ biên dịch cùng kernel (os/)
  và port AVR (machines/avr/) → avrdude nạp vào Uno.
Đồ án kỹ sư của mình ở repo ~/Documents/can-ecu-network. KHÔNG được sửa bất cứ thứ gì trong repo đó, chỉ được đọc.
Máy chạy Windows. Không cài phần mềm, không sửa PATH hệ thống, không nạp board khi mình chưa đồng ý.
Mình tự viết .oil và code ứng dụng để học. Bạn lo môi trường, sắp xếp repo. Comment trong code viết tiếng Việt.

## Cách dùng

Mở **cmd** (không phải PowerShell hay Git Bash), vào thư mục repo, chạy `env.bat` trước khi build:

```bat
cd %USERPROFILE%\Documents\autosar-homework
env.bat
```

`env.bat` chỉ đặt PATH cho phiên cmd đó (avr-gcc, avrdude, goil, DLL WinLibs của goil; arm-none-eabi-gcc, make,
STM32_Programmer_CLI của STM32CubeIDE), rồi in phiên bản các công cụ.
Phiên cmd đã chạy bản `env.bat` cũ (chưa có phần ARM) thì mở cmd mới, vì biến `AUTOSAR_HW_ENV` chặn thêm PATH lần hai.

## HW1 — Dio cho STM32F103C8

- Board: Yahboom YB-MSV01, STM32F103C8T6 (Flash 64 KB, RAM 20 KB), HSE 8 MHz, nạp qua ST-Link SWD.
- Build: `arm-none-eabi-gcc` + Makefile, bare-metal, KHÔNG HAL. Mục tiêu 0 cảnh báo với `-Wall -Wextra`.
- LED trên board: **PC13, tích cực mức thấp** (ghi `STD_LOW` thì LED sáng, `STD_HIGH` thì tắt).
- **Chân cấm dùng: PA9, PA10, PA11, PA12, PA13, PA14, PA15, PB2, PB3, PB4, PB8, PB9.**
  Không cấu hình, không ghi các chân này trong code mẫu hay test.
  - PA13, PA14: SWD (SWDIO, SWCLK). Đổi sang GPIO thì ST-Link mất kết nối.
  - PA15, PB3, PB4: JTAG (JTDI, JTDO, NJTRST), mặc định bật sau reset.
  - PB2: BOOT1, có điện trở kéo xuống trên board.
- Các file chép từ can-ecu-network (CMSIS, startup, linker, `clock_f103.c/.h`, `stm32f103.mk`) giữ nguyên từng byte.
  Nguồn ghi trong `hw1-dio-f103/README.md`. Muốn đổi gì thì sửa `hw1-dio-f103/Makefile`.

Quy tắc:
- **Không viết Dio.h/Dio.c, Port, Det thay Thắng.** Chỉ được dựng khung build, file khuôn mẫu (`Std_Types.h`, `Platform_Types.h`...),
  giải thích, chỉ lỗi. Muốn sửa code của Thắng thì nói chỗ sai, để Thắng tự sửa.
- **Nạp (`make flash`) phải được Thắng xác nhận từng lần.** Build, `make -n flash` thì cứ chạy.

```bat
cd hw1-dio-f103
make            REM build\dio_f103.elf/.hex/.bin + in kích thước
make clean
make flash      REM chỉ khi Thắng đồng ý: STM32_Programmer_CLI -c port=SWD -w build/dio_f103.elf -v -rst
```

## HW2 — Trampoline blink trên Arduino Uno

Lệnh build mẫu lấy từ `third_party/trampoline/examples/avr/arduinoUno/blink/README.md`:

```sh
goil --target=avr/arduino/uno --templates=../../../../goil/templates/ blink.oil
./make.py
# nạp:
export AVRDUDE_PORT="/dev/ttyACM0"
./make.py flash
```

Đổi lại cho Windows/cmd khi file .oil nằm ở `hw2-trampoline-blink/`:

```bat
cd hw2-trampoline-blink
goil --target=avr/arduino/uno --templates=../third_party/trampoline/goil/templates/ blink.oil
python make.py
REM nạp (chỉ khi đã xác nhận cổng COM):
set AVRDUDE_PORT=COMx
python make.py flash
```

Lưu ý:
- Trong .oil, `TRAMPOLINE_BASE_PATH` phải trỏ tới submodule tính từ thư mục chứa .oil: `"../third_party/trampoline"`
  (bản gốc của ví dụ là `"../../../.."`).
- `--templates` phải là đường dẫn **tương đối**, vì make.py gọi lại goil từ thư mục app.
- Dùng `--target=`, không dùng `-t=`: bản goil Windows bỏ qua `-t=` (fork đã vá chỗ này trong make.py).
- Gõ `goil` mà không kèm file .oil thì goil mở hộp thoại chọn file. Muốn chỉ xem phiên bản thì dùng `goil --version --no-dialog`.
- Mỗi lần đổi tên file .oil hoặc `APP_NAME`, sửa lại các dòng tương ứng trong `.gitignore`.

## Môi trường

| Thành phần | Vị trí |
|---|---|
| avr-gcc 7.3.0 | `%LOCALAPPDATA%\Arduino15\packages\arduino\tools\avr-gcc\7.3.0-atmel3.6.1-arduino7\bin` |
| avrdude 8.0 | `%LOCALAPPDATA%\Arduino15\packages\arduino\tools\avrdude\8.0.0-arduino1\bin` |
| goil 3.1.16 | `%USERPROFILE%\tools\goil\goil.exe` (chép từ can-ecu-network, không có trong git) |
| DLL cho goil | `C:\winlibs\winlibs-x86_64-posix-seh-gcc-16.1.0-mingw-w64msvcrt-14.0.0-r4\mingw64\bin` (`libstdc++-6.dll`, `libgcc_s_seh-1.dll`, `libwinpthread-1.dll`) |
| Trampoline | submodule `third_party/trampoline`, fork `tnguyenmanht-arch/trampoline`, nhánh `amr-f407`, ghim `bb20df1` |
| ArduinoCore-avr | submodule lồng `third_party/trampoline/machines/avr/arduino` (`4d3df3b`) |
| arm-none-eabi-gcc 14.3.1 | `C:\ST\STM32CubeIDE_2.1.1\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*\tools\bin` |
| GNU Make 4.4.1 (kèm `sh.exe`) | `...\plugins\com.st.stm32cube.ide.mcu.externaltools.make.*\tools\bin` |
| STM32_Programmer_CLI 2.22.0 | `...\plugins\com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.*\tools\bin` |

## Clone lại repo

Không dùng `--recursive`, vì Trampoline có nhiều submodule nặng (tflite-micro, lwip...). Chỉ kéo những gì cần:

```sh
git clone <repo> autosar-homework
cd autosar-homework
git submodule update --init third_party/trampoline
git -C third_party/trampoline submodule update --init --depth 1 machines/avr/arduino
```
