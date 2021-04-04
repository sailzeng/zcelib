#ifndef ZCE_BYTES_BASE_H_
#define ZCE_BYTES_BASE_H_

#include "zce_predefine.h"

//==================================================================================================

//һЩ�ֽ��򽻻��ĺ�
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

//����һ��ntol ntos,nothll�ȣ���Ҫ������64��ת������������ϵͳ���У�
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


//����д��Ϊ�˱���BUS ERROR���⣬�����Լ�Ҳ����100%�϶���������ܽ��BUS ERROR��
//��Ϊ��û�л���������صĲ��ԡ�
//ע��#pragma pack(push, 1) ��__attribute__ ((packed)) ��������ģ�
//���²ο�֮ ����
//ǰ�߸��߱������ṹ������ڲ��ĳ�Ա��������ڵ�һ�������ĵ�ַ��ƫ�����Ķ��뷽ʽ��
//ȱʡ����£�������������Ȼ�߽���룬�������������Ȼ����߽��n�� ʱ������n���룬
//��������Ȼ�߽���룻
//���߸��߱�����һ���ṹ�������������ϻ���һ�����͵ı���
//(����)�����ַ�ռ�ʱ�ĵ�ַ���뷽ʽ��Ҳ�������������__attribute__((aligned(m)))
//������һ�����ͣ���ô�����͵ı����ڷ����ַ�ռ�ʱ�����ŵĵ�ַһ������m�ֽڶ���
//(m�� ����2���ݴη�)��������ռ�õĿռ䣬����С,Ҳ��m�����������Ա�֤�����������洢�ռ��ʱ��
//ÿһ��Ԫ�صĵ�ַҲ�ǰ���m�ֽڶ��롣 __attribute__((aligned(m)))Ҳ����������һ�������ı�����
//���Ͽ��Կ���__attribute__((aligned(m)))�Ĺ��ܸ�ȫ��

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
//__attribute__ ((packed)) �������Ŀ�������������Ա���BUS ERROR����
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


///��һ��(char *)ָ���ڶ�ȡ(Ҳ��������д��)һ��uint16_t,or uint32_t or uint64_t

# define ZBYTE_TO_UINT16(ptr)  ((ZU16_STRUCT *)(ptr))->value_
# define ZBYTE_TO_UINT32(ptr)  ((ZU32_STRUCT *)(ptr))->value_
# define ZBYTE_TO_UINT64(ptr)  ((ZU64_STRUCT *)(ptr))->value_

///��һ��(char *)ָ���ڶ�ȡuint16_t,or uint32_t or uint64_t �������ڵ�ary_index��Ԫ��ע�������±���ֵ�������ε��±꣬(������ptr���±�)
# define ZINDEX_TO_UINT16(ptr,ary_index)  (((ZU16_STRUCT *)(ptr))+(ary_index))->value_
# define ZINDEX_TO_UINT32(ptr,ary_index)  (((ZU32_STRUCT *)(ptr))+(ary_index))->value_
# define ZINDEX_TO_UINT64(ptr,ary_index)  (((ZU64_STRUCT *)(ptr))+(ary_index))->value_

///��һ��(char *)ָ����д��һ��uint16_t,or uint32_t or uint64_t
# define ZUINT16_TO_BYTE(ptr,wr_data)  ((ZU16_STRUCT *)(ptr))->value_ = (wr_data)
# define ZUINT32_TO_BYTE(ptr,wr_data)  ((ZU32_STRUCT *)(ptr))->value_ = (wr_data)
# define ZUINT64_TO_BYTE(ptr,wr_data)  ((ZU64_STRUCT *)(ptr))->value_ = (wr_data)

//��һ��(char *)ָ����д��һ��uint16_t,or uint32_t or uint64_t�������ڲ���ary_index��Ԫ��ע�������±���ֵ�������ε��±꣬(������ptr���±�)
# define ZUINT16_TO_INDEX(ptr,ary_index,wr_data)  (((((ZU16_STRUCT *)(ptr))+(ary_index))->value_) = (wr_data))
# define ZUINT32_TO_INDEX(ptr,ary_index,wr_data)  (((((ZU32_STRUCT *)(ptr))+(ary_index))->value_) = (wr_data))
# define ZUINT64_TO_INDEX(ptr,ary_index,wr_data)  (((((ZU64_STRUCT *)(ptr))+(ary_index))->value_) = (wr_data))


