/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/bytes/encrypt.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2013年4月5日
*             清明节，禽流感H7N9在上海江浙一带肆掠，10年一个轮回
*             从开始写到CBC完成，居然用了20多天，一方面中间有点忙，一方面
*             发现松了一口气后，效率急剧降低，
*             2013年4月20日
*             芦山地震，7级，逝者安息，
*
* @brief      加密函数，提供了一些 分组加密函数，以及交织（块）加密的算法，可以直接使用分组加
*             密算法作为策略，目前已经支持的策略包括TEA,XTEA,XXTEA,RC5,RC6,CAST5,CAST6
*             (CAST-256)，MARS,DES,3DES,AES,其中很多代码根据KEY长度，加密轮数，给出了不
*             同定义。你用这些东西足可以写个对抗系统了.
*             也许未来会考虑的算法是twofish，safer+，
*
*             算法内部对数据的处理完全采用了LE(little end)小头编码处理方式，（就是在BE的CPU上也
*             会转换为LE进行处理）。字节序的处理方式主要体现在如何把一个BLOCK块里面的数据视为怎样的
*             整数。ZINDEX_TO_LEUINT32 和 ZLEUINT32_TO_INDEX
*
*             交织算法我值选择了最常用的策略CBC（其实我们的实现更接近CBC-MAC）：
*             http://zh.wikipedia.org/zh-hant/%E5%9D%97%E5%AF%86%E7%A0%81%E7%9A%84%E5%B7%A5%E4%BD%9C%E6%A8%A1%E5%BC%8F
*             这个代码库已经可以和相当多的加密代码库叫板一下了。吼吼。
*             ZCE_Crypt 是一个对于流数据的加密模版，里面根据加密算法的KEY_SIZE（密钥长度）
*             和BLOCK_SIZE （分组加密算法每次处理的块长度）进行处理，对于流，使用CBC交织算
*             法（就是前面一个BLOCK数据和后面BLOCK异或）和填充算法进行加密。加密算法只是里面
*             的策略，
*
* @details    CBC的代码，我拥有完全版权，是参考维基算法实现的模版
*             而具体加密算法大部分代码是从openssl，Cryptlib，libtom-crypt，taocrypt
*             PolarSSL，移植出来，也有相当的代码来自维基，和 http://code.ohloh.net
*             的搜索，我的移植过程是把这些代码完全转化成了自己的东东，包括宏转换等，
*             一般代码没有说明轮数，key长度，和代码实现的关系，我尽最大的努力明晰了一部分
*             关系
*
*             根据学习的知识，我也重新看了TX的算法，
*             http://www.cnblogs.com/fullsail/archive/2013/04/29/3051456.html
*
*             性能测试数据，如果每次处理251个字节的数据，每次处理都计算sub key,这时候得到的
*             性能数据是
*             算法（名称_块大小_key大小_轮数）  加密解密处理的耗时                            加密包数量(251字节)    解密包数量(251字节)  加密速度(M/s)      解密速度(M/s)           保存SUBKEY 加密，解密
*                                                                                                                                                                   对速度的影响(%)
*             XOR_Crypt_128_128_1          usetime[   6193.0279 us][   7550.2505 us] times [  3306944.57/s][  2712492.78/s],speed [   949.278 M/s][   778.637 M/s],subkey[ -2.75][   12.87].
*             DES_Crypt_64_64_16           usetime[  93373.4321 us][  86166.8413 us] times [   219334.34/s][   237678.43/s],speed [    62.961 M/s][    68.227 M/s],subkey[  8.02][    5.06].
*             DES3_Crypt_64_192_16         usetime[ 240679.3577 us][ 237867.5682 us] times [    85092.47/s][    86098.33/s],speed [    24.426 M/s][    24.715 M/s],subkey[  6.41][    8.19].
*             TEA_Crypt_64_128_16          usetime[  42721.7227 us][  41723.7100 us] times [   479381.42/s][   490848.01/s],speed [   137.609 M/s][   140.901 M/s],subkey[  1.64][   -0.15].
*             TEA_Crypt_64_128_32          usetime[  82524.0484 us][  83136.1048 us] times [   248170.08/s][   246343.03/s],speed [    71.239 M/s][    70.714 M/s],subkey[ -0.87][   -4.29].
*             TEA_Crypt_64_128_64          usetime[ 165654.5551 us][ 161183.5578 us] times [   123630.77/s][   127060.11/s],speed [    35.489 M/s][    36.473 M/s],subkey[ -0.26][   -1.05].
*             XTEA_Crypt_64_128_16         usetime[  46351.4535 us][  43234.2577 us] times [   441841.59/s][   473698.43/s],speed [   126.833 M/s][   135.978 M/s],subkey[ -0.23][    1.04].
*             XTEA_Crypt_64_128_32         usetime[  90651.2118 us][  88223.8235 us] times [   225920.86/s][   232136.84/s],speed [    64.852 M/s][    66.636 M/s],subkey[ -0.77][    0.84].
*             XTEA_Crypt_64_128_64         usetime[ 181532.5668 us][ 175920.8059 us] times [   112817.22/s][   116416.02/s],speed [    32.385 M/s][    33.418 M/s],subkey[  0.04][   -1.77].
*             XXTEA_Crypt_64_128_32        usetime[ 187377.2699 us][ 118152.3859 us] times [   109298.21/s][   173335.48/s],speed [    31.375 M/s][    49.757 M/s],subkey[ -0.94][    0.58].
*             XXTEA_Crypt_128_128_16       usetime[  86094.0663 us][  96435.5801 us] times [   237879.34/s][   212369.75/s],speed [    68.285 M/s][    60.962 M/s],subkey[ -3.74][    9.43].
*             GOST_Crypt_64_256_32         usetime[ 216748.8236 us][ 171384.8087 us] times [    94487.25/s][   119497.17/s],speed [    27.123 M/s][    34.302 M/s],subkey[ 11.68][   -8.60].
*             RC5_Crypt_64_128_12          usetime[  23352.6882 us][  19211.9774 us] times [   876986.83/s][  1066001.67/s],speed [   251.744 M/s][   306.002 M/s],subkey[  4.31][    7.84].
*             RC5_Crypt_64_128_20          usetime[  37303.4064 us][  30671.8625 us] times [   549011.52/s][   667712.96/s],speed [   157.597 M/s][   191.671 M/s],subkey[  5.60][    3.67].
*             RC6_Crypt_128_128_12         usetime[  19221.9295 us][  17158.4162 us] times [  1065449.75/s][  1193583.35/s],speed [   305.844 M/s][   342.625 M/s],subkey[  7.16][    7.90].
*             RC6_Crypt_128_128_20         usetime[  30216.2413 us][  28319.7373 us] times [   677781.19/s][   723170.55/s],speed [   194.561 M/s][   207.590 M/s],subkey[  3.42][   12.60].
*             CAST5_Crypt_64_128_8         usetime[  31392.1484 us][  29553.8022 us] times [   652392.43/s][   692973.44/s],speed [   187.273 M/s][   198.922 M/s],subkey[  5.99][    4.83].
*             CAST5_Crypt_64_96_16         usetime[  61906.3317 us][  58983.8247 us] times [   330822.38/s][   347213.84/s],speed [    94.965 M/s][    99.670 M/s],subkey[  5.44][    4.19].
*             CAST5_Crypt_64_128_12        usetime[  47192.7200 us][  43538.1089 us] times [   433965.24/s][   470392.50/s],speed [   124.572 M/s][   135.029 M/s],subkey[  6.00][    4.75].
*             CAST6_Crypt_128_128_48       usetime[  82599.6224 us][  81944.3365 us] times [   247943.02/s][   249925.75/s],speed [    71.174 M/s][    71.743 M/s],subkey[ 20.23][   20.65].
*             CAST6_Crypt_128_192_48       usetime[  82101.3936 us][  82218.9532 us] times [   249447.65/s][   249090.98/s],speed [    71.605 M/s][    71.503 M/s],subkey[ 21.16][   18.25].
*             CAST6_Crypt_128_256_48       usetime[  84457.2509 us][  80539.2192 us] times [   242489.54/s][   254286.05/s],speed [    69.608 M/s][    72.994 M/s],subkey[ 23.33][   19.46].
*             MARS_Crypt_128_128_1         usetime[  64761.3509 us][  64620.4659 us] times [   316238.00/s][   316927.46/s],speed [    90.778 M/s][    90.976 M/s],subkey[ 35.59][   34.18].
*             MARS_Crypt_128_192_1         usetime[  65772.1147 us][  63665.9939 us] times [   311378.16/s][   321678.79/s],speed [    89.383 M/s][    92.340 M/s],subkey[ 35.44][   35.48].
*             MARS_Crypt_128_256_1         usetime[  65402.0197 us][  63582.3337 us] times [   313140.18/s][   322102.05/s],speed [    89.889 M/s][    92.461 M/s],subkey[ 34.10][   35.39].
*             AES_Crypt_128_128_10         usetime[  29016.3868 us][  29941.3135 us] times [   705808.07/s][   684004.73/s],speed [   202.606 M/s][   196.348 M/s],subkey[  4.78][   10.31].
*             AES_Crypt_128_192_12         usetime[  33515.0635 us][  35894.8681 us] times [   611068.51/s][   570555.10/s],speed [   175.411 M/s][   163.781 M/s],subkey[  4.44][   10.73].
*             AES_Crypt_128_256_14         usetime[  38541.8254 us][  40393.5448 us] times [   531370.78/s][   507011.70/s],speed [   152.533 M/s][   145.541 M/s],subkey[  3.61][   10.83].
*
*             对比一下，信心爆棚，快60%                       加密耗时              解密耗时              加密速度(M/s)    解密速度(M/s)
*             TEA_Crypt_8_16_16                           297709.87           305490.29           85.459          87.692
*             TXTEA16                                     184472.20           185296.85           52.853          53.644
*
*             每次都使用SUB KEY进行操作，对性能的影响并不太大，我认为原因如下，
*             1.目前选择的算法KEY == >SUB KEY还并不是最主要消耗CPU的地方，（Blowfish这种算法销毁可能就大不少）
*             2.密钥的长度还是很短的，（对比起要加密的密文）
*             3.负数部分应该都不用看，主要可能是计时误差导致的
*             4.RC，CAST，MARS，AES这类算法使用sub key性能应该都有提升。
*
* @note       1.很多算法的模板都有typedef，请直接使用typedef的类，因为不少实现是有假设的，
*             2.实现的算法很多时候没有考虑某些平台可能出现的BUS ERROR问题，算了，我的代码能移植到
*               RISC( SPARC)CPU上吗？(ZINDEX_TO_LEUINT32这类转换是从指针读取整数的，不考虑对齐
*               ，这个在X86体系上OK，但在一些 SPARC 架构 CPU上是会报错。不知现在如何了，吼吼)
*               如果真出现这种需求CRYPT_XOR_BLOCK，ZINDEX_TO_LEUINT32，ZLEUINT32_TO_INDEX
*               这些宏要改进。
*             3.所有代码在读取整数是默认采用小头序列，这个是因为我们绝绝大部分时候在小头的机器上运行
*               而所有代码考虑过大头机器的问题，各种环境都可以正常运行，
*
*             艾琳娜
*             唱：动力火车    曲：尹恒   词：武雄
*             L O V E   L O V E   L O V E GO GO GO
*             L O V E   L O V E   L O V E GO GO GO
*             风梳过的头发 雨淋过的潇洒
*             踏上了这条路   我踩着轻快的步伐
*             满天的晚霞   染红了天之涯
*             思念像一杯烈酒  不断的升华
*             热血满腔 离乡  迈向那未知的远方
*             每一次再回头望   回家的路又更长
*             我流浪 闯荡 可从来都不曾遗忘
*             那个让我朝思暮想 美丽的姑娘
*             艾琳娜 我回来啦 可知我心多牵挂
*             艾琳娜 我回来啦 你的心是我的家
*             L O V E   L O V E   L O V E GO GO GO
*             久违的亲人啊 梦中的爱人吶
*             用红红的炉火 温一盏思念的热茶
*             我背上老吉他 热泪已不听话
*             一句句唱着那首 古老的问答
*             别来无恙 故乡  我白发苍苍的爹娘
*             是你紧紧的拥抱 温暖漂泊的肩膀
*             我流浪 闯荡 可从来都不曾遗忘
*             永远让我朝思暮想  归去的地方
*             艾琳娜 我回来啦 可知我心多牵挂
*             艾琳娜 我回来啦 你的心是我的家
*             L O V E   L O V E   L O V E  L O V E
*             艾琳娜 我回来啦 可知我心多牵挂
*             艾琳娜 我回来啦 你的心是我的家
*             艾琳娜 我回来啦 可知我心多牵挂
*             艾琳娜 我回来啦 你的心是我的家
*             L O V E  L O V E   L O V E GO GO GO
*             L O V E  L O V E   L O V E GO GO GO
*/

#ifndef ZCE_LIB_BYTES_ENCRYPT_H_
#define ZCE_LIB_BYTES_ENCRYPT_H_

#include "zce/bytes/bytes_common.h"
#include "zce/math/random.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/string.h"

//因为模版参数的缘故，可能让部分代码恒定为真，所以先暂时屏蔽
#if defined (ZCE_OS_WINDOWS)
#pragma warning ( push )
#pragma warning ( disable : 4127)
#endif

namespace zce
{
//=================================================================================================================

/*!
@brief      提供ECB块加密，CBC交织加密的加密方式，使用各种加密算法作为
            策略，（一般情况大家主要就使用typedef就OK了）

            其中CBC的交织算法，我们的实现和标准基本一致，
             A.)一个BLOCK长度的IV，用随机数填充
             B.)数据区，
             C.)一个4字节的0，用于校验数据(所以其实准确的说，这是CBC-MAC模式)
             D.)补齐标识，补齐算法为需要补齐3，填写3个字节的0x3，如果需要补齐长度11，
                填写11个字节的0xB，如果正好余0，补齐一个BLOCK_SIZE字节的BLOCK_SIZE值。

            其他交织算法我也看过（对比而言和CBC并没有优势），暂时没有实现的必要，以后也许
            会实现一套算法加密文件。慢慢来把。

@tparam     ENCRYPT_STRATEGY 加密算法策略，
@note       加密算法，BLOCK_SIZE 必须是4的倍数，大于等于8，必须小于等于256，
*/
template < typename ENCRYPT_STRATEGY >
class ZCE_Crypt
{
public:
    //从策略类萃取SUBKEY结构
    typedef typename ENCRYPT_STRATEGY::SUBKEY_STRUCT   CRYPT_SUBKEY_STRUCT;

    /*!
    @brief
    @param      key        密钥
    @param      key_len    密钥长度
    @param      sub_key    密钥更换的子key
    @param      if_encrypt 是否是进行加密处理，因为有些算法（就是AES了）的SKEY，加密和解密不同，
    */
    inline static void key_setup(const unsigned char* key,
                                 size_t key_len,
                                 typename ENCRYPT_STRATEGY::SUBKEY_STRUCT* sub_key,
                                 bool if_encrypt)
    {
        unsigned char real_key[ENCRYPT_STRATEGY::KEY_SIZE];
        const unsigned char* proc_key = key;
        //如果key的长度不够，对key进行一下处理，
        if (key_len < ENCRYPT_STRATEGY::KEY_SIZE)
        {
            generate_real_key(key, key_len, real_key);
            proc_key = real_key;
        }

        ENCRYPT_STRATEGY::key_setup(proc_key, sub_key, if_encrypt);
    }

    //打开主要是为了对比测试
    //protected:

        /*!
        @brief      ECB的加密，加密一个块，块的长度就是算法BLOCK_SIZE
        @param      key         加密的密钥
        @param      key_len     加密的密钥长度
        @param      src_buf     原文
        @param      cipher_buf  生成的密文
        @note       注意，这个只对一个块进行加密，如果你希望连续对多个块进行加密，
                    优先选择cbc的方式，如果非要如此，请再封装一个函数，直接用这个
                    函数不合适，因为大部分加密算法都会对密钥进行转换为一个SKEY，如
                    果直接使用这个函数会无端增加耗时
        */
    inline static void ecb_encrypt(const unsigned char* key,
                                   size_t key_len,
                                   const unsigned char* src_buf,
                                   unsigned char* cipher_buf)
    {
        //计算得到sub key
        typename ENCRYPT_STRATEGY::SUBKEY_STRUCT sub_key;
        key_setup(key, key_len, &sub_key, true);

        ENCRYPT_STRATEGY::ecb_encrypt(&sub_key, src_buf, cipher_buf);
    }

