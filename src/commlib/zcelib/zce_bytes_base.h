#ifndef ZCE_BYTES_BASE_H_
#define ZCE_BYTES_BASE_H_

#include "zce_predefine.h"

//==================================================================================================

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
<<<<<<< HEAD
//否则按照自然边界对齐；后者告诉编译器一个结构体或者类或者联合或者一个类型的变量
=======
//否则按照自然边界对齐；
//后者告诉编译器一个结构体或者类或者联合或者一个类型的变量
>>>>>>> ecb76a1a4aa8381667ced3cb31202915f48ca78b
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
struct ZFLOAT_STRUCT
{
    float value_;
};
struct ZDOUBLE_STRUCT
{
    double value_;
};

#pragma pack(pop)

#elif defined ZCE_OS_LINUX
//__attribute__ ((packed)) 在这儿的目的是利用其特性避免BUS ERROR错误
struct ZU16_STRUCT
{
    uint16_t value_;
} __attribute__((packed));
struct ZU32_STRUCT
{
    uint32_t value_;
} __attribute__((packed));
struct ZU64_STRUCT
{
    uint64_t value_;
} __attribute__((packed));
struct ZFLOAT_STRUCT
{
    float value_;
} __attribute__((packed));
struct ZDOUBLE_STRUCT
{
    double value_;
} __attribute__((packed));
#endif


<<<<<<< HEAD

///在GCC 4.8的处理中，写
//unsigned int a = *(unsigned int *)(char_ptr);
//会出现告警 dereferencing type-punned pointer will break strict-aliasing。
//而且使用 ZBYTE_TO_UINT32 也无法绕开，请参考ZRD_U32_FROM_BYTES解决类似问题。

=======
>>>>>>> ecb76a1a4aa8381667ced3cb31202915f48ca78b
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


//FLOAT 和 DOUBLE的处理
# define ZBYTE_TO_FLOAT(ptr)  ((ZFLOAT_STRUCT *)(ptr))->value_
# define ZBYTE_TO_DOUBLE(ptr)  ((ZDOUBLE_STRUCT *)(ptr))->value_

# define ZFLOAT_TO_BYTE(ptr,wr_data)  ((ZFLOAT_STRUCT *)(ptr))->value_ = (wr_data)
# define ZDOUBLE_TO_BYTE(ptr,wr_data)  ((ZDOUBLE_STRUCT *)(ptr))->value_ = (wr_data)

# define ZFLOAT_TO_INDEX(ptr,ary_index,wr_data)  (((((ZFLOAT_STRUCT *)(ptr))+(ary_index))->value_) = (wr_data))
# define ZDOUBLE_TO_INDEX(ptr,ary_index,wr_data)  (((((ZDOUBLE_STRUCT *)(ptr))+(ary_index))->value_) = (wr_data))

# define ZINDEX_TO_FLOAT(ptr,ary_index)  (((ZFLOAT_STRUCT *)(ptr))+(ary_index))->value_
# define ZINDEX_TO_DOUBLE(ptr,ary_index)  (((ZDOUBLE_STRUCT *)(ptr))+(ary_index))->value_

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

///从一个(char *)指针内读取大头头字节序的uint16_t,or uint32_t or uint64_t，在小头字节序的机器上进行转换
# define ZBYTE_TO_BEUINT16(ptr)  ZCE_SWAP_UINT16(((ZU16_STRUCT *)(ptr))->value_)
# define ZBYTE_TO_BEUINT32(ptr)  ZCE_SWAP_UINT32(((ZU32_STRUCT *)(ptr))->value_)
# define ZBYTE_TO_BEUINT64(ptr)  ZCE_SWAP_UINT64(((ZU64_STRUCT *)(ptr))->value_)

///从一个(char *)指针内读取大头字节序的uint16_t,or uint32_t or uint64_t 的数组内的ary_index单元，注意数组下标是值对于整形的下标，(而不是ptr的下标)
# define ZINDEX_TO_BEUINT16(ptr,ary_index)  ZCE_SWAP_UINT16((((ZU16_STRUCT *)(ptr))+(ary_index))->value_)
# define ZINDEX_TO_BEUINT32(ptr,ary_index)  ZCE_SWAP_UINT32((((ZU32_STRUCT *)(ptr))+(ary_index))->value_)
# define ZINDEX_TO_BEUINT64(ptr,ary_index)  ZCE_SWAP_UINT64((((ZU64_STRUCT *)(ptr))+(ary_index))->value_)

