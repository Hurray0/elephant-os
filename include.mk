.PHONY: all # 伪目标，不是文件名

# 虚拟机名称
VBOX_OS_NAME = "MyOS2"

QEMU = qemu-system-i386
AS = nasm
ENTRY_POINT = 0xc0001500

UNAME_S := $(shell uname -s)
DEBUG_FLAG = -g
ifeq ($(UNAME_S),Darwin)
	OS = "mac"
	GCC = x86_64-elf-gcc $(DEBUG_FLAG)
	LD = x86_64-elf-ld
	GDB = x86_64-elf-gdb
else
	OS = "linux"
	GCC = gcc $(DEBUG_FLAG)
	LD = ld
	GDB = gdb
endif

BUILD_DIR = build

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# 修改bochsrc.disk文件，适配不同的操作系统
env:
	@if [ $(OS) = "mac" ]; then \
		sed -i '' 's/^#display_library: sdl2/display_library: sdl2/' bochsrc.disk; \
	elif [ $(OS) = "linux" ]; then \
		sed -i 's/^display_library: sdl2/#display_library: sdl2/' bochsrc.disk; \
	fi

# 恢复到原始状态
unenv:
	@if [ $(OS) = "mac" ]; then \
		sed -i '' 's/^display_library: sdl2/#display_library: sdl2/' bochsrc.disk; \
	else \
		sed -i 's/^display_library: sdl2/#display_library: sdl2/' bochsrc.disk; \
	fi

# 创建virtualbox虚拟机构建的硬盘
$(BUILD_DIR)/hd60M.vdi: $(BUILD_DIR)/hd60M.img
	@rm -f $(BUILD_DIR)/hd60M.vdi
	@VBoxManage convertfromraw --format VDI $(BUILD_DIR)/hd60M.img $(BUILD_DIR)/hd60M.vdi

# 让用户选择1. bochs 2. qemu 3. virtualbox
run:
	@echo "1. bochs(default)"
	@echo "2. qemu"
	@echo "3. virtualbox"
	@read -p "Enter a number to run: " number; \
	if [ -z "$$number" ]; then \
		$(MAKE) run_bochs; \
	elif [ $$number -eq 1 ]; then \
		$(MAKE) run_bochs; \
	elif [ $$number -eq 2 ]; then \
		$(MAKE) run_qemu; \
	elif [ $$number -eq 3 ]; then \
		$(MAKE) run_vb; \
	fi

run_bochs: $(BUILD_DIR)/hd60M.img env
	@bochs -f bochsrc.disk

run_vb: $(BUILD_DIR)/hd60M.vdi
	@if [ -n "`VBoxManage showvminfo ${VBOX_OS_NAME} | grep 'Port 0,'`" ]; then \
		VBoxManage storageattach $$(VBoxManage list vms | grep '${VBOX_OS_NAME}' | awk '{print $$2}') --storagectl "IDE" --port 0 --device 0 --type hdd --medium none; \
		VBoxManage closemedium disk $$(VBoxManage list hdds | grep 'hd60M.vdi' | awk '{print $$2}') --delete; \
	fi
	$(MAKE) $(BUILD_DIR)/hd60M.vdi
	@VBoxManage storageattach $$(VBoxManage list vms | grep '${VBOX_OS_NAME}' | awk '{print $$2}') --storagectl "IDE" --port 0 --device 0 --type hdd --medium build/hd60M.vdi
	@VBoxManage startvm $$(VBoxManage list vms | grep '${VBOX_OS_NAME}' | awk '{print $$2}')

run_qemu: $(BUILD_DIR)/hd60M.img
	@nohup $(QEMU) -s -S -hda $^ > /dev/null 2>&1 &
	$(GDB) -q -ex "file $(BUILD_DIR)/kernel.bin" -ex "target remote :1234" -ex "b *0x7c00" -ex "c"

$(BUILD_DIR)/kernel.bin.asm: $(BUILD_DIR)/kernel.bin
	@objdump -d $(BUILD_DIR)/kernel.bin > $(BUILD_DIR)/kernel.bin.asm

debug-asm: $(BUILD_DIR)/kernel.bin.asm

clean: unenv
	@rm -rf $(BUILD_DIR)