    /*!
    @brief      ECB模式的解密，解密一个块，块的长度就是算法BLOCK_SIZE
    @param      key         加密的密钥
    @param      key_len     密钥的长度
    @param      cipher_buf  密文
    @param      src_buf     生成的原文
    @note       见ecb_encrypt 函数
    */
    inline  static void ecb_decrypt(const unsigned char* key,
                                    size_t key_len,
                                    const unsigned char* cipher_buf,
                                    unsigned char* src_buf)
    {
        //计算得到sub key
        typename ENCRYPT_STRATEGY::SUBKEY_STRUCT sub_key;
        key_setup(key, key_len, &sub_key, false);

        ENCRYPT_STRATEGY::ecb_decrypt(&sub_key, cipher_buf, src_buf);
    }

public:
    /*!
    @brief      从原文加密得到密文，密文BUFFER所需长度，和加密算法的BLOCK_SIZE有关系，
                密文长度=IV(一个BLOCK)+原文长度+4个字节的校验0长度+补齐pid的长度，

                补齐的每个字符都填写为这个长度，
    @return     size_t     返回密文BUFFER所需要长度
    @param[in]  source_len 原文长度
    */
    inline static size_t cbc_cipher_buflen(size_t src_len)
    {
        return ((src_len + sizeof(uint32_t)) / ENCRYPT_STRATEGY::BLOCK_SIZE + 2) * ENCRYPT_STRATEGY::BLOCK_SIZE;
    }

    /*!
    @brief      从密文解密得到原文，原文BUFFER所需的长度，注意这个长度比实际原文要长一点，方便处理和校验
    @return     size_t     原文BUFFER所需的长度
    @param[in]  cipher_len
    @note
    */
    inline static size_t cbc_source_buflen(size_t cipher_len)
    {
        return cipher_len - ENCRYPT_STRATEGY::BLOCK_SIZE;
    }

    /*!
    @brief      如果用户的密钥长度比算法要求的少，用这个函数生成一个长度足够的密钥
    @param      key       用户的密钥
    @param      key_len   用户的密钥的长度
    @param      real_key  长度足够的密钥
    */
    inline static void generate_real_key(const unsigned char* key,
                                         size_t key_len,
                                         unsigned char* real_key)
    {
        size_t copykey_count = 0;
        size_t once_copy = 0;
        do
        {
            once_copy = (ENCRYPT_STRATEGY::KEY_SIZE - copykey_count > key_len) ? key_len : ENCRYPT_STRATEGY::KEY_SIZE - copykey_count;
            memcpy(real_key + copykey_count, key, once_copy);
            copykey_count += once_copy;
        } while (copykey_count < ENCRYPT_STRATEGY::KEY_SIZE);
    }

    //先异或，使用整数，加快速度，原来对uint64_t做过优化，希望在OS64位上，能更加高效，
    //但结果又被编译器优化折腾了2天，对于O1正常，O2以上就会发生问题问题，导致执行错误，（错误现象是取出的一个值的部分错误）
    //简单的疑惑而已，没有考虑字节序
#ifndef CRYPT_XOR_BLOCK
#define CRYPT_XOR_BLOCK(result_ptr,x_ptr,y_ptr)  \
    for (size_t i = 0; i < ENCRYPT_STRATEGY::BLOCK_SIZE / sizeof(uint32_t); ++i) \
    { \
        *(((uint32_t *)result_ptr) + i) = (*(((uint32_t *)x_ptr) + i)) ^ (*((uint32_t *)(y_ptr) + i)); \
    }
#endif

    /*!
    @brief      CBC加密
                补齐的算法：加密都是按照一个BLOCK进行，所以必须补齐到一个BLOCK，补几个字节，
                如果空间刚好被BLOCK长度整除，那么就完整填充一个BLOCK
    @return     int         返回0标识加密成功，-1标识失败，一般而言失败表示参数错误
    @param[in]     key         密钥
    @param[in]     key_len     密钥长度
    @param[in]     src_buf     原文
    @param[in]     src_len     原文长度
    @param[out]    cipher_buf  密文的BUFFER
    @param[in,out] cipher_len  密文长度，入参标识密文BUFFER的长度，返回时，返回密文的实际长度
    @param[in]     iv          initialization vector,初始化的向量，为nullptr表示你不关心，我会用随机数帮你填充，
    */
    static int cbc_encrypt(const unsigned char* key,
                           size_t key_len,
                           const unsigned char* src_buf,
                           size_t src_len,
                           unsigned char* cipher_buf,
                           size_t* cipher_len,
                           const uint32_t* iv = nullptr)
    {
        //检查参数，如果不合适断言或者返回错误
        ZCE_ASSERT(key
                   && src_buf
                   && cipher_buf
                   && key_len > 0
                   && src_len > 0);
        if (key == nullptr
            || src_buf == nullptr
            || cipher_buf == nullptr
            || key_len <= 0
            || src_len <= 0)
        {
            ZCE_LOG(RS_ERROR, "Fun[%s] key[%p][%lu] soucre[%p][%lu] cipher[%p][%lu]",
                    __ZCE_FUNC__,
                    key,
                    key_len,
                    src_buf,
                    src_len,
                    cipher_buf,
                    *cipher_len
            );
            return -1;
        }

        //计算得到sub key
        typename ENCRYPT_STRATEGY::SUBKEY_STRUCT sub_key;
        key_setup(key, key_len, &sub_key, true);

        return cbc_encrypt_skey(&sub_key,
                                src_buf,
                                src_len,
                                cipher_buf,
                                cipher_len,
                                iv);
    }

    /*!
    @brief      很多加密算法其实会根据key，产生一个sub_key，其实后面加密，解密都是用这个sub_key进行，
                将其这部分代码独立出来，如果你需要加速处理，可以考虑保存sub_key，然后进行处理
    @return     int
    @param      sub_key 用key生成的子key，使用key_setup函数生成
    @note       其他参数亲请参考cbc_encrypt
    */
    static int cbc_encrypt_skey(const typename ENCRYPT_STRATEGY::SUBKEY_STRUCT* sub_key,
                                const unsigned char* src_buf,
                                size_t src_len,
                                unsigned char* cipher_buf,
                                size_t* cipher_len,
                                const uint32_t* iv = nullptr)
    {
        //加密BUF所需要的长度，
        size_t cphbuf_need_len = ((src_len + sizeof(uint32_t)) / ENCRYPT_STRATEGY::BLOCK_SIZE + 2)
            * ENCRYPT_STRATEGY::BLOCK_SIZE;

        //检查参数，如果不合适断言或者返回错误
        ZCE_ASSERT(sub_key
                   && src_buf
                   && cipher_buf
                   && src_len > 0
                   && *cipher_len >= cphbuf_need_len);
        if (sub_key == nullptr
            || src_buf == nullptr
            || cipher_buf == nullptr
            || src_len <= 0
            || *cipher_len < cphbuf_need_len)
        {
            ZCE_LOG(RS_ERROR, "Fun[%s] sub_key [%p] soucre[%p][%lu] cipher[%p][%lu], cipher buffer need len[%lu]. ",
                    __ZCE_FUNC__,
                    sub_key,
                    src_buf,
                    src_len,
                    cipher_buf,
                    *cipher_len,
                    cphbuf_need_len
            );
            return -1;
        }

        //得到密文长度
        *cipher_len = cphbuf_need_len;

        //用于记录异或结果的数据区，+4是因为尾巴补0的时候，可能溢出一个BLOCK，免得写一堆判断代码。
        unsigned char xor_result[ENCRYPT_STRATEGY::BLOCK_SIZE] = { 0 };
        unsigned char last_prc_block[ENCRYPT_STRATEGY::BLOCK_SIZE + sizeof(uint32_t)] = { 0 };

        //用你给出的iv以及随机数算法生成IV.填补进入密文数据区，目前设计的可以填充的iv 4个字节
        unsigned char* write_ptr = cipher_buf;

        for (size_t i = 0; i < ENCRYPT_STRATEGY::BLOCK_SIZE / sizeof(uint32_t); ++i)
        {
            if (i == 0 && iv)
            {
                ZLEUINT32_TO_BYTE(write_ptr, *iv);
            }
            else
            {
                ZLEUINT32_TO_INDEX(write_ptr, i, zce::mt19937_instance::instance()->rand());
            }
            write_ptr += sizeof(uint32_t);
        }
        const unsigned char* xor_ptr = write_ptr;
        write_ptr += ENCRYPT_STRATEGY::BLOCK_SIZE;
        const unsigned char* read_ptr = src_buf;

        //
        size_t remain_len = src_len;
        while (remain_len > ENCRYPT_STRATEGY::BLOCK_SIZE)
        {
            //先异或，如果是64bit的的代码，做出优化处理
            CRYPT_XOR_BLOCK(xor_result, read_ptr, xor_ptr);

            //使用加密算法的ECB模式对数据块进行加密
            ENCRYPT_STRATEGY::ecb_encrypt(sub_key, xor_result, write_ptr);

            remain_len -= ENCRYPT_STRATEGY::BLOCK_SIZE;
            xor_ptr = write_ptr;
            write_ptr += ENCRYPT_STRATEGY::BLOCK_SIZE;
            read_ptr += ENCRYPT_STRATEGY::BLOCK_SIZE;
        }

        //最后一些数据，可能要在处理成2个BLOCK
        if (remain_len > 0)
        {
            memcpy(last_prc_block, src_buf + src_len - remain_len, remain_len);
        }

        //尾部放4个字节的0，用于校验验证，这种方法的有效性应该接近CRC32
        //空间上考虑了补0溢出的可能
        *((uint32_t*)(last_prc_block + remain_len)) = 0;
        remain_len += sizeof(uint32_t);

        //如果已经大于一个BLOCK
        if (remain_len + sizeof(uint32_t) >= ENCRYPT_STRATEGY::BLOCK_SIZE)
        {
            //异或BLOCK的数据
            CRYPT_XOR_BLOCK(xor_result, last_prc_block, xor_ptr);

            //使用加密算法的ECB模式对数据块进行加密
            ENCRYPT_STRATEGY::ecb_encrypt(sub_key, xor_result, write_ptr);

            remain_len -= ENCRYPT_STRATEGY::BLOCK_SIZE;
            xor_ptr = write_ptr;
            write_ptr += ENCRYPT_STRATEGY::BLOCK_SIZE;

            //补的0要放在第二个2个BLOCK了，
            for (size_t k = 0; k < remain_len; ++k)
            {
                *(last_prc_block + k) = 0;
            }
        }

        //补齐一个block,
        size_t pad_len = ENCRYPT_STRATEGY::BLOCK_SIZE - remain_len;
        for (size_t i = 0; i < pad_len; ++i)
        {
            *(last_prc_block + remain_len + i) = (uint8_t)(pad_len);
        }

        //异或
        CRYPT_XOR_BLOCK(xor_result, last_prc_block, xor_ptr);

        //使用加密算法的ECB模式对数据块进行加密
        ENCRYPT_STRATEGY::ecb_encrypt(sub_key, xor_result, write_ptr);

        return 0;
    }

    /*!
    @brief      CBC解密，
    @return     int         返回0表示加密成功，-1表示失败，原因可能是校验位错误，补齐数据错误,参数错误等
    @param[in]     key         密钥
    @param[in]     key_len     密钥长度
    @param[in]     cipher_buf  密文
    @param[in]     cipher_len  密文的长度
    @param[out]    src_buf     解密原文的BUFFER
    @param[in,out] src_len     输入参数是表示原文BUFFER的长度，返回时表示原文的长度
    */
    static int cbc_decrypt(const unsigned char* key,
                           size_t key_len,
                           const unsigned char* cipher_buf,
                           size_t cipher_len,
                           unsigned char* src_buf,
                           size_t* src_len,
                           uint32_t* iv = nullptr)
    {
        //
        size_t srcbuf_need_len = cipher_len - ENCRYPT_STRATEGY::BLOCK_SIZE;

        //检查参数，如果不合适断言或者返回错误
        ZCE_ASSERT(key
                   && src_buf
                   && cipher_buf
                   && key_len > 0
                   && cipher_len >= ENCRYPT_STRATEGY::BLOCK_SIZE * 2
                   && 0 == cipher_len % ENCRYPT_STRATEGY::BLOCK_SIZE
                   && *src_len >= srcbuf_need_len);
        if (key == nullptr
            || src_buf == nullptr
            || cipher_buf == nullptr
            || key_len <= 0
            || cipher_len < ENCRYPT_STRATEGY::BLOCK_SIZE * 2
            || 0 != cipher_len % ENCRYPT_STRATEGY::BLOCK_SIZE
            || *src_len < srcbuf_need_len)
        {
            ZCE_LOG(RS_ERROR, "Fun[%s] key[%p][%lu] cipher[%p][%lu] soucre[%p][%lu] cipher buffer need len[%lu].",
                    __ZCE_FUNC__,
                    key,
                    key_len,
                    cipher_buf,
                    cipher_len,
                    src_buf,
                    *src_len,
                    srcbuf_need_len
            );
            return -1;
        }

        //计算得到sub key
        typename ENCRYPT_STRATEGY::SUBKEY_STRUCT sub_key;
        key_setup(key, key_len, &sub_key, false);

        return cbc_decrypt_skey(&sub_key,
                                cipher_buf,
                                cipher_len,
                                src_buf,
                                src_len,
                                iv);
    }

    /*!
    @brief
    @return     int
    @param      sub_key
    @note       其他参数说明清参考cbc_decrypt
    */
    static int cbc_decrypt_skey(const typename ENCRYPT_STRATEGY::SUBKEY_STRUCT* sub_key,
                                const unsigned char* cipher_buf,
                                size_t cipher_len,
                                unsigned char* src_buf,
                                size_t* src_len,
                                uint32_t* iv = nullptr)
    {
        //
        size_t srcbuf_need_len = cipher_len - ENCRYPT_STRATEGY::BLOCK_SIZE;

        //检查参数，如果不合适断言或者返回错误
        ZCE_ASSERT(sub_key
                   && src_buf
                   && cipher_buf
                   && cipher_len >= ENCRYPT_STRATEGY::BLOCK_SIZE * 2
                   && 0 == cipher_len % ENCRYPT_STRATEGY::BLOCK_SIZE
                   && *src_len >= srcbuf_need_len);
        if (sub_key == nullptr
            || src_buf == nullptr
            || cipher_buf == nullptr
            || cipher_len < ENCRYPT_STRATEGY::BLOCK_SIZE * 2
            || 0 != cipher_len % ENCRYPT_STRATEGY::BLOCK_SIZE
            || *src_len < srcbuf_need_len)
        {
            ZCE_LOG(RS_ERROR, "Fun[%s] sub_key[%p] cipher[%p][%lu] soucre[%p][%lu] cipher buffer need len[%lu].",
                    __ZCE_FUNC__,
                    sub_key,
                    cipher_buf,
                    cipher_len,
                    src_buf,
                    *src_len,
                    srcbuf_need_len
            );
            return -1;
        }

        unsigned char* write_ptr = src_buf;
        const unsigned char* xor_ptr = cipher_buf;
        const unsigned char* read_ptr = (cipher_buf + ENCRYPT_STRATEGY::BLOCK_SIZE);
        size_t remain_len = cipher_len - ENCRYPT_STRATEGY::BLOCK_SIZE;
        unsigned char decrypt_result[ENCRYPT_STRATEGY::BLOCK_SIZE];
        if (iv)
        {
            *iv = ZBYTE_TO_LEUINT32(cipher_buf);
        }
        while (remain_len > 0)
        {
            //使用加密算法的ECB模式对数据块进行加密
            ENCRYPT_STRATEGY::ecb_decrypt(sub_key, read_ptr, decrypt_result);

            //再异或
            CRYPT_XOR_BLOCK(write_ptr, decrypt_result, xor_ptr);

            remain_len -= ENCRYPT_STRATEGY::BLOCK_SIZE;
            xor_ptr = read_ptr;
            write_ptr += ENCRYPT_STRATEGY::BLOCK_SIZE;
            read_ptr += ENCRYPT_STRATEGY::BLOCK_SIZE;
        }

        //看最后pid了几个字节,而且这些pid的字节应该都是这个值。
        size_t pid_len = src_buf[srcbuf_need_len - 1];
        if (pid_len > ENCRYPT_STRATEGY::BLOCK_SIZE)
        {
            return -1;
        }
        for (size_t i = 1; i <= pid_len; ++i)
        {
            if (pid_len != src_buf[srcbuf_need_len - i])
            {
                ZCE_LOG(RS_ERROR, "Fun[%s] pid data fill error.", __ZCE_FUNC__);
                return -1;
            }
        }

        //检查0校验
        for (size_t j = 1; j <= sizeof(uint32_t); ++j)
        {
            if ('\0' != src_buf[srcbuf_need_len - pid_len - j])
            {
                ZCE_LOG(RS_ERROR, "Fun[%s] zero data verify error.", __ZCE_FUNC__);
                return -1;
            }
        }

        //原文实际长度
        *src_len = srcbuf_need_len - pid_len - sizeof(uint32_t);
        return 0;
    }

#undef CRYPT_XOR_BLOCK

public:
    //
    const static size_t CRYPT_BLOCK_SIZE = ENCRYPT_STRATEGY::BLOCK_SIZE;
};

//=================================================================================================================

/*!
@brief      子key和sub key相同的是的key_setup函数的基类
*/
class SubKey_Is_Key_ECB
{
public:

