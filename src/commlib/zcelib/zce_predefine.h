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
*             请各位大神动之前放慢你奔腾野马式样的思维，仔细阅读一下注释
*             头文件分成几个个部分，操作系统定义，头文件包含，数值定义，一些常用宏，
* 
*             头文件包含情况如下
*             1.WINDOWS特有部分的，主要是WINDOWS兼容东西多，容易冲突，甚至Windows
*               那排文件的定义顺序也是有讲究的
*             2.LINUX特有部分的，
*             3.C头文件
*             4.C++特有部分的，
*             5.依赖的第3方的库的,请务必不要搞乱，（大部分都是可以打开关闭的）
* 
*             数值定义typedef部分代码，以及相关的头文件信息,
*             宏的定义以宏为核心，不按照操作系统分开，免得找起来痛苦，不要试图归类，
*             而改变顺序，反而让人难以理解，
* 
*             记录一点纯属YY，的东东，
*             一个得道修仙老前辈送的一段话，记录下来：
*             侠者，性情也，意气也。故不文，不饰，不求，不争，合则留，不合则去。故卫青，
*             将也；周亚夫，侠也；徐达，将也；常遇春，侠也。真侠近乎道。
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

#if defined(__LP64__)
#define ZCE_LINUX64 1
#else
#define ZCE_LINUX32 1
#endif

#endif

#if  ( defined (ZCE_WIN64) || defined(ZCE_LINUX64) || defined(__x86_64__) || defined(__x86_64) || defined(__amd64__)  \
  || defined(__amd64) || defined(__ppc64__) || defined(_WIN64) || defined(__LP64__) || defined(_LP64) || defined(__ia64__) )
#  define ZCE_OS64 1
#else
#  define ZCE_OS32 1
#endif

//如果你啥都不是或者啥都是，我不活了。
#if (!defined (ZCE_OS_WINDOWS) && !defined (ZCE_OS_LINUX)) \
    || (defined (ZCE_OS_WINDOWS) && defined (ZCE_OS_LINUX))
#error " ZCE_OS_WINDOWS and ZCE_OS_LINUX all defined or all undefined.  error."
#endif 

//Windows下必须使用VS作为编辑器，不是我对GCC无爱，是太太麻烦了。
#if defined (ZCE_OS_WINDOWS)
#if !defined (_MSC_VER) || (_MSC_VER < 1310)
#error "Only support 2003 or upper visual studio version."
#endif
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
#define _CRT_NONSTDC_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_WARNINGS  1

// 用于告诉VC++，我不强迫使用_s类型的API，_CRT_SECURE_NO_DEPRECATE是_CRT_SECURE_NO_WARNINGS的老版本
#define _CRT_SECURE_NO_WARNINGS   1
#define _CRT_SECURE_NO_DEPRECATE  1

//==================================================================================================

//为什么Windows头文件必须放在前面，因为里面有大量的定义，如果步不按照这个规矩来，很容易形成冲突，

//支持WINSERVER2008,VISTA 将下面调整成1，如果不支持下面调整成0
#define ZCE_SUPPORT_WINSVR2008 1

//Windows 的Vista和WinServer2008以后，支持了很多新的API,如果需要支持，需要打开支持开关
#if (defined _WIN32_WINNT) && (_WIN32_WINNT >=  0x0600) && defined (_MSC_VER) && (_MSC_VER >= 1400)
#ifndef ZCE_SUPPORT_WINSVR2008
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

//hash_map,hash_set的包含
#if defined ZCE_OS_WINDOWS
//在VS2008以后，才有unordered_map和unordered_set，所以在这之前，你必须用stlport，
//当然由于stlport的性能强过微软自带的容器，其他版本也建议大家用stlport
#if !defined _STLP_CONFIX_H && defined (_MSC_VER) && (_MSC_VER <= 1400)
#error " Please use stlport ,in Visual studio 2005, have not unordered_map and unordered_set ."
#endif

// hash扩展 windows下用的stlport
#include <unordered_map>
#include <unordered_set>
using std::tr1::unordered_map;
using std::tr1::unordered_set;

#elif defined ZCE_OS_LINUX

#include <unordered_set>
#include <unordered_map>
using std::unordered_map;
using std::unordered_set;

#endif  //#if defined ZCE_OS_WINDOWS

