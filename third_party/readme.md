ZCELIB外部库说明
=============================================
我们使用了如下的外部库：

1. MySQL
2. RapidXML
3. SQLite
4. Protobuf
5. Lualib

你可以在zce_predefine.h文件里面找到相应的开关关闭实用这些外部库，当然如果关闭了，对应的封装你也就无法实用了。
相应外部库包括头文件部分和库文件部分。头文件部分放在include目录下，库文件放在lib目录下，库文件目录安装Linux和Windows的做了细分。package目录用于存放各种安装包。

大致目录结构是：


    ├─package
    ├─include
    │  ├─lualib-5.1.5
    │  ├─mysql-linux-5.6.22
    │  ├─mysql-win-5.6.22
    │  ├─protobuf-2.6.1
    │  ├─rapidxml-1.13
    │  └─sqlite-3.8.8.1
    ├─lib
    │   ├─linux
    │   │  ├─x86_32
    │   │  └─x86_64
    │   ├─win
    │   │  ├─Win32-v120-Debug
    │   │  ├─Win32-v120-Release
    │   │  ├─x64-v120-Debug
    │   │  └─x64-v120-Release

# MySQL
版本5.6.22
MySQL的头文件和库文件都来自MySQL的各种安装包提取出来的。
MySQL的头文件在Windows 下和Linux略有区别，所以头文件分成了2个目录存放。而相应的Windows Library，MySQL的安装包里面有2套。一套是static 库，一套是动态库，但static库在Visual Studio 2013下无法实用，会提示链接错误，只能实用动态库的版本。
Linux下，我们只使用相应的静态库。

# RepidXML
版本1.13
RepidXML的源码全部来自源安装包。
这个库用于解析XML文件，在配置读取等代码有实用。据说这个库的速度比tinyXML还要快，而且其只有头文件，不需要编译。这是我们选择他的原因。

# SQLite
版本3.8.8.1
SQLite的源码全部来自源安装包。里面的shell.c是命令行工具的去掉了。
在include的目录下包括SQLite的所有源码，以及我增加相应的Visual Studio 2013的工程文件。编译后相应的static库文件会放入lib目录下对应的子目录。
Linux 相应的Makefile也是我根据自己的Makefile简化一个通用的。
注意SQLite实用GCC编译的。不是G++.

# Protobuf
版本2.6.1
Protobuf的源码全部来自源安装包。
google本来就提供了工程文件，但为了编译的方便，我们升级到了Visual Studio 2013的版本，我还统一修改了输出目录的位置。
Linux下，Protobuf也有相应的AutoMakefile，编译会产生动态库和静态库，我们只使用相应的静态库。

# Lualib
版本5.1.5
在include的目录下包括Lualib的所有源码，以及我增加相应的Visual Studio 2013的工程文件。编译后相应的static库文件会放入lib目录下对应的子目录。
Linux下提供了Lualib相应的Makefile，Linux下，我们只使用相应的静态库。

# Visual Studio 工程说明
原来ZCELIB对Visual Studio的版本兼容做了很多努力，甚至可以实用2003的Visual C++编译，但这实在不是什么让人开心的事情，而且我喜欢C+11的部分特性，我干脆懒到低，只支持Visual C++ 2013了。好处是显而易见的。坏处是如果你要移植会讨厌不少。而且工程文件升级挺容易，降级就……，算了，说的好像有多少人会看你的代码一样。

Visual Studio的版本库管理其实挺麻烦的，（其实Linux下也不容易）。各种Visual Studio C运行时库之间的兼容性都不是特别好。所以我们坚持对不同的版本，把库文件放到不同的目录，而不是通过文件名称带d，之类的方法区分。熟悉Windows下开发的人会明白我的，这样能省略很很多麻烦。在工程设置上也会简单很多。

工程文件内部的设置如下,对于各种Configuration 和 Platform（填写时，Configuration 可以选择 All Configurations Platform 可以选择 All Platforms ），我们的配置都是一致的，参考如下：

- Output Directory  即 $(OutDir) 填写  $(SolutionDir)../lib/$(PlatformName)-$(PlatformToolset)-$(Configuration)/
- Intermediate Directory 填写  $(SolutionDir)../obj/$(PlatformName)-$(PlatformToolset)-$(Configuration)/$(ProjectName)/
- Character Set 填写  Not Set
- TargetName 选择  $(ProjectName)
- Output File 填写  $(OutDir)$(TargetName)$(TargetExt)

如果外部需要包含我们的库，目录配置上这样写，，引入库的文件名称也可以填写成一样。省事很多，也容易找到问题。
-Additional Libary Directories可以填写为 $(SolutionDir)../lib/$(PlatformName)-$(PlatformToolset)-$(Configuration)/

解释一下一些关键字。
- $(PlatformName)是Win32或者x64
- $(PlatformToolset)是v120，也就是Visual Studio 2013的版本号。大致是v110是2012，v100是2010，v90是2008，v80是2005，v71是2003，v70是是2002，v60是大名鼎鼎的Visaul Studio 6.0。为啥我这么熟悉？我都用过。
- $(Configuration) 是Debug或者Release。


注意Runtime Library和Character Set的填写。这个地方在Visual Studio的链接的时候是非常容易惹麻烦的。Runtime Library我们选择
- Debug版本的Runtime Library 选择  Multi-threaded Debug DLL (/MDd)
- Realse版本的Runtime Library 选择  Multi-threaded DLL (/MD)
如果你有特殊需求请自己调整。

#GCC 的版本
由于我使用了部分C++11的特性，所以最好选择GCC 4.7以后的版本。















