/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_predefine.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2003-5-14
* @brief      所有预定义信息描述，包括所有的外部头文件，全局使用的宏，
*             个个平台兼容性的一些小东东，数值的typedef，
*
*
*  @details   在yunfei改进后的再改进一下，发现每个人看问题的思路还是不一样的。
*             也吸取教训，写注释，免得大家不理解为啥要这样
*             请大家仔细看一下每段的分割线和说明，我认为我的划分是很清晰的，不要
*             一看就认为代码宏定义混乱，
*             请各位大神动之前放慢你奔腾野马式样的思维，仔细阅读一下注释头文
*             件分成几个个部分，操作系统定义，头文件包含，数值定义，一些常用宏，
*
*             头文件包含情况如下
*             1.WINDOWS特有部分的，主要是WINDOWS兼容东西多，容易冲突，甚至
*               Windows那排文件的定义顺序也是有讲究的
*             2.LINUX特有部分的，
*             3.C头文件
*             4.C++特有部分的，
*             5.依赖的第3方的库的,请务必不要搞乱，（大部分都是可以打开关闭的）
*
*             数值定义typedef部分代码，以及相关的头文件信息,
*             宏的定义以宏为核心，不按照操作系统分开，免得找起来痛苦，不要试图
*             归类，而改变顺序，反而让人难以理解，
*
*             记录一点纯属YY，的东东，
*             一个得道修仙老前辈送的一段话，记录下来：
*             侠者，性情也，意气也。故不文，不饰，不求，不争，合则留，不合则去。
*             故卫青，将也；周亚夫，侠也；徐达，将也；常遇春，侠也。真侠近乎道。
*             我回复：
*             神，请赐予我平静， 去接受我无法改变的。
*             给予我勇气，去改变我能改变的，
*             赐我智慧，分辨这两者的区别。
*/

#ifndef ZCE_LIB_PREDEFINE_H_
#define ZCE_LIB_PREDEFINE_H_


//==================================================================================================
//根据操作系统，编译器，给出不同的定义，因为这些定义会影响全局，所以在最开始的部分
//我的库只打算适应两个环境，Windows(MSVC)和Linux(GCC)，

//WINDOWS平台(MSVC)
#if defined(WIN32) || defined(WIN64) || defined(_WIN32) || defined(_WIN64)

#define ZCE_OS_WINDOWS 1

#if (defined(WIN64) || defined(_WIN64))
#define ZCE_WIN64 1
#endif //

#if (defined(WIN32) || defined(_WIN32)) && !(defined(WIN64) || defined(_WIN64))
#define ZCE_WIN32 1
#endif

#endif //#if defined(WIN32) || defined(WIN64) || defined(_WIN32) || defined(_WIN64)

// LINUX平台(GCC)我只打算支持GCC，不好意思
#if defined(__linux__) && defined (__GNUC__)
#define ZCE_OS_LINUX 1
#ifndef _GCC_VER
#define _GCC_VER (__GNUC__ * 10000 \
                  + __GNUC_MINOR__ * 100 \
                  + __GNUC_PATCHLEVEL__)
#endif
#if defined(__LP64__)
#define ZCE_LINUX64 1
#else
#define ZCE_LINUX32 1
#endif

#endif

#if  ( defined (ZCE_WIN64) || defined(ZCE_LINUX64)  \
  || defined(__x86_64__)|| defined(__x86_64) || defined(__amd64__)  \
  || defined(__amd64) || defined(__ppc64__) || defined(_WIN64) \
  || defined(__LP64__) || defined(_LP64) || defined(__ia64__) )
#  define ZCE_OS64 1
#else
#  define ZCE_OS32 1
#endif

//如果你啥都不是或者啥都是，我不活了。
#if (!defined (ZCE_OS_WINDOWS) && !defined (ZCE_OS_LINUX)) \
    || (defined (ZCE_OS_WINDOWS) && defined (ZCE_OS_LINUX))
#error "[Error]ZCE_OS_WINDOWS and ZCE_OS_LINUX all defined or all undefined.  error."
#endif