#if defined ZCE_OS_WINDOWS
#pragma warning ( pop )
#endif

//==================================================================================================
//我们引入的外部库，目前包括,rapidxml,MYSQL,SQLite,

//rapidxml XML文件的头文件以及开关，我们引入的库是rapidxml 库，他的优势是只有头文件，
#ifndef ZCE_USE_RAPIDXML
#define ZCE_USE_RAPIDXML 0
#endif

#if defined ZCE_USE_RAPIDXML && ZCE_USE_RAPIDXML == 1
#if defined (ZCE_OS_WINDOWS)
#pragma warning ( push )
#pragma warning ( disable : 4244)
#pragma warning ( disable : 4100)
#endif

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_utils.hpp>
#include <rapidxml/rapidxml_print.hpp>

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( pop )
#endif
#endif

// mysql开关以及头文件包含
#ifndef ZCE_USE_MYSQL
#define ZCE_USE_MYSQL 1
#endif

#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1
#include <mysql.h>
#endif

//SQLite3的头文件
#ifndef ZCE_USE_SQLITE3
#define ZCE_USE_SQLITE3 0
#endif

#if defined ZCE_USE_SQLITE3 && ZCE_USE_SQLITE3 == 1
#include <sqlite3.h>
#endif

//IPV6的宏，暂时不打开
//#if !defined ZCE_HAS_IPV6    1
//#define ZCE_HAS_IPV6
//#endif

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

//标准整数数值定义部分，由于VS2010前的库没有按照C99的标准执行，所以下面这两个库没有。
//所有的代码中,(除非用于兼容API)，不准出现long，longlong这种定义，不准，出现就弹小JJ . 注：弹到死

//整数类型定义,推荐使用，特别是64位的uint64_t,

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
//VC++ 2005
#else

//The stdint declaras
typedef  signed char        int8_t;
typedef  short              int16_t;
typedef  int                int32_t;
typedef  unsigned char      uint8_t;
typedef  unsigned short     uint16_t;
typedef  unsigned int       uint32_t;

//
#if _MSC_VER >= 1300
typedef unsigned long long  uint64_t;
typedef long long           int64_t;
#else
typedef unsigned __int64    uint64_t;
typedef __int64             int64_t;
#endif //#if _MSC_VER >= 1300

#endif //#if _MSC_VER >= 1500

#endif //#if defined(ZCE_OS_WINDOWS)

//==================================================================================================
//各种宏定义，编译定义，一些比较常用的宏，帮助你节省一些代码

//外部头文件，都放在这前面呀，否则预编译头文件就不起作用了
//这个红用于定义告诉编译器，后面的代码就不做预处理了，主要用于加快代码编译，但LINUX目前还不支持

#if defined ZCE_OS_WINDOWS
#pragma hdrstop
#endif //#if defined ZCE_OS_WINDOWS

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

//我是抄ACE_UNUSED_ARG的呀。我承认呀。windows下也许也可以考虑定义成__noop呀，
#if !defined (ZCE_UNUSED_ARG)
#if defined ZCE_OS_LINUX
#  if defined (__GNUC__) && ((__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)))
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
#if defined ZCE_OS_WINDOWS
#define __ZCE_FUNCTION__   __FUNCSIG__
#elif defined ZCE_OS_LINUX
#define __ZCE_FUNCTION__   __PRETTY_FUNCTION__
#else
#define __ZCE_FUNCTION__   __FUNCTION__
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

//一些字节序交换的宏
//#if (_MSC_VER > 1300) && (defined(CPU_IA32) || defined(CPU_X64)) /* MS VC */
//_MSC_VER > 1300  _byteswap_ushort,_byteswap_ulong,_byteswap_uint64
//#if defined(__GNUC__) && (__GNUC__ >= 4) && (__GNUC__ > 4 || __GNUC_MINOR__ >= 3)
//GCC 4.3__builtin_bswap64,__builtin_bswap16,__builtin_bswap32

#ifndef ZCE_SWAP_UINT16
#define ZCE_SWAP_UINT16(x)  ((((x) & 0xff00) >>  8) | (((x) & 0x00ff) <<  8))
#endif
#ifndef ZCE_SWAP_UINT32
#define ZCE_SWAP_UINT32(x)  ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | \
                             (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))
