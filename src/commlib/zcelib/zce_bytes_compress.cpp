#include "zce_predefine.h"
#include "zce_trace_debugging.h"
#include "zce_os_adapt_string.h"
#include "zce_bytes_compress.h"

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( push )
#pragma warning ( disable : 4127)
#endif

//打开下面宏定义，会增加调试信息，对每次压缩结果进行记录
#define  ZCE_LZ_DEBUG 0

//使用HASH函数，只保留低位13位的信息，所以右移动19位
//解释一下((4*8)-13)，缓冲的大小是 8192，所以是2^13，
#define ZCE_LZ_HASH(ptr)       (((*(uint32_t *)(ptr)) *2654435761U)  >> ((4*8)-13))

//你要包装dst有足够的空间，都是8字节补齐的。
#if defined ZCE_OS64
#define ZCE_LZ_FAST_COPY_STOP(dst,src,stop)  \
    do \
    { \
        ZBYTE_TO_UINT64(dst) = ZBYTE_TO_UINT64(src); \
        dst += 8; \
        src += 8; \
    }while( (dst) < (stop));
#elif defined ZCE_OS32
#define ZCE_LZ_FAST_COPY_STOP(dst,src,stop)  \
    do \
    { \
        ZBYTE_TO_UINT32(dst) = ZBYTE_TO_UINT32(src); \
        dst += 4; \
        src += 4; \
        ZBYTE_TO_UINT32(dst) = ZBYTE_TO_UINT32(src); \
        dst += 4; \
        src += 4; \
    }while( (dst) < (stop));
#endif


//可以记录的最大偏移长度，偏移长度不可能记录的更大
const size_t  ZCE_LZ_MAX_OFFSET  = 0xFFFF;

//步进长度的幂，1<<6,64次
const size_t ZCE_LZ_STEP_LEN_POW  = 6;
//最大的步进长度，
//LZ4的步进长度是不处理的，所以他最后可能非常快，但也有可能是他对某些极端情况无法压缩
const size_t ZCE_LZ_STEP_MAX_LEN  = 32;


//尾部不处理的模块，因为比较的长度是8，而快速拷贝的长度也是8，所以保留16个字节是安全的
const size_t ZCE_LZ_NOPROCESS_TAIL = 8 * 2;

//HASHTABLE长度等于2多少次方
const uint32_t HASH_TABLE_LEN_2_POWER = 13;
//HASHTABLE的长度
const size_t HASH_TABLE_LEN = 0x1 << (HASH_TABLE_LEN_2_POWER);


//===============================================================================================================

ZCE_LIB::ZLZ_Compress_Format::ZLZ_Compress_Format()
{
    hash_lz_offset_ = new uint32_t[HASH_TABLE_LEN];
}

ZCE_LIB::ZLZ_Compress_Format::~ZLZ_Compress_Format()
{
    if (hash_lz_offset_)
    {
        delete[] hash_lz_offset_;
    }
}


