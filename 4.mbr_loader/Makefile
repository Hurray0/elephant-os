.PHONY: all run clean

all: mbr.bin loader.bin hd60M.img

mbr.bin: mbr.asm
	nasm -o mbr.bin mbr.asm -I ./

loader.bin: loader.asm
	nasm -o loader.bin loader.asm -I ./

hd60M.img: mbr.bin loader.bin
	if [ ! -f hd60M.img ]; then bximage -hd=60M -func="create" -q hd60M.img; fi
	dd if=mbr.bin of=hd60M.img bs=512 count=1 conv=notrunc
	dd if=loader.bin of=hd60M.img bs=512 count=4 seek=2 conv=notrunc

run: all
	bochs -f bochsrc.disk

clean:
	rm -f *.bin
	rm -f hd60M.img
	rm -f .log
	rm -f hd60M.img.lock
	rm -f ..un~