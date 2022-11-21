/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_data_proc_hash.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2013年2月16日
* @brief
*             数据的HASH处理的函数，包括，MD5，SHA1，SHA256，CRC32等，
*             如果想就爱如MD4,SHA224这些函数，目前的框架增加起来很简单，如果有需要，
*             言语一声，
*             必须要要承认，这个代码有炫技的成分了，为了不写重复代码，我最后还是模版
*             化了算法，
*             我敢说我的代码实现敢说是目前MD5，SHA1算法中写的最清晰的一套（之一），
*             至少我看懂了MD5,SHA1的BLOCK数据处理部分了，才动的手。
*             其实MD5，SHA这类算法的方法都几乎一致，都是将数据信息进行分组，每组固
*             定N个字节，然后对这个分组进行摘要（杂凑）运算，然后将结果用于下一个分组
*             运算，最后一个分组一般用一个标识符号(0x80)结束，后面补充0，最后放入长度.
*
* @details    整体的算法参考过rhash这个库，好处是后面发现，其实辛亏参考的是这套库
*             后面发现其他库，在某些环节陷得更深，这套库某些程度还好。
*             当然他的bug也不算少，比如冗余代码，某些地方字节序处理错误等。
*             本来以为拖几个代码进来，迅速搞掂的一些代码，结果发现，上帝呀。他们都在
*             搞咩，我稀里糊涂的看了1天多，最后决定看懂算法再动手。
*             我总结这些代码这样难以看懂的原因大致如下：
*             大体大家当年可以参考的代码有几套，（可以看出流派差别），RSA的代码，
*             openssl的代码等，这些代码当年估计估计来自很多数学家，数学家很多时候
*             写的代码不具备可读性，比如大部分算法里面的xxx_update函数，在final
*             函数又调用，可能他们太聪明了，思维可以多路径化。目前的代码多是在这些
*             基础上改进的。很多动手改的人也没有真正理解问题，就动了手，结果很多代
*             码反而让我这种吹毛求疵的疑惑。
*             而且早期机器的字节序估计都是一种（BE），而后面的改进过程，字节序的问
*             题慢慢被考虑，结果代码就别的有点乱了。
*             另外，很多书和说明，对于MD5,SHA1算法的说明都很含混，《应用密码学》
*             里面对于SHA1的每次处理的块BLOCK只有一句话描述，和MD5一样，但实际
*             呢？SHA1算法里面的数据都是用BE编码的（最后一个长度也要求用BE格式），
*             而MD5算法内部数据是LE，这些也造成了理解的痛苦。
*
*             BTW: rhashlib采用的协议是MIT，在此再次感谢原来的作者，
*             另外维基上面的伪代码帮助非常大，
*
*             从某个意义上讲，MD5，SHA1,SHA256算法的代码重复度很高，crypt++库采用模板的
*             方法避免了一定意义的代码重复
*             http://www.cnblogs.com/fullsail/archive/2013/02/22/2921505.html
*             当时文档中的代码还是是C代码，我这儿已经模板策略化，这也是crypto++库的
*             代码的算法里面往往你就看见摘要算法一个函数的原因，我这儿不费力了，
*
*             做的一些简单的性能测试benchmark数据，270个字节的字符串，执行
*             10000000次，VC++ 2010, Realse 版本 O2
*             为了防止内部优化，每次测试字符串数据都不一样，感谢airfu教我了怎么看
*             编译器优化问题。
*             hash_bkdr 3.514  seconds.  768.355M/s
*             crc32     5.433  seconds.  496.963M/s
*             md5       10.137 seconds.  266.351M/s
*             sha1      17.173 seconds.  157.224M/s
*
*             《红日》 李克勤
*             命运就算颠沛流离
*             命运就算曲折离奇
*             命运就算恐吓着你做人没趣味
*             别流泪 心酸 更不应舍弃
*             我愿能 一生永远陪伴你
*             命运就算颠沛流离
*             命运就算曲折离奇
*             命运就算恐吓着你做人没趣味
*             别流泪 心酸 更不应舍弃
*             我愿能 一生永远陪伴你
*             哦~~
*             # 一生之中兜兜转转 那会看清楚
*             彷徨时我也试过独坐一角像是没协助
*             在某年 那幼小的我
*             跌倒过几多几多落泪在雨夜滂沱
*             一生之中弯弯曲曲我也要走过
*             从何时有你有你伴我给我热烈地拍和
*             像红日之火 燃点真的我
*             结伴行 千山也定能踏过
*             让晚风 轻轻吹过
*             伴送着清幽花香像是在祝福你我
*             让晚星 轻轻闪过
*             闪出你每个希冀如浪花 快要沾湿我
*             命运就算颠沛流离
*             命运就算曲折离奇
*             命运就算恐吓着你做人没趣味
*             别流泪 心酸 更不应舍弃
*             我愿能 一生永远陪伴你
*             我愿能 一生永远陪伴你
*             哦~~
*             # 一生之中兜兜转转 那会看清楚
*             彷徨时我也试过独坐一角像是没协助
*             在某年 那幼小的我
*             跌倒过几多几多落泪在雨夜滂沱
*             一生之中弯弯曲曲我也要走过
*             从何时有你有你伴我给我热烈地拍和
*             像红日之火 燃点真的我
*             结伴行 千山也定能踏过
*             让晚风 轻轻吹过
*             伴送着清幽花香像是在祝福你我
*             让晚星 轻轻闪过
*             闪出你每个希冀如浪花 快要沾湿我
*             命运就算颠沛流离
*             命运就算曲折离奇
*             命运就算恐吓着你做人没趣味
*             别流泪 心酸 更不应舍弃
*             我愿能 一生永远陪伴你
*             命运就算颠沛流离
*             命运就算曲折离奇
*             命运就算恐吓着你做人没趣味
*             别流泪 心酸 更不应舍弃
*             我愿能 一生永远陪伴你
*             命运就算颠沛流离
*             命运就算曲折离奇
*             命运就算恐吓着你做人没趣味
*             别流泪 心酸 更不应舍弃
*             我愿能 一生永远陪伴你
*             命运就算颠沛流离
*             命运就算曲折离奇
*             命运就算恐吓着你做人没趣味
*             别流泪 心酸 更不应舍弃
*             我愿能 一生永远陪伴你
*             命运就算颠沛流离
*             命运就算曲折离奇
*             命运就算恐吓着你做人没趣味
*             别流泪 心酸 更不应舍弃
*             我愿能 一生永远陪伴你
*             哦~~
*/