#endif
#ifndef ZCE_SWAP_UINT64
#define ZCE_SWAP_UINT64(x)   ((((x) & 0xff00000000000000ULL) >> 56) | (((x) & 0x00ff000000000000ULL) >>  40) | \
                              (((x) & 0x0000ff0000000000ULL) >> 24) | (((x) & 0x000000ff00000000ULL) >>  8) |   \
                              (((x) & 0x00000000ff000000ULL) << 8 ) | (((x) & 0x0000000000ff0000ULL) <<  24) |  \
                              (((x) & 0x000000000000ff00ULL) << 40 ) | (((x) & 0x00000000000000ffULL) <<  56))
#endif

//定义一组ntol ntos,nothll等，主要问题是64的转换，不是所有系统都有，
#if (ZCE_BYTES_ORDER == ZCE_LITTLE_ENDIAN)
# define ZCE_HTONS(x)  ZCE_SWAP_UINT16 (x)
# define ZCE_NTOHS(x)  ZCE_SWAP_UINT16 (x)
# define ZCE_HTONL(x)  ZCE_SWAP_UINT32 (x)
# define ZCE_NTOHL(x)  ZCE_SWAP_UINT32 (x)
# define ZCE_HTONLL(x) ZCE_SWAP_UINT64 (x)
# define ZCE_NTOHLL(x) ZCE_SWAP_UINT64 (x)
#else
# define ZCE_HTONS(x)  (x)
# define ZCE_NTOHS(x)  (x)
# define ZCE_HTONL(x)  (x)
# define ZCE_NTOHL(x)  (x)
# define ZCE_HTONLL(x) (x)
# define ZCE_NTOHLL(x) (x)
#endif /* end if (ZCE_BYTES_ORDER == ZCE_LITTLE_ENDIAN) */


//这样写是为了避免BUS ERROR问题，但我自己也不能100%肯定这个方法能解决BUS ERROR，
//因为我没有环境进行相关的测试。
//注意#pragma pack(push, 1) 和__attribute__ ((packed)) 是有区别的，
//以下参考之 德问
//前者告诉编译器结构体或类内部的成员变量相对于第一个变量的地址的偏移量的对齐方式，
//缺省情况下，编译器按照自然边界对齐，当变量所需的自然对齐边界比n大 时，按照n对齐，
//否则按照自然边界对齐；后者告诉编译器一个结构体或者类或者联合或者一个类型的变量
//(对象)分配地址空间时的地址对齐方式。也就是所，如果将__attribute__((aligned(m)))
//作用于一个类型，那么该类型的变量在分配地址空间时，其存放的地址一定按照m字节对齐
//(m必 须是2的幂次方)。并且其占用的空间，即大小,也是m的整数倍，以保证在申请连续存储空间的时候，
//每一个元素的地址也是按照m字节对齐。 __attribute__((aligned(m)))也可以作用于一个单独的变量。
//由上可以看出__attribute__((aligned(m)))的功能更全。


#if defined ZCE_OS_WINDOWS

#pragma pack(push, 1)

struct ZU16_STRUCT
{
    uint16_t value_;
};
struct ZU32_STRUCT
{
    uint32_t value_;
};
struct ZU64_STRUCT
{
    uint64_t value_;
};

#pragma pack(pop)

#elif defined ZCE_OS_LINUX
//__attribute__ ((packed)) 在这儿的目的是利用其特性避免BUS ERROR错误
struct ZU16_STRUCT
{
    uint16_t value_;
} __attribute__ ((packed));
struct ZU32_STRUCT
{
    uint32_t value_;
} __attribute__ ((packed));
struct ZU64_STRUCT
{
    uint64_t value_;
} __attribute__ ((packed));

#endif


///从一个(char *)指针内读取(也可以用于写入)一个uint16_t,or uint32_t or uint64_t
# define ZBYTE_TO_UINT16(ptr)  ((ZU16_STRUCT *)(ptr))->value_
# define ZBYTE_TO_UINT32(ptr)  ((ZU32_STRUCT *)(ptr))->value_
# define ZBYTE_TO_UINT64(ptr)  ((ZU64_STRUCT *)(ptr))->value_

