include ../include.mk

all: build/hd60M.img

$(BUILD_DIR)/mbr.bin: boot/mbr.asm
	@nasm -o $(BUILD_DIR)/mbr.bin boot/mbr.asm -I ./include/

$(BUILD_DIR)/loader.bin: boot/loader.asm
	@nasm -o $(BUILD_DIR)/loader.bin boot/loader.asm -I ./include/

$(BUILD_DIR)/main.o: kernel/main.c lib/kernel/print.h
	@$(GCC) -m32 -c -o $(BUILD_DIR)/main.o kernel/main.c -I ./lib/kernel/ -I ./lib/

$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/main.o $(BUILD_DIR)/print.o
	@mkdir -p build
	@$(LD) -m elf_i386 -Ttext 0c0001500 -e main -o $(BUILD_DIR)/kernel.bin $(BUILD_DIR)/main.o $(BUILD_DIR)/print.o

$(BUILD_DIR)/hd60M.img: $(BUILD_DIR) $(BUILD_DIR)/mbr.bin $(BUILD_DIR)/loader.bin $(BUILD_DIR)/kernel.bin
	@if [ ! -f build/hd60M.img ]; then bximage -hd=60M -func="create" -q $(BUILD_DIR)/hd60M.img; fi
	@dd if=$(BUILD_DIR)/mbr.bin of=$(BUILD_DIR)/hd60M.img bs=512 count=1 conv=notrunc
	@dd if=$(BUILD_DIR)/loader.bin of=$(BUILD_DIR)/hd60M.img bs=512 count=4 seek=2 conv=notrunc
	@dd if=$(BUILD_DIR)/kernel.bin of=$(BUILD_DIR)/hd60M.img bs=512 count=200 seek=9 conv=notrunc

$(BUILD_DIR)/print.o: lib/kernel/print.asm
	@nasm -f elf -o build/print.o lib/kernel/print.asm