#pragma once

#include "zce/bytes/bytes_common.h"
#include "zce/util/scope_guard.h"

namespace zce
{
//=====================================================================================================

/*!
* @brief      64字节为BLOCK的HASH算法的处理策略类的基类，定义相关的context结构等
*             如果是从MD的算法系列出来的算法，BLOCK长度是64的，都可以用这个函数扩展出来
*             比如MD4,MD5,SHA0,SHA1,SHA256,SHA224,
*             BLOCK是128的SHA512这些类不能从这个方法扩展得到
* @tparam     result_size    HASH结果的尺寸，字节
* @tparam     little_endian  这个算法是否采用小头编码，如果是用true，如果不是，用false，
* @tparam     HASH_STRATEGY  算法的策略类
*/
template <size_t result_size, bool little_endian, typename HASH_STRATEGY>
class hashfun_block64
{
public:

    ///结果数据长度
    static const size_t HASH_RESULT_SIZE = result_size;
    ///每次处理的BLOCK的大小
    static const size_t PROCESS_BLOCK_SIZE = 64;

    //算法的上下文，保存一些状态，中间数据，结果
    typedef struct hash_fun_ctx
    {
        ///处理的数据的总长度
        uint64_t length_;
        ///还没有处理的数据长度
        uint64_t unprocessed_;
        ///(中间)计算结果保存的地方
        uint32_t hash_[HASH_RESULT_SIZE / 4];
    } hash_fun_ctx;

    typedef hash_fun_ctx context;

