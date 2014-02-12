/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_data_proc_encrypt.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013年6月16日
* @brief      内存高速压缩算法，这个代码是根据LZ4 和 QuickLZ 得到的。
*             两个代码都比较简单，不算复杂，我有比较好的基础库，实现起来应该比他们更简单一些。
* 
*             最后发现这2个函数大概是这几年来写的最累的代码了，也真算的上颠沛流离。前前后后，加上看2个算法，
*             大概用了一半个月的零散时间。最后调试，特别是上JPG文件的用例的时候，还是发现问题不少。
*             慢的和蜗牛一样，其实总计也就不到1000行代码。
*             到7月10，基本OK，大体达到了我想要的目标，把自己的2个实现做一下包装，收工
* 
* @details    几种内存数据的压缩算法，主要用于数据压缩，
*             希望参考过的压缩算法包括，
*             快速压缩算法的  LZ4   QuickLZ   Zippy/Snappy  LZO等，
*             高压缩比的函数  LZMA    GZIP(zlib)
*             目前我的目标也就是实现一个内存的告诉压缩算法，
* 
*             最后还是要说一下LZ4的算法，我的2个算法都参考过这个算法，
* 
*             我承认，多年来我对性能优化是有一点点小小自负的，但LZ4这个算法完败了我
*             如果不信，你可以去挑战一下他。
*             更有我自己特色的第一个格式的算法，ZLZ，最后的结果是在压缩算法上我略差一节，
*             但在WIN64位的的解压算法上，我被甩开了40%，（其他平台落后20%-30%）
*             可以说，LZ4算法在压缩和性能上的平衡达到了一个极致，我认为这个极致的平衡
*             达到了一个境界，而且绝对是佛的境界。（见葛优，冯小刚的《大腕》）
*             输的心服口服，山外有山，人外有人。
* 
*             算法速度，WIN32下的速度，AMD Phenom II X4 3.2G
*             其中RAW LZ4是LZ4原生代码的数据 ZCE ZLZ 是法改进得到的算法，ZCE LZ4算法是基本模拟LZ4的代码。
*             RAW LZ4 [   2271498] compress use us[    7159750.486]us [        304.686]Mb/s decompress use [    3044644.086]us [        716.496]Mb/s.
*             ZCE ZLZ [   2297174] compress use us[    8423219.146]us [        258.984]Mb/s decompress use [    3605885.627]us [        604.977]Mb/s.
*             ZCE LZ4 [   2314403] compress use us[    8034556.044]us [        271.512]Mb/s decompress use [    2990780.992]us [        729.400]Mb/s.
*             算法速度 WIN64
*             RAW LZ4 [   2271498] compress use us[    7700759.320]us [        283.281]Mb/s decompress use [    2053244.006]us [       1062.453]Mb/s.
*             ZCE ZLZ [   2297174] compress use us[    7838718.628]us [        278.295]Mb/s decompress use [    3052662.310]us [        714.614]Mb/s.
*             ZCE LZ4 [   2314403] compress use us[    7617242.962]us [        286.387]Mb/s decompress use [    2221799.686]us [        981.851]Mb/s.
*             算法速度 LINUX64，虚拟机，所以数据并不好，大家主要看速度对比了。
*             RAW LZ4 [   2271498] compress use us[   15232820.350]us [        143.209]Mb/s decompress use [    5388874.147]us [        404.811]Mb/s.
*             ZCE ZLZ [   2297174] compress use us[   16014259.992]us [        136.221]Mb/s decompress use [    6448855.066]us [        338.273]Mb/s.
*             ZCE LZ4 [   2314403] compress use us[   16264991.011]us [        134.121]Mb/s decompress use [    5559644.472]us [        392.377]Mb/s.
* 
*             64为下，解压速度还是落后LZ4 10%，但我个人感觉LZ4的解压安全检查做的并不够，
* 
*             改进算法最后几天，耳机边一直回想的曲调。真的从时不我与变成了时不予我了
* 
*             山丘 - 李宗盛
*             词：李宗盛
*             曲：李宗盛
* 
*             想说却还没说的 还很多
*             攒着是因为想写成歌
*             让人轻轻地唱着 淡淡地记着
*             就算终于忘了  也值了
*             说不定我一生涓滴意念
*             侥幸汇成河
*             然后我俩各自一端
*             望着大河弯弯  终于敢放胆
*             嘻皮笑脸  面对  人生的难
* 
*             也许我们从未成熟
*             还没能晓得  就快要老了
*             尽管心里活着的还是那个
*             年轻人
*             因为不安而频频回首
*             无知地索求  羞耻于求救
*             不知疲倦地翻越   每一个山丘
* 
*             越过山丘  虽然已白了头
*             喋喋不休  时不我予的哀愁
*             还未如愿见着不朽
*             就把自己先搞丢
*             越过山丘  才发现无人等候
*             喋喋不休 再也唤不回温柔
*             为何记不得上一次是谁给的拥抱
*             在什么时候
* 
*             我没有刻意隐藏  也无意让你感伤
*             多少次我们无醉不欢
*             咒骂人生太短  唏嘘相见恨晚
*             让女人把妆哭花了  也不管
*             遗憾我们从未成熟
*             还没能晓得  就已经老了
*             尽力却仍不明白
*             身边的年轻人
*             给自己随便找个理由
*             向情爱的挑逗  命运的左右
*             不自量力地还手 直至死方休
* 
*             越过山丘  虽然已白了头
*             喋喋不休  时不我予的哀愁
*             还未如愿见着不朽
*             就把自己先搞丢
*             越过山丘  才发现无人等候
*             喋喋不休 再也唤不回了温柔
*             为何记不得上一次是谁给的拥抱
*             在什么时候
* 
*             越过山丘  虽然已白了头
*             喋喋不休  时不我予的哀愁
*             还未如愿见着不朽
*             就把自己先搞丢
*             越过山丘  才发现无人等候
*             喋喋不休 再也唤不回了温柔
*             为何记不得上一次是谁给的拥抱
*             在什么时候
* 
*             喋喋不休  时不我予的哀愁
*             向情爱的挑逗  命运的左右
*             不自量力地还手 直至死方休
*             为何记不得上一次是谁给的拥抱
*             在什么时候
* 
*/

