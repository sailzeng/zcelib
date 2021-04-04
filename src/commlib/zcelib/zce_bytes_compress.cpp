#include "zce_predefine.h"
#include "zce_log_logging.h"
#include "zce_os_adapt_string.h"
#include "zce_bytes_compress.h"

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( push )
#pragma warning ( disable : 4127)
#endif

//������궨�壬�����ӵ�����Ϣ����ÿ��ѹ��������м�¼
#define  ZCE_LZ_DEBUG 0

//ʹ��HASH������ֻ������λ13λ����Ϣ���������ƶ�19λ
//����һ��((4*8)-13)������Ĵ�С�� 8192��������2^13��
#define ZCE_LZ_HASH(ptr)       (((*(uint32_t *)(ptr)) *2654435761U)  >> ((4*8)-13))

//��Ҫ��װdst���㹻�Ŀռ䣬����8�ֽڲ���ġ�
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


//���Լ�¼�����ƫ�Ƴ��ȣ�ƫ�Ƴ��Ȳ����ܼ�¼�ĸ���
const size_t  ZCE_LZ_MAX_OFFSET  = 0xFFFF;

//�������ȵ��ݣ�1<<6,64��
const size_t ZCE_LZ_STEP_LEN_POW  = 6;
//���Ĳ������ȣ�
//LZ4�Ĳ��������ǲ�����ģ������������ܷǳ��죬��Ҳ�п���������ĳЩ��������޷�ѹ��
const size_t ZCE_LZ_STEP_MAX_LEN  = 32;


//β���������ģ�飬��Ϊ�Ƚϵĳ�����8�������ٿ����ĳ���Ҳ��8�����Ա���16���ֽ��ǰ�ȫ��
const size_t ZCE_LZ_NOPROCESS_TAIL = 8 * 2;

//HASHTABLE���ȵ���2���ٴη�
const uint32_t HASH_TABLE_LEN_2_POWER = 13;
//HASHTABLE�ĳ���
const size_t HASH_TABLE_LEN = 0x1 << (HASH_TABLE_LEN_2_POWER);


//===============================================================================================================

zce::ZLZ_Compress_Format::ZLZ_Compress_Format()
{
    hash_lz_offset_ = new uint32_t[HASH_TABLE_LEN];
}

zce::ZLZ_Compress_Format::~ZLZ_Compress_Format()
{
    if (hash_lz_offset_)
    {
        delete[] hash_lz_offset_;
    }
}


