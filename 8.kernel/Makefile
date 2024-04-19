.PHONY: kernel boot

all: boot disk env

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	OS = mac
	GCC = x86_64-elf-gcc
	LD = x86_64-elf-ld
else
	OS = linux
	GCC = gcc
	LD = ld
endif

boot: boot/mbr.asm boot/loader.asm
	@mkdir -p build
	@nasm -o build/mbr.bin boot/mbr.asm -I ./include/
	@nasm -o build/loader.bin boot/loader.asm -I ./include/

kernel: kernel/main.c
	@mkdir -p build
	@$(GCC) -m32 -c -o build/main.o kernel/main.c
	@$(LD) -m elf_i386 -Ttext 0c0001500 -e main -o build/kernel.bin build/main.o

disk: boot kernel
	@if [ ! -f build/hd60M.img ]; then bximage -hd=60M -func="create" -q build/hd60M.img; fi
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