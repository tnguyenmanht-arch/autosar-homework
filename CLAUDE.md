## Ngữ cảnh
Mình là Thắng, sinh viên ô tô HUST. Mình làm 2 bài tập môn AUTOSAR, hạn 26/10/2026:
- HW1: driver Dio theo chuẩn AUTOSAR MCAL cho STM32F103 (làm sau).
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

`env.bat` chỉ đặt PATH cho phiên cmd đó (avr-gcc, avrdude, goil, DLL WinLibs của goil), rồi in phiên bản ba công cụ.

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

## Clone lại repo

Không dùng `--recursive`, vì Trampoline có nhiều submodule nặng (tflite-micro, lwip...). Chỉ kéo những gì cần:

```sh
git clone <repo> autosar-homework
cd autosar-homework
git submodule update --init third_party/trampoline
git -C third_party/trampoline submodule update --init --depth 1 machines/avr/arduino
```