    /*!
    * @brief      内部函数，处理数据的前面部分(>PROCESS_BLOCK_SIZE 65字节的部分)，每次组成一个PROCESS_BLOCK_SIZE字节的block就进行杂凑处理
    * @param[out] ctx  算法的context，用于记录一些处理的上下文和结果
    * @param[in]  buf  处理的数据，
    * @param[in]  buf_size 处理的数据长度
    */
    static void process(context* ctx, const char* buf, size_t buf_size)
    {
        //为什么不是=，因为在某些环境下，可以多次调用zce_md5_update，但这种情况，必须保证前面的调用，每次都没有unprocessed_
        ctx->length_ += buf_size;

        //每个处理的块都是PROCESS_BLOCK_SIZE字节
        while (buf_size >= PROCESS_BLOCK_SIZE)
        {
            HASH_STRATEGY::process_block(ctx->hash_, reinterpret_cast<const uint32_t*>(buf));
            buf += PROCESS_BLOCK_SIZE;
            buf_size -= PROCESS_BLOCK_SIZE;
        }

        ctx->unprocessed_ = buf_size;
    }

    /*!
    * @brief      根据算法要求和当前机器的字节序类型，进行N个uint32_t数据的拷贝操作，
    *             如果本地序列和当前机器的字节序类型不符合，会进行SWAP
    * @param      dst      目标指针
    * @param      src      拷贝的源头指针
    * @param      length   长度，注意是字节长度
    */
    inline static void endian_copy(void* dst, const void* src, size_t length)
    {
        //根据算法的大小头要求，以及当前CPU的大小头，进行运算
        bool if_le = little_endian;
#if ZCE_ENDIAN_ORDER == ZCE_ENDIAN_LITTLE
        //如果环境是是小头，而且算法编码要求小头的，直接使用指针
        if (if_le)
        {
            ::memcpy(dst, src, length);
        }
        //如果环境是是小头，而算法编码要求大头的，将所有的uint32_t数据swap
        else
        {
            ::memcpy(dst, src, length);
            //所有的数据反转
            for (size_t i = 0; i < length / 4; ++i)
            {
                ((uint32_t*)dst)[i] = ZCE_SWAP_UINT32(((uint32_t*)dst)[i]);
            }
        }
#else
        //如果环境是大头，但算法内部要求的数据是小头，，将所有的uint32_t数据swap
        if (if_le)
        {
            ::memcpy(dst, src, length);
            //所有的数据反转
            for (size_t i = 0; i < length / 4; ++i)
            {
                ((uint32_t*)dst)[i] = ZCE_SWAP_UINT32(((uint32_t*)dst)[i]);
            }
        }
        //如果环境是大头，但算法内部要求的数据是大头，，直接使用指针
        else
        {
            ::memcpy(dst, src, length);
        }
#endif
    }

    /*!
    * @brief      内部函数，处理数据的末尾部分，我们要拼出最后1个（或者两个）要处理的BLOCK，加上0x80，加上长度进行处理
    * @param[in]  ctx    算法的context，用于记录一些处理的上下文和结果
    * @param[in]  buf    处理的数据
    * @param[in]  buf_size   处理buffer的长度
    * @param[out] result 返回的结果，
    */
    static void finalize(context* ctx,
                         const char* buf,
                         size_t buf_size,
                         char result[HASH_RESULT_SIZE])
    {
        uint32_t message[PROCESS_BLOCK_SIZE / 4] = { 0 };

        //保存剩余的数据，我们要拼出最后1个（或者两个）要处理的块，前面的算法保证了，最后一个块肯定小于64个字节
        memset(message, 0, PROCESS_BLOCK_SIZE);
        if (ctx->unprocessed_)
        {
            memcpy(message, buf + buf_size - ctx->unprocessed_, static_cast<size_t>(ctx->unprocessed_));
        }

        //每个处理的块都是64字节

        //得到0x80要添加在的位置（在uint32_t 数组中），
        uint32_t index = (uint32_t)((ctx->length_ & 63) >> 2);
        uint32_t shift = (uint32_t)((ctx->length_ & 3) * 8);

        //添加0x80进去，并且把余下的空间补充0
        message[index] &= ~(0xFFFFFFFF << shift);
        message[index++] ^= 0x80 << shift;

        //如果这个block还无法处理，其后面的长度无法容纳长度64bit，那么先处理这个block
        if (index > 14)
        {
            while (index < 16)
            {
                message[index++] = 0;
            }

            HASH_STRATEGY::process_block(ctx->hash_, message);

            index = 0;
        }

        //补0
        while (index < 14)
        {
            message[index++] = 0;
        }

        //保存长度，注意是bit位的长度,这个问题让我看着郁闷了半天，
        uint64_t data_len = (ctx->length_) << 3;

        //注意MD5算法要求的64bit的长度是小头LITTLE-ENDIAN编码，注意下面的比较是!=
        bool if_le = little_endian;
#if ZCE_ENDIAN_ORDER == ZCE_ENDIAN_LITTLE
        if (!if_le)
        {
            data_len = ZCE_SWAP_UINT64(data_len);
        }
#else
        if (if_le)
        {
            data_len = ZCE_SWAP_UINT64(data_len);
        }
#endif

        message[14] = (uint32_t)(data_len & 0x00000000FFFFFFFFULL);
        message[15] = (uint32_t)((data_len & 0xFFFFFFFF00000000ULL) >> 32);

        HASH_STRATEGY::process_block(ctx->hash_, message);

        //注意结果是小头党的，在大头的世界要进行转换
        endian_copy(result, ctx->hash_, HASH_RESULT_SIZE);
    }