//压缩的关键函数，内部函数，不对对外暴漏
void ZCE_LIB::ZLZ_Compress_Format::compress_core(const unsigned char *original_buf,
                                                 size_t original_size,
                                                 unsigned char *compressed_buf,
                                                 size_t *compressed_size)
{
    //初始化各种初始值
    const unsigned char *read_pos = original_buf;
    const unsigned char *read_end = original_buf + original_size;
    unsigned char *write_pos = compressed_buf;
    unsigned char *write_stop = NULL;

    //因为快速拷贝，一次处理8个字节，用16个字节保证不溢出，
    const unsigned char *match_end = read_end - ZCE_LZ_NOPROCESS_TAIL;

    //0xFFFFFFFF(-1)是一个不可能存在的值
    memset(hash_lz_offset_, 0, sizeof(uint32_t)*HASH_TABLE_LEN);

    const unsigned char *ref_offset = NULL;
    const unsigned char *nomatch_achor =  NULL;

    unsigned char *offset_token = NULL;

    size_t nomatch_count = 0, match_count = 0, match_offset = 0;

    for (;;)
    {


#if defined ZCE_LZ_DEBUG && ZCE_LZ_DEBUG==1
        ZCE_LOG(RS_DEBUG,
                "zlz no match size [%10u],match size [%10u],read len [%10u] ,write len[%10u],remain read[%10u] ",
                nomatch_count,
                match_count,
                read_pos - original_buf,
                write_pos - compressed_buf,
                original_size - (read_pos - original_buf)
               );
#endif

        //你可以认为ZCELZ算法的多个块组成，一个块中间有一个不能压缩字段（可选），一个可以压缩字段组成（可选），

        nomatch_achor = read_pos;

        nomatch_count = 0;
        match_count = 0;

        size_t step_len = 1;
        //等于(1 << ZCE_LZ_STEP_LEN_POW)+1
        //size_t step_attempts = 65;

        //如果不匹配
        for (;;)
        {
            //这个地方说明一下，如果table_old_offset == -1(0xFFFFFFFF)，那么也认为是没有匹配
            //每4个字节都区一次HASH值，然后查询是否有重复的数据
            uint32_t hash_value = ZCE_LZ_HASH(read_pos);
            //取出偏移量
            uint32_t table_old_offset = hash_lz_offset_[hash_value];
            //找到这个东西第一次出现的位置，
            ref_offset = original_buf + table_old_offset;
            //更新HASHTABLE
            hash_lz_offset_[hash_value] = (uint32_t)(read_pos - original_buf);
            match_offset = (size_t)(read_pos - ref_offset);

            //如果发现匹配,而且两者间的间距不大，（间距要2个字节表述，如果更长需要更多字节，那么就完全起不到压缩的效果了）
            //间距长度ZCE_LZ_MAX_OFFSET,用2个字节（或者一个字节）表述，
            if ((table_old_offset != 0)
                && ZBYTE_TO_UINT32(ref_offset) == ZBYTE_TO_UINT32(read_pos)
                && (match_offset) < ZCE_LZ_MAX_OFFSET)
            {
                break;
            }
            nomatch_count += step_len;

            //长度也是最大记录2个字节,
            //下一轮 step_len 可能还会自增，所以这儿是<,注意是0xFFDF，= 0xFFFF -32
            if (ZCE_UNLIKELY((nomatch_count > 0xFFDF) || (read_pos + step_len > match_end)))
            {
                ++read_pos;
                goto zlz_token_process;
            }
            //step_attempts++的目的是在长期发现无法压缩的情况下，
            //简单说，相当于，2^ZCE_LZ_STEP_LEN_POW次后，步进长度2,再2^ZCE_LZ_STEP_LEN_POW次后，步进变为3
            //当步进长度达到一定程度后，停止步进增加，
            //step_len = ((step_attempts++) >> ZCE_LZ_STEP_LEN_POW);
            //if (ZCE_UNLIKELY(step_len > ZCE_LZ_STEP_MAX_LEN))
            //{
            //    step_len = ZCE_LZ_STEP_MAX_LEN;
            //}

            read_pos += step_len;
        }

        //由于前面步进的速度是提速过的，如果步进长度>1，那么回头看看是否前面是否还有相等的
        //step_len > 1表示前面有跳动
        //if (step_len > 1)
        //{
            for (; read_pos > nomatch_achor && ref_offset > original_buf && ref_offset[-1] == read_pos[-1]; )
            {
                --ref_offset;
                --read_pos;
                --nomatch_count;
            }
        //}

        //步进4位，（前面发现了至少4个字节相等）
        //read_pos += 4;
        //ref_offset += 4;
        //match_count = 4;

        //持续的需找相等
        for (;;)
        {
            //0xFFF7是避免溢出
            if (ZCE_UNLIKELY((read_pos > match_end) || (match_count > 0xFFF7)) )
            {
                goto zlz_token_process;
            }

            //-----------------------------------------------------------------------------
            //下面这几段实现在不同的OS下快速的查询相等的数据，为了加速，代码分64位，32位处理，
            //（曾经尝试过在32位平台下用64位处理，速度差不多把）
            //理解比较复杂，本来打算写个宏，但感觉宏一样没法让人理解，认真写写注释把。
            uint32_t match_bytes = 0;

#if defined ZCE_OS64
            //64位平台，每次比较64bits
            uint64_t diff = ZBYTE_TO_UINT64(ref_offset) ^ ZBYTE_TO_UINT64(read_pos);
            //如果不等，用指令函数迅速得到有多少字节相等.
            if (!diff)
            {
                read_pos += sizeof(uint64_t);
                ref_offset += sizeof(uint64_t);
                match_count += sizeof(uint64_t);
                continue;
            }

            //如果是LINUX平台，用__builtin_ctzll,__builtin_clzll 指令得到最开始为1的位置，从而判定有多少个想同，
            //同时根据大头和小头平台使用不同的函数，小头用LBE to MBE ,大头用 MBE to LBE,
            //本来我对这个问题有点疑惑，其他压缩库代码处理MBE to LBE，后面LZ4的作者回复了我，（开源的都是好人），
            //这儿为了速度，我们取出64bit的数值作为longlong比较的时候，没有考虑字节序

            //右移3位是为了找到第几个字节不同
#if defined ZCE_LINUX64

#if defined ZCE_LITTLE_ENDIAN
            match_bytes += __builtin_ctzll(diff) >> 3;
#else
            match_bytes += __builtin_clzll(diff) >> 3;
#endif

            //WIN64平台，用_BitScanForward64，_BitScanReverse64得到最左边的那个bit为1的位置，其他参考LINUX那段注释
            //
#elif defined ZCE_WIN64

            unsigned long index = 0;
#if defined ZCE_LITTLE_ENDIAN
            _BitScanForward64(&index, diff);
#else
            _BitScanReverse64(&index, diff);
#endif
            match_bytes  += (index >> 3);

#endif //#if defined ZCE_WIN64

            //对于32位的系统进行处理，每次比较32bits，请参考64位LINUX的解释
#elif defined ZCE_OS32

            uint32_t diff = ZBYTE_TO_UINT32(ref_offset) ^ ZBYTE_TO_UINT32(read_pos);

            if (!diff)
            {
                read_pos += sizeof(uint32_t);
                ref_offset += sizeof(uint32_t);
                match_count += sizeof(uint32_t);
                continue;
            }

#if defined ZCE_LINUX32
#if defined ZCE_LITTLE_ENDIAN
            match_bytes += __builtin_ctzl(diff) >> 3;
#else
            match_bytes += __builtin_clzl(diff) >> 3;
#endif
#elif defined ZCE_WIN32
            unsigned long index = 0;
#if defined ZCE_LITTLE_ENDIAN
            _BitScanForward(&index, diff);
#else
            _BitScanReverse(&index, diff);
#endif
            match_bytes  += (index >> 3);
#endif

#else
#error "[Error]Code error,please check."
#endif

            match_count += match_bytes;
            read_pos += match_bytes;
            ref_offset += match_bytes;

            goto zlz_token_process;
        }
        //Never goto heer.


zlz_token_process:

        //块的最开始是一个字节的offset_token，TOKEN的高4bit表示非压缩长度，低四位表示压缩长度
        offset_token = (write_pos++);

        //接着保存未压缩数据的长度
        if (nomatch_count <  0xE)
        {
            *offset_token = (unsigned char)(nomatch_count);
        }
        //
        else
        {
            //不匹配的字节数可以用一个字节表述
            if (ZCE_UNLIKELY(nomatch_count <  0xFF))
            {
                //offset_token 填写为0xE,标识用扩展1字节字段标识长度
                *offset_token = 0xE;
                *write_pos = (uint8_t)(nomatch_count);
                ++write_pos;
            }
            else
            {
                //offset_token 填写为0xF,标识用扩展2字节字段标识长度
                *offset_token = 0xF;
                ZLEUINT16_TO_BYTE(write_pos, ((uint16_t)(nomatch_count)));
                write_pos += 2;
            }
        }

        if (match_count)
        {
            //保存
            hash_lz_offset_[ZCE_LZ_HASH(read_pos - 2)] = (uint32_t)(read_pos - original_buf - 2);

            //相同的情况下，最小长度是4，所以0表示0，1表示4
            if (match_count <  0xE + 0x3)
            {
                *offset_token |= (unsigned char)((match_count - 0x3) << 4);
            }
            else
            {
                if (match_count <  0xFF)
                {
                    //offset_token 填写为0xE,标识用扩展1字节字段标识长度
                    *offset_token |= (0xE << 4);
                    *write_pos = (uint8_t)(match_count);
                    ++write_pos;
                }
                else
                {
                    //offset_token 填写为0xF,标识用扩展2字节字段标识长度
                    *offset_token |= (0xF << 4);
                    ZLEUINT16_TO_BYTE(write_pos, ((uint16_t)(match_count)));
                    write_pos += 2;
                }
            }

            //写入偏移长度,前面已经计算过了
            //前面已经保证了read_pos和ref_offset 相差小于0xFFFF，2个字节足够
            ZLEUINT16_TO_BYTE(write_pos, ((uint16_t)(match_offset )));
            write_pos += 2;
        }

        //如果有没有压缩的数据
        //如果有不匹配的数据，用快速拷贝的方法进行拷贝。
        //memcpy选择的准则是 小于256，用ZCE_LZ_FAST_COPY，大于256,用memcpy,这个偷懒以及为了避免选择，直接用了快速拷贝
        // http://www.cnblogs.com/fullsail/p/3160098.html
        if (nomatch_count)
        {
            //每次用8字节进行复制，读取和写入都考虑了空余
            write_stop = write_pos + nomatch_count;
            ZCE_LZ_FAST_COPY_STOP(write_pos, nomatch_achor, write_stop);
            write_pos = write_stop;
        }

        //
        if (read_pos + step_len > match_end )
        {
            break;
        }
    }

#if defined ZCE_LZ_DEBUG && ZCE_LZ_DEBUG==1
    ZCE_LOG(RS_DEBUG,
            "zlz no match size [%10u],match size [%10u],read len [%10u] ,write len[%10u],remain read[%10u]",
            nomatch_count,
            match_count,
            read_pos - original_buf,
            write_pos - compressed_buf,
            original_size - (read_pos - original_buf)
           );
#endif

    //把最后几个字节(作为非压缩数据)拷贝到压缩数据里面
    size_t remain_len =  read_end - read_pos;
    offset_token = (write_pos++);
    if (remain_len <  0xE)
    {
        *offset_token = (unsigned char)(remain_len);
    }
    else
    {
        //不匹配的字节数可以用一个字节表述
        if (remain_len <  0xFF)
        {
            //offset_token 填写为0xE,标识用扩展1字节字段标识长度
            *offset_token = 0xE;
            *write_pos = (uint8_t)(remain_len);
            ++write_pos;
        }
        else
        {
            //offset_token 填写为0xF,标识用扩展2字节字段标识长度
            *offset_token = 0xF;
            ZLEUINT16_TO_BYTE(write_pos, ((uint16_t)(remain_len)));
            nomatch_count = 0;
            write_pos += 2;
        }
    }

    memcpy(write_pos, read_pos, remain_len);
    write_pos += remain_len;

    *compressed_size = write_pos - compressed_buf;

    //压缩完成
    return;
}


//解压缩的核心处理函数，如果你对压缩的格式了解，解压的代码应该容易理解，
int ZCE_LIB::ZLZ_Compress_Format::decompress_core(const unsigned char *compressed_buf,
                                                  size_t compressed_size,
                                                  unsigned char *original_buf,
                                                  size_t original_size)
{

    //初始化各种初始值
    const uint8_t *read_pos = compressed_buf;
    const uint8_t *read_end = compressed_buf + compressed_size;

    unsigned char *write_pos = original_buf;
    unsigned char *write_end = original_buf + original_size;
    unsigned char *write_stop = NULL;
    const unsigned char *read_stop = NULL;

    unsigned char offset_token = 0;
    const unsigned char *ref_pos = NULL;

    size_t noncomp_count = 0;
    size_t comp_count = 0;
    size_t ref_offset = 0;

    for (;;)
    {

#if defined ZCE_LZ_DEBUG && ZCE_LZ_DEBUG==1

        ZCE_LOG(RS_DEBUG,
                "zlz no match size [%10u],match size [%10u],read len [%10u] ,write len[%10u]."
                "remain_read[%10u],remain_write[%10u]",
                noncomp_count,
                comp_count,
                read_pos - compressed_buf,
                write_pos - original_buf,
                read_end - read_pos,
                write_end - write_pos);
#endif

        //块的最开始是一个字节的offset_token-
        if ( ZCE_UNLIKELY(read_end - read_pos < 8 ))
        {
            return -1;
        }

        //取得各种长度，做运算
        offset_token = *(read_pos++);
        noncomp_count = (offset_token & 0x0F) ;
        comp_count = (offset_token & 0xF0) >> 4;

        if (noncomp_count)
        {
            //TOKEN的值0xE,表示用一个字节表示长度，0xF表示2个字节表示长度，否则其值就是长度
            if (noncomp_count == 0xE)
            {
                noncomp_count = (uint8_t)(*(read_pos));
                ++read_pos ;
            }
            else if (noncomp_count == 0xF)
            {
                noncomp_count = (uint16_t)(ZBYTE_TO_LEUINT16(read_pos));
                read_pos += 2;
            }

            //已经到了最后一个block，
            if ( ZCE_UNLIKELY( noncomp_count == (size_t)(read_end - read_pos) ))
            {
                break;
            }
        }

        //如果TOKEN表示压缩部分的长度是0，表示没有压缩，

        //如果表示为小于0xE,长度等于0xE + 3(如果匹配。最小长度是4)
        if (comp_count)
        {
            if ( ZCE_LIKELY( comp_count <  0xE ) )
            {
                comp_count += 0x3;
            }
            else if (comp_count == 0xE)
            {
                comp_count = (uint8_t)(*(read_pos));
                ++read_pos ;
            }
            else if (comp_count == 0xF)
            {
                comp_count = (uint16_t)(ZBYTE_TO_LEUINT16(read_pos));
                read_pos += 2;
            }
            //取得偏移地址
            ref_offset = (uint16_t)(ZBYTE_TO_LEUINT16(read_pos));
            read_pos += 2;

        }

        //如果偏移地址错误，返回错误，注意compressed_buf 第一个字节也是非参考字节，
        //偏移的长度不可能大于写位置和头位置的差  +8 因为这种复制风格，所以要留有8字节的间距，
        //保证有这些空间可读，
        if ( ZCE_UNLIKELY( (size_t)(read_end - read_pos) < ( 8 + noncomp_count)
                           || ((size_t)(write_end - write_pos) < ( 8 + noncomp_count + comp_count))  ) )
        {
            return -1;
        }

        //如果非压缩数据数据长度> 0,拷贝没有压缩的字符串到source
        if ( noncomp_count > 0 )
        {
            write_stop = write_pos + noncomp_count;
            read_stop = read_pos + noncomp_count;
            ZCE_LZ_FAST_COPY_STOP(write_pos, read_pos, write_stop);
            read_pos = read_stop;
            write_pos = write_stop;
        }

        //
        if (comp_count > 0)
        {
            ref_pos = write_pos - ref_offset;

            if ( ref_pos == write_pos || ref_pos  < original_buf  )
            {
                return -1;
            }

            //另外这个地方，用memcpy是不合适的，因为地址可能有交叠
            write_stop = write_pos + comp_count;
            if (ZCE_LIKELY( ref_offset >= sizeof(uint64_t) ) )
            {
                ZCE_LZ_FAST_COPY_STOP(write_pos, ref_pos , write_stop);
            }
            //参考的位置和当前的位置之间不足8个字节，有交错，这儿要进行特殊处理了。
            else
            {
                //即使长度不到8个字节，还是一次拷贝了8个字节，否则这儿的处理还是比较啰嗦的，
                write_pos[0] = ref_pos[0];
                write_pos[1] = ref_pos[1];
                write_pos[2] = ref_pos[2];
                write_pos[3] = ref_pos[3];
                write_pos[4] = ref_pos[4];
                write_pos[5] = ref_pos[5];
                write_pos[6] = ref_pos[6];
                write_pos[7] = ref_pos[7];

                //如果相等的数量超过8
                if (comp_count > sizeof(uint64_t))
                {
                    //第一个0没有意义，因为offset不可能为0，
                    //其他数据的意义是，因为希望进行8字节盯得快速拷贝，希望源和相对数据的长度差别大于8字节，
                    //那么就要找到一个在这种情况下重复开始的规律
                    static const size_t POS_MOVE_REFER[] = {0, 0, 2, 2, 0, 3, 2, 1};
                    ref_pos +=  POS_MOVE_REFER[ref_offset];
                    unsigned char *match_write = (write_pos + 8);
                    ZCE_LZ_FAST_COPY_STOP(match_write, ref_pos , write_stop);
                }
            }
            write_pos = write_stop;
        }
    }

    //如果空间不够，还是返回错误
    if ( ZCE_UNLIKELY( (size_t)(read_pos - read_end) < noncomp_count
                       || (size_t)(write_end - write_pos) < noncomp_count ) )
    {
        return -1;
    }

    ::memcpy(write_pos, read_pos, noncomp_count);

    //成功解压
    return 0;
}