//ѹ���Ĺؼ��������ڲ����������Զ��Ⱪ©
void zce::ZLZ_Compress_Format::compress_core(const unsigned char *original_buf,
                                             size_t original_size,
                                             unsigned char *compressed_buf,
                                             size_t *compressed_size)
{
    //��ʼ�����ֳ�ʼֵ
    const unsigned char *read_pos = original_buf;
    const unsigned char *read_end = original_buf + original_size;
    const unsigned char *next_read_pos = NULL;
    unsigned char *write_pos = compressed_buf;
    unsigned char *write_stop = NULL;

    //��Ϊ���ٿ�����һ�δ���8���ֽڣ���16���ֽڱ�֤�������
    const unsigned char *match_end = read_end - ZCE_LZ_NOPROCESS_TAIL;

    //�����0
    memset(hash_lz_offset_, 0, sizeof(uint32_t)*HASH_TABLE_LEN);

    const unsigned char *ref_offset = NULL;
    const unsigned char *nomatch_achor =  NULL;

    unsigned char *offset_token = NULL;

    size_t nomatch_count = 0, match_count = 0, match_offset = 0;

    for (;;)
    {

        //�������ΪZCELZ�㷨�Ķ������ɣ�һ�����м���һ������ѹ���ֶΣ���ѡ����һ������ѹ���ֶ���ɣ���ѡ����

        nomatch_achor = read_pos;
        nomatch_count = 0;
        match_count = 0;

        size_t step_len = 1;
        //����(1 << ZCE_LZ_STEP_LEN_POW)+1
        size_t step_attempts = 65;
        next_read_pos = read_pos;
        //�����ƥ��
        do
        {
            //����Ҳ������¼2���ֽ�,
            //��һ�� step_len ���ܻ������������������<,ע����0xFFDF��= 0xFFFF -32
            if (ZCE_UNLIKELY(read_pos - nomatch_achor > 0xFFDF))
            {
                goto zlz_token_process;
            }
            if (ZCE_UNLIKELY(read_pos > match_end))
            {
                goto zlz_end_process;
            }
            read_pos = next_read_pos;
            //����ط�˵��һ�£����table_old_offset == 0����ôҲ��Ϊ��û��ƥ��
            uint32_t hash_value = ZCE_LZ_HASH(read_pos);

            uint32_t table_old_offset = hash_lz_offset_[hash_value];
            ref_offset = original_buf + table_old_offset;
            hash_lz_offset_[hash_value] = (uint32_t)(read_pos - original_buf);
            match_offset = (size_t)(read_pos - ref_offset);

            //step_attempts++��Ŀ�����ڳ��ڷ����޷�ѹ��������£�
            //�൱�ڣ�2^ZCE_LZ_STEP_LEN_POW�κ󣬲�������2,��2^ZCE_LZ_STEP_LEN_POW�κ󣬲�����Ϊ3
            //���������ȴﵽһ���̶Ⱥ�ֹͣ�������ӣ�
            step_len = ((step_attempts++) >> ZCE_LZ_STEP_LEN_POW);
            if (ZCE_UNLIKELY(step_len > ZCE_LZ_STEP_MAX_LEN))
            {
                step_len = ZCE_LZ_STEP_MAX_LEN;
            }
            //�����������������Ĵ���ע�������ZLZ���㷨������
            //ZLZ���㷨������Token����ZLZ���㷨ÿ��Token���޷�ѹ���������������������Ƶ�
            //LZ4���㷨����ÿ��Token�޷�ѹ��������������û������,��������ط�ֱ�����������
            next_read_pos = read_pos + step_len;


        }
        while (ZBYTE_TO_UINT32(ref_offset) != ZBYTE_TO_UINT32(read_pos)
               || (match_offset) >= ZCE_LZ_MAX_OFFSET
               || match_offset == 0 );

        while (read_pos > nomatch_achor && ref_offset > original_buf && ref_offset[-1] ==  read_pos[-1])
        {
            --ref_offset;
            --read_pos;
        }
        nomatch_count = read_pos - nomatch_achor;
        //����4λ����ǰ�淢��������4���ֽ���ȣ�
        //read_pos += 4;
        //ref_offset += 4;
        //match_count+= 4;

        //�������������
        for (;;)
        {
            //0xFFF7�Ǳ������
            if (ZCE_UNLIKELY((read_pos > match_end) || (match_count > 0xFFF7)) )
            {
                goto zlz_token_process;
            }

            //-----------------------------------------------------------------------------
            //�����⼸��ʵ���ڲ�ͬ��OS�¿��ٵĲ�ѯ��ȵ����ݣ�Ϊ�˼��٣������64λ��32λ����
            //���������Թ���32λƽ̨����64λ�����ٶȲ��ѣ�
            //���Ƚϸ��ӣ���������д���꣬���о���һ��û��������⣬����ддע�Ͱѡ�
            uint32_t tail_match = 0;

#if defined ZCE_OS64
            //64λƽ̨��ÿ�αȽ�64bits
            uint64_t diff = ZBYTE_TO_UINT64(ref_offset) ^ ZBYTE_TO_UINT64(read_pos);
            //������ȣ���ָ���Ѹ�ٵõ��ж����ֽ����.
            if (!diff)
            {
                read_pos += sizeof(uint64_t);
                ref_offset += sizeof(uint64_t);
                match_count += sizeof(uint64_t);
                continue;
            }

            //�����LINUXƽ̨����__builtin_ctzll,__builtin_clzll ָ��õ��ʼΪ1��λ�ã��Ӷ��ж��ж��ٸ���ͬ��
            //ͬʱ���ݴ�ͷ��Сͷƽ̨ʹ�ò�ͬ�ĺ�����Сͷ��LBE to MBE ,��ͷ�� MBE to LBE,
            //�����Ҷ���������е��ɻ�����ѹ������봦��MBE to LBE������LZ4�����߻ظ����ң�����Դ�Ķ��Ǻ��ˣ���
            //���Ϊ���ٶȣ�����ȡ��64bit����ֵ��Ϊlonglong�Ƚϵ�ʱ��û�п����ֽ���

            //����3λ��Ϊ���ҵ��ڼ����ֽڲ�ͬ
#if defined ZCE_LINUX64

#if defined ZCE_LITTLE_ENDIAN
            tail_match += __builtin_ctzll(diff) >> 3;
#else
            tail_match += __builtin_clzll(diff) >> 3;
#endif

            //WIN64ƽ̨����_BitScanForward64��_BitScanReverse64�õ�����ߵ��Ǹ�bitΪ1��λ�ã������ο�LINUX�Ƕ�ע��
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

            //����32λ��ϵͳ���д���ÿ�αȽ�32bits����ο�64λLINUX�Ľ���
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
            match_count += tail_match;
            break;
        }


zlz_token_process:

        //����ʼ��һ���ֽڵ�offset_token��TOKEN�ĸ�4bit��ʾ��ѹ�����ȣ�����λ��ʾѹ������
        offset_token = (write_pos++);

        //���ű���δѹ�����ݵĳ���
        if (nomatch_count <  0xE)
        {
            *offset_token = (unsigned char)(nomatch_count);
        }
        else
        {
            //��ƥ����ֽ���������һ���ֽڱ���
            if (ZCE_UNLIKELY(nomatch_count <  0xFF))
            {
                //offset_token ��дΪ0xE,��ʶ����չ1�ֽ��ֶα�ʶ����
                *offset_token = 0xE;
                *write_pos = (uint8_t)(nomatch_count);
                ++write_pos;
            }
            else
            {
                //offset_token ��дΪ0xF,��ʶ����չ2�ֽ��ֶα�ʶ����
                *offset_token = 0xF;
                ZLEUINT16_TO_BYTE(write_pos, ((uint16_t)(nomatch_count)));
                write_pos += 2;
            }
        }

        if (match_count)
        {
            //��ͬ������£���С������4������0��ʾ0��1��ʾ4������ط�ZLZ��LZ4�Ĵ��벻̫һ��,
            if (match_count <  0xE + 0x3)
            {
                *offset_token |= (unsigned char)((match_count - 0x3) << 4);
            }
            else
            {
                if (match_count <  0xFF)
                {
                    //offset_token ��дΪ0xE,��ʶ����չ1�ֽ��ֶα�ʶ����
                    *offset_token |= (0xE << 4);
                    *write_pos = (uint8_t)(match_count);
                    ++write_pos;
                }
                else
                {
                    //offset_token ��дΪ0xF,��ʶ����չ2�ֽ��ֶα�ʶ����
                    *offset_token |= (0xF << 4);
                    ZLEUINT16_TO_BYTE(write_pos, ((uint16_t)(match_count)));
                    write_pos += 2;
                }
            }
            //д��ƫ�Ƴ���,ǰ���Ѿ��������
            //ǰ���Ѿ���֤��read_pos��ref_offset ���С��0xFFFF��2���ֽ��㹻
            ZLEUINT16_TO_BYTE(write_pos, ((uint16_t)(match_offset )));
            write_pos += 2;
        }

        //�����û��ѹ��������
        if (nomatch_count)
        {
            //����в�ƥ������ݣ��ÿ��ٿ����ķ������п�����
            //memcpyѡ���׼���� С��256����ZCE_LZ_FAST_COPY������256,��memcpy,���͵���Լ�Ϊ�˱���ѡ��ֱ�����˿��ٿ���
            //ÿ����8�ֽڽ��и��ƣ���ȡ��д�붼�����˿���  http://www.cnblogs.com/fullsail/p/3160098.html
            write_stop = write_pos + nomatch_count;
            ZCE_LZ_FAST_COPY_STOP(write_pos, nomatch_achor, write_stop);
            write_pos = write_stop;
        }

#if defined ZCE_LZ_DEBUG && ZCE_LZ_DEBUG==1
        ZCE_LOG(RS_DEBUG,
                "zlz compress no match size [%10u],match size [%10u],read len [%10u] ,write len[%10u],remain read[%10u]",
                nomatch_count,
                match_count,
                read_pos - original_buf,
                write_pos - compressed_buf,
                original_size - (read_pos - original_buf)
               );
#endif
    }

zlz_end_process:

    //����󼸸��ֽ�(��Ϊ��ѹ������)������ѹ����������
    read_pos = nomatch_achor;
    size_t remain_len =  read_end - read_pos;
    offset_token = (write_pos++);
    if (remain_len <  0xE)
    {
        *offset_token = (unsigned char)(remain_len);
    }
    else
    {
        //��ƥ����ֽ���������һ���ֽڱ���
        if (remain_len <  0xFF)
        {
            //offset_token ��дΪ0xE,��ʶ����չ1�ֽ��ֶα�ʶ����
            *offset_token = 0xE;
            *write_pos = (uint8_t)(remain_len);
            ++write_pos;
        }
        else
        {
            //offset_token ��дΪ0xF,��ʶ����չ2�ֽ��ֶα�ʶ����
            *offset_token = 0xF;
            ZLEUINT16_TO_BYTE(write_pos, ((uint16_t)(remain_len)));
            nomatch_count = 0;
            write_pos += 2;
        }
    }

    //ע������õ���memcpyร����������Խ�磬���Բ�����ZCE_LZ_FAST_COPY_STOP
    memcpy(write_pos, read_pos, remain_len);
    write_pos += remain_len;

    *compressed_size = write_pos - compressed_buf;

    //ѹ�����
    return;
}