#ifndef ZCE_LIB_BYTES_COMPRESS_H_
#define ZCE_LIB_BYTES_COMPRESS_H_

namespace ZCE_LIB
{

template < typename COMPRESS_STRATEGY >
class ZCE_Compress
{
public:
    /*!
    * @brief      得到加密所需的内存BUFFER大小
    * @return     int  ==0表示成功，否则失败，一般原因是原文长度过长
    * @param      original_size      原文的长度
    * @param      need_cmpbuf_size   所需的压缩buffer的长度，一般会比原文长，但内部会尽力保证用最短的尺寸。
    */
    inline static int need_compressed_bufsize(size_t original_size,
                                       size_t *need_cmpbuf_size)
    {
        //大约可以处理2G的数据，
        return COMPRESS_STRATEGY::need_comp_size(original_size,
                                                 need_cmpbuf_size);
    }

    /*!
    * @brief      从压缩后的的数据里面得到原文的尺寸，
    * @return     int           压缩数据格式错误
    * @param      original_buf  压缩的数据buffer
    * @param      original_size 返回参数，原文的尺寸
    */
    static int get_original_size(const unsigned char *compressed_buf,
                                 size_t *original_size)
    {
        uint32_t srclen_type = *compressed_buf & 0x7;
        size_t need_srclen = 0;
        if ( srclen_type == 0x1 )
        {
            need_srclen = *(compressed_buf + 1);
        }
        else if (srclen_type == 0x2)
        {
            need_srclen = ZBYTE_TO_LEUINT16((compressed_buf + 1));
        }
        else if (srclen_type == 0x4)
        {
            need_srclen = ZBYTE_TO_LEUINT32((compressed_buf + 1));
        }
        //不可能出现这种情况
        else
        {
            return -1;
        }
        return 0;
    }

