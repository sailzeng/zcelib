# 注意！:
# 1. 在你写自己项目的Makefile之前，为了更好的理解Makefile框架，你应该先读读'common.mk'

# 0表示是发行版，1表示是调试版
DEBUG := 1

# 首先设置PROJECT_DIR, 此路径应该设置成绝对路径，你要做的就是数..的个数。
# 项目的路径应该是import和src所在的目录。
PROJECT_DIR := $(shell (cd ../../../; pwd))

###############################################################################
# -- 注意: common.mk包含有Makefile所需要的功能的定义
include $(PROJECT_DIR)/src/build/linux/common.mk
###############################################################################

# !!-- 你应该在下面添加你自己项目的设置 --!!
# 注意！：
# 1. 必须使用'+=', 不允许使用'='
# 2. 如果没有就留空。例子： TDR_META_DIRS +=

# 添加源代码所在的目录, 所有在源代码中的c/cpp文件都会被编译
# 默认当前目录下的代码会被自动添加
# 注意：
# 1. 必须使用绝对路径
# 2. 必须使用'+=', 不允许使用'='
SRCS_DIRS +=

# 设置tdr配置描述文件路径
# 默认会将和Makefile同一个目录下的xml都认为是tdr配置描述文件
# 注意：
# 1. 必须使用绝对路径
# 2. 必须使用'+=', 不允许使用'='
TDR_META_DIRS +=

# 设置tdr协议描述文件路径
# 注意：
# 1. 必须使用绝对路径
# 2. 必须使用'+=', 不允许使用'='
#PROTOCOL_DIRS += ${PROTO_DIR}

# 预编译头文件
PREDEFINE_HEADER = soar_predefine.h

# 设置你的项目所依赖的库
# 注意：
# 1. 必须使用'+=', 不允许使用'='
DEP_LIBS += zenlib
DEP_LIBS += vas_key
DEP_LIBS += soar_proto
DEP_LIBS += tencrypt

# 添加编译所需要的额外编译选项
# 注意：
# 1. 必须使用'+=', 不允许使用'='
CXXFLAGS +=

# 添加链接所需要的额外链接选项
# 注意：
# 1. 必须使用'+=', 不允许使用'='
LDFLAGS +=

# 在build之前运行， 没有就留空
PREBUILD :=

# 在build之后运行， 没有就留空
POSTBUILD :=

# 静态库的短名字，最终的二进制的文件名为lib$(LIB_SHORTNAME).a
LIB_SHORTNAME := framework

# 包含生成静态库的Makefile框架
include $(PROJECT_DIR)/src/build/linux/static_library.mk

# vim: set ft=make ff=unix noexpandtab:
