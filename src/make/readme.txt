Makefile中间用了gcc,g++的很多选项，做一些说明


-Wpointer-arith 选项
5.17 Arithmetic on void- and Function-Pointers
In GNU C, addition and subtraction operations are supported on pointers to void and on pointers to functions.
This is done by treating the size of a void or of a function as 1.
A consequence of this is that sizeof is also allowed on void and on function types, and returns 1.
The option -Wpointer-arith requests a warning if these extensions are used.
注意这个不在-Wall里面



-fvisibility=hidden 选项
在windows中，我们可以指定__declspec(dllexport)定义来控制导出函数，在linux下，我们也有类似的控制参数。

在GCC帮助文档 -fvisibility=default|internal|hidden|protected参数下有这样一段描述：

a superior solution made possible by this option to marking things 
hidden when the default is public is to make the default hidden and 
mark things public. This is the norm with DLL's on Windows and with 
-fvisibility=hidden and "__attribute__ ((visibility("default")))" 
instead of "__declspec(dllexport)" you get almost identical semantics 
with identical syntax. This is a great boon to those working with cross-platform projects.

需要了解的是，在linux下，源文件中的所有函数都有一个默认的visibility属性为public，在编译命令中加入 -fvisibility=hidden参数，
会将所有默认的public的属性变为hidden。此时，如果对函数设置__attribute__ ((visibility("default")))参数，
使特定的函数仍然按默认的public属性处理，则-fvisibility=hidden参数不会对该函数起作用。
所以，设置了-fvisibility=hidden参数之后，只有设置了__attribute__ ((visibility("default")))的函数才是对外可见的，
如此则效果等同于Visual Studio下的__declspec(dllexport)定义。




gcc 的 -g 和 -ggdb 选项

-g 和 -ggdb 都是令 gcc 生成调试信息，但是它们也是有区别的

-g
该选项可以利用操作系统的“原生格式（native format）”生成调试
信息。GDB 可以直接利用这个信息，其它调试器也可以使用这个调试信息

-ggdb
使 GCC 为 GDB 生成专用的更为丰富的调试信息，但是，此时就不能用其
他的调试器来进行调试了 (如 ddx)

-g 和 -ggdb 也是分级别的

-g2
这是默认的级别，此时产生的调试信息包括扩展的符号表、行号、局部
或外部变量信息。

-g3
包含级别2中的所有调试信息，以及源代码中定义的宏

-g1
级别1（-g1）不包含局部变量和与行号有关的调试信息，因此只能够用于回溯
跟踪和堆栈转储之用。回溯跟踪指的是监视程序在运行过程中的函数调用历
史，堆栈转储则是一种以原始的十六进制格式保存程序执行环境的方法，两者
都是经常用到的调试手段。




如果在/home/xiaowp/lib/目录下有链接时所需要的库文件libfoo.so和libfoo.a

为了让GCC在链接时只用到静态链接库，可以使用下面的命令：

gcc -o test test.c -L /home/xiaowp/lib -static -lfoo


-x 选项，
-x选项用于预编译头文件处理，


-E
这个选项我的makefile没有用，但是请你注意一下这个选项，如果希望吧代码的宏展开看看。
进行预编译，使用-E参数可以让GCC在预处理结束后停止编译过程：
gcc -E hello.c -o hello.i
调试宏错误时常用。
　　只激活预处理,这个不生成文件,你需要把它重定向到一个输出文件里面. 
　　例子用法: 
　　gcc -E hello.c > pianoapan.txt 
　　gcc -E hello.c | more 
　　慢慢看吧,一个hello word 也要预处理成800行的代码 

-C 
　　在预处理的时候,不删除注释信息,一般和-E使用,有时候分析程序，用这个很方便的

-static 
　　此选项将禁止使用动态库，尽量使用动态库，所以，编译出来的东西，一般都很大，也不需要什么 
动态连接库，就可以运行. 
-share 
　　此选项将尽量使用动态库，所以生成文件比较小，但是需要系统由动态库. 



-M 
　　生成文件关联的信息。包含目标文件所依赖的所有源代码你可以用gcc -M hello.c 
来测试一下，很简单。 
　　 
-MM 
　　和上面的那个一样，但是它将忽略由系统头文件，更早的GCC3.0以前是用<>和""区分的。
　　 
-MD 
　　和-M相同，但是输出将导入到.d的文件里面 
　　 
-MMD 
　　和-MM相同，但是输出将导入到.d的文件里面 


-pthread，-pthread等同与-D_REENTRANT 连接时等同于-lpthread
注意多线程库有时候需要最后-lrt，但要注意GCC 4.8以后，链接是有顺序的，
你可以简单认为-lrt,-lpthread 要放到链接顺序的最后面。

-rdynamic 
-rdynamic选项新添加的是动态连接符号信息，用于动态连接功能，比如dlopen()系列函数、
backtrace()系列函数使用，不能被strip掉，即强制strip将导致程序无法执行：
-rdynamic却是一个连接选项，它将指示连接器把所有符号（而不仅仅只是程序已使用到的外部符号，
但不包括静态符号，比如被static修饰的函数）都添加到动态符号表（即.dynsym表）里，
以便那些通过dlopen()或backtrace()（这一系列函数使用.dynsym表内符号）这样的函数使用。
http://www.lenky.info/archives/2013/01/2190


32位版：加上 -m32 参数，生成32位的代码。
64位版：加上 -m64 参数，生成64位的代码。