//FLOAT �� DOUBLE�Ĵ���
# define ZBYTE_TO_FLOAT(ptr)  ((ZFLOAT_STRUCT *)(ptr))->value_
# define ZBYTE_TO_DOUBLE(ptr)  ((ZDOUBLE_STRUCT *)(ptr))->value_

# define ZFLOAT_TO_BYTE(ptr,wr_data)  ((ZFLOAT_STRUCT *)(ptr))->value_ = (wr_data)
# define ZDOUBLE_TO_BYTE(ptr,wr_data)  ((ZDOUBLE_STRUCT *)(ptr))->value_ = (wr_data)

# define ZFLOAT_TO_INDEX(ptr,ary_index,wr_data)  (((((ZFLOAT_STRUCT *)(ptr))+(ary_index))->value_) = (wr_data))
# define ZDOUBLE_TO_INDEX(ptr,ary_index,wr_data)  (((((ZDOUBLE_STRUCT *)(ptr))+(ary_index))->value_) = (wr_data))

# define ZINDEX_TO_FLOAT(ptr,ary_index)  (((ZFLOAT_STRUCT *)(ptr))+(ary_index))->value_
# define ZINDEX_TO_DOUBLE(ptr,ary_index)  (((ZDOUBLE_STRUCT *)(ptr))+(ary_index))->value_

//����Ĵ���ӹ����ϵ�ͬ����Ĵ��롣������Щд�ᵼ��BUS ERROR���⡣
//# define ZBYTE_TO_UINT32(ptr)  (*(uint32_t *)(ptr))
//# define ZINDEX_TO_UINT32(ptr,ary_index)  (*(((uint32_t *)(ptr))+(ary_index)))
//# define ZUINT32_TO_BYTE(ptr,wr_data)  ((*(uint32_t *)(ptr)) = (wr_data))
//# define ZUINT32_TO_INDEX(ptr,ary_index,wr_data)  ((*(((uint32_t *)(ptr))+(ary_index))) = (wr_data))

//�����ֽ���Ķ�ȡ�����ͣ����Σ�64λ�����εķ�ʽ
//������˼����ʹ��ˣ�����������е�������ˣ��м���д��ζ��������벻���׼��ܣ�MD5�ȴ�����ΪɶҪ��
//���ֽ������أ�
//��һ��Ϊ�˱����㷨��һ���ԣ�һ̨Сͷ�ֽ�������ϼ���ĵõ������ģ�������һ̨��ͷ��Ļ���Ҫ�ܽ⿪��
//������˵��ͷ��Сͷ�Ļ�����ͬ����һ��buffer�����ܵõ�������Ӧ����һ�µģ�
//���Ҫ���������ֽ�����ȡ��������������һ�µġ����Ծ�Ҫ�����ֽ��������ˡ�������ؼ������ء�
//�ڶ�����Щ�㷨��Ϊ�˼ӿ촦������������������ģ���ʵ�����棬�Ǳ��뿼���ֽ�˳��ģ��������ǵ�
//CRC32�㷨������������⣬���������㷨������뿼���ֽ������⣬Сͷ�������������������Ҫ����ת��
//��������Щ�㷨��ʵ��ȷҪ����������ô�ͷ�򣬻���Сͷ��������ͱ��뿼����Ļ�����
//
//��Ĭ�ϱ���������ϣ��һ���������Сͷ��һ�����ҵĴ������99.99%��������X86�ܹ�֮�£���һ����
//Сͷ���Ǵӵ͵��ߣ����ֽ�˳��һ�£�������Ҳ����һ�㡣

#if (ZCE_BYTES_ORDER == ZCE_LITTLE_ENDIAN)