    struct SUBKEY_IS_KEY
    {
        const unsigned char* skey_;
    };

    //为了通配ZCE_Crypt 实现的转换函数
    static void key_setup(const unsigned char* key,
                          SUBKEY_IS_KEY* sub_key,
                          bool  /*if_encrypt*/)
    {
        sub_key->skey_ = key;
    }
};

/*!
@brief      如果Key是被转换成整数使用，使用这个作为基类

@tparam     key_size  key的长度，必须是4的倍数
*/
template <size_t key_size>
class SubKey_Is_Uint32Ary_ECB
{
public:

    struct SUBKEY_IS_Uint32Ary
    {
        uint32_t skey_[key_size / sizeof(uint32_t)];
    };

    //为了通配ZCE_Crypt 实现的转换函数
    static void key_setup(const unsigned char* key,
                          SUBKEY_IS_Uint32Ary* sub_key,
                          bool  /*if_encrypt*/)
    {
        //为什么要用ZINDEX_TO_LEUINT32 这个宏，保证各个平台使用的字节序都一致
        for (size_t i = 0; i < key_size / sizeof(uint32_t); ++i)
        {
            sub_key->skey_[i] = ZINDEX_TO_LEUINT32(key, i);
        }
    }
};

/*!
@brief      对16个字节，4个UIN32的KEY做特化
*/
template<>
class SubKey_Is_Uint32Ary_ECB<16>
{
public:

    struct SUBKEY_IS_Uint32Ary
    {
        uint32_t skey_[4];
    };

    //为了通配ZCE_Crypt 实现的转换函数
    static void key_setup(const unsigned char* key,
                          SUBKEY_IS_Uint32Ary* sub_key,
                          bool  /*if_encrypt*/)
    {
        sub_key->skey_[0] = ZINDEX_TO_LEUINT32(key, 0);
        sub_key->skey_[1] = ZINDEX_TO_LEUINT32(key, 1);
        sub_key->skey_[2] = ZINDEX_TO_LEUINT32(key, 2);
        sub_key->skey_[3] = ZINDEX_TO_LEUINT32(key, 3);
    };
};

/*!
@brief      对32个字节，8个UIN32的KEY做特化
*/
template<>
class SubKey_Is_Uint32Ary_ECB<32>
{
public:

    struct SUBKEY_IS_Uint32Ary
    {
        uint32_t skey_[8];
    };

    //为了通配ZCE_Crypt 实现的转换函数
    static void key_setup(const unsigned char* key,
                          SUBKEY_IS_Uint32Ary* sub_key,
                          bool  /*if_encrypt*/)
    {
        sub_key->skey_[0] = ZINDEX_TO_LEUINT32(key, 0);
        sub_key->skey_[1] = ZINDEX_TO_LEUINT32(key, 1);
        sub_key->skey_[2] = ZINDEX_TO_LEUINT32(key, 2);
        sub_key->skey_[3] = ZINDEX_TO_LEUINT32(key, 3);
        sub_key->skey_[4] = ZINDEX_TO_LEUINT32(key, 4);
        sub_key->skey_[5] = ZINDEX_TO_LEUINT32(key, 5);
        sub_key->skey_[6] = ZINDEX_TO_LEUINT32(key, 6);
        sub_key->skey_[7] = ZINDEX_TO_LEUINT32(key, 7);
    };
};

//=================================================================================================================
/*!
@brief      使用异或进行加密的策略类，主要是为了测试CBC函数的，
            很简单（当然速度也快,吼吼，有时候也会想，有些时候这样就足够了），
            更多情况下，这是一个例子，教你如何写某个加密算法的ECB加密策略，
            好在CBC中使用
*/
class XOR_ECB : public SubKey_Is_Key_ECB
{
public:

    //XOR算法直接使用用户的KEY，
    typedef  SUBKEY_IS_KEY SUBKEY_STRUCT;

    ///异或加密函数
    static void ecb_encrypt(const SUBKEY_STRUCT* skey,
                            const unsigned char* src_block,
                            unsigned char* cipher_block);
    ///异或解密函数
    static void ecb_decrypt(const SUBKEY_STRUCT* skey,
                            const unsigned char* cipher_block,
                            unsigned char* src_block);

public:
    //异或算法一次处理一个BLOCK的长度
    static const size_t BLOCK_SIZE = 16;
    //异或算法KEY的长度
    static const size_t KEY_SIZE = 16;
};

//简单的异或加密，主要用于测试ZCE_Crypt，
//typedef命名原则是，加密算法名称，算法处理的BLOCK长度，key长度，轮数(推荐的轮数往往和key长度有一定关系)
typedef ZCE_Crypt<XOR_ECB>   XOR_Crypt_128_128_1;

//=================================================================================================================
//DES 和 TDES(DES3)的设计实现

class DES_Base
{
public:

public:
    //
    static void des_setkey(const unsigned char key[8],
                           uint32_t sk[32]);
public:
    ///Expanded DES S-boxes
    static const uint32_t SB[8][64];
    ///left halves bit - swap
    static const uint32_t DES_LHS[16];
    ///right halves bit - swap
    static const uint32_t DES_RHS[16];
};

/*!
* @brief      DES（Data Encryption Standard）加密算法，是一种使用密钥加密的块密码，
*             1976年被美国联邦政府的国家标准局确定为联邦资料处理标准（FIPS），随后在国
*             际上广泛流传开来。它基于使用56位密钥的对称算法。这个算法因为包含一些机密设
*             计元素，相对短的密钥长度以及怀疑内含美国国家安全局（NSA）的后门而在开始时
*             有争议，DES因此受到了强烈的学院派式的审查，并以此推动了现代的块密码及其密码
*             分析的发展。
*             在2001年，DES作为一个标准已经被高级加密标准（AES）所取代。
*             http://zh.wikipedia.org/wiki/DES                   -- 来自维基
* @note
*/
class DES_ECB : public DES_Base
{
public:
    //秘钥长度
    static const size_t KEY_SIZE = 8;
    //SUB Key长度
    static const size_t SUB_KEY_SIZE = 32;
    //DES算法一次处理一个BLOCK的长度
    static const size_t BLOCK_SIZE = 8;

    struct DES_SUBKEY
    {
        uint32_t sub_key_[SUB_KEY_SIZE];
    };

    //DES的subkey定义，方便萃取
    typedef  DES_SUBKEY  SUBKEY_STRUCT;

    ///生成DES算法的sub key，其加密解密的sub key不同
    static void key_setup(const unsigned char* key,
                          SUBKEY_STRUCT* subkey,
                          bool  if_encrypt);

    ///DES块加密函数
    inline static void ecb_encrypt(const SUBKEY_STRUCT* skey,
                                   const unsigned char* src_block,
                                   unsigned char* cipher_block)
    {
        return des_crypt_ecb(skey, src_block, cipher_block);
    }
    ///DES块解密函数
    inline static void ecb_decrypt(const SUBKEY_STRUCT* skey,
                                   const unsigned char* cipher_block,
                                   unsigned char* src_block)
    {
        return des_crypt_ecb(skey, cipher_block, src_block);
    }

protected:
    //DES 的加密解密是一个函数，（但SUB Key 不同）
    static void  des_crypt_ecb(const SUBKEY_STRUCT* sk,
                               const unsigned char input[BLOCK_SIZE],
                               unsigned char output[BLOCK_SIZE]);
};

//DES 秘钥8个字节，加密的block 8个字节，每次处理16轮，
typedef ZCE_Crypt<DES_ECB > DES_Crypt_64_64_16;

/*!
* @brief      DES3（或称为Triple DES 或者 3DES）是三重数据加密算法（TDEA，Triple
*             Data Encryption Algorithm）块密码的通称。它相当于是对每个数据块应用
*             三次DES加密算法。由于计算机运算能力的增强，原版DES密码的密钥长度变得容易
*             被暴力破解；3DES即是设计用来提供一种相对简单的方法，即通过增加DES的密钥
*             长度来避免类似的攻击，而不是设计一种全新的块密码算法。
* @note       密文 = EK3(DK2(EK1(平文)))
*             平文 = DK1(EK2(DK3(密文)))
*/
class DES3_ECB : public DES_Base
{
public:
    //秘钥长度
    static const size_t KEY_SIZE = 24;
    //3倍的DES sub key長度
    static const size_t SUB_KEY_SIZE = 96;
    //DES算法一次处理一个BLOCK的长度
    static const size_t BLOCK_SIZE = 8;

    struct DES3_SUBKEY
    {
        uint32_t sub_key_[SUB_KEY_SIZE];
    };

    //DES的subkey定义，方便萃取
    typedef  DES3_SUBKEY  SUBKEY_STRUCT;

    ///生成DES算法的sub key，其加密解密的sub key不同
    static void key_setup(const unsigned char* key,
                          SUBKEY_STRUCT* subkey,
                          bool  if_encrypt);

    ///DES3块加密函数
    inline static void ecb_encrypt(const SUBKEY_STRUCT* skey,
                                   const unsigned char* src_block,
                                   unsigned char* cipher_block)
    {
        return des3_crypt_ecb(skey, src_block, cipher_block);
    }
    ///DES3块解密函数
    inline static void ecb_decrypt(const SUBKEY_STRUCT* skey,
                                   const unsigned char* cipher_block,
                                   unsigned char* src_block)
    {
        return des3_crypt_ecb(skey, cipher_block, src_block);
    }

protected:

    //计算DES3算法的sub key
    static void des3_set3key(const unsigned char key[KEY_SIZE],
                             uint32_t esk[SUB_KEY_SIZE],
                             uint32_t dsk[SUB_KEY_SIZE]);

    //DES 的加密解密是一个函数，（但SUB Key 不同）
    static void  des3_crypt_ecb(const SUBKEY_STRUCT* subkey,
                                const unsigned char input[BLOCK_SIZE],
                                unsigned char output[BLOCK_SIZE]);
};

//DES 秘钥8个字节，加密的block 8个字节，每次处理16轮，
typedef ZCE_Crypt<DES3_ECB > DES3_Crypt_64_192_16;

//=================================================================================================================

/*!
* @brief      TEA的加密算法，代码参考来自维基，
*             这个算法估计是因为Tecent用的多，所以这个算法知名度较大，
*             该算法使用了一个神秘常数δ作为倍数，它来源于黄金比率，以保证每一轮加密都不相同。
*             但δ的精确值似乎并不重要，这里 TEA 把它定义为 δ=「(√5 - 1)231」
*             （也就是程序中的 0×9E3779B9）
* @tparam     round_size  加密的轮数，推荐轮数是32或者64，TX的值用了16轮，所以加密性降低，当然性能好一些，
*                         其他数值的轮数应该也可以。
* @note       算法中使用了ZINDEX_TO_LEUINT32，这些宏，其实是相当于我认为算法的编码是小头党
*/
template <size_t round_size>
class TEA_ECB : public SubKey_Is_Uint32Ary_ECB<16>
{
public:

    //使用数组作为SUB KEY，
    typedef   SUBKEY_IS_Uint32Ary SUBKEY_STRUCT;

    //加密函数
    static void ecb_encrypt(const SUBKEY_STRUCT* sub_key,
                            const unsigned char* src_block,
                            unsigned char* cipher_block)
    {
        //为什么要用ZINDEX_TO_LEUINT32,因为要保证多平台下计算一致
        uint32_t v0 = ZINDEX_TO_LEUINT32(src_block, 0);
        uint32_t v1 = ZINDEX_TO_LEUINT32(src_block, 1);
        uint32_t sum = 0;

        const uint32_t* k = sub_key->skey_;

        /* basic cycle start */
        for (size_t i = 0; i < round_size; i++)
        {
            sum += DELTA;
            v0 += ((v1 << 4) + k[0]) ^ (v1 + sum) ^ ((v1 >> 5) + k[1]);
            v1 += ((v0 << 4) + k[2]) ^ (v0 + sum) ^ ((v0 >> 5) + k[3]);
        }
        ZLEUINT32_TO_INDEX(cipher_block, 0, v0);
        ZLEUINT32_TO_INDEX(cipher_block, 1, v1);
    }
    //解密函数
    static void ecb_decrypt(const SUBKEY_STRUCT* sub_key,
                            const unsigned char* cipher_block,
                            unsigned char* src_block)
    {
        uint32_t v0 = ZINDEX_TO_LEUINT32(cipher_block, 0);
        uint32_t v1 = ZINDEX_TO_LEUINT32(cipher_block, 1);
        uint32_t sum = DELTA;
        sum *= round_size;

        const uint32_t* k = sub_key->skey_;

        //轮加密循环
        for (size_t i = 0; i < round_size; i++)
        {
            /* basic cycle start */
            v1 -= ((v0 << 4) + k[2]) ^ (v0 + sum) ^ ((v0 >> 5) + k[3]);
            v0 -= ((v1 << 4) + k[0]) ^ (v1 + sum) ^ ((v1 >> 5) + k[1]);
            sum -= DELTA;
        }                                              /* end cycle */
        ZLEUINT32_TO_INDEX(src_block, 0, v0);
        ZLEUINT32_TO_INDEX(src_block, 1, v1);
    }

protected:
    //神秘常数δ
    static const uint32_t DELTA = 0x9e3779b9;

public:
    //TEA算法一次处理一个BLOCK的长度
    const static size_t BLOCK_SIZE = 8;
    //TEA算法KEY的长度
    const static size_t KEY_SIZE = 16;
};

//TEA算法，TX用的是16轮，算法作者推荐是32轮，甚至64轮，
//typedef命名原则是，加密算法名称，，算法处理的BLOCK长度key长度，轮数(推荐的轮数往往和key长度有一定关系)，
typedef ZCE_Crypt<TEA_ECB<16 > > TEA_Crypt_64_128_16;
typedef ZCE_Crypt<TEA_ECB<32 > > TEA_Crypt_64_128_32;
typedef ZCE_Crypt<TEA_ECB<64 > > TEA_Crypt_64_128_64;

//=================================================================================================================

/*!
@brief      XTEA 跟 TEA 使用了相同的简单运算，但它采用了截然不同的顺序，
            为了阻止密钥表攻击，四个子密钥（在加密过程中，原 128 位的密
            钥被拆分为 4 个 32 位的子密钥）采用了一种不太正规的方式进行
            混合，但速度更慢了。
            http://en.wikipedia.org/wiki/XTEA
@tparam     round_size 加密的轮数
*/
template <size_t round_size>
class XTEA_ECB : public SubKey_Is_Uint32Ary_ECB<16>
{
public:

    //使用数组作为SUB KEY，
    typedef   SUBKEY_IS_Uint32Ary SUBKEY_STRUCT;

    //加密函数
    static void ecb_encrypt(const SUBKEY_STRUCT* sub_key,
                            const unsigned char* src_block,
                            unsigned char* cipher_block)
    {
        //为什么要用ZINDEX_TO_LEUINT32,因为要保证多平台下计算一致
        uint32_t v0 = ZINDEX_TO_LEUINT32(src_block, 0);
        uint32_t v1 = ZINDEX_TO_LEUINT32(src_block, 1);
        uint32_t sum = 0;

        const uint32_t* k = sub_key->skey_;

        //轮循环
        for (size_t i = 0; i < round_size; i++)
        {
            v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + k[sum & 3]);
            sum += DELTA;
            v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + k[(sum >> 11) & 3]);
        }
        ZLEUINT32_TO_INDEX(cipher_block, 0, v0);
        ZLEUINT32_TO_INDEX(cipher_block, 1, v1);
    }
    //解密函数
    static void ecb_decrypt(const SUBKEY_STRUCT* sub_key,
                            const unsigned char* cipher_block,
                            unsigned char* src_block)
    {
        uint32_t v0 = ZINDEX_TO_LEUINT32(cipher_block, 0);
        uint32_t v1 = ZINDEX_TO_LEUINT32(cipher_block, 1);
        uint32_t sum = DELTA;
        sum *= round_size;

        const uint32_t* k = sub_key->skey_;

        for (size_t i = 0; i < round_size; i++)
        {
            v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + k[(sum >> 11) & 3]);
            sum -= DELTA;
            v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + k[sum & 3]);
        }
        ZLEUINT32_TO_INDEX(src_block, 0, v0);
        ZLEUINT32_TO_INDEX(src_block, 1, v1);
    }