//==================================================================================================
//关于C++11的特性使用问题，C++11的很多特效如此的诱人，但想真心爱他还是有一些门槛的。
//我曾经安装C++98的推广速度认为，我们到2015年可以开始使用C++11特性，2018年才能推广，
//但这一次编译机的厂商感觉都比较努力，到2013年，VC++和GCC 已经大部分完成任务，但要在现
//有的代码里面使用C++11容易，兼容之就比较蛋疼了。
//如果有兴趣看看这两篇文档
//http://gcc.gnu.org/projects/cxx0x.html
//http://msdn.microsoft.com/en-us/library/hh567368.aspx
//随便列举几个
//auto-typed variables                               GCC 4.4 VC++从2010开始逐步支持
//Non-static data member initializers                GCC 4.7 VC++2013
//Variadic templates                                 GCC 4.3 VC++2013
//Default template arguments for function templates  GCC 4.3 VC++2013
//如果抛开上面的繁杂的特效可以认为，VC++，从2010版本开始支持，在2013版本支持特效比较完整，
//GCC 从4.3版本开始到.到4.8版本支持比较晚上，GCC4.8的支持特性数量程度都远好于VC++2013
#if (defined (ZCE_OS_WINDOWS) && defined (_MSC_VER) &&  (_MSC_VER >= 1800)) \
    || (defined (ZCE_OS_LINUX) && defined (_GCC_VER) &&  (_GCC_VER >= 40800))
#define ZCE_SUPPORT_CPP11 1
#else
#define ZCE_SUPPORT_CPP11 0
#endif

//我懒了，我不想倒退回去支持非C11的编译器了，我努力拥抱C11，如果你要向更低的VS编译器靠近，要修改
//一些auto或者默认非静态成员初始化，
#if ZCE_SUPPORT_CPP11 == 0
#error "[Error]Only support C11 compiler, include Visual Studio 2013 and "\
"upper version, or GCC 4.8 and upper version."
#endif

//==================================================================================================
//LINUX GCC下的告警屏蔽功能，其必须在GCC 4.2以后才有，而且push,pop的功能，必须在GCC，4.6以后才有，
//这两个屏蔽告警的东东要等待GCC4.6以后才有。命苦的人。
//#pragma GCC diagnostic push
//#pragma GCC diagnostic pop

//==================================================================================================


//WINDOWS的特有头文件部分，

#if defined(ZCE_OS_WINDOWS)

// 在WINDOWS下和POSIX标准兼容的宏，VS2003以下版本如何，我没有测试，2003以后，Windows对于很多代码宏会使用"_"前缀，
#ifndef _CRT_NONSTDC_NO_DEPRECATE 
#define _CRT_NONSTDC_NO_DEPRECATE 1
#endif
#ifndef _CRT_NONSTDC_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS  1
#endif
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#endif

// 用于告诉VC++，我不强迫使用_s类型的API，_CRT_SECURE_NO_DEPRECATE是_CRT_SECURE_NO_WARNINGS的老版本
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS   1
#endif
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE  1
#endif


//==================================================================================================

//为什么Windows头文件必须放在前面，因为里面有大量的定义，如果步不按照这个规矩来，很容易形成冲突，

//支持WINSERVER2008,VISTA 将下面调整成1，如果不支持下面调整成0
#define ZCE_SUPPORT_WINSVR2008 1

//Windows 的Vista和WinServer2008以后，支持了很多新的API,如果需要支持，需要打开支持开关
#ifndef ZCE_SUPPORT_WINSVR2008
#if (defined _WIN32_WINNT) && (_WIN32_WINNT >=  0x0600) && defined (_MSC_VER) && (_MSC_VER >= 1400)
#define ZCE_SUPPORT_WINSVR2008 1
#else
#define ZCE_SUPPORT_WINSVR2008 0
#endif
#endif

//很多头文件以及数值定义本来可以放到各个OS的配置文件里面去的，但是感觉比较重要，还是放到这个地方了
// pretend it's at least Windows XP or Win2003，如果不定义这个，有时候会有一些API无法使用的问题
#if !defined (_WIN32_WINNT)
#if (defined ZCE_SUPPORT_WINSVR2008) && (ZCE_SUPPORT_WINSVR2008 == 1)
# define _WIN32_WINNT 0x0600
#else
# define _WIN32_WINNT 0x0501
#endif
#endif

