.PHONY: all run clean virtualbox vb

all: mbr.bin hd60M.img

mbr.bin: mbr.asm
	nasm -o mbr.bin mbr.asm

hd60M.img: mbr.bin
	if [ ! -f hd60M.img ]; then bximage -hd=60M -func="create" -q hd60M.img; fi
	dd if=mbr.bin of=hd60M.img bs=512 count=1 conv=notrunc

run: all
	bochs -f bochsrc.disk

virtualbox: all
	# 创建virtualbox虚拟机构建的硬盘
	VBoxManage convertfromraw --format VDI hd60M.img hd60M.vdi

vb: virtualbox

clean:
	rm -f *.bin
	rm -f *.img
	rm -f *.log
	rm -f *.lock
	rm -f .*.un~
	rm -f *.vdi
