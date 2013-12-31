#============================================================================================================================
#Author : Sail (ZENGXING) 
#2009 06 16 纪念1888年的今天《國際歌》誕生。 今天，接近完美的Makefile诞生了.
#编译COMMLIB库,定义MAKE规则,
#
#你无需再修改这个makefile,把.c,.cpp.h文件摔在目录下就可以了.依赖关系我会自己处理,
#这个Makefile已经对.h文件的依赖做了检查,是一个很酷的Makefile，你可以崇拜我.
#其实Makefile对于我，一致是一个旁门左道.我每次用改都会很痛苦,但是我希望证明的一点是，别人能做到的，我也可以.
#也许每一次使用都是一次进步，哈哈.
#所以鸣谢三篇Makefile文章的作者《GNU Make 指南》《跟我一起写 Makefile》《GNUMake 手册》,也像大叔表示敬意Richard Stallman,Roland McGrath
#
#另外，我没有使用GNUMake的默认依赖关系，我实在不喜欢那种方式，虽然ACE也用了这种关系,(或者说我还没有搞懂)
#
#Richard Stallman说：软件就像性，免费的比较好.
#============================================================================================================================

#SAILLIB_ROOT必现提前定义，


#你要定义输出文件,和PRGNAME的名称这两个变量
#PRGNAME表示你的程序名称，一般就是你的目录名称，当然希望目录名称不要重复，
#OUTFILE,最总输出的文件名称，为啥要定义这个呢，最后删除的时候可以统一
PRGNAME = zealot

#如果你要编译静态库 staticlib
#compile_object = staticlib
#如果你要编译动态库
#compile_object = dynamiclib
#如果你要编译可执行性程序
compile_object = executeprg

#如果你使用预定头文件,请定义PREH_FILE,比如下面这样
#对应生成的GCH文件还只能放在当前目录下比较简单，放入其他目录很难保证其正常运行，打包源文件的脚本必须删除gch文件.
#如果你不适用预定头文件，可以屏蔽下面这个定义
PREH_FILE = ./zealot_predefine.h


#DEPLIB_DIR,标识编译当前的前提（依赖）目标目录  标识依赖目标的编译的库（目录）是什么，可以是多个
#如果依赖多个目录，请按照依赖的顺序填写，用空格隔开，
DEPENDS_DIR := $(SAILLIB_ROOT)/src/commlib/zenlib   

DEPENDS_LIB := $(SAILLIB_ROOT)/lib/libzenlib.a

#这个里面定义所有的变量和定义，以及包含了平台性相关定义,你自己看吧
include $(SAILLIB_ROOT)/src/make/makefile.define

#如果你想改变Mafile的规则，请在这个地方加入你自己的定义.

#这个定义其他所有的规则,还是你自己看吧
include $(SAILLIB_ROOT)/src/make/makefile.rule