///向一个(char *)指针内写入一个大头字节序的uint16_t,or uint32_t or uint64_t，在小头字节序的机器上要进行转换
# define ZBEUINT16_TO_BYTE(ptr,wr_data)  ZBYTE_TO_UINT16(ptr) = ZCE_SWAP_UINT16(wr_data)
# define ZBEUINT32_TO_BYTE(ptr,wr_data)  ZBYTE_TO_UINT32(ptr) = ZCE_SWAP_UINT32(wr_data)
# define ZBEUINT64_TO_BYTE(ptr,wr_data)  ZBYTE_TO_UINT64(ptr) = ZCE_SWAP_UINT64(wr_data)

//向一个(char *)指针内写入一个大头字节序的uuint16_t,or uint32_t or uint64_t的数组内部的ary_index单元，注意数组下标是值对于整形的下标，(而不是ptr的下标)
# define ZBEUINT16_TO_INDEX(ptr,ary_index,wr_data)  ZINDEX_TO_UINT16(ptr,ary_index) = ZCE_SWAP_UINT16(wr_data)
# define ZBEUINT32_TO_INDEX(ptr,ary_index,wr_data)  ZINDEX_TO_UINT32(ptr,ary_index) = ZCE_SWAP_UINT32(wr_data)
# define ZBEUINT64_TO_INDEX(ptr,ary_index,wr_data)  ZINDEX_TO_UINT64(ptr,ary_index) = ZCE_SWAP_UINT64(wr_data)

//对大头字节序进行定义
#else

# define ZBYTE_TO_LEUINT16(ptr)  ZCE_SWAP_UINT16(((ZU16_STRUCT *)(ptr))->value_)
# define ZBYTE_TO_LEUINT32(ptr)  ZCE_SWAP_UINT32(((ZU32_STRUCT *)(ptr))->value_)
# define ZBYTE_TO_LEUINT64(ptr)  ZCE_SWAP_UINT64(((ZU64_STRUCT *)(ptr))->value_)

# define ZINDEX_TO_LEUINT16(ptr,ary_index)  ZCE_SWAP_UINT16((((ZU16_STRUCT *)(ptr))+(ary_index))->value_)
# define ZINDEX_TO_LEUINT32(ptr,ary_index)  ZCE_SWAP_UINT32((((ZU32_STRUCT *)(ptr))+(ary_index))->value_)
# define ZINDEX_TO_LEUINT64(ptr,ary_index)  ZCE_SWAP_UINT64((((ZU64_STRUCT *)(ptr))+(ary_index))->value_)

# define ZLEUINT16_TO_BYTE(ptr,wr_data)  ZBYTE_TO_UINT16(ptr) = ZCE_SWAP_UINT16(wr_data))
# define ZLEUINT32_TO_BYTE(ptr,wr_data)  ZBYTE_TO_UINT32(ptr) = ZCE_SWAP_UINT32(wr_data))
# define ZLEUINT64_TO_BYTE(ptr,wr_data)  ZBYTE_TO_UINT64(ptr) = ZCE_SWAP_UINT64(wr_data))

# define ZLEUINT16_TO_INDEX(ptr,ary_index,wr_data)  ZINDEX_TO_UINT16(ptr,ary_index) = ZCE_SWAP_UINT16(wr_data)
# define ZLEUINT32_TO_INDEX(ptr,ary_index,wr_data)  ZINDEX_TO_UINT32(ptr,ary_index) = ZCE_SWAP_UINT32(wr_data)
# define ZLEUINT64_TO_INDEX(ptr,ary_index,wr_data)  ZINDEX_TO_UINT64(ptr,ary_index) = ZCE_SWAP_UINT64(wr_data)

# define ZBYTE_TO_BEUINT16(ptr)    ZBYTE_TO_UINT16(ptr)
# define ZBYTE_TO_BEUINT32(ptr)    ZBYTE_TO_UINT32(ptr)
# define ZBYTE_TO_BEUINT64(ptr)    ZBYTE_TO_UINT64(ptr)

# define ZINDEX_TO_BEUINT16(ptr,ary_index)  ZINDEX_TO_UINT16(ptr,ary_index)
# define ZINDEX_TO_BEUINT32(ptr,ary_index)  ZINDEX_TO_UINT32(ptr,ary_index)
# define ZINDEX_TO_BEUINT64(ptr,ary_index)  ZINDEX_TO_UINT64(ptr,ary_index)

# define ZBEUINT16_TO_BYTE(ptr,wr_data)  ZUINT16_TO_BYTE(ptr,wr_data)
# define ZBEUINT32_TO_BYTE(ptr,wr_data)  ZUINT32_TO_BYTE(ptr,wr_data)
# define ZBEUINT64_TO_BYTE(ptr,wr_data)  ZUINT64_TO_BYTE(ptr,wr_data)