protected:
    //神秘常数δ
    static const uint32_t DELTA = 0x9e3779b9;

public:
    //XTEA算法一次处理一个BLOCK的长度
    const static size_t BLOCK_SIZE = 8;
    //XTEA算法KEY的长度
    const static size_t KEY_SIZE = 16;
};

//XTEA算法，
//typedef命名原则是，加密算法名称，算法处理的BLOCK长度，key长度，轮数(推荐的轮数往往和key长度有一定关系)
typedef ZCE_Crypt<XTEA_ECB<16 > > XTEA_Crypt_64_128_16;
typedef ZCE_Crypt<XTEA_ECB<32 > > XTEA_Crypt_64_128_32;
typedef ZCE_Crypt<XTEA_ECB<64 > > XTEA_Crypt_64_128_64;

//=================================================================================================================

/*!
@brief      也被称为Corrected Block TEA,代码是参考的维基，XXTEA和TEA，XTEA
            的一个重大的不同是的其加密
            http://en.wikipedia.org/wiki/XXTEA
            使用维基的代码还发生一次优化导致代码失效问题，见下。
@tparam     block_size
@tparam     round_size  round_size = 6 + 52/(block_size/4)

*/
template <size_t block_size, size_t round_size>
class XXTEA_ECB : public SubKey_Is_Uint32Ary_ECB<16>
{
public:

    //使用数组作为SUB KEY，
    typedef   SUBKEY_IS_Uint32Ary SUBKEY_STRUCT;

#define XXTEA_MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (k[(p&3)^e] ^ z)))

    //加密函数
    static void ecb_encrypt(const SUBKEY_STRUCT* sub_key,
                            const unsigned char* src_block,
                            unsigned char* cipher_block)
    {
        const uint32_t* k = sub_key->skey_;

        uint32_t  sum = 0, y, z, e, p;

        size_t rounds = round_size;
        size_t num_uint32 = block_size / sizeof(uint32_t);

        uint32_t v[block_size / sizeof(uint32_t)];
        for (size_t i = 0; i < num_uint32; ++i)
        {
            v[i] = ZINDEX_TO_LEUINT32(src_block, i);
        }

        z = v[num_uint32 - 1];
        do
        {
            sum += DELTA;
            e = (sum >> 2) & 3;
            for (p = 0; p < num_uint32 - 1; ++p)
            {
                y = v[p + 1];
                z = v[p] += XXTEA_MX;
            }
            y = v[0];
            z = v[num_uint32 - 1] += XXTEA_MX;
        } while (--rounds);

        for (size_t i = 0; i < num_uint32; ++i)
        {
            ZLEUINT32_TO_INDEX(cipher_block, i, v[i]);
        }
    }

    //解密函数
    static void ecb_decrypt(const SUBKEY_STRUCT* sub_key,
                            const unsigned char* cipher_block,
                            unsigned char* src_block)
    {
        uint32_t sum = DELTA;
        sum *= round_size;

        const uint32_t* k = sub_key->skey_;

        uint32_t v[block_size / sizeof(uint32_t)];
        uint32_t num_uint32 = block_size / sizeof(uint32_t);
        size_t rounds = round_size;

        for (size_t i = 0; i < num_uint32; ++i)
        {
            v[i] = ZINDEX_TO_LEUINT32(cipher_block, i);
        }
        uint32_t y, z, e, p;
        y = v[0];
        do
        {
            e = (sum >> 2) & 3;
            for (p = num_uint32 - 1; p > 0; p--)
            {
                z = v[p - 1];
                y = v[p] -= XXTEA_MX;
            }
            z = v[num_uint32 - 1];
            y = v[0] -= XXTEA_MX;
            sum -= DELTA;
        } while (--rounds);

        //这儿发生了一个中彩票概率事情，优化出现问题，
        //本来这段代码来自维基，上面的while循环原来的代码是如下形式
        //while ((sum -= DELTA) != 0);
        //但在VS2010的Realse 版本的优化选项（O1，O2）打开后，上面的
        //while循环居然不起左右了，找一些兄弟用看过汇编，也是如此。。。shit happen！

        for (size_t i = 0; i < num_uint32; ++i)
        {
            ZLEUINT32_TO_INDEX(src_block, i, v[i]);
        }
    }

#undef XXTEA_MX

protected:
    //神秘常数δ
    static const uint32_t DELTA = 0x9e3779b9;

public:
    //XXTEA算法一次处理一个BLOCK的长度
    const static size_t BLOCK_SIZE = block_size;
    //XXTEA算法KEY的长度
    const static size_t KEY_SIZE = 16;
};

typedef ZCE_Crypt<XXTEA_ECB<8, 32 > >  XXTEA_Crypt_64_128_32;
typedef ZCE_Crypt<XXTEA_ECB<16, 16 > > XXTEA_Crypt_128_128_16;

//=================================================================================================================

/*!
@brief      GOST是当年苏联KKB搞出来的加密算法，其实相对DES，
            这个东东更加脆弱，但由于GOST的密钥要求256位，加密轮数也多
            所以安全性好于DES,这个实现的轮数只能是32轮，（轮数无法改变）
*/
class GOST_ECB : public SubKey_Is_Uint32Ary_ECB<32>
{
public:

    //算法直接使用用户的KEY，
    typedef   SUBKEY_IS_Uint32Ary SUBKEY_STRUCT;

    ///GOST加密函数
    static void ecb_encrypt(const SUBKEY_STRUCT* sub_key,
                            const unsigned char* src_block,
                            unsigned char* cipher_block);
    ///GOST解密函数
    static void ecb_decrypt(const SUBKEY_STRUCT* sub_key,
                            const unsigned char* cipher_block,
                            unsigned char* src_block);

    //GOST的密钥长度是32字节256bits，GOST算法本质比DES简单，但之所以安全性要好一些，主要就是因为key长
    const static size_t KEY_SIZE = 32;
    //GOST每次处理的BLOCK大小
    const static size_t BLOCK_SIZE = 8;

    //
    static const uint32_t GOST_WZ_SPKEY[32];
    //
    static const unsigned char GOST_WZ_SP[8][16];
};

//默认GOST加密8字节的BLOCK，32字节（256bits）的key，32轮,
//typedef命名原则是，加密算法名称，算法处理的BLOCK长度，key长度，轮数(推荐的轮数往往和key长度有一定关系)
typedef ZCE_Crypt < GOST_ECB > GOST_Crypt_64_256_32;

//=================================================================================================================

/*!
@brief      RC算法的S盒定义，S盒子的长度和轮数有关系，我这儿支持的最大轮数
            是32轮
*/
class RC_SBOX_Define
{
protected:
    //最大的加密轮数
    const static size_t MAX_ROUND_SIZE = 32;
    //存放一些我也不懂的魔术数字,最多可以使用32轮，所以RC算法的轮数都不要大于32,RC_STAB其实也可以生成，但为了速度，算了
    //68 = 32 * 2 + 2 + 2; 其实RC5最多用66，RC6最多才用68
    static const uint32_t RC_SBOX[(MAX_ROUND_SIZE + 2) * 2];
};

/*!
@brief      RC加密算法的基类，用于扩展RC5，RC6算法，RC比较DES的优势是快，非常快，
            如果你能保存SKEY（KEY和S盒计算得到的东东），这家伙的速度应该可以飞
            RC算法其实也可以可以变换key的长度，以及BLOCK长度，以及加密的轮数的东东，
            我对偷懒KEY长度做了固定 这个类主要帮助RC的skey的生成
@tparam     round_size  加密的轮数，为4，8，……12……20……32,最大值32
@tparam     skey_size   SBOX的大小，RC5，RC6算法有点不一样，RC5算法为轮数+2
                        RC6算法为轮数+4
*/
template <size_t key_size, size_t round_size, size_t sbox_size >
class RC_ECB_Base : public RC_SBOX_Define
{
public:

    //RC6的key长度可以是8-128字节(64-1024bit)，我这儿选择16字节(128bit)
    static const size_t KEY_SIZE = key_size;

    struct RC_SUBKEY
    {
        uint32_t skey_[sbox_size];
    };

    //加密的轮数
    static const size_t ROUND_SIZE = round_size;
    //S BOX的大小
    static const size_t SBOX_SIZE = sbox_size;

    //
    static void key_setup(const unsigned char* key,
                          RC_SUBKEY* subkey,
                          bool  /*if_encrypt*/)
    {
        uint32_t a = 0, b = 0;

        // copy the key into the L array
        uint32_t lkey[KEY_SIZE / sizeof(uint32_t)];
        for (size_t i = 0; i < key_size / sizeof(uint32_t); i++)
        {
            lkey[i] = ZINDEX_TO_LEUINT32(key, i);
        }

        /* key_setup the S array */
        memcpy(subkey->skey_, RC_SBOX, SBOX_SIZE * sizeof(uint32_t));

        for (size_t i = 0, j = 0; i < sbox_size; ++i)
        {
            a = subkey->skey_[i] = ZCE_ROTL32(subkey->skey_[i] + a + b, 3);
            b = lkey[j] = ZCE_ROTL32(lkey[j] + a + b, (a + b) & 31);

            if (++j == KEY_SIZE / sizeof(uint32_t))
            {
                j = 0;
            }
        }
    }
};

/*!
@brief      RC5的加密算法，直接从代码分析，比RC6简单不少
            http://en.wikipedia.org/wiki/RC5
@tparam     round_size 加密的轮数，
@tparam     sbox_size  SBOX的大小
*/
template <size_t key_size, size_t round_size, size_t sbox_size >
class RC5_ECB : public RC_ECB_Base< key_size, round_size, sbox_size >
{
public:

    //为了方便ZCE_Crypt,定义的子KEY类型，
    typedef typename RC_ECB_Base<key_size, round_size, sbox_size>::RC_SUBKEY  SUBKEY_STRUCT;

    //加密函数
    static void ecb_encrypt(const SUBKEY_STRUCT* sub_key,
                            const unsigned char* src_block,
                            unsigned char* cipher_block)
    {
        uint32_t  a = ZINDEX_TO_LEUINT32(src_block, 0) + sub_key->skey_[0];
        uint32_t  b = ZINDEX_TO_LEUINT32(src_block, 1) + sub_key->skey_[1];
        for (size_t i = 1; i <= round_size; i++)
        {
            a = ZCE_ROTL32(a ^ b, b & 31) + sub_key->skey_[2 * i];
            b = ZCE_ROTL32(b ^ a, a & 31) + sub_key->skey_[2 * i + 1];
        }
        ZLEUINT32_TO_INDEX(cipher_block, 0, a);
        ZLEUINT32_TO_INDEX(cipher_block, 1, b);
    }
    //解密函数
    static void ecb_decrypt(const SUBKEY_STRUCT* subkey,
                            const unsigned char* cipher_block,
                            unsigned char* src_block)
    {
        uint32_t  a = ZINDEX_TO_LEUINT32(cipher_block, 0);
        uint32_t  b = ZINDEX_TO_LEUINT32(cipher_block, 1);

        for (size_t i = round_size; i > 0; i--)
        {
            b = ZCE_ROTR32(b - subkey->skey_[2 * i + 1], a) ^ a;
            a = ZCE_ROTR32(a - subkey->skey_[2 * i], b) ^ b;
        }
        ZLEUINT32_TO_INDEX(src_block, 0, a - subkey->skey_[0]);
        ZLEUINT32_TO_INDEX(src_block, 1, b - subkey->skey_[1]);
    }

public:
    //每次处理的BLOCK长度是8个字节
    const static size_t BLOCK_SIZE = 8;
};

//RC5推荐的加密算法的轮数是12轮，RC5_Crypt_16_12_8，但维基后面也写了一句
//12-round RC5 (with 64-bit blocks) is susceptible to a differential attack using 2^44 chosen plaintexts
//typedef命名原则是，加密算法名称，算法处理的BLOCK长度，key长度，轮数(推荐的轮数往往和key长度有一定关系)
typedef ZCE_Crypt < RC5_ECB < 16, 12, 12 * 2 + 2 > > RC5_Crypt_64_128_12;
typedef ZCE_Crypt < RC5_ECB < 16, 20, 20 * 2 + 2 > > RC5_Crypt_64_128_20;

/*!
@brief      RC6曾经是AES的候选方案之一。
            http://en.wikipedia.org/wiki/RC6
            RC6 一般情况下，推荐128bit密钥，128bitBLOCK，轮数20.
*/
template <size_t key_size, size_t round_size, size_t sbox_size >
class RC6_ECB : public RC_ECB_Base<key_size, round_size, sbox_size >
{
public:

#define EN_RC6_RND(i,a,b,c,d)                             \
    t = ZCE_ROTL32(b * (b + b + 1), 5);                   \
    u = ZCE_ROTL32(d * (d + d + 1), 5);                   \
    a = ZCE_ROTL32(a ^ t, u ) + skey_ptr[i];              \
    c = ZCE_ROTL32(c ^ u, t ) + skey_ptr[i + 1]

#define DE_RC6_RND(i,a,b,c,d)                             \
    u = ZCE_ROTL32(d * (d + d + 1), 5);                   \
    t = ZCE_ROTL32(b * (b + b + 1), 5);                   \
    c = ZCE_ROTR32(c - skey_ptr[i + 1], t ) ^ u;          \
    a = ZCE_ROTR32(a - skey_ptr[i], u ) ^ t

    //为了方便ZCE_Crypt的typedef定义
    typedef typename RC_ECB_Base<key_size, round_size, sbox_size>::RC_SUBKEY  SUBKEY_STRUCT;

    //加密函数
    static void ecb_encrypt(const SUBKEY_STRUCT* sub_key,
                            const unsigned char* src_block,
                            unsigned char* cipher_block)
    {
        uint32_t  t = 0, u = 0;
        const uint32_t* skey_ptr = sub_key->skey_;
        uint32_t a = ZINDEX_TO_LEUINT32(src_block, 0);
        uint32_t b = ZINDEX_TO_LEUINT32(src_block, 1) + skey_ptr[0];
        uint32_t c = ZINDEX_TO_LEUINT32(src_block, 2);
        uint32_t d = ZINDEX_TO_LEUINT32(src_block, 3) + skey_ptr[1];

        for (size_t i = 0; i < round_size / sizeof(uint32_t); ++i)
        {
            EN_RC6_RND(i * 8 + 2, a, b, c, d);
            EN_RC6_RND(i * 8 + 4, b, c, d, a);
            EN_RC6_RND(i * 8 + 6, c, d, a, b);
            EN_RC6_RND(i * 8 + 8, d, a, b, c);
        }

        ZLEUINT32_TO_INDEX(cipher_block, 0, a + skey_ptr[sbox_size - 2]);
        ZLEUINT32_TO_INDEX(cipher_block, 1, b);
        ZLEUINT32_TO_INDEX(cipher_block, 2, c + skey_ptr[sbox_size - 1]);
        ZLEUINT32_TO_INDEX(cipher_block, 3, d);
    }
    //解密函数
    static void ecb_decrypt(const SUBKEY_STRUCT* sub_key,
                            const unsigned char* cipher_block,
                            unsigned char* src_block)
    {
        uint32_t t = 0, u = 0;
        const uint32_t* skey_ptr = sub_key->skey_;
        uint32_t a = ZINDEX_TO_LEUINT32(cipher_block, 0) - skey_ptr[sbox_size - 2];
        uint32_t b = ZINDEX_TO_LEUINT32(cipher_block, 1);
        uint32_t c = ZINDEX_TO_LEUINT32(cipher_block, 2) - skey_ptr[sbox_size - 1];
        uint32_t d = ZINDEX_TO_LEUINT32(cipher_block, 3);

        for (size_t i = 0; i < round_size / sizeof(uint32_t); ++i)
        {
            DE_RC6_RND(sbox_size - 8 * i - 4, d, a, b, c);
            DE_RC6_RND(sbox_size - 8 * i - 6, c, d, a, b);
            DE_RC6_RND(sbox_size - 8 * i - 8, b, c, d, a);
            DE_RC6_RND(sbox_size - 8 * i - 10, a, b, c, d);
        }
        ZLEUINT32_TO_INDEX(src_block, 0, a);
        ZLEUINT32_TO_INDEX(src_block, 1, b - skey_ptr[0]);
        ZLEUINT32_TO_INDEX(src_block, 2, c);
        ZLEUINT32_TO_INDEX(src_block, 3, d - skey_ptr[1]);
    }
#undef EN_RC6_RND
#undef DE_RC6_RND

public:
    //每次处理的BLOCK长度是16个字节
    const static size_t BLOCK_SIZE = 16;
};

