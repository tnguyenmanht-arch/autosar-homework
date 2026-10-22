# Makefile chạy thử Dio/Port/Det trên PC với thanh ghi GPIO giả lập. KHÔNG liên quan build firmware.
#
#   make -C tests/host          -> biên dịch bằng g++ rồi chạy, mã thoát 0 = đạt hết
#   make -C tests/host clean    -> xoá tests/host/build/
#
# Cần g++ trong PATH. Máy này: sau env.bat là g++ 16.1 của WinLibs (đứng trước trong PATH),
# không chạy env.bat thì là g++ 6.3 của MinGW (C:\MinGW\bin). Cả hai đều chạy được.
# Nguồn .c được dịch như C++ (-x c++) vì thanh ghi giả lập là lớp C++.
# NULL_PTR = 0 vì C++ không tự đổi (void *)0 sang con trỏ kiểu khác; Compiler.h có #ifndef NULL_PTR.

ROOT  := ../..

# make của CubeIDE có sẵn CXX = x86_64-w64-mingw32-g++, nên "?=" không có tác dụng.
# Chỉ thay khi CXX còn là giá trị mặc định; vẫn ghi đè được bằng: make -C tests/host CXX=...
ifeq ($(origin CXX),default)
CXX := g++
endif
BUILD := build
EXE   := $(BUILD)/dio_host_test.exe

SRCS := $(ROOT)/drivers/dio/Dio.c $(ROOT)/drivers/dio/Dio_Cfg.c \
        $(ROOT)/drivers/port/Port.c $(ROOT)/drivers/port/Port_Cfg.c \
        $(ROOT)/services/det/Det.c

# -I. đứng đầu: stm32f1xx.h giả lập ở đây được chọn thay bản CMSIS
INCS := -I. -I$(ROOT)/general -I$(ROOT)/app -I$(ROOT)/drivers/dio -I$(ROOT)/drivers/port \
        -I$(ROOT)/services/det -I$(ROOT)/drivers/mcu/stm32f103

CXXFLAGS := -std=gnu++11 -Wall -Wextra -DNULL_PTR=0

.PHONY: all run clean
all: run

$(EXE): sim.cpp stm32f1xx.h $(SRCS) $(ROOT)/app/main.c
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCS) sim.cpp -x c++ $(SRCS) -o $@

run: $(EXE)
	./$(EXE)

clean:
	rm -rf $(BUILD)