//===============================================================================================================

ZCE_LIB::LZ4_Compress_Format::LZ4_Compress_Format()
{
    hash_lz_offset_ = new uint32_t[HASH_TABLE_LEN];
}

ZCE_LIB::LZ4_Compress_Format::~LZ4_Compress_Format()
{
    if (hash_lz_offset_)
    {
        delete[] hash_lz_offset_;
    }
}

//压缩的关键函数，内部函数，
//模仿LZ4的算法的格式进行的函数，
void ZCE_LIB::LZ4_Compress_Format::compress_core(const unsigned char *original_buf,
                                                 size_t original_size,
                                                 unsigned char *compressed_buf,
                                                 size_t *compressed_size)
{
    //初始化各种初始值
    const unsigned char *read_pos = original_buf;
    const unsigned char *next_read_pos = NULL;
    const unsigned char *read_end = original_buf + original_size;
    unsigned char *write_pos = compressed_buf;
    unsigned char *write_stop = NULL;

    //因为快速拷贝，比较等，用16个字节保证不溢出，
    const unsigned char *match_end = read_end - ZCE_LZ_NOPROCESS_TAIL;

    //清0
    memset(hash_lz_offset_, 0, sizeof(uint32_t)*HASH_TABLE_LEN);

    const unsigned char *ref_offset = NULL;
    const unsigned char *nomatch_achor =  NULL, *match_achor = NULL;
    unsigned char *offset_token = NULL;

    size_t match_offset , nomatch_count = 0, match_count = 0;
    nomatch_achor = read_pos;
    hash_lz_offset_[ZCE_LZ_HASH(read_pos)] = (uint32_t)(read_pos - original_buf);
    ++read_pos;
    //处理每一个要压缩的字节
    for (;;)
    {

#if defined ZCE_LZ_DEBUG && ZCE_LZ_DEBUG==1
        ZCE_LOG(RS_DEBUG,
                "lz4 compress no match size [%10u],match size [%10u],read len [%10u] ,write len[%10u],remain read[%10u] ",
                nomatch_count ,
                match_count ,
                read_pos - original_buf,
                write_pos - compressed_buf,
                original_size - (read_pos - original_buf)
               );
#endif

        //你可以认为ZLZ算法的多个块组成，一个块中间有一个不能压缩字段（可选），一个可以压缩字段组成（可选），

        //nomatch_achor = read_pos;

        //等于(1 << ZCE_LZ_STEP_LEN_POW)+1
        size_t step_attempts = 67;

        //找到一个Token（包括可以压缩的数据和不可以压缩的数据）
        next_read_pos = read_pos;
        //如果不匹配
        do
        {
            read_pos = next_read_pos;
            //这个地方说明一下，如果table_old_offset == 0，那么也认为是没有匹配
            uint32_t hash_value = ZCE_LZ_HASH(read_pos);

            uint32_t table_old_offset = hash_lz_offset_[hash_value];
            ref_offset = original_buf + table_old_offset;
            hash_lz_offset_[hash_value] = (uint32_t)(read_pos - original_buf);
            match_offset = (size_t)(read_pos - ref_offset);

            //step_attempts++的目的是在长期发现无法压缩的情况下，
            //相当于，2^ZCE_LZ_STEP_LEN_POW次后，步进长度2,再2^ZCE_LZ_STEP_LEN_POW次后，步进变为3
            //当步进长度达到一定程度后，停止步进增加，
            size_t step_len = ((step_attempts++) >> ZCE_LZ_STEP_LEN_POW);

            //如果到了最后，跳到最后的处理，注意这儿和ZLZ的算法有区别
            //ZLZ的算法是跳入Token处理，ZLZ的算法每个Token内无法压缩的数据区长度是有限制的
            //LZ4的算法对于每个Token无法压缩的数据区长度没有限制,所以这个地方直接跳入了最后
            next_read_pos = read_pos + step_len;
            if (ZCE_UNLIKELY(next_read_pos > match_end))
            {
                goto lz4_end_process;
            }
            //如果发现匹配,而且两者间的间距不大，（间距要2个字节表述，如果更长需要更多字节，那么就完全起不到压缩的效果了）
            //间距长度ZCE_LZ_MAX_OFFSET,用2个字节（或者一个字节）表述，
        } while (ZBYTE_TO_UINT32(ref_offset) != ZBYTE_TO_UINT32(read_pos)
                 || (match_offset) >= ZCE_LZ_MAX_OFFSET);


        //因为其实前面做过HASH检查，所以其实如果step_len 等于1的时候，前面还有相等的情况是很少的，但确实存在
        while (read_pos > nomatch_achor && ref_offset > original_buf && ref_offset[-1] ==  read_pos[-1])
        {
            --ref_offset ;
            --read_pos ;
        }
        nomatch_count = read_pos - nomatch_achor;

        //块的最开始是一个字节的offset_token，TOKEN的高4bit表示非压缩长度，低四位表示压缩长度
        offset_token = (write_pos++);

        //
        if (ZCE_LIKELY(nomatch_count < 0xF))
        {
            *offset_token = (unsigned char)(nomatch_count);
        }
        else
        {
            size_t remain_len = nomatch_count - 0xF;
            *offset_token = 0x0F;
            for (; remain_len >= 0xFF; remain_len -= 0xFF)
            {
                *write_pos++ = 0xFF;
            }
            *write_pos++ = (unsigned char)remain_len;
        }

        //如果有没有压缩的数据
        //如果有不匹配的数据，用快速拷贝的方法进行拷贝。
        //memcpy选择的准则是 小于256，用ZCE_LZ_FAST_COPY，大于256,用memcpy,这个偷懒以及为了避免选择，直接用了快速拷贝
        // http://www.cnblogs.com/fullsail/p/3160098.html

        //每次用8字节进行复制，读取和写入都考虑了空余
        write_stop = write_pos + nomatch_count;
        ZCE_LZ_FAST_COPY_STOP(write_pos, nomatch_achor, write_stop);
        write_pos = write_stop;

lz4_match_process:
        //到这儿来了就是发现有(至少)4字节的匹配了，
        match_achor = read_pos;
        read_pos += 4;
        ref_offset += 4;

        //快速的找出有多少数据是相同的，
        for (;;)
        {
            if (ZCE_UNLIKELY((read_pos > match_end) ) )
            {
                break;
            }


#if defined ZCE_OS64
            //64位平台，每次比较64bits
            uint64_t diff = ZBYTE_TO_UINT64(ref_offset) ^ ZBYTE_TO_UINT64(read_pos);
            if (!diff)
            {
                read_pos += 8;
                ref_offset += 8;
                continue;
            }
            //如果不等，用指令函数迅速得到有多少字节相等.
            //-----------------------------------------------------------------------------
            //下面这几段比较复杂，本来打算写个宏，但感觉宏一样没法让人理解，认真写写注释把。
            //为了加速，代码分64位，32位处理，（曾经尝试过在32位平台下用64位处理，速度差不多把）
            uint32_t tail_match = 0;
            //如果是LINUX平台，用__builtin_ctzll,__builtin_clzll 指令得到最开始为1的位置，从而判定有多少个想同，
            //同时根据大头和小头平台使用不同的函数，小头用LBE to MBE ,大头用 MBE to LBE,
            //本来我对这个问题有点异或，其他压缩库代码处理MBE to LBE，后面LZ4的作者回复了我，（开源的都是好人），
            //这儿为了速度，我们取出64bit的数值作为longlong比较的时候，没有考虑字节序
#if defined ZCE_LINUX64

#if defined ZCE_LITTLE_ENDIAN
            tail_match += __builtin_ctzll(diff) >> 3;
#else
            tail_match += __builtin_clzll(diff) >> 3;
#endif

            //WIN64平台，用_BitScanForward64，_BitScanReverse64得到最左边的那个bit为1的位置，其他参考LINUX那段注释
            //
#elif defined ZCE_WIN64

            unsigned long index = 0;
#if defined ZCE_LITTLE_ENDIAN
            _BitScanForward64(&index, diff);
#else
            _BitScanReverse64(&index, diff);
#endif
            tail_match  += (index >> 3);

#endif //#if defined ZCE_WIN64

            //对于32位的系统进行处理，每次比较32bits，请参考64位LINUX的解释
            //我的测试感觉是如果32位系统还是用32位处理，会快一点点点点点。
#elif defined ZCE_OS32

            uint32_t diff = ZBYTE_TO_UINT32(ref_offset) ^ ZBYTE_TO_UINT32(read_pos);

            if (!diff)
            {
                read_pos += 4;
                ref_offset += 4;
                continue;
            }

#if defined ZCE_LINUX32
#if defined ZCE_LITTLE_ENDIAN
            tail_match += __builtin_ctzl(diff) >> 3;
#else
            tail_match += __builtin_clzl(diff) >> 3;
#endif
#elif defined ZCE_WIN32
            unsigned long index = 0;
#if defined ZCE_LITTLE_ENDIAN
            _BitScanForward(&index, diff);
#else
            _BitScanReverse(&index, diff);
#endif
            tail_match  += (index >> 3);
#endif

#else
#error "[Error]Code error,please check."
#endif

            read_pos += tail_match;
            ref_offset += tail_match;
            match_count = read_pos - match_achor;
            break;
        }

        //
        match_count = read_pos - match_achor;
        if (ZCE_LIKELY(match_count < 0xF))
        {
            *offset_token |= (unsigned char)(match_count) << 4;
        }
        else
        {
            size_t remain_len = match_count - 0xF;
            *offset_token |= 0xF0;
            for (; remain_len >= 0xFF ; remain_len -= 0xFF)
            {
                *write_pos++ = 0xFF;
            }
            *write_pos++ = (unsigned char )remain_len;
        }

        //写入偏移长度,
        match_offset = read_pos - ref_offset ;

        //前面已经保证了read_pos和ref_offset 相差小于0xFFFF，2个字节足够
        ZLEUINT16_TO_BYTE(write_pos, ((uint16_t)(match_offset )));
        write_pos += 2;

        if (ZCE_UNLIKELY((read_pos > match_end) ) )
        {
            nomatch_achor = read_pos;
            break;
        }

        // Fill table
        hash_lz_offset_[ZCE_LZ_HASH((read_pos - 2))] = 
            (uint32_t)(read_pos - 2 - original_buf);
        // Test next position
        ref_offset = original_buf + hash_lz_offset_[ZCE_LZ_HASH(read_pos)];
        hash_lz_offset_[ZCE_LZ_HASH(read_pos)] = (uint32_t)(read_pos - original_buf);

        if ((ref_offset -read_pos >= ZCE_LZ_MAX_OFFSET) && (ZBYTE_TO_UINT32(ref_offset) == ZBYTE_TO_UINT32(read_pos)))
        {
            offset_token =  write_pos ++;
            *offset_token = 0;
            goto lz4_match_process;
        }
        nomatch_achor = read_pos++;
    }

lz4_end_process:

    //把最后几个字节(作为非压缩数据)拷贝到压缩数据里面
    nomatch_count = read_end - nomatch_achor;
    match_count = 0;

#if defined ZCE_LZ_DEBUG && ZCE_LZ_DEBUG==1
    ZCE_LOG(RS_DEBUG,
            "lz4 compress no match size [%10u],match size [%10u],read len [%10u] ,write len[%10u],remain read[%10u]",
            nomatch_count ,
            match_count ,
            read_pos - original_buf,
            write_pos - compressed_buf,
            original_size - (read_pos - original_buf)
           );
#endif

    offset_token = (write_pos++);
    if (ZCE_LIKELY(nomatch_count < 0xF))
    {
        *offset_token = (unsigned char)(nomatch_count);
    }
    else
    {
        size_t remain_len = nomatch_count - 0xF;
        *offset_token = 0xF;
        for (; remain_len >= 0xFF ; remain_len -= 0xFF)
        {
            *write_pos++ = 0xFF;
        }
        *write_pos++ = (unsigned char )remain_len;
    }

    memcpy(write_pos, nomatch_achor, nomatch_count);
    write_pos += nomatch_count;

    *compressed_size = write_pos - compressed_buf;

    //压缩完成
    return;
}