    //你要实现算法需要实现的两个函数，
public:

    /*!
    * @brief      初始化context，内容
    * @param      ctx  context 的指针
    */
    //static void initialize(context *ctx);

    /*!
    * @brief      将64个字节，16个uint32_t的数组进行摘要（杂凑）处理，
    * @param      state 存放处理的hash数据结果,
    * @param      block 要处理的block，64个字节，16个uint32_t的数组  这个参数没有对编码进行转换
    */
    //static void process_block(uint32_t state[HASH_RESULT_SIZE / 4],
    //    const uint32_t block[PROCESS_BLOCK_SIZE / 4]);
};

//=====================================================================================================
//MD5的处理策略类，主要是实现算法，不用直接使用，
//本来是希望用protected和friends 友元避免外部能感知这些函数的，但友元对对模版的支持还不够，我只能public了
class hash_md5 : public zce::hashfun_block64<16, true, hash_md5>
{
public:

    static void initialize(context* ctx);

    static void process_block(uint32_t state[HASH_RESULT_SIZE / 4],
                              const uint32_t block[PROCESS_BLOCK_SIZE / 4]);
};

//=====================================================================================================
//SHA1的处理策略类
class hash_sha1 : public zce::hashfun_block64<20, false, hash_sha1>
{
public:

    static void initialize(context* ctx);

    static void process_block(uint32_t state[HASH_RESULT_SIZE / 4],
                              const uint32_t block[PROCESS_BLOCK_SIZE / 4]);
};

//=====================================================================================================

//SHA256，SHA2的算法策略
class hash_sha256 : public zce::hashfun_block64<32, false, hash_sha256>
{
public:

    static void initialize(context* ctx);

    static void process_block(uint32_t state[HASH_RESULT_SIZE / 4],
                              const uint32_t block[PROCESS_BLOCK_SIZE / 4]);
};

//=====================================================================================================

///CRC32 主要是为了方便计算文件CRC32等实现的，其实如果只是计算内存CRC值，下面有一个函数实现简单
class hash_crc32
{
public:
    //结果数据长度
    static const size_t HASH_RESULT_SIZE = 4;
    //每次处理的BLOCK的大小,其实对于CRC32这个值没有啥用，主要用于文件处理。
    static const size_t PROCESS_BLOCK_SIZE = 256;

    typedef uint32_t context;

    static void process(context* ctx,
                        const char* buf,
                        size_t buf_size);

    static void finalize(context* ctx,
                         const char* buf,
                         size_t buf_size,
                         char result[HASH_RESULT_SIZE]);