#pragma warning ( push )
#pragma warning ( disable : 4091)

// 重新定义FD_SETSIZE来，要在winsock2.h前面，也请其他人注意
#ifndef FD_SETSIZE
#define FD_SETSIZE   1024
#endif

//有些文件前缀是大写的，看起来怪怪的，但Windows下他就真是大写的。

#include <winsock2.h>
#include <MSWSock.h>
#include <winerror.h>
#include <windows.h>
#include <winnt.h>
#include <winbase.h>
#include <Psapi.h>
#include <windef.h>
#include <WTypes.h>
#include <process.h>
#include <ws2tcpip.h>
#include <direct.h>
#include <crtdbg.h>
#include <io.h>
#include <share.h>
#include <DbgHelp.h>
#include <intrin.h>

#pragma warning ( pop )

#endif //#ifdef ZCE_OS_WINDOWS

//==================================================================================================
//LINUX 下特有的部分头文件
#if defined(ZCE_OS_LINUX)

// 为了使用
#define _BSD_SOURCE

#include <errno.h>
#include <getopt.h>
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#include <semaphore.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/io.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/epoll.h>
#include <time.h>
#include <netdb.h>
#include <pthread.h>
#include <iconv.h>
#include <ucontext.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sysinfo.h>
#include <sys/inotify.h>

#if defined(__GNUC__)
#include <execinfo.h>
#endif //#if defined(__GNUC__)

#endif // #ifdef ZCE_OS_LINUX

//==================================================================================================

// C 头文件
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <stdarg.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>

//==================================================================================================
#if defined ZCE_OS_WINDOWS
#pragma warning ( push )
#pragma warning ( disable : 4702)
#pragma warning ( disable : 4267)
#endif

// c++头文件
#include <vector>
#include <list>
#include <set>
#include <map>
#include <deque>
#include <queue>
#include <string>
#include <fstream>
#include <ctime>
#include <sstream>
#include <utility>
#include <iostream>
#include <iomanip>
#include <functional>
#include <iomanip>
#include <typeinfo>
#include <algorithm>
#include <memory>
#include <limits>

//hash_map,hash_set的头文件包含处理要麻烦一点

//在VS2008以后，才有unordered_map和unordered_set，所以在这之前，你必须用stlport，
//当然由于stlport的性能强过微软自带的容器，其他版本也建议大家用stlport
#if defined ZCE_OS_WINDOWS && !defined _STLP_CONFIX_H && defined (_MSC_VER) && (_MSC_VER <= 1400)
#error " Please use stlport ,in Visual studio 2005, have not unordered_map and unordered_set ."
#endif

//在VC++2008版本,VC++2005+STLport，GCC 4.6版本以及更早的版本，unordered_map的名字空间是std::tr1
//在VC++2008版本以前(包括),必须实用STLport
//在VC++2008版本后，可以考虑是否实用STLport,如果_STLP_CONFIX_H 被定义了，我认为你有使用
#if (defined ZCE_OS_LINUX && (_GCC_VER <= 40600)) \
    || ( defined ZCE_OS_WINDOWS && (_MSC_VER <= 1400) ) \
    || ( defined ZCE_OS_WINDOWS && (_MSC_VER > 1400) && defined _STLP_CONFIX_H)
#include <unordered_map>
#include <unordered_set>
using std::tr1::unordered_map;
using std::tr1::unordered_set;

//后面的版本都是直接用了std的名字空间
#else
#include <unordered_set>
#include <unordered_map>
using std::unordered_map;
using std::unordered_set;
#endif
//更早的版本其实是支持hash_map和hash_set的头文件的，先我放弃支持了,那个要改一点代码。

#if defined ZCE_OS_WINDOWS
#pragma warning ( pop )
#endif

//==================================================================================================

