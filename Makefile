.PHONY: all clean

# 使用shell命令找到所有以数字和点开头的目录
DIRS := $(shell find . -maxdepth 1 -type d -name '[0-9]*.*' -exec basename {} \; | sort -V)

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	OS = "mac"
	SED = sed -i ''
	ECHO = echo
else
	OS = "linux"
	SED = sed -i
	ECHO = echo -e
endif

# 默认目标，依赖于所有目录的make目标
all: $(DIRS)
	@$(foreach dir,$(DIRS),$(MAKE) all -C $(dir);)

clean:
	@$(foreach dir,$(DIRS),$(MAKE) -C $(dir) clean;)

# run目标，让用户选择一个目录来执行make run
# 如果用户输入的目录不存在，则输出错误信息
# 如果用户输入的不是数字，则运行最后一个目录
run:
	@printf "Available directories:\n"; \
	$(foreach dir,$(DIRS),printf "%s\t%s\n" $(dir) "`head -n 1 $(dir)/README.md`";)
	@printf "\n"
	@read -p "Enter a directory number to run: " number; \
	if [ -z "$$number" ]; then \
		dir=$(shell echo $(DIRS) | awk '{print $$NF}'); \
	else \
		dir="./$$number.*"; \
	fi; \
	if [ -d $$dir ]; then \
		echo "Running $$dir"; \
		$(MAKE) -C $$dir run; \
	else \
		echo "Directory $$dir does not exist."; \
	fi

vb: virtualbox

virtualbox: $(DIRS)
	$(foreach dir,$(DIRS),$(MAKE) -C $(dir) virtualbox;)

# 首先遍历读取所有的文件夹，读取其README.md文件的全部内容，将其内容写到一个临时文件中（改变标题级别为3级，并输出文件夹名称）
# 修改tmp.md中的路径，由'../doc/'改为'./doc/'
# 删除README.md中的项目列表部分，将临时文件的内容追加到README.md中
README.md: $(DIRS) Makefile
	@rm -f tmp.md
	@$(foreach dir,$(DIRS),$(ECHO) "### $(dir) - `head -n 1 $(dir)/README.md | cut -c 3-`" >> tmp.md\
		&& $(ECHO) "目录链接：[$(dir)](./$(dir))\n" >> tmp.md \
		&& $(ECHO) "`tail -n +2 $(dir)/README.md`" >> tmp.md  && $(ECHO) "\n\n" >> tmp.md;)
	@$(SED) 's/\.\.\/doc\//\.\/doc\//g' tmp.md
	@$(SED) '/## 项目列表/,$$d' README.md
	@$(ECHO) "## 项目列表" >> README.md
	@cat tmp.md >> README.md
	@rm -f tmp.md

docs: README.md

env:
	$(foreach dir,$(DIRS),$(MAKE) -C $(dir) env;)