//��ѹ���ĺ��Ĵ�������������ѹ���ĸ�ʽ�˽⣬��ѹ�Ĵ���Ӧ��������⣬
int zce::ZLZ_Compress_Format::decompress_core(const unsigned char *compressed_buf,
                                              size_t compressed_size,
                                              unsigned char *original_buf,
                                              size_t original_size)
{

    //��ʼ�����ֳ�ʼֵ
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
        //����ʼ��һ���ֽڵ� ������Ҫ6���ֽ����һ��TOKEN�ĳ���Ԥ�У�����ƫ����2���ֽ�,6+2=9
        //��β���������������㹻�Ŀռ�16�ֽڣ���������ж��ǰ�ȫ��
        if ( ZCE_UNLIKELY(read_end - read_pos < 8 ))
        {
            return -1;
        }

        //ȡ�ø��ֳ��ȣ�������
        offset_token = *(read_pos++);
        noncomp_count = (offset_token & 0x0F) ;
        comp_count = (offset_token & 0xF0) >> 4;

        //TOKEN��ֵ0xE,��ʾ��һ���ֽڱ�ʾ���ȣ�0xF��ʾ2���ֽڱ�ʾ���ȣ�������ֵ���ǳ���
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
        //�Ѿ��������һ��block��
        if ( ZCE_UNLIKELY( noncomp_count == (size_t)(read_end - read_pos) ))
        {
            break;
        }

        //���TOKEN��ʾѹ�����ֵĳ�����0����ʾû��ѹ����
        if (comp_count)
        {
            //�����ʾΪС��0xE,���ȵ���0xE + 3(���ƥ�䡣��С������4)
            if (ZCE_LIKELY(comp_count < 0xE))
            {
                comp_count += 0x3;
            }
            else if (comp_count == 0xE)
            {
                comp_count = (uint8_t)(*(read_pos));
                ++read_pos;
            }
            else if (comp_count == 0xF)
            {
                comp_count = (uint16_t)(ZBYTE_TO_LEUINT16(read_pos));
                read_pos += 2;
            }
            //ȡ��ƫ�Ƶ�ַ
            ref_offset = (uint16_t)(ZBYTE_TO_LEUINT16(read_pos));
            read_pos += 2;
        }

        //���ƫ�Ƶ�ַ���󣬷��ش���ע��compressed_buf ��һ���ֽ�Ҳ�Ƿǲο��ֽڣ�
        //ƫ�Ƶĳ��Ȳ����ܴ���дλ�ú�ͷλ�õĲ�  +8 ��Ϊ���ָ��Ʒ������Ҫ����8�ֽڵļ�࣬
        //��֤����Щ�ռ�ɶ���
        if ( ZCE_UNLIKELY( (size_t)(read_end - read_pos) < ( 8 + noncomp_count)
                           || ((size_t)(write_end - write_pos) < ( 8 + noncomp_count + comp_count))  ) )
        {
            return -1;
        }

        if (noncomp_count)
        {
            write_stop = write_pos + noncomp_count;
            read_stop = read_pos + noncomp_count;
            ZCE_LZ_FAST_COPY_STOP(write_pos, read_pos, write_stop);
            read_pos = read_stop;
            write_pos = write_stop;
        }

        //���TOKEN��ʾѹ�����ֵĳ�����0����ʾû��ѹ����
        if (comp_count)
        {
            //�����ѹ���������ݳ���> 0,����û��ѹ�����ַ�����source
            ref_pos = write_pos - ref_offset;
            if ( ref_pos == write_pos || ref_pos  < original_buf  )
            {
                return -1;
            }

            //��������ط�����memcpy�ǲ����ʵģ���Ϊ��ַ�����н���
            write_stop = write_pos + comp_count;
            if (ZCE_LIKELY( ref_offset >= sizeof(uint64_t) ) )
            {
                ZCE_LZ_FAST_COPY_STOP(write_pos, ref_pos, write_stop);
            }
            //�ο���λ�ú͵�ǰ��λ��֮�䲻��8���ֽڣ��н������Ҫ�������⴦���ˡ�
            else
            {
                //��ʹ���Ȳ���8���ֽڣ�����һ�ο�����8���ֽڣ���������Ĵ����ǱȽφ��µģ�
                write_pos[0] = ref_pos[0];
                write_pos[1] = ref_pos[1];
                write_pos[2] = ref_pos[2];
                write_pos[3] = ref_pos[3];
                write_pos[4] = ref_pos[4];
                write_pos[5] = ref_pos[5];
                write_pos[6] = ref_pos[6];
                write_pos[7] = ref_pos[7];

                //�����ȵ���������8
                if (comp_count > sizeof(uint64_t))
                {
                    //��һ��0û�����壬��Ϊoffset������Ϊ0��
                    //�������ݵ������ǣ���Ϊϣ������8�ֽڶ��ÿ��ٿ�����ϣ��Դ��������ݵĳ��Ȳ�����8�ֽڣ�
                    //��ô��Ҫ�ҵ�һ��������������ظ���ʼ�Ĺ���
                    static const size_t POS_MOVE_REFER[] = {0, 0, 2, 2, 0, 3, 2, 1};
                    ref_pos +=  POS_MOVE_REFER[ref_offset];
                    unsigned char *match_write = (write_pos + 8);
                    ZCE_LZ_FAST_COPY_STOP(match_write, ref_pos, write_stop);
                }
            }
            write_pos = write_stop;
        }

#if defined ZCE_LZ_DEBUG && ZCE_LZ_DEBUG==1

        ZCE_LOG(RS_DEBUG,
                "zlz decompress no match size [%10u],match size [%10u],read len [%10u] ,write len[%10u]."
                "remain_read[%10u],remain_write[%10u]",
                noncomp_count,
                comp_count,
                read_pos - compressed_buf,
                write_pos - original_buf,
                read_end - read_pos,
                write_end - write_pos);
#endif
    }

    //����ռ䲻�������Ƿ��ش���
    if ( ZCE_UNLIKELY( (size_t)(read_pos - read_end) < noncomp_count
                       || (size_t)(write_end - write_pos) < noncomp_count ) )
    {
        return -1;
    }

    ::memcpy(write_pos, read_pos, noncomp_count);

    //�ɹ���ѹ
    return 0;
}