//标准整数数值定义部分，由于VS2010前的库没有按照C99的标准执行，所以下面这两个库没有。
//所有的代码中,(除非用于兼容API)，不准出现long，longlong这种定义，不准，出现就弹小JJ . 注：弹到死

//整数类型定义,推荐使用，特别是64位的uint64_t,
//在Linux int64_t 被特殊定义了，在32位系统上是long long ,在64位系统是long，
//造成的麻烦就是你如果你要printf打印int64_t在64系统上用%lld格式flag就会有告警。TNNNNNNNND，
//所以你要在32系统用%lld,64位系统用%d，这完全是给自己找麻烦，
//inttypes.h 中有PRId64 ，PRIu64的定义辅助解决类似（累死）问题，其在
//如果实用了-std=c++11 or -std=c++0x,可以直接使用，或者使用宏__STDC_FORMAT_MACROS

//LINUX下已经有相关的定义了，万幸
#if defined(ZCE_OS_LINUX)
#include <stdint.h>
#include <inttypes.h>

#endif //#if defined(ZCE_OS_LINUX)

//WINDOWS下，各种不同，各种不一致，你只能自己来
#if defined(ZCE_OS_WINDOWS)

//到VS2010为止，ssize_t还没有被支持
#if defined (ZCE_WIN64)
typedef SSIZE_T ssize_t;
#else
typedef int ssize_t;
#endif

//VC++ 2010，以及遵守这个标准了
#if _MSC_VER >= 1500
#include <stdint.h>
#include <inttypes.h>

//VC++ 2005
#else
//The stdint declaras
typedef  signed char        int8_t;
typedef  short              int16_t;
typedef  int                int32_t;
typedef  unsigned char      uint8_t;
typedef  unsigned short     int8_tuint16_t;
typedef  unsigned int       uint32_t;

//
#if _MSC_VER >= 1300
typedef unsigned long long  uint64_t;
typedef long long           floatint64_t;
#ifndef PRId64
#define PRId64     "lld"
#endif
#ifndef PRIu64
#define PRIu64     "llu" 
#endif
#else
typedef unsigned __int64    uint64_t;
typedef __int64             int64_t;
#ifndef PRId64
#define PRId64     "I64d" 
#endif
#ifndef PRIu64
#define PRIu64     "I64u" 
#endif
#endif //#if _MSC_VER >= 1300

#endif //#if _MSC_VER >= 1500

#endif //#if defined(ZCE_OS_WINDOWS)


//==================================================================================================
//我们引入的外部库，目前包括,rapidxml,MYSQL,SQLite,
//外部文件头文件包含，控制开关一般放在zce_config.h文件

#include "zce_config.h"

//rapidxml XML文件的头文件以及开关，我们引入的库是rapidxml 库，他的优势是只有头文件，

#if defined ZCE_USE_RAPIDXML && ZCE_USE_RAPIDXML == 1
#if defined (ZCE_OS_WINDOWS)
#pragma warning ( push )
#pragma warning ( disable : 4244)
#pragma warning ( disable : 4100)
#endif

#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include <rapidxml_print.hpp>

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( pop )
#endif
#endif

//MySQL
#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1
#include <mysql.h>
#endif

//SQLite
#if defined ZCE_USE_SQLITE && ZCE_USE_SQLITE == 1
#include <sqlite3.h>
#endif

//Lua
#if defined ZCE_USE_LUA && ZCE_USE_LUA == 1
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
};
#endif



//IPV6的宏，暂时不打开
//#if !defined ZCE_HAS_IPV6    1
//#define ZCE_HAS_IPV6
//#endif

//==================================================================================================

//是否使用Google Protobuf,
//ZCE_USE_PROTOBUF 宏在zce_config.h文件里面控制

#if defined ZCE_USE_PROTOBUF && ZCE_USE_PROTOBUF == 1
#if defined (ZCE_OS_WINDOWS)
#pragma warning ( push )
#pragma warning ( disable : 4512)
#pragma warning ( disable : 4100)
#elif defined (ZCE_OS_LINUX)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/text_format.h>

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( pop )
#elif defined (ZCE_OS_LINUX)
#pragma GCC diagnostic pop
#endif
#endif

