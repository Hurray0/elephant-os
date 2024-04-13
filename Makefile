.PHONY: all

# 使用shell命令找到所有以数字和点开头的目录
DIRS := $(shell find . -maxdepth 1 -type d -name '[0-9]*.*' | sort)

# 默认目标，依赖于所有目录的make目标
all: $(DIRS)
	$(foreach dir,$(DIRS),$(MAKE) -C $(dir);)

clean: $(DIRS)
	$(foreach dir,$(DIRS),$(MAKE) -C $(dir) clean;)

# run目标，让用户选择一个目录来执行make run
run:
	@printf "Available directories:\n"; \
	$(foreach dir,$(DIRS),printf "%s\n" $(dir);)
	@read -p "Enter a directory number to run: " number; \
	dir="./$$number.*"; \
	if [ -d $$dir ]; then \
		$(MAKE) -C $$dir run; \
	else \
		echo "Directory $$dir does not exist."; \
	fi