//===============================================================================================================

zce::LZ4_Compress_Format::LZ4_Compress_Format()
{
    hash_lz_offset_ = new uint32_t[HASH_TABLE_LEN];
}

zce::LZ4_Compress_Format::~LZ4_Compress_Format()
{
    if (hash_lz_offset_)
    {
        delete[] hash_lz_offset_;
    }
}

//ѹ���Ĺؼ��������ڲ�������
//ģ��LZ4���㷨�ĸ�ʽ���еĺ�����
void zce::LZ4_Compress_Format::compress_core(const unsigned char *original_buf,
                                             size_t original_size,
                                             unsigned char *compressed_buf,
                                             size_t *compressed_size)
{
    //��ʼ�����ֳ�ʼֵ
    const unsigned char *read_pos = original_buf;
    const unsigned char *next_read_pos = NULL;
    const unsigned char *read_end = original_buf + original_size;
    unsigned char *write_pos = compressed_buf;
    unsigned char *write_stop = NULL;

    //��Ϊ���ٿ������Ƚϵȣ���16���ֽڱ�֤�������
    const unsigned char *match_end = read_end - ZCE_LZ_NOPROCESS_TAIL;

    //��0
    memset(hash_lz_offset_, 0, sizeof(uint32_t)*HASH_TABLE_LEN);

    const unsigned char *ref_offset = NULL;
    const unsigned char *nomatch_achor =  NULL, *match_achor = NULL;
    unsigned char *offset_token = NULL;

    size_t match_offset, nomatch_count = 0, match_count = 0;
    nomatch_achor = read_pos;
    hash_lz_offset_[ZCE_LZ_HASH(read_pos)] = (uint32_t)(read_pos - original_buf);
    ++read_pos;
    //����ÿһ��Ҫѹ�����ֽ�
    for (;;)
    {

#if defined ZCE_LZ_DEBUG && ZCE_LZ_DEBUG==1
        ZCE_LOG(RS_DEBUG,
                "lz4 compress no match size [%10u],match size [%10u],read len [%10u] ,write len[%10u],remain read[%10u] ",
                nomatch_count,
                match_count,
                read_pos - original_buf,
                write_pos - compressed_buf,
                original_size - (read_pos - original_buf)
               );
#endif

        //�������ΪZLZ�㷨�Ķ������ɣ�һ�����м���һ������ѹ���ֶΣ���ѡ����һ������ѹ���ֶ���ɣ���ѡ����

        //nomatch_achor = read_pos;

        //����(1 << ZCE_LZ_STEP_LEN_POW)+1
        size_t step_attempts = 65;

        //�ҵ�һ��Token����������ѹ�������ݺͲ�����ѹ�������ݣ�
        next_read_pos = read_pos;
        //�����ƥ��
        do
        {
            read_pos = next_read_pos;
            //����ط�˵��һ�£����table_old_offset == 0����ôҲ��Ϊ��û��ƥ��
            uint32_t hash_value = ZCE_LZ_HASH(read_pos);

            uint32_t table_old_offset = hash_lz_offset_[hash_value];
            ref_offset = original_buf + table_old_offset;
            hash_lz_offset_[hash_value] = (uint32_t)(read_pos - original_buf);
            match_offset = (size_t)(read_pos - ref_offset);

            //step_attempts++��Ŀ�����ڳ��ڷ����޷�ѹ��������£�
            //�൱�ڣ�2^ZCE_LZ_STEP_LEN_POW�κ󣬲�������2,��2^ZCE_LZ_STEP_LEN_POW�κ󣬲�����Ϊ3
            //���������ȴﵽһ���̶Ⱥ�ֹͣ�������ӣ�
            size_t step_len = ((step_attempts++) >> ZCE_LZ_STEP_LEN_POW);

            //�����������������Ĵ���ע�������ZLZ���㷨������
            //ZLZ���㷨������Token����ZLZ���㷨ÿ��Token���޷�ѹ���������������������Ƶ�
            //LZ4���㷨����ÿ��Token�޷�ѹ��������������û������,��������ط�ֱ�����������
            next_read_pos = read_pos + step_len;
            if (ZCE_UNLIKELY(next_read_pos > match_end))
            {
                goto lz4_end_process;
            }
            //�������ƥ��,�������߼�ļ�಻�󣬣����Ҫ2���ֽڱ��������������Ҫ�����ֽڣ���ô����ȫ�𲻵�ѹ����Ч���ˣ�
            //��೤��ZCE_LZ_MAX_OFFSET,��2���ֽڣ�����һ���ֽڣ�������
        }
        while (ZBYTE_TO_UINT32(ref_offset) != ZBYTE_TO_UINT32(read_pos)
               || (match_offset) >= ZCE_LZ_MAX_OFFSET);


        //��Ϊ��ʵǰ������HASH��飬������ʵ���step_len ����1��ʱ��ǰ�滹����ȵ�����Ǻ��ٵģ���ȷʵ����
        while (read_pos > nomatch_achor && ref_offset > original_buf && ref_offset[-1] ==  read_pos[-1])
        {
            --ref_offset ;
            --read_pos ;
        }
        nomatch_count = read_pos - nomatch_achor;

        //����ʼ��һ���ֽڵ�offset_token��TOKEN�ĸ�4bit��ʾ��ѹ�����ȣ�����λ��ʾѹ������
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

        //�����û��ѹ��������
        //����в�ƥ������ݣ��ÿ��ٿ����ķ������п�����
        //memcpyѡ���׼���� С��256����ZCE_LZ_FAST_COPY������256,��memcpy,���͵���Լ�Ϊ�˱���ѡ��ֱ�����˿��ٿ���
        // http://www.cnblogs.com/fullsail/p/3160098.html

        //ÿ����8�ֽڽ��и��ƣ���ȡ��д�붼�����˿���
        write_stop = write_pos + nomatch_count;
        ZCE_LZ_FAST_COPY_STOP(write_pos, nomatch_achor, write_stop);
        write_pos = write_stop;

lz4_match_process:
        //��������˾��Ƿ�����(����)4�ֽڵ�ƥ���ˣ�
        match_achor = read_pos;
        read_pos += 4;
        ref_offset += 4;

        //���ٵ��ҳ��ж�����������ͬ�ģ�
        for (;;)
        {
            if (ZCE_UNLIKELY((read_pos > match_end) ) )
            {
                break;
            }

            uint32_t tail_match = 0;
#if defined ZCE_OS64
            //64λƽ̨��ÿ�αȽ�64bits
            uint64_t diff = ZBYTE_TO_UINT64(ref_offset) ^ ZBYTE_TO_UINT64(read_pos);
            if (!diff)
            {
                read_pos += 8;
                ref_offset += 8;
                continue;
            }
            //������ȣ���ָ���Ѹ�ٵõ��ж����ֽ����.
            //-----------------------------------------------------------------------------
            //�����⼸�αȽϸ��ӣ���������д���꣬���о���һ��û��������⣬����ддע�Ͱѡ�
            //Ϊ�˼��٣������64λ��32λ�������������Թ���32λƽ̨����64λ�����ٶȲ��ѣ�
            //�����LINUXƽ̨����__builtin_ctzll,__builtin_clzll ָ��õ��ʼΪ1��λ�ã��Ӷ��ж��ж��ٸ���ͬ��
            //ͬʱ���ݴ�ͷ��Сͷƽ̨ʹ�ò�ͬ�ĺ�����Сͷ��LBE to MBE ,��ͷ�� MBE to LBE,
            //�����Ҷ���������е��������ѹ������봦��MBE to LBE������LZ4�����߻ظ����ң�����Դ�Ķ��Ǻ��ˣ���
            //���Ϊ���ٶȣ�����ȡ��64bit����ֵ��Ϊlonglong�Ƚϵ�ʱ��û�п����ֽ���
#if defined ZCE_LINUX64

#if defined ZCE_LITTLE_ENDIAN
            tail_match += __builtin_ctzll(diff) >> 3;
#else
            tail_match += __builtin_clzll(diff) >> 3;
#endif

            //WIN64ƽ̨����_BitScanForward64��_BitScanReverse64�õ�����ߵ��Ǹ�bitΪ1��λ�ã������ο�LINUX�Ƕ�ע��
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

            //����32λ��ϵͳ���д���ÿ�αȽ�32bits����ο�64λLINUX�Ľ���
            //�ҵĲ��Ըо������32λϵͳ������32λ�������һ�����㡣
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
            break;
        }

        //LZ4�Ĵ��뵽����ط���match_count������4
        match_count = read_pos - match_achor - 4;
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

        //д��ƫ�Ƴ���,
        match_offset = read_pos - ref_offset ;

        //ǰ���Ѿ���֤��read_pos��ref_offset ���С��0xFFFF��2���ֽ��㹻
        ZLEUINT16_TO_BYTE(write_pos, ((uint16_t)(match_offset )));
        write_pos += 2;

#if defined ZCE_LZ_DEBUG && ZCE_LZ_DEBUG==1
        ZCE_LOG(RS_DEBUG,
                "lz4 compress no match size [%10u],match size [%10u],read len [%10u] ,write len[%10u],remain read[%10u]",
                nomatch_count,
                match_count + 4,
                read_pos - original_buf,
                write_pos - compressed_buf,
                original_size - (read_pos - original_buf));
#endif

        //���������������LZ4������֮�ʣ�����԰�#if 1������0����һ�£����빦����һ���ġ���
        //�����Ǳ��ҿ�15%
#if 1
        if (ZCE_UNLIKELY((read_pos > match_end) ) )
        {
            nomatch_achor = read_pos;
            break;
        }
        //
        hash_lz_offset_[ZCE_LZ_HASH((read_pos - 2))] =
            (uint32_t)(read_pos - 2 - original_buf);
        // Test next position
        ref_offset = original_buf + hash_lz_offset_[ZCE_LZ_HASH(read_pos)];
        hash_lz_offset_[ZCE_LZ_HASH(read_pos)] = (uint32_t)(read_pos - original_buf);

        if ((static_cast<size_t>(read_pos - ref_offset) <= ZCE_LZ_MAX_OFFSET) &&
            (ZBYTE_TO_UINT32(ref_offset) == ZBYTE_TO_UINT32(read_pos)))
        {
            nomatch_count = 0;
            offset_token =  write_pos ++;
            *offset_token = 0;
            goto lz4_match_process;
        }
        nomatch_achor = read_pos++;
#else
        nomatch_achor = read_pos;
#endif


    }

lz4_end_process:

    //����󼸸��ֽ�(��Ϊ��ѹ������)������ѹ����������
    nomatch_count = read_end - nomatch_achor;
    match_count = 0;

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

    //ѹ�����
    return;
}