//==================================================================================================
//字节序的小头和大头的问题
#define ZCE_LITTLE_ENDIAN  0x1234
#define ZCE_BIG_ENDIAN     0x4321

//目前部分代码是考虑了小头党和大头党的问题，不知道有生之年这套代码是否还会为大头党服务一次？
//主要是hash和加密部分的代码，是考虑过字节序兼容性问题的。
// Little Endian or Big Endian ?
// Overwrite the #define below if you know your architecture endianess
#if defined (__GLIBC__)
#  include <endian.h>
#endif

#ifndef ZCE_BYTES_ORDER
#if ( (defined(__BYTE_ORDER) && (__BYTE_ORDER == __BIG_ENDIAN)) || defined(__BIG_ENDIAN__) || defined(__BIG_ENDIAN) || defined(_BIG_ENDIAN) ) && !(defined(__LITTLE_ENDIAN__) || defined(__LITTLE_ENDIAN) || defined(_LITTLE_ENDIAN))
#  define ZCE_BYTES_ORDER ZCE_BIG_ENDIAN
#elif defined(__sparc) || defined(__sparc__) \
    || defined(__ppc__) || defined(_POWER) || defined(__powerpc__) || defined(_ARCH_PPC) || defined(__PPC__) || defined(__PPC) || defined(PPC) \
    || defined(__powerpc__) || defined(__powerpc) || defined(powerpc) \
    || defined(__hpux)  || defined(__hppa)  || defined(_MIPSEB) || defined(__s390__)
#  define  ZCE_BYTES_ORDER ZCE_BIG_ENDIAN
#else
// Little Endian assumed. PDP Endian and other very rare endian format are unsupported.
//其实除了大头党和小头党以外，还有其他派别，人类真复杂。
#define ZCE_BYTES_ORDER    ZCE_LITTLE_ENDIAN
#endif
#endif  //#ifndef ZCE_BYTES_ORDER


//==================================================================================================
//各种宏定义，编译定义，一些比较常用的宏，帮助你节省一些代码

//外部头文件，都放在这前面呀，否则预编译头文件就不起作用了
//这个红用于定义告诉编译器，后面的代码就不做预处理了，主要用于加快代码编译，但LINUX目前还不支持

#if defined ZCE_OS_WINDOWS
#pragma hdrstop
#endif //#if defined ZCE_OS_WINDOWS



//我是抄ACE_UNUSED_ARG的呀。我承认呀。windows下也许也可以考虑定义成__noop呀，
#if !defined (ZCE_UNUSED_ARG)
#if defined ZCE_OS_LINUX
#  if ( _GCC_VER >= 40200)
#    define ZCE_UNUSED_ARG(a) (void) (a)
#  else
#    define ZCE_UNUSED_ARG(a) do {/* null */} while (&a == 0)
#  endif
#elif defined ZCE_OS_WINDOWS
#    define ZCE_UNUSED_ARG(a)    UNREFERENCED_PARAMETER(a)
#  endif
#endif //#if !defined (ZCE_UNUSED_ARG)

//BOOL变量
#ifndef FALSE
#  define FALSE               0
#endif
#ifndef TRUE
#  define TRUE                1
#endif

//NULL
#ifndef NULL
#  ifdef __cplusplus
#    define NULL    0
#  else
#    define NULL    ((void *)0)
#  endif
#endif

//__FUNCTION__定义的替换，尽最大可能帮你输出更加详细的函数名称信息
//Windows 下你可以用__FUNCSIG__，其优点是会有参数信息，但缺点也是有参数信息，一些带有模板的信息很长很长，反而影响你的感觉
#if defined ZCE_OS_WINDOWS
#define __ZCE_FUNC__   __FUNCTION__
#elif defined ZCE_OS_LINUX
#define __ZCE_FUNC__   __PRETTY_FUNCTION__
#else
#define __ZCE_FUNC__   __FUNCTION__
#endif

