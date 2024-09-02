##### global settings #####

.PHONY: nemu entry testcase kernel run gdb test submit clean count count_
# .PHONY 声明伪目标，伪目标并不直接对应实际文件，而是表示一些任务或操作,此处我们在原有的基础上,添加count和count_命令

# ---	Makefile
# +++	Makefile
# @@	-56,2	+56,2	@@
# -USERPROG	=	obj/testcase/mov
# +USERPROG	=	obj/testcase/mov-c
# ENTRY	=	$(USERPROG)

CC := gcc # 定义编译器为 gcc
LD := ld  # 定义链接器为 ld
CFLAGS := -MMD -Wall -Werror -c
# 编译选项：
# -MMD: 生成依赖文件，用于自动管理文件依赖
# -Wall: 启用所有警告
# -Werror: 将警告视为错误（这个有的时候挺恶心的，但是有助于养成编程的好习惯）
# -c: 只编译，不链接

count:
	@echo "Counting lines of code (including empty lines)..."
	@find . -name '*.[ch]' | xargs wc -l

count_:
	@echo "Counting lines of code (excluding empty lines)..."
	@find . -name '*.[ch]' | xargs grep -v '^\s*$$' | wc -l
# count统计当前目录及其子目录中所有 `.c` 和 `.h` 文件的行数，包括空行
# count_统计当前目录及其子目录中所有 `.c` 和 `.h` 文件的行数，不包括空行(使用 grep -v '^\s*$$' 过滤掉空行，然后使用 wc -l 统计剩余行数)

LIB_COMMON_DIR := lib-common  # 定义库的通用目录路径
LIBC_INC_DIR := $(LIB_COMMON_DIR)/uclibc/include # 定义 uclibc 的头文件路径
LIBC_LIB_DIR := $(LIB_COMMON_DIR)/uclibc/lib  # 定义 uclibc 的库文件路径
LIBC := $(LIBC_LIB_DIR)/libc.a  # 定义 uclibc 的库文件

#FLOAT := obj/$(LIB_COMMON_DIR)/FLOAT/FLOAT.a
# 定义浮点运算库（目前被注释掉了）

include config/Makefile.git
include config/Makefile.build
# 包含两个配置 Makefile 文件，这些文件可以定义项目的 git 设置和构建相关的通用规则

all: nemu
# 默认目标是 `nemu`，当用户直接执行 `make` 时，会执行这个目标

##### rules for building the project #####

include nemu/Makefile.part
include testcase/Makefile.part
include lib-common/FLOAT/Makefile.part
include kernel/Makefile.part
include game/Makefile.part

nemu: $(nemu_BIN)  # 定义构建 `nemu` 的规则，依赖于变量 `$(nemu_BIN)`，`$(nemu_BIN)` 应该是在 `nemu/Makefile.part` 中定义的
testcase: $(testcase_BIN)  # 定义构建 `testcase` 的规则，依赖于变量 `$(testcase_BIN)`
kernel: $(kernel_BIN)  # 定义构建 `kernel` 的规则，依赖于变量 `$(kernel_BIN)`
game: $(game_BIN)  # 定义构建 `game` 的规则，依赖于变量 `$(game_BIN)`


##### rules for cleaning the project #####

clean-nemu:  # 删除 nemu 对应的构建输出目录
	-rm -rf obj/nemu 2> /dev/null

clean-testcase:  # 删除 nemu 对应的构建输出目录
	-rm -rf obj/testcase 2> /dev/null

clean-kernel:  # 删除 kernel 对应的构建输出目录
	-rm -rf obj/kernel 2> /dev/null

clean-game:  # 删除 game 对应的构建输出目录
	-rm -rf obj/game 2> /dev/null

clean: clean-cpp
	-rm -rf obj 2> /dev/null
	-rm -f *log.txt entry $(FLOAT) 2> /dev/null
# `clean` 目标调用 `clean-cpp`（这个目标应该在包含的 Makefile 中定义），然后删除 obj 目录，
# 删除日志文件和 entry 文件，以及浮点库（如果定义了 FLOAT）

##### some convinient rules #####

USERPROG := obj/testcase/mov  # 定义用户程序路径
# USERPROG := obj/testcase/quadratic-eq
ENTRY := $(USERPROG)  # 将 ENTRY 定义为用户程序路径
# ENTRY := $(kernel_BIN)

entry: $(ENTRY)  # `entry` 目标：使用 `objcopy`
	objcopy -S -O binary $(ENTRY) entry 

run: $(nemu_BIN) $(USERPROG) entry
	$(call git_commit, "run")
	$(nemu_BIN) $(USERPROG)

gdb: $(nemu_BIN) $(USERPROG) entry
	$(call git_commit, "gdb")
	gdb -s $(nemu_BIN) --args $(nemu_BIN) $(USERPROG)

test: $(nemu_BIN) $(testcase_BIN) entry
	$(call git_commit, "test")
	bash test.sh $(testcase_BIN)

submit: clean
	cd .. && zip -r $(STU_ID).zip $(shell pwd | grep -o '[^/]*$$')
