.PHONY: kernel boot # 伪目标，不是文件名

BUILD_DIR = build
ENTRY_POINT = 0xc0001500
AS = nasm
LIB = -I ./lib/kernel/ -I ./lib/ -I ./kernel/
ASLIB = -I ./boot/include/
ASFLAGS = -f elf
LDFLAGS = -m elf_i386 -Ttext $(ENTRY_POINT) -e main
CFLAGS = -m32 -c $(LIB)
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/print.o $(BUILD_DIR)/interrupt.o $(BUILD_DIR)/init.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/timer.o
all: mk_dir build disk env

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

mk_dir:
	@mkdir -p $(BUILD_DIR)
	@if [ ! -f $(BUILD_DIR)/hd60M.img ]; then bximage -hd=60M -func="create" -q $(BUILD_DIR)/hd60M.img; fi

################ c代码编译 ################
$(BUILD_DIR)/main.o: kernel/main.c lib/kernel/print.h lib/stdint.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/interrupt.o: kernel/interrupt.c
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/init.o: kernel/init.c
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/timer.o: device/timer.c device/timer.h lib/kernel/io.h lib/kernel/print.h
	@$(GCC) $(CFLAGS) $< -o $@


################ 汇编代码编译 ################
$(BUILD_DIR)/mbr.bin: boot/mbr.asm
	@$(AS) $(ASLIB) -o $@ $<

$(BUILD_DIR)/loader.bin: boot/loader.asm
	@$(AS) $(ASLIB) -o $@ $<

$(BUILD_DIR)/print.o: lib/kernel/print.asm
	@$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/kernel.o: kernel/kernel.asm
	@$(AS) $(ASFLAGS) -o $@ $^

################ 链接 ################
$(BUILD_DIR)/kernel.bin: $(OBJS)
	@$(LD) $(LDFLAGS) -o $@ $^

build: $(BUILD_DIR)/mbr.bin $(BUILD_DIR)/loader.bin $(BUILD_DIR)/kernel.bin

disk: build
	@dd if=build/mbr.bin of=build/hd60M.img bs=512 count=1 conv=notrunc
	@dd if=build/loader.bin of=build/hd60M.img bs=512 count=4 seek=2 conv=notrunc
	@dd if=build/kernel.bin of=build/hd60M.img bs=512 count=200 seek=9 conv=notrunc

run: all
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

clean: unenv
	@rm -rf build