//模版参数是加密轮数，RC6推荐的加密算法的轮数是20，如果考虑加密强度，推荐使用RC6_Crypt_16_20_16
//typedef命名原则是，加密算法名称，算法处理的BLOCK长度，key长度，轮数(推荐的轮数往往和key长度有一定关系)
typedef ZCE_Crypt < RC6_ECB < 16, 12, 12 * 2 + 4 > > RC6_Crypt_128_128_12;
typedef ZCE_Crypt < RC6_ECB < 16, 20, 20 * 2 + 4 > > RC6_Crypt_128_128_20;

//=================================================================================================================

class CAST_SBOX_Define
{
protected:
    //SBOX，CAST5用8个，CAST6用了前面4个，
    static const uint32_t CAST_SBOX[8][256];

protected:
    //CAST6 算法的魔术数，用于KEY生成
    static const uint32_t CAST6_KEY_MASK[192];
    //CAST6 算法的魔术数，用于KEY生成
    static const uint8_t  CAST6_KEY_ROT[32];
};

//保存CAST6_KEY_ROTAST算法的SBOX，供CAST5，CAST6算法使用
template < size_t round_size>
class CAST_ECB_Base : public CAST_SBOX_Define
{
protected:

    struct CAST_SUBKEY
    {
        uint32_t mkey_[round_size];
        uint8_t  rkey_[round_size];
    };
};

#define CAST_F1(x, a, s, r)                                                   \
    t = ZCE_ROTL32(s + a, r);                                                 \
    x ^= ((CAST_SBOX_Define::CAST_SBOX[0][ZUINT32_3BYTE(t)] ^ CAST_SBOX_Define::CAST_SBOX[1][ZUINT32_2BYTE(t)]) - \
          CAST_SBOX_Define::CAST_SBOX[2][ZUINT32_1BYTE(t)]) + CAST_SBOX_Define::CAST_SBOX[3][ZUINT32_0BYTE(t)];
#define CAST_F2(x, a, s, r)                                                   \
    t = ZCE_ROTL32(s ^ a, r);                                                 \
    x ^= ((CAST_SBOX_Define::CAST_SBOX[0][ZUINT32_3BYTE(t)] - CAST_SBOX_Define::CAST_SBOX[1][ZUINT32_2BYTE(t)]) + \
          CAST_SBOX_Define::CAST_SBOX[2][ZUINT32_1BYTE(t)]) ^ CAST_SBOX_Define::CAST_SBOX[3][ZUINT32_0BYTE(t)];
#define CAST_F3(x, a, s, r)                                                   \
    t = ZCE_ROTL32(s - a, r);                                                 \
    x ^= ((CAST_SBOX_Define::CAST_SBOX[0][ZUINT32_3BYTE(t)] + CAST_SBOX_Define::CAST_SBOX[1][ZUINT32_2BYTE(t)]) ^ \
          CAST_SBOX_Define::CAST_SBOX[2][ZUINT32_1BYTE(t)]) - CAST_SBOX_Define::CAST_SBOX[3][ZUINT32_0BYTE(t)];

/*!
@brief      CAST5 ECB加密算法，又被称为CAST128,
            我这儿选择的密钥128bits，BLOCK64bit，默认轮数16轮
            CAST5的密钥长度可以更短，40-128bits，8bit递增，加密轮数可以是12，16
            （KEY长度和，轮数改成模版参数应该也不难）
@tparam     key_size   密钥长度，我这儿值接受8（64bits），12（96bits），16（128bits）
@tparam     round_size 加密的轮数，如果key_size > 80bits，最好就要16轮，我这儿值接受4,8,12,16

*/
template <size_t key_size, size_t round_size>
class CAST5_ECB : public CAST_ECB_Base < round_size >
{
public:

    //为了方便ZCE_Crypt,定义的子KEY类型，
    typedef typename CAST_ECB_Base<round_size>::CAST_SUBKEY  SUBKEY_STRUCT;

    //根据原始密钥，生成算法所需要的密钥
    static void key_setup(const unsigned char* key,
                          SUBKEY_STRUCT* sub_key,
                          bool  /*if_encrypt*/)
    {
        uint32_t x[4], z[4];
        uint32_t* mkey_ptr = sub_key->mkey_;
        uint8_t* rkey_ptr = sub_key->rkey_;
        x[0] = ZINDEX_TO_LEUINT32(key, 0);
        x[1] = ZINDEX_TO_LEUINT32(key, 1);
        x[2] = 0;
        x[3] = 0;
        //根据不同的KEY SIZE 进行处理
        if (12 == key_size || 16 == key_size)
        {
            x[2] = ZINDEX_TO_LEUINT32(key, 2);
            if (16 == key_size)
            {
                x[3] = ZINDEX_TO_LEUINT32(key, 3);
            }
        }
        size_t i = 0;

#define CAST5_GB(x, i) (((x[(15-i)>>2])>>(uint32_t)(8*((15-i)&3)))&255)

        //第一路计算skey，
        z[3] = x[3] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0xD)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0xF)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0xC)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0xE)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x8)];
        z[2] = x[1] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x0)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x2)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x1)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x3)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0xA)];
        z[1] = x[0] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x7)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x6)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x5)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x4)] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x9)];
        z[0] = x[2] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0xA)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x9)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0xb)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x8)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0xB)];
        mkey_ptr[i++] = CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x8)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x9)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x7)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x6)] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x2)];
        mkey_ptr[i++] = CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0xA)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0xB)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x5)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x4)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x6)];
        mkey_ptr[i++] = CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0xC)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0xd)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x3)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x2)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x9)];
        mkey_ptr[i++] = CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0xE)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0xF)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x1)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x0)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0xc)];

        if (round_size > 4)
        {
            x[3] = z[1] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x5)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x7)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x4)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x6)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x0)];
            x[2] = z[3] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x0)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x2)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x1)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x3)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x2)];
            x[1] = z[2] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x7)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x6)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x5)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x4)] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x1)];
            x[0] = z[0] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0xA)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x9)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0xb)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x8)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x3)];
            mkey_ptr[i++] = CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x3)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x2)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0xc)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0xd)] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x8)];
            mkey_ptr[i++] = CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x1)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x0)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0xe)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0xf)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0xd)];
            mkey_ptr[i++] = CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x7)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x6)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x8)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x9)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x3)];
            mkey_ptr[i++] = CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x5)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x4)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0xa)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0xb)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x7)];
        }
        if (round_size > 8)
        {
            z[3] = x[3] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0xD)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0xF)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0xC)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0xE)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x8)];
            z[2] = x[1] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x0)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x2)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x1)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x3)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0xA)];
            z[1] = x[0] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x7)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x6)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x5)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x4)] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x9)];
            z[0] = x[2] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0xA)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x9)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0xb)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x8)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0xB)];
            mkey_ptr[i++] = CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x3)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x2)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0xc)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0xd)] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x9)];
            mkey_ptr[i++] = CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x1)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x0)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0xe)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0xf)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0xc)];
            mkey_ptr[i++] = CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x7)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x6)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x8)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x9)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x2)];
            mkey_ptr[i++] = CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x5)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x4)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0xa)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0xb)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x6)];
        }
        if (round_size > 12)
        {
            x[3] = z[1] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x5)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x7)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x4)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x6)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x0)];
            x[2] = z[3] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x0)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x2)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x1)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x3)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x2)];
            x[1] = z[2] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x7)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x6)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x5)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x4)] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x1)];
            x[0] = z[0] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0xA)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x9)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0xb)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x8)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x3)];
            mkey_ptr[i++] = CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x8)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x9)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x7)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x6)] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x3)];
            mkey_ptr[i++] = CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0xa)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0xb)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x5)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x4)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x7)];
            mkey_ptr[i++] = CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0xc)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0xd)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x3)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x2)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x8)];
            mkey_ptr[i++] = CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0xe)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0xf)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x1)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x0)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0xd)];
        }

        //第二轮计算rkey，旋转的长度
        i = 0;
        z[3] = x[3] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0xD)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0xF)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0xC)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0xE)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x8)];
        z[2] = x[1] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x0)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x2)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x1)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x3)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0xA)];
        z[1] = x[0] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x7)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x6)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x5)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x4)] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x9)];
        z[0] = x[2] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0xA)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x9)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0xb)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x8)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0xB)];
        rkey_ptr[i++] = (CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x8)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x9)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x7)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x6)] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x2)]) & 0xFF;
        rkey_ptr[i++] = (CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0xA)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0xB)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x5)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x4)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x6)]) & 0xFF;
        rkey_ptr[i++] = (CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0xC)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0xd)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x3)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x2)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x9)]) & 0xFF;
        rkey_ptr[i++] = (CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0xE)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0xF)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x1)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x0)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0xc)]) & 0xFF;

        if (round_size > 4)
        {
            x[3] = z[1] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x5)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x7)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x4)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x6)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x0)];
            x[2] = z[3] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x0)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x2)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x1)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x3)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x2)];
            x[1] = z[2] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x7)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x6)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x5)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x4)] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x1)];
            x[0] = z[0] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0xA)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x9)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0xb)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x8)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x3)];
            rkey_ptr[i++] = (CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x3)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x2)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0xc)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0xd)] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x8)]) & 0xFF;
            rkey_ptr[i++] = (CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x1)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x0)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0xe)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0xf)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0xd)]) & 0xFF;
            rkey_ptr[i++] = (CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x7)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x6)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x8)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x9)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x3)]) & 0xFF;
            rkey_ptr[i++] = (CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x5)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x4)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0xa)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0xb)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x7)]) & 0xFF;
        }
        if (round_size > 8)
        {
            z[3] = x[3] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0xD)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0xF)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0xC)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0xE)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x8)];
            z[2] = x[1] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x0)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x2)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x1)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x3)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0xA)];
            z[1] = x[0] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x7)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x6)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x5)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x4)] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x9)];
            z[0] = x[2] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0xA)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x9)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0xb)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x8)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0xB)];
            rkey_ptr[i++] = (CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x3)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x2)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0xc)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0xd)] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x9)]) & 0xFF;
            rkey_ptr[i++] = (CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x1)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x0)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0xe)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0xf)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0xc)]) & 0xFF;
            rkey_ptr[i++] = (CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x7)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x6)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x8)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x9)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x2)]) & 0xFF;
            rkey_ptr[i++] = (CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x5)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x4)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0xa)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0xb)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x6)]) & 0xFF;
        }
        if (round_size > 12)
        {
            x[3] = z[1] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x5)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x7)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x4)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x6)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(z, 0x0)];
            x[2] = z[3] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x0)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x2)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x1)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x3)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(z, 0x2)];
            x[1] = z[2] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x7)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x6)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x5)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x4)] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(z, 0x1)];
            x[0] = z[0] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0xA)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x9)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0xb)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x8)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(z, 0x3)];
            rkey_ptr[i++] = (CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x8)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x9)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x7)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x6)] ^ CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0x3)]) & 0xFF;
            rkey_ptr[i++] = (CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0xa)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0xb)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x5)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x4)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0x7)]) & 0xFF;
            rkey_ptr[i++] = (CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0xc)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0xd)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x3)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x2)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x8)]) & 0xFF;
            rkey_ptr[i++] = (CAST_SBOX_Define::CAST_SBOX[4][CAST5_GB(x, 0xe)] ^ CAST_SBOX_Define::CAST_SBOX[5][CAST5_GB(x, 0xf)] ^ CAST_SBOX_Define::CAST_SBOX[6][CAST5_GB(x, 0x1)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0x0)] ^ CAST_SBOX_Define::CAST_SBOX[7][CAST5_GB(x, 0xd)]) & 0xFF;
        }
#undef CAST5_GB
    }

    //加密函数，CAST5，一般建议密钥>80bits,所以处理16轮
    static void ecb_encrypt(const SUBKEY_STRUCT* sub_key,
                            const unsigned char* src_block,
                            unsigned char* cipher_block)
    {
        const uint32_t* mkey_ptr = sub_key->mkey_;
        const uint8_t* rkey_ptr = sub_key->rkey_;
        uint32_t t = 0;
        uint32_t l = ZINDEX_TO_LEUINT32(src_block, 0);
        uint32_t r = ZINDEX_TO_LEUINT32(src_block, 1);

        CAST_F1(l, r, mkey_ptr[0], rkey_ptr[0]);
        CAST_F2(r, l, mkey_ptr[1], rkey_ptr[1]);
        CAST_F3(l, r, mkey_ptr[2], rkey_ptr[2]);
        CAST_F1(r, l, mkey_ptr[3], rkey_ptr[3]);

        if (round_size > 4)
        {
            CAST_F2(l, r, mkey_ptr[4], rkey_ptr[4]);
            CAST_F3(r, l, mkey_ptr[5], rkey_ptr[5]);
            CAST_F1(l, r, mkey_ptr[6], rkey_ptr[6]);
            CAST_F2(r, l, mkey_ptr[7], rkey_ptr[7]);
        }
        if (round_size > 8)
        {
            CAST_F3(l, r, mkey_ptr[8], rkey_ptr[8]);
            CAST_F1(r, l, mkey_ptr[9], rkey_ptr[9]);
            CAST_F2(l, r, mkey_ptr[10], rkey_ptr[10]);
            CAST_F3(r, l, mkey_ptr[11], rkey_ptr[11]);
        }
        //如果要进行16轮处理
        if (round_size > 12)
        {
            CAST_F1(l, r, mkey_ptr[12], rkey_ptr[12]);
            CAST_F2(r, l, mkey_ptr[13], rkey_ptr[13]);
            CAST_F3(l, r, mkey_ptr[14], rkey_ptr[14]);
            CAST_F1(r, l, mkey_ptr[15], rkey_ptr[15]);
        }

        //注意这个顺序和上面读取顺序相反
        ZLEUINT32_TO_INDEX(cipher_block, 0, r);
        ZLEUINT32_TO_INDEX(cipher_block, 1, l);
    }

    //解密函数，密钥>80bits,所以处理16轮
    static void ecb_decrypt(const SUBKEY_STRUCT* sub_key,
                            const unsigned char* cipher_block,
                            unsigned char* src_block)
    {
        const uint32_t* mkey_ptr = sub_key->mkey_;
        const uint8_t* rkey_ptr = sub_key->rkey_;
        uint32_t t = 0;
        uint32_t r = ZINDEX_TO_LEUINT32(cipher_block, 0);
        uint32_t l = ZINDEX_TO_LEUINT32(cipher_block, 1);

        //如果要进行16轮处理
        if (round_size > 12)
        {
            CAST_F1(r, l, mkey_ptr[15], rkey_ptr[15]);
            CAST_F3(l, r, mkey_ptr[14], rkey_ptr[14]);
            CAST_F2(r, l, mkey_ptr[13], rkey_ptr[13]);
            CAST_F1(l, r, mkey_ptr[12], rkey_ptr[12]);
        }
        if (round_size > 8)
        {
            CAST_F3(r, l, mkey_ptr[11], rkey_ptr[11]);
            CAST_F2(l, r, mkey_ptr[10], rkey_ptr[10]);
            CAST_F1(r, l, mkey_ptr[9], rkey_ptr[9]);
            CAST_F3(l, r, mkey_ptr[8], rkey_ptr[8]);
        }
        if (round_size > 4)
        {
            CAST_F2(r, l, mkey_ptr[7], rkey_ptr[7]);
            CAST_F1(l, r, mkey_ptr[6], rkey_ptr[6]);
            CAST_F3(r, l, mkey_ptr[5], rkey_ptr[5]);
            CAST_F2(l, r, mkey_ptr[4], rkey_ptr[4]);
        }
        CAST_F1(r, l, mkey_ptr[3], rkey_ptr[3]);
        CAST_F3(l, r, mkey_ptr[2], rkey_ptr[2]);
        CAST_F2(r, l, mkey_ptr[1], rkey_ptr[1]);
        CAST_F1(l, r, mkey_ptr[0], rkey_ptr[0]);

        //注意这个顺序和上面读取顺序相反
        ZLEUINT32_TO_INDEX(src_block, 0, l);
        ZLEUINT32_TO_INDEX(src_block, 1, r);
    }

public:
    //
    const static size_t BLOCK_SIZE = 8;
    //密钥长度，我这儿值接受8（64bits），12（96bits），16（128bits）
    const static size_t KEY_SIZE = key_size;
};

