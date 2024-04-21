.PHONY: all lib kernel # 伪目标，不是文件名

BUILD_DIR = build
ENTRY_POINT = 0xc0001500
AS = nasm
LIB = -I ./lib/kernel/ -I ./lib/ -I ./kernel/
ASLIB = -I ./boot/include/
ASFLAGS = -f elf
LDFLAGS = -m elf_i386 -Ttext $(ENTRY_POINT) -e main
CFLAGS = -m32 -c $(LIB) -fno-builtin -W -Wstrict-prototypes -Wmissing-prototypes -mno-sse # 这里加上-mno-sse是为了解决macOS下编译时出现的错误

all: $(BUILD_DIR)/hd60M.img debug-asm

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	OS = "mac"
	GCC = x86_64-elf-gcc
	LD = x86_64-elf-ld
else
	OS = "linux"
	GCC = gcc
	LD = ld
endif

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

################ c代码编译 ################
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/interrupt.o $(BUILD_DIR)/init.o $(BUILD_DIR)/timer.o $(BUILD_DIR)/debug.o $(BUILD_DIR)/string.o
$(BUILD_DIR)/main.o: kernel/main.c lib/kernel/print.h kernel/debug.h kernel/init.h lib/string.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/interrupt.o: kernel/interrupt.c kernel/global.h lib/kernel/io.h lib/kernel/print.h lib/stdint.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/init.o: kernel/init.c lib/kernel/print.h kernel/interrupt.h device/timer.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/timer.o: device/timer.c device/timer.h lib/kernel/io.h lib/kernel/print.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/debug.o: kernel/debug.c kernel/debug.h lib/kernel/print.h kernel/interrupt.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/string.o: lib/string.c lib/string.h kernel/debug.h
	@$(GCC) $(CFLAGS) $< -o $@


################ 汇编代码编译 ################
BOOT = $(BUILD_DIR)/mbr.bin $(BUILD_DIR)/loader.bin

$(BUILD_DIR)/mbr.bin: boot/mbr.asm
	@$(AS) $(ASLIB) -o $@ $< -l $(BUILD_DIR)/mbr.bin.asm

$(BUILD_DIR)/loader.bin: boot/loader.asm
	@$(AS) $(ASLIB) -o $@ $< -l $(BUILD_DIR)/loader.bin.asm

OBJS += $(BUILD_DIR)/kernel.o $(BUILD_DIR)/print.o

$(BUILD_DIR)/print.o: lib/kernel/print.asm
	@$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/kernel.o: kernel/kernel.asm
	@$(AS) $(ASFLAGS) -o $@ $^

################ 链接 ################
KERNEL = $(BUILD_DIR)/kernel.bin
$(BUILD_DIR)/kernel.bin: $(OBJS)
	@$(LD) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/hd60M.img: $(BUILD_DIR) $(BOOT) $(KERNEL)
	@if [ ! -f $(BUILD_DIR)/hd60M.img ]; then bximage -hd=60M -func="create" -q $(BUILD_DIR)/hd60M.img; fi
	@dd if=$(BUILD_DIR)/mbr.bin of=$(BUILD_DIR)/hd60M.img bs=512 count=1 conv=notrunc
	@dd if=$(BUILD_DIR)/loader.bin of=$(BUILD_DIR)/hd60M.img bs=512 count=4 seek=2 conv=notrunc
	@dd if=$(BUILD_DIR)/kernel.bin of=$(BUILD_DIR)/hd60M.img bs=512 count=200 seek=9 conv=notrunc

run: all env
	@bochs -f bochsrc.disk

# 创建virtualbox虚拟机构建的硬盘
virtualbox: all
	@VBoxManage convertfromraw --format VDI build/hd60M.img build/hd60M.vdi

vb: virtualbox

# 修改bochsrc.disk文件，适配不同的操作系统
env:
	@if [ $(OS) = "mac" ]; then \
		sed -i '' 's/^#display_library: sdl2/display_library: sdl2/' bochsrc.disk; \
	elif [ $(OS) = "linux" ]; then \
		sed -i 's/^display_library: sdl2/#display_library: sdl2/' bochsrc.disk; \
	fi

# 恢复到原始状态
unenv:
	@if [ $(OS) = "mac" ]; then \
		sed -i '' 's/^display_library: sdl2/#display_library: sdl2/' bochsrc.disk; \
	else \
		sed -i 's/^display_library: sdl2/#display_library: sdl2/' bochsrc.disk; \
	fi

$(BUILD_DIR)/kernel.bin.asm: $(BUILD_DIR)/kernel.bin
	@objdump -d $(BUILD_DIR)/kernel.bin > $(BUILD_DIR)/kernel.bin.asm

debug-asm: $(BUILD_DIR)/kernel.bin.asm

clean: unenv
	@rm -rf build