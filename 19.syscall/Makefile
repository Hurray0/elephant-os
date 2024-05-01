include ../include.mk
LIB = -I ./lib/kernel/ -I ./lib/ -I ./kernel/ -I ./thread/ -I ./device/ -I ./userprog/ -I ./lib/user/
ASLIB = -I ./boot/include/
ASFLAGS = -f elf
LDFLAGS = -m elf_i386 -Ttext $(ENTRY_POINT) -e main
CFLAGS = -m32 -c $(LIB) -fno-builtin -W -Wstrict-prototypes -Wmissing-prototypes -mno-sse # 这里加上-mno-sse是为了解决macOS下编译时出现的错误

all: $(BUILD_DIR)/hd60M.img debug-asm

################ c代码编译 ################
OBJS = $(BUILD_DIR)/main.o \
		$(BUILD_DIR)/interrupt.o \
		$(BUILD_DIR)/init.o \
		$(BUILD_DIR)/timer.o \
		$(BUILD_DIR)/debug.o \
		$(BUILD_DIR)/string.o \
		$(BUILD_DIR)/bitmap.o \
		$(BUILD_DIR)/memory.o \
		$(BUILD_DIR)/thread.o \
		$(BUILD_DIR)/list.o \
		$(BUILD_DIR)/console.o \
		$(BUILD_DIR)/sync.o \
		$(BUILD_DIR)/keyboard.o \
		$(BUILD_DIR)/ioqueue.o \
		$(BUILD_DIR)/tss.o \
		$(BUILD_DIR)/process.o \
		$(BUILD_DIR)/syscall.o \
		$(BUILD_DIR)/syscall-init.o

$(BUILD_DIR)/main.o: kernel/main.c lib/kernel/print.h kernel/debug.h kernel/init.h lib/string.h kernel/memory.h thread/thread.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/interrupt.o: kernel/interrupt.c kernel/global.h lib/kernel/io.h lib/kernel/print.h lib/stdint.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/init.o: kernel/init.c lib/kernel/print.h kernel/interrupt.h device/timer.h thread/thread.h device/console.h device/keyboard.h userprog/tss.h userprog/syscall-init.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/debug.o: kernel/debug.c kernel/debug.h lib/kernel/print.h kernel/interrupt.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/string.o: lib/string.c lib/string.h kernel/debug.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/bitmap.o: lib/kernel/bitmap.c lib/kernel/bitmap.h lib/stdint.h kernel/debug.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/memory.o: kernel/memory.c kernel/memory.h lib/stdint.h lib/kernel/bitmap.h kernel/debug.h lib/kernel/print.h lib/string.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/list.o: lib/kernel/list.c lib/kernel/list.h lib/stdint.h kernel/global.h kernel/interrupt.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/timer.o: device/timer.c device/timer.h lib/kernel/io.h lib/kernel/print.h kernel/interrupt.h kernel/debug.h thread/thread.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/console.o: device/console.c device/console.h lib/kernel/print.h lib/stdint.h thread/sync.h thread/thread.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/keyboard.o: device/keyboard.c device/keyboard.h lib/kernel/print.h lib/stdint.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/ioqueue.o: device/ioqueue.c device/ioqueue.h kernel/interrupt.h kernel/global.h kernel/debug.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/thread.o: thread/thread.c thread/thread.h lib/stdint.h lib/string.h kernel/global.h kernel/memory.h kernel/debug.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/sync.o: thread/sync.c thread/sync.h kernel/interrupt.h thread/thread.h kernel/debug.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/tss.o: userprog/tss.c userprog/tss.h lib/stdint.h lib/kernel/print.h kernel/global.h lib/string.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/process.o: userprog/process.c userprog/process.h device/console.h kernel/debug.h kernel/global.h kernel/interrupt.h lib/kernel/list.h kernel/memory.h lib/string.h thread/thread.h userprog/tss.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/syscall.o: lib/user/syscall.c lib/user/syscall.h lib/stdint.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/syscall-init.o: userprog/syscall-init.c userprog/syscall-init.h lib/stdint.h lib/kernel/print.h lib/user/syscall.h
	@$(GCC) $(CFLAGS) $< -o $@

################ 汇编代码编译 ################
BOOT = $(BUILD_DIR)/mbr.bin $(BUILD_DIR)/loader.bin

$(BUILD_DIR)/mbr.bin: boot/mbr.asm
	@$(AS) $(ASLIB) -o $@ $< -l $(BUILD_DIR)/mbr.bin.asm

$(BUILD_DIR)/loader.bin: boot/loader.asm
	@$(AS) $(ASLIB) -o $@ $< -l $(BUILD_DIR)/loader.bin.asm

OBJS += $(BUILD_DIR)/kernel.o $(BUILD_DIR)/print.o $(BUILD_DIR)/switch.o

$(BUILD_DIR)/print.o: lib/kernel/print.asm
	@$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/kernel.o: kernel/kernel.asm
	@$(AS) $(ASFLAGS) -o $@ $^

$(BUILD_DIR)/switch.o: thread/switch.asm
	@$(AS) $(ASFLAGS) -o $@ $^

################ 链接 ################
KERNEL = $(BUILD_DIR)/kernel.bin
$(BUILD_DIR)/kernel.bin: $(OBJS)
	@$(LD) $(LDFLAGS) -o $@ $^

################ 虚拟机构建 ################
$(BUILD_DIR)/hd60M.img: $(BUILD_DIR) $(BOOT) $(KERNEL)
	@if [ ! -f $(BUILD_DIR)/hd60M.img ]; then bximage -hd=60M -func="create" -q $(BUILD_DIR)/hd60M.img; fi
	@dd if=$(BUILD_DIR)/mbr.bin of=$(BUILD_DIR)/hd60M.img bs=512 count=1 conv=notrunc
	@dd if=$(BUILD_DIR)/loader.bin of=$(BUILD_DIR)/hd60M.img bs=512 count=4 seek=2 conv=notrunc
	@dd if=$(BUILD_DIR)/kernel.bin of=$(BUILD_DIR)/hd60M.img bs=512 count=200 seek=9 conv=notrunc