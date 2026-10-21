@echo off
REM ============================================================================
REM env.bat - Đặt môi trường build cho phiên cmd HIỆN TẠI:
REM   HW1 (STM32F103, arm-none-eabi-gcc) và HW2 (AVR + Trampoline).
REM
REM Cách dùng: mở cmd, cd vào thư mục repo, gõ:   env.bat
REM Không dùng setlocal, để PATH vẫn giữ sau khi script chạy xong.
REM Chỉ sửa PATH của phiên này. PATH hệ thống không bị đụng tới.
REM ============================================================================

REM --- Thư mục bin của từng công cụ ---
REM avr-gcc, avr-g++, avr-objcopy: toolchain đi kèm Arduino IDE
set "AVR_GCC_BIN=%LOCALAPPDATA%\Arduino15\packages\arduino\tools\avr-gcc\7.3.0-atmel3.6.1-arduino7\bin"
REM avrdude: tự tìm avrdude.conf ở ..\etc, không cần khai báo -C
set "AVRDUDE_BIN=%LOCALAPPDATA%\Arduino15\packages\arduino\tools\avrdude\8.0.0-arduino1\bin"
REM goil 3.1.16: bản chép từ đồ án can-ecu-network (build bằng WinLibs GCC 16.1)
set "GOIL_BIN=%USERPROFILE%\tools\goil"
REM WinLibs: goil.exe cần 3 DLL runtime ở đây (libstdc++-6, libgcc_s_seh-1,
REM libwinpthread-1). Thiếu thì goil chết ngay với mã 0xC000007B.
set "WINLIBS_BIN=C:\winlibs\winlibs-x86_64-posix-seh-gcc-16.1.0-mingw-w64msvcrt-14.0.0-r4\mingw64\bin"

REM HW1 (STM32F103): arm-none-eabi-gcc, make, STM32_Programmer_CLI đi kèm STM32CubeIDE.
REM Tên thư mục plugin có số phiên bản + ngày build, nên dò bằng ký tự đại diện.
REM make của CubeIDE có sẵn sh.exe, nên lệnh "mkdir -p", "rm -rf" trong Makefile chạy được từ cmd.
set "CUBE_PLUGINS=C:\ST\STM32CubeIDE_2.1.1\STM32CubeIDE\plugins"
for /d %%D in ("%CUBE_PLUGINS%\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*") do set "ARM_GCC_BIN=%%~D\tools\bin"
for /d %%D in ("%CUBE_PLUGINS%\com.st.stm32cube.ide.mcu.externaltools.make.*") do set "MAKE_BIN=%%~D\tools\bin"
for /d %%D in ("%CUBE_PLUGINS%\com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.*") do set "STM32PROG_BIN=%%~D\tools\bin"

REM --- Báo nếu thiếu thư mục nào (không dừng, để vẫn thấy lỗi ở phần in phiên bản) ---
for %%D in ("%AVR_GCC_BIN%" "%AVRDUDE_BIN%" "%GOIL_BIN%" "%WINLIBS_BIN%" "%ARM_GCC_BIN%" "%MAKE_BIN%" "%STM32PROG_BIN%") do (
  if not exist "%%~D\" echo [CANH BAO] Khong tim thay thu muc: %%~D
)

REM --- Thêm vào ĐẦU PATH; chạy env.bat nhiều lần trong một phiên cũng chỉ thêm một lần ---
REM (Phiên cmd đã chạy bản env.bat cũ thì mở cmd mới để nhận thêm công cụ ARM.)
if not defined AUTOSAR_HW_ENV (
  set "PATH=%AVR_GCC_BIN%;%AVRDUDE_BIN%;%GOIL_BIN%;%WINLIBS_BIN%;%ARM_GCC_BIN%;%MAKE_BIN%;%STM32PROG_BIN%;%PATH%"
  set "AUTOSAR_HW_ENV=1"
)

REM Cổng COM của Uno khi nạp (make.py flash đọc biến này). Bỏ REM và sửa COMx khi đã cắm board:
REM set "AVRDUDE_PORT=COMx"

REM --- In phiên bản để tự kiểm tra ---
echo.
echo === avr-gcc ===
avr-gcc --version | findstr /b "avr-gcc"
echo === avrdude ===
REM avrdude -? in ra stderr, nên phải chuyển sang stdout rồi lọc dòng phiên bản
avrdude -? 2>&1 | findstr /i /c:"avrdude version"
echo === goil ===
REM --no-dialog: bản goil Windows mở hộp thoại chọn file nếu không có file .oil
goil --version --no-dialog
echo === arm-none-eabi-gcc ===
arm-none-eabi-gcc --version | findstr /b "arm-none-eabi-gcc"
echo === make ===
make --version | findstr /b "GNU Make"
echo === STM32_Programmer_CLI ===
REM --version chỉ in phiên bản, không kết nối ST-Link
STM32_Programmer_CLI --version | findstr /i "version"
echo.