//��ѹ���ĺ��Ĵ�������������ѹ���ĸ�ʽ�˽⣬��ѹ�Ĵ���Ӧ��������⣬
int zce::LZ4_Compress_Format::decompress_core(const unsigned char *compressed_buf,
                                              size_t compressed_size,
                                              unsigned char *original_buf,
                                              size_t original_size)
{

    //��ʼ�����ֳ�ʼֵ
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


        //ȡ�ø��ֳ��ȣ�������
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

        //�Ѿ��������һ��block��
        if (ZCE_UNLIKELY(noncomp_count == (size_t)(read_end - read_pos)))
        {
            break;
        }

        //����û��ѹ�����ַ�����source, ���Ϊ�˼ӿ��ٶȣ�������noncomp_count �Ƿ�Ϊ0
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
        comp_count += 4;
        //���ƫ�Ƶ�ַ���󣬷��ش���ע��compressed_buf ��һ���ֽ�Ҳ�Ƿǲο��ֽڣ�
        //ƫ�Ƶĳ��Ȳ����ܴ���дλ�ú�ͷλ�õĲ�  +8 ��Ϊ���ָ��Ʒ������Ҫ����8�ֽڵļ�࣬
        //��֤����Щ�ռ�ɶ���
        //if ( ZCE_LIKELY( (size_t)(read_end - read_pos) < (2 + 8 + noncomp_count)
        //                 || ((size_t)(write_end - write_pos) < ( 8 + noncomp_count + comp_count))  ) )
        //{
        //    return -1;
        //}

        //ȡ��ƫ�Ƶ�ַ
        ref_offset = (uint16_t)(ZBYTE_TO_LEUINT16(read_pos));
        read_pos += 2;

        ref_pos = write_pos - ref_offset;
        //ref_pos������һ�������ֵ
        if ( ZCE_LIKELY(ref_offset == 0 || ref_pos  < original_buf ))
        {
            return -1;
        }

        //��������ط�����memcpy�ǲ����ʵģ���Ϊ��ַ�����н���
        write_stop = write_pos + comp_count;
        if (ZCE_LIKELY( ref_offset >= sizeof(uint64_t) ) )
        {
            ZCE_LZ_FAST_COPY_STOP(write_pos, ref_pos, write_stop);
        }
        //���Ҫ�������⴦���ˡ�
        else
        {
            //��ʹ���Ȳ���8���ֽڣ�����һ�ο�����8���ֽڣ���������Ĵ����ǱȽφ��µģ�
            write_pos[0] = ref_pos[0];
            write_pos[1] = ref_pos[1];
            write_pos[2] = ref_pos[2];
            write_pos[3] = ref_pos[3];
            write_pos[4] = ref_pos[4];
            write_pos[5] = ref_pos[5];
            write_pos[6] = ref_pos[6];
            write_pos[7] = ref_pos[7];

            //�����ȵ���������8
            if (comp_count > sizeof(uint64_t))
            {
                //��һ��0û�����壬��Ϊoffset������Ϊ0��
                //�������ݵ������ǣ���Ϊϣ������8�ֽڶ��ÿ��ٿ�����ϣ��Դ��������ݵĳ��Ȳ�����8�ֽڣ�
                //��ô��Ҫ�ҵ�һ��������������ظ���ʼ�Ĺ���
                static const size_t POS_MOVE_REFER[] = {0, 0, 2, 2, 0, 3, 2, 1};
                ref_pos +=  POS_MOVE_REFER[ref_offset];
                unsigned char *match_write = (write_pos + 8);
                ZCE_LZ_FAST_COPY_STOP(match_write, ref_pos, write_stop);
            }
        }
        write_pos = write_stop;

    }

    //����ռ䲻�������Ƿ��ش���
    if ( ZCE_UNLIKELY( (size_t)(read_pos - read_end) < noncomp_count
                       || (size_t)(write_end - write_pos) < noncomp_count ) )
    {
        return -1;
    }

    ::memcpy(write_pos, read_pos, noncomp_count);

    //�ɹ���ѹ
    return 0;
}



#if defined (ZCE_OS_WINDOWS)
#pragma warning ( pop )
#endif



