include ../include.mk
LIB = -I ./lib/kernel/ -I ./lib/ -I ./kernel/ -I ./thread/ -I ./device/ -I ./userprog/ -I ./lib/user/ -I ./fs/
ASLIB = -I ./boot/include/
ASFLAGS = -f elf
LDFLAGS = -m elf_i386 -Ttext $(ENTRY_POINT) -e main
CFLAGS = -m32 -c $(LIB) -fno-builtin -W -Wstrict-prototypes -Wmissing-prototypes -mno-sse # 这里加上-mno-sse是为了解决macOS下编译时出现的错误

all: $(BUILD_DIR)/hd80M.img $(BUILD_DIR)/hd60M.img debug-asm
# all: $(BUILD_DIR)/hd60M.img debug-asm

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
		$(BUILD_DIR)/syscall-init.o \
		$(BUILD_DIR)/stdio.o \
		$(BUILD_DIR)/ide.o \
		$(BUILD_DIR)/stdio-kernel.o \
		$(BUILD_DIR)/fs.o \
		$(BUILD_DIR)/inode.o \
		$(BUILD_DIR)/file.o \
		$(BUILD_DIR)/dir.o \
		$(BUILD_DIR)/fork.o

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

$(BUILD_DIR)/syscall-init.o: userprog/syscall-init.c userprog/syscall-init.h lib/stdint.h lib/kernel/print.h lib/user/syscall.h device/console.h lib/string.h fs/fs.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/stdio.o: lib/stdio.c lib/stdio.h lib/user/syscall.h lib/string.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/ide.o: device/ide.c device/ide.h lib/stdint.h thread/sync.h \
    	lib/kernel/list.h kernel/global.h thread/thread.h lib/kernel/bitmap.h \
     	kernel/memory.h lib/kernel/io.h lib/stdio.h lib/stdint.h lib/kernel/stdio-kernel.h \
	kernel/interrupt.h kernel/debug.h device/console.h device/timer.h lib/string.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/stdio-kernel.o: lib/kernel/stdio-kernel.c lib/kernel/stdio-kernel.h lib/kernel/print.h device/console.h lib/stdio.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/fs.o: fs/fs.c fs/fs.h kernel/debug.h fs/dir.h kernel/global.h \
					device/ide.h fs/inode.h lib/kernel/list.h kernel/memory.h \
					lib/stdint.h lib/kernel/stdio-kernel.h lib/string.h fs/super_block.h \
					fs/file.h device/console.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/inode.o: fs/inode.c fs/inode.h kernel/debug.h fs/fs.h kernel/global.h device/ide.h lib/kernel/list.h kernel/memory.h lib/stdint.h lib/kernel/stdio-kernel.h lib/string.h fs/super_block.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/file.o: fs/file.c fs/file.h kernel/debug.h fs/fs.h kernel/global.h fs/inode.h kernel/interrupt.h kernel/memory.h lib/kernel/stdio-kernel.h lib/string.h fs/super_block.h thread/thread.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/dir.o: fs/dir.c fs/dir.h kernel/debug.h fs/file.h fs/fs.h kernel/global.h fs/inode.h kernel/interrupt.h kernel/memory.h lib/stdint.h lib/kernel/stdio-kernel.h lib/string.h fs/super_block.h
	@$(GCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/fork.o: userprog/fork.c userprog/fork.h kernel/debug.h fs/file.h kernel/interrupt.h kernel/memory.h userprog/process.h lib/string.h thread/thread.h
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

$(BUILD_DIR)/hd80M_no.img: $(BUILD_DIR)
	@if [ ! -f $(BUILD_DIR)/hd80M.img ]; then bximage -hd=80M -func="create" -q $(BUILD_DIR)/hd80M_no.img; fi

partitionDisk-mac: $(BUILD_DIR)/hd80M_no.img
	@DEVICE=$$(hdiutil attach -nomount $(BUILD_DIR)/hd80M_no.img | cut -f 1); \
	diskutil partitionDisk $$DEVICE 2 MBR "MS-DOS FAT12" "BOOT" 20M "MS-DOS FAT12" "ROOT1" 10M; \
	hdiutil detach $$DEVICE; \
	hdiutil pmap $(BUILD_DIR)/hd80M_no.img
	@mv $(BUILD_DIR)/hd80M_no.img $(BUILD_DIR)/hd80M.img

# 还没有测试
partitionDisk-linux: $(BUILD_DIR)/hd80M_no.img
	@echo -e "x\nc\n162\nh\n16\nr\nn\np\n1\n2048\n18175\nn\ne\n4\n18432\n163295\nn\n20480\n29551\nn\n32768\n45367\nn\n49152\n70000\nn\n130000\n163295\nt\n5\n66\nt\n6\n66\nt\n7\n66\nt\n8\n66\nw\n" | fdisk $(BUILD_DIR)/hd80M_no.img
	@mv $(BUILD_DIR)/hd80M_no.img $(BUILD_DIR)/hd80M.img

$(BUILD_DIR)/hd80M.img:
	make partitionDisk-$(OS)