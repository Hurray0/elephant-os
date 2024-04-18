.PHONY: kernel boot # 伪目标，不是文件名

all: boot disk env

boot: boot/mbr.asm boot/loader.asm
	@mkdir -p build
	@nasm -o build/mbr.bin boot/mbr.asm -I ./include/
	@nasm -o build/loader.bin boot/loader.asm -I ./include/

kernel: kernel/main.c
	@mkdir -p build
	@gcc -m32 -c -o build/main.o kernel/main.c -I ./lib/kernel/ -I ./lib/
	@nasm -f elf -o build/print.o lib/kernel/print.asm  # 这里要用elf格式
	@ld -m elf_i386 -Ttext 0c0001500 -e main -o build/kernel.bin build/main.o build/print.o

disk: boot kernel
	@if [ ! -f build/hd60M.img ]; then bximage -hd=60M -func="create" -q build/hd60M.img; fi
	@dd if=build/mbr.bin of=build/hd60M.img bs=512 count=1 conv=notrunc
	@dd if=build/loader.bin of=build/hd60M.img bs=512 count=4 seek=2 conv=notrunc
	@dd if=build/kernel.bin of=build/hd60M.img bs=512 count=200 seek=9 conv=notrunc

run: all
	@bochs -f bochsrc.disk

virtualbox: all
	# 创建virtualbox虚拟机构建的硬盘
	VBoxManage convertfromraw --format VDI build/hd60M.img build/hd60M.vdi

vb: virtualbox

# 对于macOS，需要将存在的"#display_library: sdl2"取消注释；对于CentOS, 需要添加注释
# 首先判断操作系统，然后根据操作系统执行不同的命令
# 遍历所有目录，使用sed命令修改bochsrc.disk文件
env:
	@os=`uname -s`; \
	if [ $$os = "Darwin" ]; then \
		sed -i '' 's/^#display_library: sdl2/display_library: sdl2/' bochsrc.disk; \
	elif [ $$os = "Linux" ]; then \
		sed -i 's/^display_library: sdl2/#display_library: sdl2/' bochsrc.disk; \
	fi


clean:
	@rm -rf build