///��һ��(char *)ָ���ڶ�ȡСͷ�ֽ����uint16_t,or uint32_t or uint64_t����Сͷ�ֽ���Ļ����ϲ������ı�
# define ZBYTE_TO_LEUINT16(ptr)    ZBYTE_TO_UINT16(ptr)
# define ZBYTE_TO_LEUINT32(ptr)    ZBYTE_TO_UINT32(ptr)
# define ZBYTE_TO_LEUINT64(ptr)    ZBYTE_TO_UINT64(ptr)

///��һ��(char *)ָ���ڶ�ȡСͷ�ֽ����uint16_t,or uint32_t or uint64_t �������ڵ�ary_index��Ԫ��ע�������±���ֵ�������ε��±꣬(������ptr���±�)
# define ZINDEX_TO_LEUINT16(ptr,ary_index)  ZINDEX_TO_UINT16(ptr,ary_index)
# define ZINDEX_TO_LEUINT32(ptr,ary_index)  ZINDEX_TO_UINT32(ptr,ary_index)
# define ZINDEX_TO_LEUINT64(ptr,ary_index)  ZINDEX_TO_UINT64(ptr,ary_index)

///��һ��(char *)ָ����д��һ��Сͷ�ֽ����uint16_t,or uint32_t or uint64_t����Сͷ�ֽ���Ļ����ϲ������ı�
# define ZLEUINT16_TO_BYTE(ptr,wr_data)  ZUINT16_TO_BYTE(ptr,wr_data)
# define ZLEUINT32_TO_BYTE(ptr,wr_data)  ZUINT32_TO_BYTE(ptr,wr_data)
# define ZLEUINT64_TO_BYTE(ptr,wr_data)  ZUINT64_TO_BYTE(ptr,wr_data)

//��һ��(char *)ָ����д��һ��Сͷ�ֽ����uuint16_t,or uint32_t or uint64_t�������ڲ���ary_index��Ԫ��ע�������±���ֵ�������ε��±꣬(������ptr���±�)
# define ZLEUINT16_TO_INDEX(ptr,ary_index,wr_data)  ZUINT16_TO_INDEX(ptr,ary_index,wr_data)
# define ZLEUINT32_TO_INDEX(ptr,ary_index,wr_data)  ZUINT32_TO_INDEX(ptr,ary_index,wr_data)
# define ZLEUINT64_TO_INDEX(ptr,ary_index,wr_data)  ZUINT64_TO_INDEX(ptr,ary_index,wr_data)

///��һ��(char *)ָ���ڶ�ȡ��ͷͷ�ֽ����uint16_t,or uint32_t or uint64_t����Сͷ�ֽ���Ļ����Ͻ���ת��
# define ZBYTE_TO_BEUINT16(ptr)  ZCE_SWAP_UINT16(((ZU16_STRUCT *)(ptr))->value_)
# define ZBYTE_TO_BEUINT32(ptr)  ZCE_SWAP_UINT32(((ZU32_STRUCT *)(ptr))->value_)
# define ZBYTE_TO_BEUINT64(ptr)  ZCE_SWAP_UINT64(((ZU64_STRUCT *)(ptr))->value_)

///��һ��(char *)ָ���ڶ�ȡ��ͷ�ֽ����uint16_t,or uint32_t or uint64_t �������ڵ�ary_index��Ԫ��ע�������±���ֵ�������ε��±꣬(������ptr���±�)
# define ZINDEX_TO_BEUINT16(ptr,ary_index)  ZCE_SWAP_UINT16((((ZU16_STRUCT *)(ptr))+(ary_index))->value_)
# define ZINDEX_TO_BEUINT32(ptr,ary_index)  ZCE_SWAP_UINT32((((ZU32_STRUCT *)(ptr))+(ary_index))->value_)
# define ZINDEX_TO_BEUINT64(ptr,ary_index)  ZCE_SWAP_UINT64((((ZU64_STRUCT *)(ptr))+(ary_index))->value_)