//
#if defined(ZCE_OS_LINUX) && defined(__GNUC__)
#define ZCE_LIKELY(x)      __builtin_expect (x, 1)
#define ZCE_UNLIKELY(x)    __builtin_expect (x, 0)
#else
#define ZCE_LIKELY(x)      (x)
#define ZCE_UNLIKELY(x)    (x)
#endif

//BIT位的一些操作定义宏

//设置或者清理数值某个位置上的bit位
//_value 没有用()保护是有理由的，请好好想想，呵呵
#if !defined(ZCE_SET_BITS)
#  define ZCE_SET_BITS(set_value, bits) (set_value |= (bits))
#endif
#if !defined(ZCE_CLR_BITS)
#  define ZCE_CLR_BITS(clr_value, bits) (clr_value &= ~(bits))
#endif

//检查某个bit位是否设置了
#if !defined(ZCE_BIT_IS_SET)
#  define ZCE_BIT_IS_SET(compare_value, bits) (((compare_value) & (bits)) != 0)
#endif
#if !defined(ZCE_BIT_ISNOT_SET)
#  define ZCE_BIT_ISNOT_SET(compare_value, bits) (((compare_value) & (bits)) == 0)
#endif

///计算数组个数,注意传递的型别喔
#ifndef ZCE_ARRAY_SIZE
#define ZCE_ARRAY_SIZE(ary) (sizeof(ary)/sizeof((ary)[0]))
#endif


// 取大小值, min max在linux下没有定义
#define ZCE_MAX(a,b) (((a) > (b)) ? (a) : (b))
#define ZCE_MIN(a,b) (((a) < (b)) ? (a) : (b))

//==================================================================================================

// 一些C函数的重命名，保持兼容，避免费力折腾
#if defined ZCE_OS_WINDOWS

#if _MSC_VER <= 1300
#define snprintf     _snprintf
#define vsnprintf    _vsnprintf
#else
//在VS2005以后，使用安全API,保证WINDOWS下更加接近于LINUX，保证末尾会添加'\0'
#define snprintf(buffer,buf_count,fmt,...) _snprintf_s((buffer),(buf_count),((buf_count)-1),(fmt),__VA_ARGS__)
#define vsnprintf(buffer,buf_count,fmt,argptr_list)  _vsnprintf_s((buffer),((buf_count)),(buf_count-1),(fmt),(argptr_list))
#endif

#define strtoull _strtoui64
#define strcasecmp   _stricmp
#define strncasecmp  _strnicmp

#endif


//==================================================================================================
// 名称的最大长度
#if !defined (NAME_MAX)
#  if defined (FILENAME_MAX)
#    define NAME_MAX FILENAME_MAX
#  elif defined (_MAX_FNAME)
#    define NAME_MAX _MAX_FNAME
#  else /* _MAX_FNAME */
#    define NAME_MAX 256
#  endif /* MAXNAMLEN */
#endif /* !NAME_MAX */

// 主机名称的最大长度
#if !defined (HOST_NAME_MAX)
#  define HOST_NAME_MAX 256
#endif /* !HOST_NAME_MAX */

//路径的最大长度，
//普及一下小知识，注意一下其实MAX_PATH未必真正够用，MS一般的定义是260，但是，其实你可以超过,
#ifndef MAX_PATH
#  define MAX_PATH 512
#endif
#if !defined (PATH_MAX)
#  if defined (_MAX_PATH)
#    define PATH_MAX _MAX_PATH
#  elif defined (MAX_PATH)
#    define PATH_MAX MAX_PATH
#  elif defined (_POSIX_PATH_MAX)
#     define PATH_MAX _POSIX_PATH_MAX
#  else /* !_MAX_PATH */
#    define PATH_MAX 512
#  endif /* _MAX_PATH */
#endif /* !PATH_MAX */



//==================================================================================================

