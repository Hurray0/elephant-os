.PHONY: all

# 使用shell命令找到所有以数字和点开头的目录
DIRS := $(shell find . -maxdepth 1 -type d -name '[0-9]*.*')

# 默认目标，依赖于所有目录的make目标
all: $(DIRS)
	$(foreach dir,$(DIRS),$(MAKE) -C $(dir);)

clean: $(DIRS)
	$(foreach dir,$(DIRS),$(MAKE) -C $(dir) clean;)