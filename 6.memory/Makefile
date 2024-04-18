.PHONY: all run clean

all: mbr.bin loader.bin hd60M.img env

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

virtualbox: all
	# 创建virtualbox虚拟机构建的硬盘
	VBoxManage convertfromraw --format VDI hd60M.img hd60M.vdi

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
	@rm -f *.bin
	@rm -f *.img
	@rm -f *.log
	@rm -f *.lock
	@rm -f .*.un~
	@rm -f *.vdi
