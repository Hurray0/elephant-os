.PHONY: all run clean hd60M.img

all: mbr.bin loader.bin hd60M.img env

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	OS = "mac"
else
	OS = "linux"
endif

mbr.bin: mbr.asm
	@nasm -o mbr.bin mbr.asm -I ./

loader.bin: loader.asm
	@nasm -o loader.bin loader.asm -I ./

hd60M.img: mbr.bin loader.bin
	@if [ ! -f hd60M.img ]; then bximage -hd=60M -func="create" -q hd60M.img; fi
	@dd if=mbr.bin of=hd60M.img bs=512 count=1 conv=notrunc
	@dd if=loader.bin of=hd60M.img bs=512 count=4 seek=2 conv=notrunc

run: all
	@bochs -f bochsrc.disk

# 创建virtualbox虚拟机构建的硬盘
virtualbox: all
	@VBoxManage convertfromraw --format VDI hd60M.img hd60M.vdi

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
	@rm -f *.bin
	@rm -f *.img
	@rm -f *.log
	@rm -f *.lock
	@rm -f .*.un~
	@rm -f *.vdi