//第一个参数是密钥长度，第二个参数是加密轮数，如果考虑加密效果，推荐CAST5_Crypt_16_16_8
//typedef命名原则是，加密算法名称，算法处理的BLOCK长度，key长度，轮数(推荐的轮数往往和key长度有一定关系)
//注意密钥长度如果是8，
typedef ZCE_Crypt<CAST5_ECB<8, 12>  >  CAST5_Crypt_64_64_12;
typedef ZCE_Crypt<CAST5_ECB<12, 16> >  CAST5_Crypt_64_96_16;
typedef ZCE_Crypt<CAST5_ECB<16, 8>  >  CAST5_Crypt_64_128_8;
typedef ZCE_Crypt<CAST5_ECB<16, 12> >  CAST5_Crypt_64_128_12;
typedef ZCE_Crypt<CAST5_ECB<16, 16> >  CAST5_Crypt_64_128_16;

/*!
@brief      CAST6 又被称为CAST256，也是AES的备选方案，
            一般推荐，BLOCK64bit，轮数48轮,CAST6的轮数是固定的48，
            我也不太确认是否可以减少，从算法上看应该可以，但48轮里面存在6*4的正向，6*4的反向，
            有点说不清
            CAST6的密钥长度 128, 160, 192, 224 or 256 bit
@tparam     key_size   密钥长度，我这儿值接受16（128bits）,20,24,28,32
@tparam     round_size 加密的轮数，我这儿支持12，24.36，48轮
*/
template <size_t key_size, size_t round_size>
class CAST6_ECB : public CAST_ECB_Base < round_size >
{
public:

    //为了方便ZCE_Crypt,定义的子KEY类型，
    typedef typename CAST_ECB_Base<round_size>::CAST_SUBKEY  SUBKEY_STRUCT;

    //根据原始密钥，生成算法所需要的密钥
    static void key_setup(const unsigned char* key,
                          SUBKEY_STRUCT* sub_key,
                          bool  /*if_encrypt*/)
    {
        uint32_t x[8], t = 0;
        uint32_t* mkey_ptr = sub_key->mkey_;
        uint8_t* rkey_ptr = sub_key->rkey_;

        x[0] = ZINDEX_TO_LEUINT32(key, 0);
        x[1] = ZINDEX_TO_LEUINT32(key, 1);
        x[2] = ZINDEX_TO_LEUINT32(key, 2);
        x[3] = ZINDEX_TO_LEUINT32(key, 3);
        x[4] = x[5] = x[6] = x[7] = 0;

        for (size_t j = 4; j < key_size / sizeof(uint32_t); ++j)
        {
            x[j] = ZINDEX_TO_LEUINT32(key, j);
        }

        for (size_t j = 0; j < round_size; j += 4)
        {
            CAST_F1(x[6], x[7], CAST_SBOX_Define::CAST6_KEY_MASK[4 * j + 0], CAST_SBOX_Define::CAST6_KEY_ROT[(4 * j + 0) & 31]);
            CAST_F2(x[5], x[6], CAST_SBOX_Define::CAST6_KEY_MASK[4 * j + 1], CAST_SBOX_Define::CAST6_KEY_ROT[(4 * j + 1) & 31]);
            CAST_F3(x[4], x[5], CAST_SBOX_Define::CAST6_KEY_MASK[4 * j + 2], CAST_SBOX_Define::CAST6_KEY_ROT[(4 * j + 2) & 31]);
            CAST_F1(x[3], x[4], CAST_SBOX_Define::CAST6_KEY_MASK[4 * j + 3], CAST_SBOX_Define::CAST6_KEY_ROT[(4 * j + 3) & 31]);
            CAST_F2(x[2], x[3], CAST_SBOX_Define::CAST6_KEY_MASK[4 * j + 4], CAST_SBOX_Define::CAST6_KEY_ROT[(4 * j + 4) & 31]);
            CAST_F3(x[1], x[2], CAST_SBOX_Define::CAST6_KEY_MASK[4 * j + 5], CAST_SBOX_Define::CAST6_KEY_ROT[(4 * j + 5) & 31]);
            CAST_F1(x[0], x[1], CAST_SBOX_Define::CAST6_KEY_MASK[4 * j + 6], CAST_SBOX_Define::CAST6_KEY_ROT[(4 * j + 6) & 31]);
            CAST_F2(x[7], x[0], CAST_SBOX_Define::CAST6_KEY_MASK[4 * j + 7], CAST_SBOX_Define::CAST6_KEY_ROT[(4 * j + 7) & 31]);
            CAST_F1(x[6], x[7], CAST_SBOX_Define::CAST6_KEY_MASK[4 * j + 8], CAST_SBOX_Define::CAST6_KEY_ROT[(4 * j + 8) & 31]);
            CAST_F2(x[5], x[6], CAST_SBOX_Define::CAST6_KEY_MASK[4 * j + 9], CAST_SBOX_Define::CAST6_KEY_ROT[(4 * j + 9) & 31]);
            CAST_F3(x[4], x[5], CAST_SBOX_Define::CAST6_KEY_MASK[4 * j + 10], CAST_SBOX_Define::CAST6_KEY_ROT[(4 * j + 10) & 31]);
            CAST_F1(x[3], x[4], CAST_SBOX_Define::CAST6_KEY_MASK[4 * j + 11], CAST_SBOX_Define::CAST6_KEY_ROT[(4 * j + 11) & 31]);
            CAST_F2(x[2], x[3], CAST_SBOX_Define::CAST6_KEY_MASK[4 * j + 12], CAST_SBOX_Define::CAST6_KEY_ROT[(4 * j + 12) & 31]);
            CAST_F3(x[1], x[2], CAST_SBOX_Define::CAST6_KEY_MASK[4 * j + 13], CAST_SBOX_Define::CAST6_KEY_ROT[(4 * j + 13) & 31]);
            CAST_F1(x[0], x[1], CAST_SBOX_Define::CAST6_KEY_MASK[4 * j + 14], CAST_SBOX_Define::CAST6_KEY_ROT[(4 * j + 14) & 31]);
            CAST_F2(x[7], x[0], CAST_SBOX_Define::CAST6_KEY_MASK[4 * j + 15], CAST_SBOX_Define::CAST6_KEY_ROT[(4 * j + 15) & 31]);

            rkey_ptr[j] = (x[0]) & 0xFF;
            rkey_ptr[j + 1] = (x[2]) & 0xFF;
            rkey_ptr[j + 2] = (x[4]) & 0xFF;
            rkey_ptr[j + 3] = (x[6]) & 0xFF;

            mkey_ptr[j] = x[7];
            mkey_ptr[j + 1] = x[5];
            mkey_ptr[j + 2] = x[3];
            mkey_ptr[j + 3] = x[1];
        }
    }

    //CAST6加密函数
    static void ecb_encrypt(const SUBKEY_STRUCT* sub_key,
                            const unsigned char* src_block,
                            unsigned char* cipher_block)
    {
        const uint32_t* mkey_ptr = sub_key->mkey_;
        const uint8_t* rkey_ptr = sub_key->rkey_;
        uint32_t t = 0;

        uint32_t a = ZINDEX_TO_LEUINT32(src_block, 0);
        uint32_t b = ZINDEX_TO_LEUINT32(src_block, 1);
        uint32_t c = ZINDEX_TO_LEUINT32(src_block, 2);
        uint32_t d = ZINDEX_TO_LEUINT32(src_block, 3);

        //48轮处理
        //正向，Qi
        CAST_F1(c, d, mkey_ptr[0], rkey_ptr[0]);
        CAST_F2(b, c, mkey_ptr[1], rkey_ptr[1]);
        CAST_F3(a, b, mkey_ptr[2], rkey_ptr[2]);
        CAST_F1(d, a, mkey_ptr[3], rkey_ptr[3]);
        CAST_F1(c, d, mkey_ptr[4], rkey_ptr[4]);
        CAST_F2(b, c, mkey_ptr[5], rkey_ptr[5]);
        CAST_F3(a, b, mkey_ptr[6], rkey_ptr[6]);
        CAST_F1(d, a, mkey_ptr[7], rkey_ptr[7]);
        CAST_F1(c, d, mkey_ptr[8], rkey_ptr[8]);
        CAST_F2(b, c, mkey_ptr[9], rkey_ptr[9]);
        CAST_F3(a, b, mkey_ptr[10], rkey_ptr[10]);
        CAST_F1(d, a, mkey_ptr[11], rkey_ptr[11]);
        if (round_size > 12)
        {
            CAST_F1(c, d, mkey_ptr[12], rkey_ptr[12]);
            CAST_F2(b, c, mkey_ptr[13], rkey_ptr[13]);
            CAST_F3(a, b, mkey_ptr[14], rkey_ptr[14]);
            CAST_F1(d, a, mkey_ptr[15], rkey_ptr[15]);
            CAST_F1(c, d, mkey_ptr[16], rkey_ptr[16]);
            CAST_F2(b, c, mkey_ptr[17], rkey_ptr[17]);
            CAST_F3(a, b, mkey_ptr[18], rkey_ptr[18]);
            CAST_F1(d, a, mkey_ptr[19], rkey_ptr[19]);
            CAST_F1(c, d, mkey_ptr[20], rkey_ptr[20]);
            CAST_F2(b, c, mkey_ptr[21], rkey_ptr[21]);
            CAST_F3(a, b, mkey_ptr[22], rkey_ptr[22]);
            CAST_F1(d, a, mkey_ptr[23], rkey_ptr[23]);
        }
        //反向，QBARi
        if (round_size > 24)
        {
            CAST_F1(d, a, mkey_ptr[27], rkey_ptr[27]);
            CAST_F3(a, b, mkey_ptr[26], rkey_ptr[26]);
            CAST_F2(b, c, mkey_ptr[25], rkey_ptr[25]);
            CAST_F1(c, d, mkey_ptr[24], rkey_ptr[24]);
            CAST_F1(d, a, mkey_ptr[31], rkey_ptr[31]);
            CAST_F3(a, b, mkey_ptr[30], rkey_ptr[30]);
            CAST_F2(b, c, mkey_ptr[29], rkey_ptr[29]);
            CAST_F1(c, d, mkey_ptr[28], rkey_ptr[28]);
            CAST_F1(d, a, mkey_ptr[35], rkey_ptr[35]);
            CAST_F3(a, b, mkey_ptr[34], rkey_ptr[34]);
            CAST_F2(b, c, mkey_ptr[33], rkey_ptr[33]);
            CAST_F1(c, d, mkey_ptr[32], rkey_ptr[32]);
        }
        if (round_size > 36)
        {
            CAST_F1(d, a, mkey_ptr[39], rkey_ptr[39]);
            CAST_F3(a, b, mkey_ptr[38], rkey_ptr[38]);
            CAST_F2(b, c, mkey_ptr[37], rkey_ptr[37]);
            CAST_F1(c, d, mkey_ptr[36], rkey_ptr[36]);
            CAST_F1(d, a, mkey_ptr[43], rkey_ptr[43]);
            CAST_F3(a, b, mkey_ptr[42], rkey_ptr[42]);
            CAST_F2(b, c, mkey_ptr[41], rkey_ptr[41]);
            CAST_F1(c, d, mkey_ptr[40], rkey_ptr[40]);
            CAST_F1(d, a, mkey_ptr[47], rkey_ptr[47]);
            CAST_F3(a, b, mkey_ptr[46], rkey_ptr[46]);
            CAST_F2(b, c, mkey_ptr[45], rkey_ptr[45]);
            CAST_F1(c, d, mkey_ptr[44], rkey_ptr[44]);
        }

        ZLEUINT32_TO_INDEX(cipher_block, 0, a);
        ZLEUINT32_TO_INDEX(cipher_block, 1, b);
        ZLEUINT32_TO_INDEX(cipher_block, 2, c);
        ZLEUINT32_TO_INDEX(cipher_block, 3, d);
    }
    //CAST6解密函数
    static void ecb_decrypt(const SUBKEY_STRUCT* sub_key,
                            const unsigned char* cipher_block,
                            unsigned char* src_block)
    {
        const uint32_t* mkey_ptr = sub_key->mkey_;
        const uint8_t* rkey_ptr = sub_key->rkey_;
        uint32_t t = 0;

        uint32_t a = ZINDEX_TO_LEUINT32(cipher_block, 0);
        uint32_t b = ZINDEX_TO_LEUINT32(cipher_block, 1);
        uint32_t c = ZINDEX_TO_LEUINT32(cipher_block, 2);
        uint32_t d = ZINDEX_TO_LEUINT32(cipher_block, 3);

        //48轮处理
        //反向，QBARi
        if (round_size > 36)
        {
            CAST_F1(c, d, mkey_ptr[44], rkey_ptr[44]);
            CAST_F2(b, c, mkey_ptr[45], rkey_ptr[45]);
            CAST_F3(a, b, mkey_ptr[46], rkey_ptr[46]);
            CAST_F1(d, a, mkey_ptr[47], rkey_ptr[47]);
            CAST_F1(c, d, mkey_ptr[40], rkey_ptr[40]);
            CAST_F2(b, c, mkey_ptr[41], rkey_ptr[41]);
            CAST_F3(a, b, mkey_ptr[42], rkey_ptr[42]);
            CAST_F1(d, a, mkey_ptr[43], rkey_ptr[43]);
            CAST_F1(c, d, mkey_ptr[36], rkey_ptr[36]);
            CAST_F2(b, c, mkey_ptr[37], rkey_ptr[37]);
            CAST_F3(a, b, mkey_ptr[38], rkey_ptr[38]);
            CAST_F1(d, a, mkey_ptr[39], rkey_ptr[39]);
        }
        if (round_size > 24)
        {
            CAST_F1(c, d, mkey_ptr[32], rkey_ptr[32]);
            CAST_F2(b, c, mkey_ptr[33], rkey_ptr[33]);
            CAST_F3(a, b, mkey_ptr[34], rkey_ptr[34]);
            CAST_F1(d, a, mkey_ptr[35], rkey_ptr[35]);
            CAST_F1(c, d, mkey_ptr[28], rkey_ptr[28]);
            CAST_F2(b, c, mkey_ptr[29], rkey_ptr[29]);
            CAST_F3(a, b, mkey_ptr[30], rkey_ptr[30]);
            CAST_F1(d, a, mkey_ptr[31], rkey_ptr[31]);
            CAST_F1(c, d, mkey_ptr[24], rkey_ptr[24]);
            CAST_F2(b, c, mkey_ptr[25], rkey_ptr[25]);
            CAST_F3(a, b, mkey_ptr[26], rkey_ptr[26]);
            CAST_F1(d, a, mkey_ptr[27], rkey_ptr[27]);
        }
        //正向，Qi
        if (round_size > 12)
        {
            CAST_F1(d, a, mkey_ptr[23], rkey_ptr[23]);
            CAST_F3(a, b, mkey_ptr[22], rkey_ptr[22]);
            CAST_F2(b, c, mkey_ptr[21], rkey_ptr[21]);
            CAST_F1(c, d, mkey_ptr[20], rkey_ptr[20]);
            CAST_F1(d, a, mkey_ptr[19], rkey_ptr[19]);
            CAST_F3(a, b, mkey_ptr[18], rkey_ptr[18]);
            CAST_F2(b, c, mkey_ptr[17], rkey_ptr[17]);
            CAST_F1(c, d, mkey_ptr[16], rkey_ptr[16]);
            CAST_F1(d, a, mkey_ptr[15], rkey_ptr[15]);
            CAST_F3(a, b, mkey_ptr[14], rkey_ptr[14]);
            CAST_F2(b, c, mkey_ptr[13], rkey_ptr[13]);
            CAST_F1(c, d, mkey_ptr[12], rkey_ptr[12]);
        }
        CAST_F1(d, a, mkey_ptr[11], rkey_ptr[11]);
        CAST_F3(a, b, mkey_ptr[10], rkey_ptr[10]);
        CAST_F2(b, c, mkey_ptr[9], rkey_ptr[9]);
        CAST_F1(c, d, mkey_ptr[8], rkey_ptr[8]);
        CAST_F1(d, a, mkey_ptr[7], rkey_ptr[7]);
        CAST_F3(a, b, mkey_ptr[6], rkey_ptr[6]);
        CAST_F2(b, c, mkey_ptr[5], rkey_ptr[5]);
        CAST_F1(c, d, mkey_ptr[4], rkey_ptr[4]);
        CAST_F1(d, a, mkey_ptr[3], rkey_ptr[3]);
        CAST_F3(a, b, mkey_ptr[2], rkey_ptr[2]);
        CAST_F2(b, c, mkey_ptr[1], rkey_ptr[1]);
        CAST_F1(c, d, mkey_ptr[0], rkey_ptr[0]);

        ZLEUINT32_TO_INDEX(src_block, 0, a);
        ZLEUINT32_TO_INDEX(src_block, 1, b);
        ZLEUINT32_TO_INDEX(src_block, 2, c);
        ZLEUINT32_TO_INDEX(src_block, 3, d);
    }

public:
    //
    const static size_t BLOCK_SIZE = 16;
    //
    const static size_t KEY_SIZE = key_size;
};