#if defined ZCE_WIN32
// vc71(2003):
#if defined(_MSC_VER) && (_MSC_VER == 1310)
#  define ZCE_PLATFORM_TOOLSET "win32-v71"
// vc80(2005):
#elif defined(_MSC_VER) && (_MSC_VER == 1400)
#  define ZCE_PLATFORM_TOOLSET "win32-v80"
// vc90(VS2008):
#elif defined(_MSC_VER) && (_MSC_VER == 1500)
#  define ZCE_PLATFORM_TOOLSET "win32-v90"
// vc10(VS2010):
#elif defined(_MSC_VER) && (_MSC_VER == 1600)
#  define ZCE_PLATFORM_TOOLSET "win32-v100"
// vc11(VS2012):
#elif defined(_MSC_VER) && (_MSC_VER == 1700)
#  define ZCE_PLATFORM_TOOLSET "win32-v110"
#elif defined(_MSC_VER) && (_MSC_VER == 1800)
#  define ZCE_PLATFORM_TOOLSET "win32-v120"
#endif
#elif defined ZCE_WIN64
// vc71(2003):
#if defined(_MSC_VER) && (_MSC_VER == 1310)
#  define ZCE_PLATFORM_TOOLSET "x64-v71"
// vc80(2005):
#elif defined(_MSC_VER) && (_MSC_VER == 1400)
#  define ZCE_PLATFORM_TOOLSET "x64-v80"
// vc90(VS2008):
#elif defined(_MSC_VER) && (_MSC_VER == 1500)
#  define ZCE_PLATFORM_TOOLSET "x64-v90"
// vc10(VS2010):
#elif defined(_MSC_VER) && (_MSC_VER == 1600)
#  define ZCE_PLATFORM_TOOLSET "x64-v100"
// vc11(VS2012):
#elif defined(_MSC_VER) && (_MSC_VER == 1700)
#  define ZCE_PLATFORM_TOOLSET "x64-v110"
#elif defined(_MSC_VER) && (_MSC_VER == 1800)
#  define ZCE_PLATFORM_TOOLSET "x64-v120"
#endif
#endif

//工程内部可以统一使用ZCE_PLAT_TOOLSET_CONF这个宏，简化大家的编译工程设置
#if defined _DEBUG || defined DEBUG
#  define ZCE_PLAT_TOOLSET_CONF ZCE_PLATFORM_TOOLSET"-Debug"
#else
#  define ZCE_PLAT_TOOLSET_CONF ZCE_PLATFORM_TOOLSET"-Release"
#endif

//Windows 下的自动链接
#if defined (ZCE_OS_WINDOWS) && defined (_MSC_VER)

// 包含库的引用链接
#  pragma comment(lib, "ws2_32.lib")
#  pragma comment(lib, "mswsock.lib")
#  pragma comment(lib, "psapi.lib")
#  pragma comment(lib, "dbghelp.lib")
#  if defined (ZCE_HAS_IPV6) && (ZCE_HAS_IPV6==1)
#    pragma comment(lib, "iphlpapi.lib")
#  endif

//这个功能到2008才支持
#if defined ZCE_SUPPORT_WINSVR2008
#  pragma comment(lib, "dbghelp.lib")
#endif



//如果使用了LUA，自动链接LUA的库，
#if defined ZCE_USE_LUA && ZCE_USE_LUA == 1
#pragma comment(lib, "lualib.lib" )
#endif

#if defined ZCE_USE_PROTOBUF && ZCE_USE_PROTOBUF == 1
#pragma comment(lib, "libprotobuf.lib" )
#endif

#if defined ZCE_USE_SQLITE && ZCE_USE_SQLITE == 1
#pragma comment(lib, "sqlite.lib" )
#endif

#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1
#pragma comment(lib, "libmysql.lib" )
#endif



//自动包含的包含连接，简化你的操作
#pragma comment(lib, "zcelib.lib"  )


//编译动态库用的东西
#if defined ZCE_OS_WINDOWS && defined ZCELIB_HASDLL
#  ifdef BUILD_ZCELIB_DLL
#    define ZCELIB_EXPORT __declspec (dllexport)
#  else
#    define ZCELIB_EXPORT __declspec (dllimport)
#  endif //#if defined ZCE_OS_WINDOWS && defined ZCELIB_HASDLL
#else
#  define ZCELIB_EXPORT
#endif //#if defined ZCE_OS_WINDOWS && defined ZCELIB_HASDLL

#endif


#endif //ZCE_LIB_PREDEFINE_H_