# define ZBEUINT16_TO_INDEX(ptr,ary_index,wr_data)  ZUINT16_TO_INDEX(ptr,ary_index,wr_data)
# define ZBEUINT32_TO_INDEX(ptr,ary_index,wr_data)  ZUINT32_TO_INDEX(ptr,ary_index,wr_data)
# define ZBEUINT64_TO_INDEX(ptr,ary_index,wr_data)  ZUINT64_TO_INDEX(ptr,ary_index,wr_data)

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



<<<<<<< HEAD
//GCC 对于对其采取了默认认为不是对齐的方式编译，所以写
//unsigned int a = *(unsigned int *)(char_ptr);
//还有告警错误，方法是采用下面的宏规避，
//或者使用参数 -fstrict-aliasing 和 -Wstrict-aliasing 可以消除掉这个warning。
=======
//在GCC 4.8的处理中，写
//unsigned int a = *(unsigned int *)(char_ptr);
//会出现告警 dereferencing type-punned pointer will break strict-aliasing。
//特别是如果你的数据是alignas(1) pack(1),那么即使你使用ZCE_SWAP_UINT32，还是可能出现这个告警。
//那么请参考ZRD_U32_FROM_BYTES解决类似问题。
//或者使用参数 -fno-strict-aliasing 和 -Wno-strict-aliasing 可以消除掉这个warning。
>>>>>>> ecb76a1a4aa8381667ced3cb31202915f48ca78b

union ZU16_UNION
{
    char char_data_[2];
    uint16_t value_;
};
union ZU32_UNION
{
    char char_data_[4];
    uint32_t value_;
};
union ZU64_UNION
{
    char char_data_[8];
    uint64_t value_;
};
union ZFLOAT_UNION
{
    char char_data_[4];
    float value_;
};
union ZDOUBLE_UNION
{
    char char_data_[8];
    double value_;
};

<<<<<<< HEAD

# define ZRD_U16_FROM_BYTES(rd_data,bytes_ptr)  \
=======
#define ZRD_U16_FROM_BYTES(bytes_ptr,rd_data)  \
>>>>>>> ecb76a1a4aa8381667ced3cb31202915f48ca78b
    {\
        ZU16_UNION __tmp_var_; \
        __tmp_var_.char_data_[0] = bytes_ptr[0];\
        __tmp_var_.char_data_[1] = bytes_ptr[1];\
        rd_data = __tmp_var_.value_; \
    }
<<<<<<< HEAD

# define ZRD_U32_FROM_BYTES(rd_data,bytes_ptr)  \
=======
#define ZRD_U32_FROM_BYTES(bytes_ptr,rd_data)  \
>>>>>>> ecb76a1a4aa8381667ced3cb31202915f48ca78b
    {\
        ZU32_UNION __tmp_var_; \
        __tmp_var_.char_data_[0] = bytes_ptr[0];\
        __tmp_var_.char_data_[1] = bytes_ptr[1];\
        __tmp_var_.char_data_[2] = bytes_ptr[2];\
        __tmp_var_.char_data_[3] = bytes_ptr[3];\
        rd_data = __tmp_var_.value_; \
    }
<<<<<<< HEAD


# define ZRD_U64_FROM_BYTES(rd_data,bytes_ptr)  \
=======
#define ZRD_U64_FROM_BYTES(bytes_ptr,rd_data)  \
>>>>>>> ecb76a1a4aa8381667ced3cb31202915f48ca78b
    {\
        ZU64_UNION __tmp_var_; \
        __tmp_var_.char_data_[0] = bytes_ptr[0];\
        __tmp_var_.char_data_[1] = bytes_ptr[1];\
        __tmp_var_.char_data_[2] = bytes_ptr[2];\
        __tmp_var_.char_data_[3] = bytes_ptr[3];\
        __tmp_var_.char_data_[4] = bytes_ptr[4];\
        __tmp_var_.char_data_[5] = bytes_ptr[5];\
        __tmp_var_.char_data_[6] = bytes_ptr[6];\
        __tmp_var_.char_data_[7] = bytes_ptr[7];\
        rd_data = __tmp_var_.value_; \
    }
<<<<<<< HEAD