///从一个(char *)指针内读取uint16_t,or uint32_t or uint64_t 的数组内的ary_index单元，注意数组下标是值对于整形的下标，(而不是ptr的下标)
# define ZINDEX_TO_UINT16(ptr,ary_index)  (((ZU16_STRUCT *)(ptr))+(ary_index))->value_
# define ZINDEX_TO_UINT32(ptr,ary_index)  (((ZU32_STRUCT *)(ptr))+(ary_index))->value_
# define ZINDEX_TO_UINT64(ptr,ary_index)  (((ZU64_STRUCT *)(ptr))+(ary_index))->value_

///向一个(char *)指针内写入一个uint16_t,or uint32_t or uint64_t
# define ZUINT16_TO_BYTE(ptr,wr_data)  ((ZU16_STRUCT *)(ptr))->value_ = (wr_data)
# define ZUINT32_TO_BYTE(ptr,wr_data)  ((ZU32_STRUCT *)(ptr))->value_ = (wr_data)
# define ZUINT64_TO_BYTE(ptr,wr_data)  ((ZU64_STRUCT *)(ptr))->value_ = (wr_data)

//向一个(char *)指针内写入一个uint16_t,or uint32_t or uint64_t的数组内部的ary_index单元，注意数组下标是值对于整形的下标，(而不是ptr的下标)
# define ZUINT16_TO_INDEX(ptr,ary_index,wr_data)  (((((ZU16_STRUCT *)(ptr))+(ary_index))->value_) = (wr_data))
# define ZUINT32_TO_INDEX(ptr,ary_index,wr_data)  (((((ZU32_STRUCT *)(ptr))+(ary_index))->value_) = (wr_data))
# define ZUINT64_TO_INDEX(ptr,ary_index,wr_data)  (((((ZU64_STRUCT *)(ptr))+(ary_index))->value_) = (wr_data))


//上面的代码从功能上等同下面的代码。但是这些写会导致BUS ERROR问题。
//# define ZBYTE_TO_UINT32(ptr)  (*(uint32_t *)(ptr))
//# define ZINDEX_TO_UINT32(ptr,ary_index)  (*(((uint32_t *)(ptr))+(ary_index)))
//# define ZUINT32_TO_BYTE(ptr,wr_data)  ((*(uint32_t *)(ptr)) = (wr_data))
//# define ZUINT32_TO_INDEX(ptr,ary_index,wr_data)  ((*(((uint32_t *)(ptr))+(ary_index))) = (wr_data))

//考虑字节序的读取短整型，整形，64位长整形的方式
//不好意思，年纪大了，记忆力真的有点成问题了，有几次写这段东东就是想不明白加密，MD5等代码中为啥要考
//虑字节序处理呢？
//第一，为了保持算法的一致性，一台小头字节序机器上计算的得到的密文，在另外一台大头序的机器要能解开，
//（或者说大头和小头的机器对同样的一段buffer，加密得到的密文应该是一致的）
//这就要求计算机从字节流中取出的整数含义是一致的。所以就要求考虑字节序问题了。这是最关键的因素。
//第二，有些算法是为了加快处理，表面是用整数处理的，但实际里面，是必须考虑字节顺序的，比如我们的
//CRC32算法，就有这个问题，所以这种算法，你必须考虑字节序问题，小头在这种情况下往往不需要进行转，
//第三，有些算法其实明确要求了里面采用大头序，还是小头序，这种你就必须考虑你的环境，
//
//在默认编码的问题上，我还是倾向了小头，一方面我的代码估计99.99%都是跑在X86架构之下，另一方面
//小头的是从低到高，和字节顺序一致，处理上也方便一点。

#if (ZCE_BYTES_ORDER == ZCE_LITTLE_ENDIAN)

///从一个(char *)指针内读取小头字节序的uint16_t,or uint32_t or uint64_t，在小头字节序的机器上不发生改变
# define ZBYTE_TO_LEUINT16(ptr)    ZBYTE_TO_UINT16(ptr)
# define ZBYTE_TO_LEUINT32(ptr)    ZBYTE_TO_UINT32(ptr)
# define ZBYTE_TO_LEUINT64(ptr)    ZBYTE_TO_UINT64(ptr)

