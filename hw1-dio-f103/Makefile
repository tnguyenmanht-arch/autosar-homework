# Makefile HW1 — driver Dio theo AUTOSAR MCAL cho STM32F103C8 (bare-metal, KHÔNG HAL)
#
#   make          -> build ra build/dio_f103.elf/.hex/.bin và in kích thước
#   make clean    -> xoá build/
#   make flash    -> nạp qua ST-Link (SWD). CHỈ chạy khi đã cắm board và đồng ý nạp.
#
# Cần arm-none-eabi-gcc, make, STM32_Programmer_CLI trong PATH: chạy env.bat ở gốc repo trước.

# Cờ build, CMSIS, startup, linker nằm trong drivers/mcu/stm32f103/stm32f103.mk (chép từ can-ecu-network).
# File .mk tìm mọi thứ theo REPO_ROOT, nên ở đây REPO_ROOT là chính thư mục hw1-dio-f103.
REPO_ROOT := .
TARGET    := dio_f103

# Thêm Dio.c, Port.c, Det.c vào đây khi viết xong (đặt trong mcal/)
SRCS := app/main.c

INCS := -Igeneral -Iapp

include $(REPO_ROOT)/drivers/mcu/stm32f103/stm32f103.mk

# ---- Nạp ----
# -c port=SWD : kết nối ST-Link qua SWD
# -w <elf>    : ghi file ELF (địa chỉ lấy trong ELF, không cần khai báo)
# -v          : đọc lại để so sánh sau khi ghi
# -rst        : reset chip để chạy chương trình mới
PROGRAMMER ?= STM32_Programmer_CLI

.PHONY: flash
flash: $(ELF)
	$(PROGRAMMER) -c port=SWD -w $(ELF) -v -rst
