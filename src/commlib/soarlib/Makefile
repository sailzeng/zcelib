#SAILLIB_ROOT必现提前定义，
#所有的工程下面的代码，可以使用相同的编译选项和方法，
#所有的预定义，编译方法，编译参数在src/make/makefile.define 
# src/make/makefile.linux src/make/makefile.rule 3个文件中都已经进行了
#预定义，你可以直接使用。
#如果你没有特殊需求，你就只用打开关闭下面定义就可以了。

#你要定义输出文件,和PRGNAME的名称这两个变量
#PRGNAME表示你的程序名称，一般就是你的目录名称，当然希望目录名称不要重复，
#OUTFILE,最总输出的文件名称，为啥要定义这个呢，最后删除的时候可以统一
PRGNAME = soarlib

#如果你要编译静态库 staticlib
compile_object = staticlib
#如果你要编译动态库
#compile_object = dynamiclib
#如果你要编译可执行性程序
#compile_object = executeprg

#如果你使用预定头文件,请定义PREH_FILE,比如下面这样
#对应生成的GCH文件还只能放在当前目录下比较简单，放入其他目录很难保证其正常运行，打包源文件的脚本必须删除gch文件.
#如果你不适用预定头文件，可以屏蔽下面这个定义
PREH_FILE = ./zce_predefine.h

#DEPLIB_DIR,标识编译当前的前提（依赖）目标目录  标识依赖目标的编译的库（目录）是什么，可以是多个
#如果依赖多个目录，请按照依赖的顺序填写，用空格隔开，
#DEPENDS_DIR := $(ZCELIB_ROOT)/src/commlib/zcelib  

#这个里面定义所有的变量和定义，以及包含了平台性相关定义,你自己看吧
include $(ZCELIB_ROOT)/src/make/makefile.define

#如果你想改变Mafile的规则，请在这个地方加入你自己的定义.

#这个定义其他所有的规则,还是你自己看吧
include $(ZCELIB_ROOT)/src/make/makefile.rule