//解压缩的核心处理函数，如果你对压缩的格式了解，解压的代码应该容易理解，
int ZCE_LIB::LZ4_Compress_Format::decompress_core(const unsigned char *compressed_buf,
                                                  size_t compressed_size,
                                                  unsigned char *original_buf,
                                                  size_t original_size)
{

    //初始化各种初始值
    const unsigned char *read_pos = compressed_buf;
    const unsigned char *read_end = compressed_buf + compressed_size;

    unsigned char *write_pos = original_buf;
    unsigned char *write_end = original_buf + original_size;
    unsigned char *write_stop = NULL;
    const unsigned char *read_stop = NULL;

    unsigned char offset_token = 0;
    const unsigned char *ref_pos = NULL;

    size_t noncomp_count = 0;
    size_t comp_count = 0;
    size_t ref_offset = 0;

    for (;;)
    {
#if defined ZCE_LZ_DEBUG && ZCE_LZ_DEBUG==1

        ZCE_LOG(RS_DEBUG,
                "lz4 no match size [%10u],match size [%10u],read len [%10u] ,write len[%10u]."
                "remain_read[%10u],remain_write[%10u]",
                noncomp_count,
                comp_count,
                read_pos - compressed_buf,
                write_pos - original_buf,
                read_end - read_pos,
                write_end - write_pos);
#endif


        //取得各种长度，做运算
        offset_token = *(read_pos++);
        noncomp_count = (offset_token & 0x0F) ;
        comp_count = (offset_token & 0xF0) >> 4;

        if ( noncomp_count == 0xF)
        {
            size_t remain_len = 255;
            while ( (read_pos < read_end) && (remain_len == 255))
            {
                remain_len = *read_pos++;
                noncomp_count += remain_len;
            }
        }

        //已经到了最后一个block，
        if (ZCE_UNLIKELY(noncomp_count == (size_t)(read_end - read_pos)))
        {
            break;
        }

        //拷贝没有压缩的字符串到source, 这儿为了加快速度，不考虑noncomp_count 是否为0
        write_stop = write_pos + noncomp_count;
        read_stop = read_pos + noncomp_count;
        ZCE_LZ_FAST_COPY_STOP(write_pos, read_pos, write_stop);
        read_pos = read_stop;
        write_pos = write_stop;

        if ( comp_count == 0xF)
        {
            size_t remain_len = 255;
            while ( (read_pos < read_end) && (remain_len == 255))
            {
                remain_len = *read_pos++;
                comp_count += remain_len;
            }
        }

        //如果偏移地址错误，返回错误，注意compressed_buf 第一个字节也是非参考字节，
        //偏移的长度不可能大于写位置和头位置的差  +8 因为这种复制风格，所以要留有8字节的间距，
        //保证有这些空间可读，
        //if ( ZCE_LIKELY( (size_t)(read_end - read_pos) < (2 + 8 + noncomp_count)
        //                 || ((size_t)(write_end - write_pos) < ( 8 + noncomp_count + comp_count))  ) )
        //{
        //    return -1;
        //}

        //取得偏移地址
        ref_offset = (uint16_t)(ZBYTE_TO_LEUINT16(read_pos));
        read_pos += 2;

        ref_pos = write_pos - ref_offset;
        //if ( ref_pos == write_pos || ref_pos  < original_buf  )
        //{
        //    return -1;
        //}

        //另外这个地方，用memcpy是不合适的，因为地址可能有交叠
        write_stop = write_pos + comp_count;
        if (ZCE_LIKELY( ref_offset >= sizeof(uint64_t) ) )
        {
            ZCE_LZ_FAST_COPY_STOP(write_pos, ref_pos , write_stop);
        }
        //这儿要进行特殊处理了。
        else
        {
            //即使长度不到8个字节，还是一次拷贝了8个字节，否则这儿的处理还是比较啰嗦的，
            write_pos[0] = ref_pos[0];
            write_pos[1] = ref_pos[1];
            write_pos[2] = ref_pos[2];
            write_pos[3] = ref_pos[3];
            write_pos[4] = ref_pos[4];
            write_pos[5] = ref_pos[5];
            write_pos[6] = ref_pos[6];
            write_pos[7] = ref_pos[7];

            //如果相等的数量超过8
            if (comp_count > sizeof(uint64_t))
            {
                //第一个0没有意义，因为offset不可能为0，
                //其他数据的意义是，因为希望进行8字节盯得快速拷贝，希望源和相对数据的长度差别大于8字节，
                //那么就要找到一个在这种情况下重复开始的规律
                static const size_t POS_MOVE_REFER[] = {0, 0, 2, 2, 0, 3, 2, 1};
                ref_pos +=  POS_MOVE_REFER[ref_offset];
                unsigned char *match_write = (write_pos + 8);
                ZCE_LZ_FAST_COPY_STOP(match_write, ref_pos , write_stop);
            }
        }
        write_pos = write_stop;

    }

    //如果空间不够，还是返回错误
    if ( ZCE_UNLIKELY( (size_t)(read_pos - read_end) < noncomp_count
                       || (size_t)(write_end - write_pos) < noncomp_count ) )
    {
        return -1;
    }

    ::memcpy(write_pos, read_pos, noncomp_count);

    //成功解压
    return 0;
}



#if defined (ZCE_OS_WINDOWS)
#pragma warning ( pop )
#endif