#undef CAST_F1
#undef CAST_F2
#undef CAST_F3

//第一个参数是密钥长度，第二个参数是加密轮数，如果考虑加密效果，48轮的都值得推荐
//typedef命名原则是，加密算法名称，算法处理的BLOCK长度，key长度，轮数(推荐的轮数往往和key长度有一定关系)
typedef ZCE_Crypt<CAST6_ECB<16, 12> >  CAST6_Crypt_128_128_12;
typedef ZCE_Crypt<CAST6_ECB<16, 24> >  CAST6_Crypt_128_128_24;
typedef ZCE_Crypt<CAST6_ECB<16, 36> >  CAST6_Crypt_128_128_36;
typedef ZCE_Crypt<CAST6_ECB<16, 48> >  CAST6_Crypt_128_128_48;
typedef ZCE_Crypt<CAST6_ECB<20, 48> >  CAST6_Crypt_128_160_48;
typedef ZCE_Crypt<CAST6_ECB<24, 48> >  CAST6_Crypt_128_192_48;
typedef ZCE_Crypt<CAST6_ECB<28, 48> >  CAST6_Crypt_128_224_48;
typedef ZCE_Crypt<CAST6_ECB<32, 48> >  CAST6_Crypt_128_256_48;

//=================================================================================================================
//MARS算法年的定义
class MARS_ECB_Define
{
protected:
    //
    static const uint32_t  MARS_S_BOX[512];
    //
    static const uint32_t  MARS_IM1[15];
    //
    static const uint32_t  MARS_IM2[15];
    //
    static const uint32_t  MARS_IM7[15];
    //
    static const uint32_t  MARS_I4M[15];
    //
    static const uint32_t  MARS_B_TAB[4];
};

/*!
@brief      MARS加密算法实现
            http://en.wikipedia.org/wiki/MARS_%28cryptography%29
@tparam     key_size KEY的长度，可以是16字节（128bits） - 56字节 （448bits）每次增加4字节，
@note       和其他算法不一样的地方
            MARS has a heterogeneous structure: several rounds of a cryptographic
            core are "jacketed" by unkeyed mixing rounds, together with key whitening.
*/
template <size_t key_size>
class MARS_ECB : public MARS_ECB_Define
{
public:

    struct MARS_SUBKEY
    {
        uint32_t  ll_key_[40];
    };

    typedef MARS_SUBKEY  SUBKEY_STRUCT;

    static void key_setup(const unsigned char* key,
                          SUBKEY_STRUCT* sub_key,
                          bool  /*if_encrypt*/)
    {
        uint32_t t_key[16];
        uint32_t* ll_key = sub_key->ll_key_;

        uint32_t key_word_num = key_size / sizeof(uint32_t);
        for (uint32_t i = 0; i < key_word_num; ++i)
        {
            t_key[i] = ZINDEX_TO_LEUINT32(key, i);
        }
        //填写KEY长度
        t_key[key_word_num] = key_word_num;
        //填写0
        for (uint32_t i = key_word_num + 1; i < 15; ++i)
        {
            t_key[i] = 0;
        }

        for (size_t i = 0; i < 4; ++i)
        {
            for (size_t j = 0; j < 15; ++j)
            {
                t_key[j] ^= ZCE_ROTL32(t_key[MARS_IM7[j]] ^ t_key[MARS_IM2[j]], 3) ^ (4 * j + i);
            }
            for (size_t m = 0; m < 4; ++m)
            {
                for (size_t j = 0; j < 15; ++j)
                {
                    t_key[j] = ZCE_ROTL32(t_key[j] + MARS_S_BOX[t_key[MARS_IM1[j]] & 511], 9);
                }
            }
            for (size_t j = 0; j < 10; ++j)
            {
                ll_key[10 * i + j] = t_key[MARS_I4M[j]];
            }
        }

        uint32_t x = 0;
        uint32_t y = 0;
        for (size_t i = 5; i < 37; i += 2)
        {
            x = ll_key[i] | 3;
            y = (~x ^ (x >> 1)) & 0x7fffffff;
            y &= (y >> 1) & (y >> 2);
            y &= (y >> 3) & (y >> 6);

            if (y)
            {
                y <<= 1;
                y |= (y << 1);
                y |= (y << 2);
                y |= (y << 4);
                y &= 0xfffffffc;
                x ^= (ZCE_ROTL32(MARS_B_TAB[ll_key[i] & 3], ll_key[i - 1]) & y);
            }

            ll_key[i] = x;
        }
    }

#define MARS_F_MIX(a,b,c,d)           \
    r = ZCE_ROTR32(a, 8);             \
    b ^= MARS_S_BOX[a & 255];         \
    b += MARS_S_BOX[(r & 255) + 256]; \
    r = ZCE_ROTR32(a, 16);            \
    a = ZCE_ROTR32(a, 24);            \
    c += MARS_S_BOX[r & 255];         \
    d ^= MARS_S_BOX[(a & 255) + 256]

#define MARS_B_MIX(a,b,c,d)           \
    r = ZCE_ROTL32(a, 8);             \
    b ^= MARS_S_BOX[(a & 255) + 256]; \
    c -= MARS_S_BOX[r & 255];         \
    r = ZCE_ROTL32(a, 16);            \
    a  = ZCE_ROTL32(a, 24);           \
    d -= MARS_S_BOX[(r & 255) + 256]; \
    d ^= MARS_S_BOX[a & 255]

#define MARS_F_KTR(a,b,c,d,i)        \
    m = a + ll_key[i];               \
    a = ZCE_ROTL32(a, 13);           \
    r = a * ll_key[i + 1];           \
    l = MARS_S_BOX[m & 511];         \
    r = ZCE_ROTL32(r, 5);            \
    c += ZCE_ROTL32(m, r);           \
    l ^= r;                          \
    r = ZCE_ROTL32(r, 5);            \
    l ^= r;                          \
    d ^= r;                          \
    b += ZCE_ROTL32(l, r)

#define MARS_R_KTR(a,b,c,d,i)        \
    r = a * ll_key[i + 1];           \
    a = ZCE_ROTR32(a, 13);           \
    m = a + ll_key[i];               \
    l = MARS_S_BOX[m & 511];         \
    r = ZCE_ROTL32(r, 5);            \
    l ^= r;                          \
    c -= ZCE_ROTL32(m, r);           \
    r = ZCE_ROTL32(r, 5);            \
    l ^= r;                          \
    d ^= r;                          \
    b -= ZCE_ROTL32(l, r)

    static void ecb_encrypt(const SUBKEY_STRUCT* sub_key,
                            const unsigned char* src_block,
                            unsigned char* cipher_block)
    {
        unsigned int   a, b, c, d, l, m, r;
        const uint32_t* ll_key = sub_key->ll_key_;

        a = ZINDEX_TO_LEUINT32(src_block, 0) + ll_key[0];
        b = ZINDEX_TO_LEUINT32(src_block, 1) + ll_key[1];
        c = ZINDEX_TO_LEUINT32(src_block, 2) + ll_key[2];
        d = ZINDEX_TO_LEUINT32(src_block, 3) + ll_key[3];

        MARS_F_MIX(a, b, c, d);
        a += d;
        MARS_F_MIX(b, c, d, a);
        b += c;
        MARS_F_MIX(c, d, a, b);
        MARS_F_MIX(d, a, b, c);
        MARS_F_MIX(a, b, c, d);
        a += d;
        MARS_F_MIX(b, c, d, a);
        b += c;
        MARS_F_MIX(c, d, a, b);
        MARS_F_MIX(d, a, b, c);

        MARS_F_KTR(a, b, c, d, 4);
        MARS_F_KTR(b, c, d, a, 6);
        MARS_F_KTR(c, d, a, b, 8);
        MARS_F_KTR(d, a, b, c, 10);
        MARS_F_KTR(a, b, c, d, 12);
        MARS_F_KTR(b, c, d, a, 14);
        MARS_F_KTR(c, d, a, b, 16);
        MARS_F_KTR(d, a, b, c, 18);
        MARS_F_KTR(a, d, c, b, 20);
        MARS_F_KTR(b, a, d, c, 22);
        MARS_F_KTR(c, b, a, d, 24);
        MARS_F_KTR(d, c, b, a, 26);
        MARS_F_KTR(a, d, c, b, 28);
        MARS_F_KTR(b, a, d, c, 30);
        MARS_F_KTR(c, b, a, d, 32);
        MARS_F_KTR(d, c, b, a, 34);

        MARS_B_MIX(a, b, c, d);
        MARS_B_MIX(b, c, d, a);
        c -= b;
        MARS_B_MIX(c, d, a, b);
        d -= a;
        MARS_B_MIX(d, a, b, c);
        MARS_B_MIX(a, b, c, d);
        MARS_B_MIX(b, c, d, a);
        c -= b;
        MARS_B_MIX(c, d, a, b);
        d -= a;
        MARS_B_MIX(d, a, b, c);

        ZLEUINT32_TO_INDEX(cipher_block, 0, a - ll_key[36]);
        ZLEUINT32_TO_INDEX(cipher_block, 1, b - ll_key[37]);
        ZLEUINT32_TO_INDEX(cipher_block, 2, c - ll_key[38]);
        ZLEUINT32_TO_INDEX(cipher_block, 3, d - ll_key[39]);
    }

    //
    static void ecb_decrypt(const SUBKEY_STRUCT* sub_key,
                            const unsigned char* cipher_block,
                            unsigned char* src_block)
    {
        unsigned int   a, b, c, d, l, m, r;
        const uint32_t* ll_key = sub_key->ll_key_;

        d = ZINDEX_TO_LEUINT32(cipher_block, 0) + ll_key[36];
        c = ZINDEX_TO_LEUINT32(cipher_block, 1) + ll_key[37];
        b = ZINDEX_TO_LEUINT32(cipher_block, 2) + ll_key[38];
        a = ZINDEX_TO_LEUINT32(cipher_block, 3) + ll_key[39];

        MARS_F_MIX(a, b, c, d);
        a += d;
        MARS_F_MIX(b, c, d, a);
        b += c;
        MARS_F_MIX(c, d, a, b);
        MARS_F_MIX(d, a, b, c);
        MARS_F_MIX(a, b, c, d);
        a += d;
        MARS_F_MIX(b, c, d, a);
        b += c;
        MARS_F_MIX(c, d, a, b);
        MARS_F_MIX(d, a, b, c);

        MARS_R_KTR(a, b, c, d, 34);
        MARS_R_KTR(b, c, d, a, 32);
        MARS_R_KTR(c, d, a, b, 30);
        MARS_R_KTR(d, a, b, c, 28);
        MARS_R_KTR(a, b, c, d, 26);
        MARS_R_KTR(b, c, d, a, 24);
        MARS_R_KTR(c, d, a, b, 22);
        MARS_R_KTR(d, a, b, c, 20);
        MARS_R_KTR(a, d, c, b, 18);
        MARS_R_KTR(b, a, d, c, 16);
        MARS_R_KTR(c, b, a, d, 14);
        MARS_R_KTR(d, c, b, a, 12);
        MARS_R_KTR(a, d, c, b, 10);
        MARS_R_KTR(b, a, d, c, 8);
        MARS_R_KTR(c, b, a, d, 6);
        MARS_R_KTR(d, c, b, a, 4);

        MARS_B_MIX(a, b, c, d);
        MARS_B_MIX(b, c, d, a);
        c -= b;
        MARS_B_MIX(c, d, a, b);
        d -= a;
        MARS_B_MIX(d, a, b, c);
        MARS_B_MIX(a, b, c, d);
        MARS_B_MIX(b, c, d, a);
        c -= b;
        MARS_B_MIX(c, d, a, b);
        d -= a;
        MARS_B_MIX(d, a, b, c);

        ZLEUINT32_TO_INDEX(src_block, 0, d - ll_key[0]);
        ZLEUINT32_TO_INDEX(src_block, 1, c - ll_key[1]);
        ZLEUINT32_TO_INDEX(src_block, 2, b - ll_key[2]);
        ZLEUINT32_TO_INDEX(src_block, 3, a - ll_key[3]);
    }

#undef MARS_F_MIX
#undef MARS_B_MIX
#undef MARS_F_KTR
#undef MARS_R_KTR

    //
    const static size_t BLOCK_SIZE = 16;
    //
    const static size_t KEY_SIZE = key_size;
};

//理论可以选择16，20，24，28，32，36，40，44，48，52，56作为key长度
//typedef命名原则是，加密算法名称，算法处理的BLOCK长度，key长度，轮数(推荐的轮数往往和key长度有一定关系)
typedef ZCE_Crypt<MARS_ECB<16> > MARS_Crypt_128_128_1;
typedef ZCE_Crypt<MARS_ECB<24> > MARS_Crypt_128_192_1;
typedef ZCE_Crypt<MARS_ECB<32> > MARS_Crypt_128_256_1;

//=================================================================================================================
//AES 的算法代码来自Taocrypt库，
class AES_ECB_Define
{
protected:
    //加密用的SBOX
    static const uint32_t TE_SBOX[5][256];
    //解密用的BOX
    static const uint32_t TD_SBOX[5][256];
    //
    static const uint32_t AES_RCON[10];
};

/*!
@brief      终于上大餐了,AES的加密算法，
            该算法为比利时密码学家Joan Daemen和Vincent Rijmen所设计，结合两位作者的名字，
            以Rijndael为名投稿高级加密标准的甄选流程。（Rijndael的发音近于 "Rhine doll"）
            从Square改进而来

            http://zh.wikipedia.org/wiki/%E9%AB%98%E7%BA%A7%E5%8A%A0%E5%AF%86%E6%A0%87%E5%87%86
            http://en.wikipedia.org/wiki/Advanced_Encryption_Standard
@tparam     key_size     16,24,32
@tparam     round_size   10,12,14 rounds_ = keylen / 4 + 6;
@note
*/
template <size_t key_size, size_t round_size>
class AES_ECB : public AES_ECB_Define
{
public:

    struct AES_SUBKEY
    {
        uint32_t skey_[round_size * 4 + 4];
    };

    //为了方便ZCE_Crypt,定义的子KEY类型，
    typedef AES_SUBKEY  SUBKEY_STRUCT;