    static void initialize(context* ctx);
};

//=====================================================================================================

//=====================================================================================================
//传统的HASH算法函数，

/*!
* @brief      计算一个内存数据的HASH值，我讨厌重复自己，于是就有了这个模版，和这种实现方式
* @tparam     HASH_STRATEGY 模版参数，对于一个数据求HASH的算法，策略，zce::hash_sha1，zce::hash_md5等
* @return     char * 返回得到的结果
* @param[in]  buf    处理的BUFFER
* @param[in]  buf_size   BUFFER的长度
* @param[out] result 返回的结果数据指针
*/
template<typename HASH_STRATEGY>
char* hash_fun(const char* buf,
               size_t buf_size,
               char result[HASH_STRATEGY::HASH_RESULT_SIZE])
{
    ZCE_ASSERT(result != nullptr);

    typename HASH_STRATEGY::context ctx;
    HASH_STRATEGY::initialize(&ctx);
    HASH_STRATEGY::process(&ctx, buf, buf_size);
    HASH_STRATEGY::finalize(&ctx, buf, buf_size, result);
    return result;
}

/*!
* @brief      计算一个内存数据的HASH值，一个典型的策略模式
* @tparam     HASH_STRATEGY   模版参数，对于一个数据求HASH的算法，策略，ZCE_Hash_SHA1，ZCE_Hash_MD5等
* @tparam     BUFFER_MULTIPLE 函数函数一般一次处理一个块，你可以通过这个参数设置读取的BUFFER是这个块的倍数，
*                             （用于调整你的IO处理和CPU计算）
* @return     int       成功返回0，失败，文件无法打开，无法读取，返回-1
* @param      file_name 文件名称
* @param      result    返回的结果数据指针
* @note
*/
template<typename HASH_STRATEGY, size_t BUFFER_MULTIPLE>
int hash_file(const char* file_name,
              char result[HASH_STRATEGY::HASH_RESULT_SIZE])
{
    //打开文件
    zce::auto_handle  fd(zce::open(file_name, O_RDONLY));
    if (ZCE_INVALID_HANDLE == fd.get())
    {
        return -1;
    }

    //获取文件尺寸，有长度可以避免有时候如果读取的文件长度和缓冲相等，要读取一次的麻烦，
    size_t file_size = 0;
    int ret = zce::filesize(fd.get(), &file_size);
    if (0 != ret)
    {
        return -1;
    }

    //每次尽力读取640K数据，注意这个buffer必须是PROCESS_BLOCK_SIZE字节的N倍喔
    const size_t READ_LEN = HASH_STRATEGY::PROCESS_BLOCK_SIZE * BUFFER_MULTIPLE;
    std::unique_ptr<char[]> read_buf(new char[READ_LEN]);

    typename HASH_STRATEGY::context ctx;
    HASH_STRATEGY::initialize(&ctx);

    ssize_t read_len = 0;

    do
    {
        //读取内容
        read_len = zce::read(fd.get(), read_buf.get(), READ_LEN);
        if (read_len < 0)
        {
            return -1;
        }
        HASH_STRATEGY::process(&ctx, (char*)read_buf.get(), read_len);
    } while ((file_size -= read_len) > 0);

    HASH_STRATEGY::finalize(&ctx, (char*)read_buf.get(), read_len, result);

    return 0;
}

/*!
* @brief      求某个内存块的MD5，写这个函数主要就是为了方便你使用
* @return     unsigned char* 返回的的结果，
* @param[in]  buf    求MD5的内存BUFFER指针
* @param[in]  buf_size   BUFFER长度
* @param[out] result 结果
*/
inline char* md5(const char* buf,
                 size_t buf_size,
                 char result[zce::hash_md5::HASH_RESULT_SIZE])
{
    return zce::hash_fun<zce::hash_md5>(buf, buf_size, result);
}

/*!
* @brief      求某个文件的MD5值，内部将文件分片处理，计算得到MD5,这个函数即使文件很大也没有关系，
*             但在32位系统，对于文件长度有限制
* @return     int         返回0标识成功
* @param      file_name   文件路径，名称
* @param      result      返回的结果
*/
inline int md5_file(const char* file_name,
                    char result[zce::hash_md5::HASH_RESULT_SIZE])
{
    return zce::hash_file<zce::hash_md5, 10240>(file_name, result);
}

/*!
* @brief      求内存块BUFFER的SHA1值
* @return     unsigned char* 返回的的结果
* @param[in]  buf    求SHA1的内存BUFFER指针
* @param[in]  buf_size   BUFFER长度
* @param[out] result 结果
*/
inline char* sha1(const char* buf,
                  size_t buf_size,
                  char result[zce::hash_sha1::HASH_RESULT_SIZE])
{
    return zce::hash_fun<zce::hash_sha1>(buf, buf_size, result);
}

/*!
* @brief      求某个文件的SHA1值，内部将文件分片处理，计算得到SHA1,这个函数即使文件很大也没有关系，
*             但在32位系统，对于文件长度有限制
* @return     int         返回0标识成功
* @param[in]  file_name   文件路径，名称
* @param[out] result      返回的结果
*/
inline int sha1_file(const char* file_name,
                     char result[zce::hash_sha1::HASH_RESULT_SIZE])
{
    return zce::hash_file<zce::hash_sha1, 10240>(file_name, result);
}

/*!
* @brief      求内存块BUFFER的SHA256值
* @return     unsigned char* 返回的的结果
* @param[in]  buf    求SHA1的内存BUFFER指针
* @param[in]  buf_size   BUFFER长度
* @param[out] result 结果
*/
inline char* sha256(const char* buf,
                    size_t buf_size,
                    char result[zce::hash_sha256::HASH_RESULT_SIZE])
{
    return zce::hash_fun<zce::hash_sha256>(buf, buf_size, result);
}

/*!
* @brief      求某个文件的SHA256值，内部将文件分片处理，计算得到SHA256,这个函数即使文件很大也没有关系，
*             但在32位系统，对于文件长度有限制
* @return     int         返回0标识成功
* @param[in]  file_name   文件路径，名称
* @param[out] result      返回的结果
*/
inline int sha256_file(const char* file_name,
                       char result[zce::hash_sha256::HASH_RESULT_SIZE])
{
    return zce::hash_file<zce::hash_sha256, 10240>(file_name, result);
}

/*!
* @brief      求一个buffer的CRC32值，可以用于一些要求速度的简单校验
* @return     uint32_t 返回求得的CRC32值
* @param[in]  crcinit  CRC的初始值，用于辅助验证,如果在2个地方验证，初始值要一致
* @param[in]  buf      求CRC的内存BUFFER
* @param[in]  buf_size     BUFFER长度
*/
uint32_t crc32(uint32_t crcinit,
               const char* buf,
               size_t buf_size);

/*!
* @brief      初始值为0的CRC32函数
* @return     uint32_t   返回的CRC32值
* @param[in]  buf        计算CRC32的内存
* @param[in]  buf_size   内存长度
*/
inline uint32_t crc32(const char* buf,
                      size_t buf_size)
{
    return crc32(0, buf, buf_size);
}

/*!
* @brief      初始值为0的计算文件的CRC32的函数
* @return     int        返回0表示成功，否则表示读取文件失败
* @param[in]  file_name  文件名称
* @param[out] result     返回的CRC32值
*/
inline  int crc32_file(const char* file_name,
                       uint32_t* result)
{
    return zce::hash_file<hash_crc32, 10240>(file_name, (char*)result);
}

uint16_t crc16(uint16_t crcinit,
               const char* buf,
               size_t buf_size);

inline uint16_t crc16(const char* buf,
                      size_t buf_size)
{
    return crc16(0, buf, buf_size);
}

//=====================================================================================================
///计算字符串的HASH算法函数，
///我选择的算法也就BKDRHash AP Hash,DJBHash,JS Hash
///前人已经做了很多工作，我就是抄过来了，有兴趣的去研究一些这些文章，
///http://blog.csdn.net/liuben/article/details/5050697
///http://www.byvoid.com/blog/string-hash-compare/
///如果你懒得出奇，那么就选择BKDRHash吧，
///我的稍微的改进是讲unsigned int 改为了size_t,
///char *  改为了unsigned char * ，同时增加了长度参数，适应二进制数据，

/*!
* @brief      计算字符串的HASH算法函数， BKDR 算法 Hash Function，这些函数没有用inline
*             的原因是，在VS2010，Release版本，O2下，发现没有进行inline优化……
* @return     size_t  返回的HASH值，使用size_t在不同环境下不同,看来inline的优化比我想的要难
* @param[in]  str     字符串
* @param[in]  str_len 字符串长度
*/
size_t hash_bkdr(const char* str, size_t str_len);

/// AP Hash Function
size_t hash_ap(const char* str, size_t str_len);

/// JS Hash Function
size_t hash_js(const char* str, size_t str_len);

/// DJB Hash Function
size_t hash_djb(const char* str, size_t str_len);

//=====================================================================================================
};