///从一个(char *)指针内读取小头字节序的uint16_t,or uint32_t or uint64_t 的数组内的ary_index单元，注意数组下标是值对于整形的下标，(而不是ptr的下标)
# define ZINDEX_TO_LEUINT16(ptr,ary_index)  ZINDEX_TO_UINT16(ptr,ary_index)
# define ZINDEX_TO_LEUINT32(ptr,ary_index)  ZINDEX_TO_UINT32(ptr,ary_index)
# define ZINDEX_TO_LEUINT64(ptr,ary_index)  ZINDEX_TO_UINT64(ptr,ary_index)

///向一个(char *)指针内写入一个小头字节序的uint16_t,or uint32_t or uint64_t，在小头字节序的机器上不发生改变
# define ZLEUINT16_TO_BYTE(ptr,wr_data)  ZUINT16_TO_BYTE(ptr,wr_data)
# define ZLEUINT32_TO_BYTE(ptr,wr_data)  ZUINT32_TO_BYTE(ptr,wr_data)
# define ZLEUINT64_TO_BYTE(ptr,wr_data)  ZUINT64_TO_BYTE(ptr,wr_data)

//向一个(char *)指针内写入一个小头字节序的uuint16_t,or uint32_t or uint64_t的数组内部的ary_index单元，注意数组下标是值对于整形的下标，(而不是ptr的下标)
# define ZLEUINT16_TO_INDEX(ptr,ary_index,wr_data)  ZUINT16_TO_INDEX(ptr,ary_index,wr_data)
# define ZLEUINT32_TO_INDEX(ptr,ary_index,wr_data)  ZUINT32_TO_INDEX(ptr,ary_index,wr_data)
# define ZLEUINT64_TO_INDEX(ptr,ary_index,wr_data)  ZUINT64_TO_INDEX(ptr,ary_index,wr_data)

#else

# define ZBYTE_TO_LEUINT16(ptr)  ZCE_SWAP_UINT16(((ZU16_STRUCT *)(ptr))->value_)
# define ZBYTE_TO_LEUINT32(ptr)  ZCE_SWAP_UINT32(((ZU32_STRUCT *)(ptr))->value_)
# define ZBYTE_TO_LEUINT64(ptr)  ZCE_SWAP_UINT64(((ZU64_STRUCT *)(ptr))->value_)

//
# define ZINDEX_TO_LEUINT16(ptr,ary_index)  ZCE_SWAP_UINT16((((ZU16_STRUCT *)(ptr))+(ary_index))->value_)
# define ZINDEX_TO_LEUINT32(ptr,ary_index)  ZCE_SWAP_UINT32((((ZU32_STRUCT *)(ptr))+(ary_index))->value_)
# define ZINDEX_TO_LEUINT64(ptr,ary_index)  ZCE_SWAP_UINT64((((ZU64_STRUCT *)(ptr))+(ary_index))->value_)

//
# define ZLEUINT16_TO_BYTE(ptr,wr_data)  ZBYTE_TO_UINT16(ptr) = ZCE_SWAP_UINT16(wr_data))
# define ZLEUINT32_TO_BYTE(ptr,wr_data)  ZBYTE_TO_UINT32(ptr) = ZCE_SWAP_UINT32(wr_data))
# define ZLEUINT64_TO_BYTE(ptr,wr_data)  ZBYTE_TO_UINT64(ptr) = ZCE_SWAP_UINT64(wr_data))

//
# define ZLEUINT16_TO_INDEX(ptr,ary_index,wr_data)  ZINDEX_TO_UINT16(ptr) = ZCE_SWAP_UINT16(wr_data))
# define ZLEUINT32_TO_INDEX(ptr,ary_index,wr_data)  ZINDEX_TO_UINT32(ptr) = ZCE_SWAP_UINT32(wr_data))
# define ZLEUINT64_TO_INDEX(ptr,ary_index,wr_data)  ZINDEX_TO_UINT64(ptr) = ZCE_SWAP_UINT64(wr_data))

#endif /* end if (ZCE_BYTES_ORDER == ZCE_LITTLE_ENDIAN) */

