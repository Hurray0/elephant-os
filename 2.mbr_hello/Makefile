.PHONY: all run clean

all: mbr.bin hd60M.img

mbr.bin: mbr.asm
	nasm -o mbr.bin mbr.asm

hd60M.img: mbr.bin
	if [ ! -f hd60M.img ]; then bximage -hd=60M -func="create" -q hd60M.img; fi
	dd if=mbr.bin of=hd60M.img bs=512 count=1 conv=notrunc

run: all
	bochs -f bochsrc.disk

clean:
	rm -f *.bin
	rm -f hd60M.img
	rm -f *.log
	rm -f hd60M.img.lock
	rm -f .*.un~
