include ../include.mk

LIB = -I ./lib/kernel/ -I ./lib/ -I ./kernel/
ASLIB = -I ./boot/include/
ASFLAGS = -f elf
LDFLAGS = -m elf_i386 -Ttext $(ENTRY_POINT) -e main
CFLAGS = -m32 -c $(LIB)
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/print.o $(BUILD_DIR)/interrupt.o $(BUILD_DIR)/init.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/timer.o
all: $(BUILD_DIR)/hd60M.img

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

$(BUILD_DIR)/hd60M.img: $(BUILD_DIR) $(BUILD_DIR)/mbr.bin $(BUILD_DIR)/loader.bin $(BUILD_DIR)/kernel.bin
	@if [ ! -f $(BUILD_DIR)/hd60M.img ]; then bximage -hd=60M -func="create" -q $(BUILD_DIR)/hd60M.img; fi
	@dd if=$(BUILD_DIR)/mbr.bin of=$(BUILD_DIR)/hd60M.img bs=512 count=1 conv=notrunc
	@dd if=$(BUILD_DIR)/loader.bin of=$(BUILD_DIR)/hd60M.img bs=512 count=4 seek=2 conv=notrunc
	@dd if=$(BUILD_DIR)/kernel.bin of=$(BUILD_DIR)/hd60M.img bs=512 count=200 seek=9 conv=notrunc