//带有旋转的移位操作，不同位数表示是针对短整，整数，64位长整的操作，
//注意n不要传入一个无意义的值，比如32位的ZCE_ROTL32，n要小于31，否则
#ifndef ZCE_ROTL16
#define ZCE_ROTL16(word, n)  (((word) <<  ((n)&15))  | ((word) >>  (16 - ((n)&15))))
#endif
#ifndef ZCE_ROTR16
#define ZCE_ROTR16(word, n)  (((word) >>  ((n)&15))  | ((word) <<  (16 - ((n)&15))))
#endif
#ifndef ZCE_ROTL32
#define ZCE_ROTL32(dword, n) (((dword) << ((n)&31)) |  ((dword) >> (32 - ((n)&31))))
#endif
#ifndef ZCE_ROTR32
#define ZCE_ROTR32(dword, n) (((dword) >> ((n)&31)) |  ((dword) << (32 - ((n)&31))))
#endif
#ifndef ZCE_ROTL64
#define ZCE_ROTL64(qword, n) (((qword) << ((n)&63)) |  ((qword) >> (64 - ((n)&63))))
#endif
#ifndef ZCE_ROTR64
#define ZCE_ROTR64(qword, n) (((qword) >> ((n)&63)) |  ((qword) << (64 - ((n)&63))))
#endif

#define ZUINT16_0BYTE(data)    ((data) & 0xFF)
#define ZUINT16_1BYTE(data)    ((data) >> 8)

#define ZUINT32_0BYTE(data)    ((data)  & 0xFF)
#define ZUINT32_1BYTE(data)    (((data) >> 8)  & 0xFF)
#define ZUINT32_2BYTE(data)    (((data) >> 16) & 0xFF)
#define ZUINT32_3BYTE(data)    ((data)  >> 24)

#define ZUINT64_0BYTE(data)    ((data)  & 0xFF)
#define ZUINT64_1BYTE(data)    (((data) >> 8) & 0xFF)
#define ZUINT64_2BYTE(data)    (((data) >> 16) & 0xFF)
#define ZUINT64_3BYTE(data)    (((data) >> 24) & 0xFF)
#define ZUINT64_4BYTE(data)    (((data) >> 32) & 0xFF)
#define ZUINT64_5BYTE(data)    (((data) >> 40) & 0xFF)
#define ZUINT64_6BYTE(data)    (((data) >> 48) & 0xFF)
#define ZUINT64_7BYTE(data)    ((data)  >> 56)

///检查指针是否32位对其或者64位对齐
#ifndef ZCE_IS_ALIGNED_32
#define ZCE_IS_ALIGNED_32(p) (0 == (0x3 & ((const char*)(p) - (const char*)0)))
#endif
#ifndef ZCE_IS_ALIGNED_64
#define ZCE_IS_ALIGNED_64(p) (0 == (0x7 & ((const char*)(p) - (const char*)0)))
#endif

// 取大小值, min max在linux下没有定义
#define ZCE_MAX(a,b) (((a) > (b)) ? (a) : (b))
#define ZCE_MIN(a,b) (((a) < (b)) ? (a) : (b))

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

// 一些C函数的重命名，保持兼容，避免费力折腾
#if defined ZCE_OS_WINDOWS

#if _MSC_VER <= 1300
#define snprintf     _snprintf
#define vsnprintf    _vsnprintf
#else
//在VS2005以后，使用安全API,保证WINDOWS下更加接近于LINUX，保证末尾会添加'\0'
#define snprintf(buffer,buf_count,fmt,...) _snprintf_s((buffer),(buf_count),((buf_count)-1),(fmt),__VA_ARGS__)
#define vsnprintf(buffer,buf_count,fmt,argptr_list)  _vsnprintf_s((buffer),((buf_count)-1),(buf_count),(fmt),(argptr_list))
#endif

#define strtoull _strtoui64
#define strcasecmp   _stricmp
#define strncasecmp  _strnicmp

#endif

//==================================================================================================
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
#  if defined (ZCE_USE_MYSQL) && (ZCE_USE_MYSQL==1)
#    pragma comment(lib, "libmysql.lib")
#  endif

//这个功能到2008才支持
#if defined ZCE_SUPPORT_WINSVR2008 
#  pragma comment(lib, "dbghelp.lib")
#endif



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

#if !defined ZCE_LIB_LIBARY_NAME
#define ZCE_LIB_LIBARY_NAME   "zcelib-"ZCE_PLAT_TOOLSET_CONF".lib"
#endif

//自动包含的包含连接，简化你的操作
#pragma comment(lib, ZCE_LIB_LIBARY_NAME  )

#endif





#endif //ZCE_LIB_PREDEFINE_H_
