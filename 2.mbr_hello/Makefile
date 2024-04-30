include ../include.mk

all: $(BUILD_DIR)/hd60M.img

$(BUILD_DIR)/mbr.bin: mbr.asm
	@nasm -o $(BUILD_DIR)/mbr.bin mbr.asm

$(BUILD_DIR)/hd60M.img: $(BUILD_DIR) $(BUILD_DIR)/mbr.bin
	@if [ ! -f $(BUILD_DIR)/hd60M.img ]; then bximage -hd=60M -func="create" -q $(BUILD_DIR)/hd60M.img; fi
	@dd if=$(BUILD_DIR)/mbr.bin of=$(BUILD_DIR)/hd60M.img bs=512 count=1 conv=notrunc