///��һ��(char *)ָ����д��һ����ͷ�ֽ����uint16_t,or uint32_t or uint64_t����Сͷ�ֽ���Ļ�����Ҫ����ת��
# define ZBEUINT16_TO_BYTE(ptr,wr_data)  ZBYTE_TO_UINT16(ptr) = ZCE_SWAP_UINT16(wr_data)
# define ZBEUINT32_TO_BYTE(ptr,wr_data)  ZBYTE_TO_UINT32(ptr) = ZCE_SWAP_UINT32(wr_data)
# define ZBEUINT64_TO_BYTE(ptr,wr_data)  ZBYTE_TO_UINT64(ptr) = ZCE_SWAP_UINT64(wr_data)

//��һ��(char *)ָ����д��һ����ͷ�ֽ����uuint16_t,or uint32_t or uint64_t�������ڲ���ary_index��Ԫ��ע�������±���ֵ�������ε��±꣬(������ptr���±�)
# define ZBEUINT16_TO_INDEX(ptr,ary_index,wr_data)  ZINDEX_TO_UINT16(ptr,ary_index) = ZCE_SWAP_UINT16(wr_data)
# define ZBEUINT32_TO_INDEX(ptr,ary_index,wr_data)  ZINDEX_TO_UINT32(ptr,ary_index) = ZCE_SWAP_UINT32(wr_data)
# define ZBEUINT64_TO_INDEX(ptr,ary_index,wr_data)  ZINDEX_TO_UINT64(ptr,ary_index) = ZCE_SWAP_UINT64(wr_data)

//�Դ�ͷ�ֽ�����ж���
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

//������ת����λ��������ͬλ����ʾ����Զ�����������64λ�����Ĳ�����
//ע��n��Ҫ����һ���������ֵ������32λ��ZCE_ROTL32��nҪС��31������
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

///���ָ���Ƿ�32λ�������64λ����
#ifndef ZCE_IS_ALIGNED_32
#define ZCE_IS_ALIGNED_32(p) (0 == (0x3 & ((const char*)(p) - (const char*)0)))
#endif
#ifndef ZCE_IS_ALIGNED_64
#define ZCE_IS_ALIGNED_64(p) (0 == (0x7 & ((const char*)(p) - (const char*)0)))
#endif



//��GCC 4.8�Ĵ����У�д
//unsigned int a = *(unsigned int *)(char_ptr);
//����ָ澯 dereferencing type-punned pointer will break strict-aliasing��
//�ر���������������alignas(1) pack(1),��ô��ʹ��ʹ��ZCE_SWAP_UINT32�����ǿ��ܳ�������澯��
//��ô��ο�ZRD_U32_FROM_BYTES����������⡣
//����ʹ�ò��� -fno-strict-aliasing �� -Wno-strict-aliasing �������������warning��

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

#define ZRD_U16_FROM_BYTES(bytes_ptr,rd_data)  \
    {\
        ZU16_UNION __tmp_var_; \
        __tmp_var_.char_data_[0] = bytes_ptr[0];\
        __tmp_var_.char_data_[1] = bytes_ptr[1];\
        rd_data = __tmp_var_.value_; \
    }
#define ZRD_U32_FROM_BYTES(bytes_ptr,rd_data)  \
    {\
        ZU32_UNION __tmp_var_; \
        __tmp_var_.char_data_[0] = bytes_ptr[0];\
        __tmp_var_.char_data_[1] = bytes_ptr[1];\
        __tmp_var_.char_data_[2] = bytes_ptr[2];\
        __tmp_var_.char_data_[3] = bytes_ptr[3];\
        rd_data = __tmp_var_.value_; \
    }
#define ZRD_U64_FROM_BYTES(bytes_ptr,rd_data)  \
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
#define ZRD_FLOAT_FROM_BYTES(bytes_ptr,rd_data)  \
    {\
        ZFLOAT_UNION __tmp_var_; \
        __tmp_var_.char_data_[0] = bytes_ptr[0];\
        __tmp_var_.char_data_[1] = bytes_ptr[1];\
        __tmp_var_.char_data_[2] = bytes_ptr[2];\
        __tmp_var_.char_data_[3] = bytes_ptr[3];\
        rd_data = __tmp_var_.value_; \
    }
#define ZRD_DOUBLE_FROM_BYTES(bytes_ptr,rd_data)  \
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

#endif

