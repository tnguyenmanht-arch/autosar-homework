@echo off
REM ============================================================================
REM env.bat - Đặt môi trường build AVR + Trampoline cho phiên cmd HIỆN TẠI.
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

REM --- Báo nếu thiếu thư mục nào (không dừng, để vẫn thấy lỗi ở phần in phiên bản) ---
for %%D in ("%AVR_GCC_BIN%" "%AVRDUDE_BIN%" "%GOIL_BIN%" "%WINLIBS_BIN%") do (
  if not exist "%%~D\" echo [CANH BAO] Khong tim thay thu muc: %%~D
)

REM --- Thêm vào ĐẦU PATH; chạy env.bat nhiều lần trong một phiên cũng chỉ thêm một lần ---
if not defined AUTOSAR_HW_ENV (
  set "PATH=%AVR_GCC_BIN%;%AVRDUDE_BIN%;%GOIL_BIN%;%WINLIBS_BIN%;%PATH%"
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
echo.