    /*!
    * @brief      实时压缩一个数据区
    * @return     int              ==0表示成功，否则失败，失败原因一般是内存不够，或者原文过长
    * @param      original_buf     原文，
    * @param      original_size    原文的长度
    * @param      compressed_buf   压缩的内存，
    * @param      compressed_size  压缩的尺寸，传入传出参数，传入时表示compressed_buf长度，必须need_compressdbuf_size的返回值，返回时，
    *                              返回使用的compressed_buf的长度
    */
    static int compress(const unsigned char *original_buf,
                        size_t original_size,
                        unsigned char *compressed_buf,
                        size_t *compressed_size)
    {
        ZCE_ASSERT(original_buf && compressed_buf && original_size > 0);

        //如果准备的压缩空间不够，
        int ret = 0;
        size_t need_compbuf_size  =0;
        ret = need_compressed_bufsize(original_size,&need_compbuf_size);
        if ( ret != 0
            || *compressed_size < need_compbuf_size
            || original_size <= 0 )
        {
            ZCE_ASSERT(false);
            return -1;
        }

        unsigned char *head_pos = compressed_buf;
        *head_pos = 0;

        //注意保存的数据全部用的LE
        //头部，1字节选项，1个1字节原长度字段，
        size_t head_size = 0;
        if (original_size <= 0xFF )
        {
            head_size = 2;
            *head_pos |= 0x1;
            *(head_pos + 1) = (uint8_t)( original_size );
        }
        //头部，1字节选项，1个2字节原长度字段，
        else if (original_size <= 0xFFFF  )
        {
            head_size = 3;
            *head_pos |= 0x2;
            ZLEUINT16_TO_BYTE((head_pos + 1), ((uint16_t)(original_size )));
        }
        //头部，1字节选项，1个4字节标识原长度字段，
        else  if (original_size <= COMPRESS_STRATEGY::LZ_MAX_ORIGINAL_SIZE  )
        {
            head_size = 5;
            *head_pos |= 0x4;
            ZLEUINT32_TO_BYTE((head_pos + 1), ((uint32_t)(original_size )));
        }
        else
        {
            ZCE_ASSERT(false);
        }

        bool if_compress = false;
        size_t compressed_data_len = 0;

        //小于这个长度就不要压缩了，浪费生命
        if (original_size > ZCE_LZ_MIN_PROCESS_LEN)
        {
            COMPRESS_STRATEGY::compress_core(original_buf,
                                             original_size,
                                             compressed_buf + head_size,
                                             &compressed_data_len,
                                             &if_compress);
        }

        //保存是否进行了压缩，如果没有压缩，源数据全部拷贝过滤
        if (if_compress)
        {
            *compressed_size = compressed_data_len + head_size;
            *head_pos |= 0x80;
        }
        else
        {
            *compressed_size = original_size + head_size;
            //字节是对齐的地方，还是使用memcpy把
            ::memcpy(compressed_buf + head_size, original_buf, original_size);
        }

        return 0;
    }