    //生成加密所需的SKEY，注意，AES的加密和解密的sub key，是不一样的，（大家终于明白为啥要有这个参数了吧）
    static void key_setup(const unsigned char* key, SUBKEY_STRUCT* sub_key, bool if_encrypt)
    {
        uint32_t temp = 0, * rk = sub_key->skey_;
        for (size_t k = 0; k < key_size / sizeof(uint32_t); ++k)
        {
            rk[k] = ZINDEX_TO_LEUINT32(key, k);
        }
        size_t p = 0;
        switch (key_size)
        {
        case 16:
            while (true)
            {
                temp = rk[3];
                rk[4] = rk[0] ^
                    (TE_SBOX[4][ZUINT32_2BYTE(temp)] & 0xff000000) ^
                    (TE_SBOX[4][ZUINT32_1BYTE(temp)] & 0x00ff0000) ^
                    (TE_SBOX[4][ZUINT32_0BYTE(temp)] & 0x0000ff00) ^
                    (TE_SBOX[4][ZUINT32_3BYTE(temp)] & 0x000000ff) ^
                    AES_RCON[p];
                rk[5] = rk[1] ^ rk[4];
                rk[6] = rk[2] ^ rk[5];
                rk[7] = rk[3] ^ rk[6];
                if (++p == 10)
                {
                    break;
                }
                rk += 4;
            }
            break;

        case 24:
            while (true)    // for (;;) here triggers a bug in VC60 SP4 w/ Pro Pack
            {
                temp = rk[5];
                rk[6] = rk[0] ^
                    (TE_SBOX[4][ZUINT32_2BYTE(temp)] & 0xff000000) ^
                    (TE_SBOX[4][ZUINT32_1BYTE(temp)] & 0x00ff0000) ^
                    (TE_SBOX[4][ZUINT32_0BYTE(temp)] & 0x0000ff00) ^
                    (TE_SBOX[4][ZUINT32_3BYTE(temp)] & 0x000000ff) ^
                    AES_RCON[p];
                rk[7] = rk[1] ^ rk[6];
                rk[8] = rk[2] ^ rk[7];
                rk[9] = rk[3] ^ rk[8];
                if (++p == 8)
                {
                    break;
                }
                rk[10] = rk[4] ^ rk[9];
                rk[11] = rk[5] ^ rk[10];
                rk += 6;
            }
            break;

        case 32:
            while (true)
            {
                temp = rk[7];
                rk[8] = rk[0] ^
                    (TE_SBOX[4][ZUINT32_2BYTE(temp)] & 0xff000000) ^
                    (TE_SBOX[4][ZUINT32_1BYTE(temp)] & 0x00ff0000) ^
                    (TE_SBOX[4][ZUINT32_0BYTE(temp)] & 0x0000ff00) ^
                    (TE_SBOX[4][ZUINT32_3BYTE(temp)] & 0x000000ff) ^
                    AES_RCON[p];
                rk[9] = rk[1] ^ rk[8];
                rk[10] = rk[2] ^ rk[9];
                rk[11] = rk[3] ^ rk[10];
                if (++p == 7)
                {
                    break;
                }
                temp = rk[11];
                rk[12] = rk[4] ^
                    (TE_SBOX[4][ZUINT32_3BYTE(temp)] & 0xff000000) ^
                    (TE_SBOX[4][ZUINT32_2BYTE(temp)] & 0x00ff0000) ^
                    (TE_SBOX[4][ZUINT32_1BYTE(temp)] & 0x0000ff00) ^
                    (TE_SBOX[4][ZUINT32_0BYTE(temp)] & 0x000000ff);
                rk[13] = rk[5] ^ rk[12];
                rk[14] = rk[6] ^ rk[13];
                rk[15] = rk[7] ^ rk[14];

                rk += 8;
            }
            break;
        }

        //如果是解密
        if (if_encrypt == false)
        {
            rk = sub_key->skey_;

            /* invert the order of the round keys: */
            for (size_t i = 0, j = 4 * round_size; i < j; i += 4, j -= 4)
            {
                temp = rk[i];
                rk[i] = rk[j];
                rk[j] = temp;
                temp = rk[i + 1];
                rk[i + 1] = rk[j + 1];
                rk[j + 1] = temp;
                temp = rk[i + 2];
                rk[i + 2] = rk[j + 2];
                rk[j + 2] = temp;
                temp = rk[i + 3];
                rk[i + 3] = rk[j + 3];
                rk[j + 3] = temp;
            }
            // apply the inverse MixColumn transform to all round keys but the
            // first and the last:
            for (size_t i = 1; i < round_size; i++)
            {
                rk += 4;
                rk[0] =
                    TD_SBOX[0][TE_SBOX[4][ZUINT32_3BYTE(rk[0])] & 0xff] ^
                    TD_SBOX[1][TE_SBOX[4][ZUINT32_2BYTE(rk[0])] & 0xff] ^
                    TD_SBOX[2][TE_SBOX[4][ZUINT32_1BYTE(rk[0])] & 0xff] ^
                    TD_SBOX[3][TE_SBOX[4][ZUINT32_0BYTE(rk[0])] & 0xff];
                rk[1] =
                    TD_SBOX[0][TE_SBOX[4][ZUINT32_3BYTE(rk[1])] & 0xff] ^
                    TD_SBOX[1][TE_SBOX[4][ZUINT32_2BYTE(rk[1])] & 0xff] ^
                    TD_SBOX[2][TE_SBOX[4][ZUINT32_1BYTE(rk[1])] & 0xff] ^
                    TD_SBOX[3][TE_SBOX[4][ZUINT32_0BYTE(rk[1])] & 0xff];
                rk[2] =
                    TD_SBOX[0][TE_SBOX[4][ZUINT32_3BYTE(rk[2])] & 0xff] ^
                    TD_SBOX[1][TE_SBOX[4][ZUINT32_2BYTE(rk[2])] & 0xff] ^
                    TD_SBOX[2][TE_SBOX[4][ZUINT32_1BYTE(rk[2])] & 0xff] ^
                    TD_SBOX[3][TE_SBOX[4][ZUINT32_0BYTE(rk[2])] & 0xff];
                rk[3] =
                    TD_SBOX[0][TE_SBOX[4][ZUINT32_3BYTE(rk[3])] & 0xff] ^
                    TD_SBOX[1][TE_SBOX[4][ZUINT32_2BYTE(rk[3])] & 0xff] ^
                    TD_SBOX[2][TE_SBOX[4][ZUINT32_1BYTE(rk[3])] & 0xff] ^
                    TD_SBOX[3][TE_SBOX[4][ZUINT32_0BYTE(rk[3])] & 0xff];
            }
        }
    }

public:

    //异或加密函数
    static void ecb_encrypt(const SUBKEY_STRUCT* sub_key,
                            const unsigned char* src_block,
                            unsigned char* cipher_block)
    {
        uint32_t s0 = ZINDEX_TO_LEUINT32(src_block, 0);
        uint32_t s1 = ZINDEX_TO_LEUINT32(src_block, 1);
        uint32_t s2 = ZINDEX_TO_LEUINT32(src_block, 2);
        uint32_t s3 = ZINDEX_TO_LEUINT32(src_block, 3);

        uint32_t t0, t1, t2, t3;
        const uint32_t* rk = sub_key->skey_;

        s0 ^= rk[0];
        s1 ^= rk[1];
        s2 ^= rk[2];
        s3 ^= rk[3];

        // Nr - 1 full rounds:
        unsigned int r = round_size >> 1;
        for (;;)
        {
            t0 =
                TE_SBOX[0][ZUINT32_3BYTE(s0)] ^
                TE_SBOX[1][ZUINT32_2BYTE(s1)] ^
                TE_SBOX[2][ZUINT32_1BYTE(s2)] ^
                TE_SBOX[3][ZUINT32_0BYTE(s3)] ^
                rk[4];
            t1 =
                TE_SBOX[0][ZUINT32_3BYTE(s1)] ^
                TE_SBOX[1][ZUINT32_2BYTE(s2)] ^
                TE_SBOX[2][ZUINT32_1BYTE(s3)] ^
                TE_SBOX[3][ZUINT32_0BYTE(s0)] ^
                rk[5];
            t2 =
                TE_SBOX[0][ZUINT32_3BYTE(s2)] ^
                TE_SBOX[1][ZUINT32_2BYTE(s3)] ^
                TE_SBOX[2][ZUINT32_1BYTE(s0)] ^
                TE_SBOX[3][ZUINT32_0BYTE(s1)] ^
                rk[6];
            t3 =
                TE_SBOX[0][ZUINT32_3BYTE(s3)] ^
                TE_SBOX[1][ZUINT32_2BYTE(s0)] ^
                TE_SBOX[2][ZUINT32_1BYTE(s1)] ^
                TE_SBOX[3][ZUINT32_0BYTE(s2)] ^
                rk[7];

            rk += 8;
            if (--r == 0)
            {
                break;
            }

            s0 =
                TE_SBOX[0][ZUINT32_3BYTE(t0)] ^
                TE_SBOX[1][ZUINT32_2BYTE(t1)] ^
                TE_SBOX[2][ZUINT32_1BYTE(t2)] ^
                TE_SBOX[3][ZUINT32_0BYTE(t3)] ^
                rk[0];
            s1 =
                TE_SBOX[0][ZUINT32_3BYTE(t1)] ^
                TE_SBOX[1][ZUINT32_2BYTE(t2)] ^
                TE_SBOX[2][ZUINT32_1BYTE(t3)] ^
                TE_SBOX[3][ZUINT32_0BYTE(t0)] ^
                rk[1];
            s2 =
                TE_SBOX[0][ZUINT32_3BYTE(t2)] ^
                TE_SBOX[1][ZUINT32_2BYTE(t3)] ^
                TE_SBOX[2][ZUINT32_1BYTE(t0)] ^
                TE_SBOX[3][ZUINT32_0BYTE(t1)] ^
                rk[2];
            s3 =
                TE_SBOX[0][ZUINT32_3BYTE(t3)] ^
                TE_SBOX[1][ZUINT32_2BYTE(t0)] ^
                TE_SBOX[2][ZUINT32_1BYTE(t1)] ^
                TE_SBOX[3][ZUINT32_0BYTE(t2)] ^
                rk[3];
        }

        //apply last round and
        //map cipher state to byte array block:
        s0 =
            (TE_SBOX[4][ZUINT32_3BYTE(t0)] & 0xff000000) ^
            (TE_SBOX[4][ZUINT32_2BYTE(t1)] & 0x00ff0000) ^
            (TE_SBOX[4][ZUINT32_1BYTE(t2)] & 0x0000ff00) ^
            (TE_SBOX[4][ZUINT32_0BYTE(t3)] & 0x000000ff) ^
            rk[0];
        s1 =
            (TE_SBOX[4][ZUINT32_3BYTE(t1)] & 0xff000000) ^
            (TE_SBOX[4][ZUINT32_2BYTE(t2)] & 0x00ff0000) ^
            (TE_SBOX[4][ZUINT32_1BYTE(t3)] & 0x0000ff00) ^
            (TE_SBOX[4][ZUINT32_0BYTE(t0)] & 0x000000ff) ^
            rk[1];
        s2 =
            (TE_SBOX[4][ZUINT32_3BYTE(t2)] & 0xff000000) ^
            (TE_SBOX[4][ZUINT32_2BYTE(t3)] & 0x00ff0000) ^
            (TE_SBOX[4][ZUINT32_1BYTE(t0)] & 0x0000ff00) ^
            (TE_SBOX[4][ZUINT32_0BYTE(t1)] & 0x000000ff) ^
            rk[2];
        s3 =
            (TE_SBOX[4][ZUINT32_3BYTE(t3)] & 0xff000000) ^
            (TE_SBOX[4][ZUINT32_2BYTE(t0)] & 0x00ff0000) ^
            (TE_SBOX[4][ZUINT32_1BYTE(t1)] & 0x0000ff00) ^
            (TE_SBOX[4][ZUINT32_0BYTE(t2)] & 0x000000ff) ^
            rk[3];

        ZLEUINT32_TO_INDEX(cipher_block, 0, s0);
        ZLEUINT32_TO_INDEX(cipher_block, 1, s1);
        ZLEUINT32_TO_INDEX(cipher_block, 2, s2);
        ZLEUINT32_TO_INDEX(cipher_block, 3, s3);
    }
    //异或解密函数
    static void ecb_decrypt(const SUBKEY_STRUCT* sub_key,
                            const unsigned char* cipher_block,
                            unsigned char* src_block)
    {
        uint32_t s0 = ZINDEX_TO_LEUINT32(cipher_block, 0);
        uint32_t s1 = ZINDEX_TO_LEUINT32(cipher_block, 1);
        uint32_t s2 = ZINDEX_TO_LEUINT32(cipher_block, 2);
        uint32_t s3 = ZINDEX_TO_LEUINT32(cipher_block, 3);

        uint32_t t0, t1, t2, t3;
        const uint32_t* rk = sub_key->skey_;

        s0 ^= rk[0];
        s1 ^= rk[1];
        s2 ^= rk[2];
        s3 ^= rk[3];

        //Nr - 1 full rounds:
        unsigned int r = round_size >> 1;
        for (;;)
        {
            t0 =
                TD_SBOX[0][ZUINT32_3BYTE(s0)] ^
                TD_SBOX[1][ZUINT32_2BYTE(s3)] ^
                TD_SBOX[2][ZUINT32_1BYTE(s2)] ^
                TD_SBOX[3][ZUINT32_0BYTE(s1)] ^
                rk[4];
            t1 =
                TD_SBOX[0][ZUINT32_3BYTE(s1)] ^
                TD_SBOX[1][ZUINT32_2BYTE(s0)] ^
                TD_SBOX[2][ZUINT32_1BYTE(s3)] ^
                TD_SBOX[3][ZUINT32_0BYTE(s2)] ^
                rk[5];
            t2 =
                TD_SBOX[0][ZUINT32_3BYTE(s2)] ^
                TD_SBOX[1][ZUINT32_2BYTE(s1)] ^
                TD_SBOX[2][ZUINT32_1BYTE(s0)] ^
                TD_SBOX[3][ZUINT32_0BYTE(s3)] ^
                rk[6];
            t3 =
                TD_SBOX[0][ZUINT32_3BYTE(s3)] ^
                TD_SBOX[1][ZUINT32_2BYTE(s2)] ^
                TD_SBOX[2][ZUINT32_1BYTE(s1)] ^
                TD_SBOX[3][ZUINT32_0BYTE(s0)] ^
                rk[7];

            rk += 8;
            if (--r == 0)
            {
                break;
            }

            s0 =
                TD_SBOX[0][ZUINT32_3BYTE(t0)] ^
                TD_SBOX[1][ZUINT32_2BYTE(t3)] ^
                TD_SBOX[2][ZUINT32_1BYTE(t2)] ^
                TD_SBOX[3][ZUINT32_0BYTE(t1)] ^
                rk[0];
            s1 =
                TD_SBOX[0][ZUINT32_3BYTE(t1)] ^
                TD_SBOX[1][ZUINT32_2BYTE(t0)] ^
                TD_SBOX[2][ZUINT32_1BYTE(t3)] ^
                TD_SBOX[3][ZUINT32_0BYTE(t2)] ^
                rk[1];
            s2 =
                TD_SBOX[0][ZUINT32_3BYTE(t2)] ^
                TD_SBOX[1][ZUINT32_2BYTE(t1)] ^
                TD_SBOX[2][ZUINT32_1BYTE(t0)] ^
                TD_SBOX[3][ZUINT32_0BYTE(t3)] ^
                rk[2];
            s3 =
                TD_SBOX[0][ZUINT32_3BYTE(t3)] ^
                TD_SBOX[1][ZUINT32_2BYTE(t2)] ^
                TD_SBOX[2][ZUINT32_1BYTE(t1)] ^
                TD_SBOX[3][ZUINT32_0BYTE(t0)] ^
                rk[3];
        }

        //apply last round and map cipher state to byte array block:
        s0 =
            (TD_SBOX[4][ZUINT32_3BYTE(t0)] & 0xff000000) ^
            (TD_SBOX[4][ZUINT32_2BYTE(t3)] & 0x00ff0000) ^
            (TD_SBOX[4][ZUINT32_1BYTE(t2)] & 0x0000ff00) ^
            (TD_SBOX[4][ZUINT32_0BYTE(t1)] & 0x000000ff) ^
            rk[0];
        s1 =
            (TD_SBOX[4][ZUINT32_3BYTE(t1)] & 0xff000000) ^
            (TD_SBOX[4][ZUINT32_2BYTE(t0)] & 0x00ff0000) ^
            (TD_SBOX[4][ZUINT32_1BYTE(t3)] & 0x0000ff00) ^
            (TD_SBOX[4][ZUINT32_0BYTE(t2)] & 0x000000ff) ^
            rk[1];
        s2 =
            (TD_SBOX[4][ZUINT32_3BYTE(t2)] & 0xff000000) ^
            (TD_SBOX[4][ZUINT32_2BYTE(t1)] & 0x00ff0000) ^
            (TD_SBOX[4][ZUINT32_1BYTE(t0)] & 0x0000ff00) ^
            (TD_SBOX[4][ZUINT32_0BYTE(t3)] & 0x000000ff) ^
            rk[2];
        s3 =
            (TD_SBOX[4][ZUINT32_3BYTE(t3)] & 0xff000000) ^
            (TD_SBOX[4][ZUINT32_2BYTE(t2)] & 0x00ff0000) ^
            (TD_SBOX[4][ZUINT32_1BYTE(t1)] & 0x0000ff00) ^
            (TD_SBOX[4][ZUINT32_0BYTE(t0)] & 0x000000ff) ^
            rk[3];

        ZLEUINT32_TO_INDEX(src_block, 0, s0);
        ZLEUINT32_TO_INDEX(src_block, 1, s1);
        ZLEUINT32_TO_INDEX(src_block, 2, s2);
        ZLEUINT32_TO_INDEX(src_block, 3, s3);
    }

public:
    //异或算法一次处理一个BLOCK的长度
    const static size_t BLOCK_SIZE = 16;
    //异或算法KEY的长度
    const static size_t KEY_SIZE = key_size;
};

//24字节的key（196bits),32字节的的key(256bits)的key也被称为AES2
//typedef命名原则是，加密算法名称，算法处理的BLOCK长度，key长度，轮数(推荐的轮数往往和key长度有一定关系)
typedef ZCE_Crypt<AES_ECB<16, 10> > AES_Crypt_128_128_10;
typedef ZCE_Crypt<AES_ECB<24, 12> > AES_Crypt_128_192_12;
typedef ZCE_Crypt<AES_ECB<32, 14> > AES_Crypt_128_256_14;
};

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( pop )
#endif

#endif //#ifndef ZCE_LIB_BYTES_ENCRYPT_H_