# define ZRD_FLOAT_FROM_BYTES(rd_data,bytes_ptr)  \
=======
#define ZRD_FLOAT_FROM_BYTES(bytes_ptr,rd_data)  \
>>>>>>> ecb76a1a4aa8381667ced3cb31202915f48ca78b
    {\
        ZFLOAT_UNION __tmp_var_; \
        __tmp_var_.char_data_[0] = bytes_ptr[0];\
        __tmp_var_.char_data_[1] = bytes_ptr[1];\
        __tmp_var_.char_data_[2] = bytes_ptr[2];\
        __tmp_var_.char_data_[3] = bytes_ptr[3];\
        rd_data = __tmp_var_.value_; \
    }
<<<<<<< HEAD


# define ZRD_DOUBLE_FROM_BYTES(rd_data,bytes_ptr)  \
=======
#define ZRD_DOUBLE_FROM_BYTES(bytes_ptr,rd_data)  \
>>>>>>> ecb76a1a4aa8381667ced3cb31202915f48ca78b
    {\
        ZDOUBLE_UNION __tmp_var_; \
        __tmp_var_.char_data_[0] = bytes_ptr[0];\
        __tmp_var_.char_data_[1] = bytes_ptr[1];\
        __tmp_var_.char_data_[2] = bytes_ptr[2];\
        __tmp_var_.char_data_[3] = bytes_ptr[3];\
        __tmp_var_.char_data_[4] = bytes_ptr[4];\
        __tmp_var_.char_data_[5] = bytes_ptr[5];\
        __tmp_var_.char_data_[6] = bytes_ptr[6];\
        __tmp_var_.char_data_[7] = bytes_ptr[7];\
        rd_data = __tmp_var_.value_; \
    }
<<<<<<< HEAD

=======
#define ZWT_U16_FROM_BYTES(bytes_ptr,wr_data)  \
    { \
        ZU16_UNION __tmp_var_; \
        __tmp_var_.value_ = wr_data;\
        bytes_ptr[0]=__tmp_var_.char_data_[0];\
        bytes_ptr[1]=__tmp_var_.char_data_[1];\
    }
#define ZWT_U32_FROM_BYTES(bytes_ptr,wr_data)  \
    { \
        ZU32_UNION __tmp_var_; \
        __tmp_var_.value_ = wr_data; \
        bytes_ptr[0]=__tmp_var_.char_data_[0];\
        bytes_ptr[1]=__tmp_var_.char_data_[1];\
        bytes_ptr[2]=__tmp_var_.char_data_[2];\
        bytes_ptr[3]=__tmp_var_.char_data_[3];\
    }
#define ZWT_U64_FROM_BYTES(bytes_ptr,wr_data)  \
    { \
        ZU64_UNION __tmp_var_; \
        __tmp_var_.value_ = wr_data; \
        bytes_ptr[0]=__tmp_var_.char_data_[0];\
        bytes_ptr[1]=__tmp_var_.char_data_[1];\
        bytes_ptr[2]=__tmp_var_.char_data_[2];\
        bytes_ptr[3]=__tmp_var_.char_data_[3];\
        bytes_ptr[4]=__tmp_var_.char_data_[4];\
        bytes_ptr[5]=__tmp_var_.char_data_[5];\
        bytes_ptr[6]=__tmp_var_.char_data_[6];\
        bytes_ptr[7]=__tmp_var_.char_data_[7];\
    }
#define ZWT_FLOAT_FROM_BYTES(bytes_ptr,wr_data)  \
    { \
        ZFLOAT_UNION __tmp_var_; \
        __tmp_var_.value_ = wr_data; \
        bytes_ptr[0]=__tmp_var_.char_data_[0];\
        bytes_ptr[1]=__tmp_var_.char_data_[1];\
        bytes_ptr[2]=__tmp_var_.char_data_[2];\
        bytes_ptr[3]=__tmp_var_.char_data_[3];\
    }
#define ZWT_DOUBLE_FROM_BYTES(bytes_ptr,wr_data)  \
    { \
        ZDOUBLE_UNION __tmp_var_; \
        __tmp_var_.value_ = wr_data; \
        bytes_ptr[0]=__tmp_var_.char_data_[0];\
        bytes_ptr[1]=__tmp_var_.char_data_[1];\
        bytes_ptr[2]=__tmp_var_.char_data_[2];\
        bytes_ptr[3]=__tmp_var_.char_data_[3];\
        bytes_ptr[4]=__tmp_var_.char_data_[4];\
        bytes_ptr[5]=__tmp_var_.char_data_[5];\
        bytes_ptr[6]=__tmp_var_.char_data_[6];\
        bytes_ptr[7]=__tmp_var_.char_data_[7];\
    }
>>>>>>> ecb76a1a4aa8381667ced3cb31202915f48ca78b

#endif