    /*!
    * @brief      实时解压内存数据
    * @return     int             ==0，表示成功，否则失败，失败原因包括压缩数据错误，空间不够等，
    * @param      compressed_buf  被压缩的数据
    * @param      compressed_size 被压缩的数据的尺寸
    * @param      original_buf    原文
    * @param      original_size   原文的大小
    */
    static int decompress(const unsigned char *compressed_buf,
                          size_t compressed_size,
                          unsigned char *original_buf,
                          size_t *original_size)
    {
        ZCE_ASSERT(original_buf && compressed_buf && compressed_size >= 3);
        if (compressed_size < 3)
        {
            return -1;
        }
        const unsigned char *head_pos = compressed_buf;
        uint32_t srclen_type = *head_pos & 0x7;
        size_t need_srclen = 0;
        size_t head_size = 0;
        if ( srclen_type == 0x1 )
        {
            need_srclen = *(head_pos + 1);
            head_size = 2;
        }
        else if (srclen_type == 0x2)
        {
            need_srclen = ZBYTE_TO_LEUINT16((head_pos + 1));
            head_size = 3;
        }
        else if (srclen_type == 0x4)
        {
            need_srclen = ZBYTE_TO_LEUINT32((head_pos + 1));
            head_size = 5;
        }
        //不可能出现这种情况
        else
        {
            return -1;
        }

        //传入的空间不够
        if (need_srclen > *original_size )
        {
            return -1;
        }

        *original_size = need_srclen;
        bool if_compressed = (*head_pos & 0x80) ? true : false;
        if (if_compressed)
        {
            return COMPRESS_STRATEGY::decompress_core(compressed_buf + head_size,
                                                      compressed_size - head_size,
                                                      original_buf,
                                                      need_srclen);
        }
        //如果没有经过压缩
        else
        {
            if (compressed_size != need_srclen + head_size )
            {
                return -1;
            }
            //字节对齐的地方还是直接用memcpy吧
            ::memcpy(original_buf , compressed_buf + head_size, need_srclen);
            return 0;
        }

        //return 0;
    }

public:
    //最后的尾部保留一些字节不进行处理，主要是为了加速，因为里面很多地方都使用uint64_t进行处理，
    static const size_t ZCE_LZ_MIN_PROCESS_LEN = sizeof(uint64_t) * 2;
};

//=====================================================================================================
//ZCE LZ4算法是部分模拟LZ4的代码，但有一些格式变化。
class ZLZ_Compress_Format
{
public:

    //压缩核心代码
    static void compress_core(const unsigned char *original_buf,
                              size_t original_size,
                              unsigned char *compressed_buf,
                              size_t *compressed_size,
                              bool *if_compress);

    //解压核心代码
    static int decompress_core(const unsigned char *compressed_buf,
                               size_t compressed_size,
                               unsigned char *original_buf,
                               size_t original_size);

    //需要的压缩的buffer长度
    inline static int need_comp_size(size_t original_size,
        size_t *need_cmpbuf_size)
    {
        if (original_size > LZ_MAX_ORIGINAL_SIZE)
        {
            return -1;
        }
        *need_cmpbuf_size = ((original_size) + ( (original_size) / 0xFFF7 + 1) *3 + 64);
        return 0;
    }

    //这个压缩长度所能支持的最大长度，
    //因为用3个字节表示（大约）64K非压缩数据，加上头部，尾部的处理
    const static size_t LZ_MAX_ORIGINAL_SIZE = 0xFFFCFF00;
};

//直接的ZLZ的typedef，使用ZCE_LIB::ZLZ_Compress::compress ,decompress函数就可以完成功能
typedef ZCE_Compress<ZLZ_Compress_Format> ZLZ_Compress;

//=====================================================================================================

//ZEN ZLZ 是法改进得到的算法，
class LZ4_Compress_Format
{
public:
    //压缩核心代码
    static void compress_core(const unsigned char *original_buf,
                              size_t original_size,
                              unsigned char *compressed_buf,
                              size_t *compressed_size,
                              bool *if_compress);
    //解压核心代码
    static int decompress_core(const unsigned char *compressed_buf,
                               size_t compressed_size,
                               unsigned char *original_buf,
                               size_t original_size);

    //需要的压缩的buffer长度
    inline static int need_comp_size(size_t original_size,
        size_t *need_cmpbuf_size)
    {
        if (original_size > LZ_MAX_ORIGINAL_SIZE)
        {
            return -1;
        }
        *need_cmpbuf_size = ((original_size) + ( (original_size) / 0xFF + 1) + 64);
        return 0;
    }

    //这个压缩长度所能支持的最大长度，
    //LZ4算法会用一个字节表示255个字节数据没有压缩，
    const static size_t LZ_MAX_ORIGINAL_SIZE = 0xFEFEFE00;
};

//直接的ZEN LZ4的typedef
typedef ZCE_Compress<LZ4_Compress_Format> LZ4_Compress;

//=====================================================================================================

};//end of ZCE_LIB

#endif

