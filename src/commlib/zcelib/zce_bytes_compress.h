/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_data_proc_encrypt.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013��6��16��
* @brief      �ڴ����ѹ���㷨����������Ǹ���LZ4 �� QuickLZ �õ��ġ�
*             �������붼�Ƚϼ򵥣����㸴�ӣ����бȽϺõĻ����⣬ʵ������Ӧ�ñ����Ǹ���һЩ��
*
*             �������2������������⼸����д�����۵Ĵ����ˣ�Ҳ������ϵ������롣ǰǰ��󣬼��Ͽ�2���㷨��
*             �������һ����µ���ɢʱ�䡣�����ԣ��ر�����JPG�ļ���������ʱ�򣬻��Ƿ������ⲻ�١�
*             д�������ĺ���ţһ������ʵ�ܼ�Ҳ�Ͳ���1000�д��롣
*             ��7��10������OK������ﵽ������Ҫ��Ŀ�꣬���Լ���2��ʵ����һ�°�װ���չ�
*
* @details    �����ڴ����ݵ�ѹ���㷨����Ҫ��������ѹ����
*             ϣ���ο�����ѹ���㷨������
*             ����ѹ���㷨��  LZ4   QuickLZ   Zippy/Snappy  LZO�ȣ�
*             ��ѹ���ȵĺ���  LZMA    GZIP(zlib)
*             Ŀǰ�ҵ�Ŀ��Ҳ����ʵ��һ���ڴ�ĸ���ѹ���㷨��
*
*             �����Ҫ˵һ��LZ4���㷨���ҵ�2���㷨���ο�������㷨��
*
*             �ҳ��ϣ��������Ҷ������Ż�����һ���СС�Ը��ģ���LZ4����㷨�������
*             ������ţ������ȥ��սһ������
*             �������Լ���ɫ�ĵ�һ����ʽ���㷨��ZLZ�����Ľ������ѹ���㷨�����Բ�һ�ڣ�
*             ����WIN64λ�ĵĽ�ѹ�㷨�ϣ��ұ�˦����40%��������ƽ̨���20%-30%��
*             ����˵��LZ4�㷨��ѹ���������ϵ�ƽ��ﵽ��һ�����£�����Ϊ������µ�ƽ��
*             �ﵽ��һ�����磬���Ҿ����Ƿ�ľ��硣�������ţ���С�յġ����󡷣�
*             ����ķ��ڷ���ɽ����ɽ���������ˡ�
*             ����һ��Ҫ˵������LZ4���㷨�ڰ�ȫ���ϼ���û�п��ǣ��������Э��ѹ�������º�Σ�յ�
*             ���Լ������ZLZ��ʱ������һЩ��صĿ��ǣ������ѹ����Ϊ���ݴ����±���
*
*             �㷨�ٶȣ�WIN32�µ��ٶȣ�AMD Phenom II X4 3.2G
*             ����RAW LZ4��LZ4ԭ����������� ZCE ZLZ �Ƿ��Ľ��õ����㷨��ZCE LZ4�㷨�ǻ���ģ��LZ4�Ĵ��롣
*             RAW LZ4 [   2271498] compress use us[    7159750.486]us [        304.686]Mb/s decompress use [    3044644.086]us [        716.496]Mb/s.
*             ZCE ZLZ [   2297174] compress use us[    8423219.146]us [        258.984]Mb/s decompress use [    3605885.627]us [        604.977]Mb/s.
*             ZCE LZ4 [   2314403] compress use us[    8034556.044]us [        271.512]Mb/s decompress use [    2990780.992]us [        729.400]Mb/s.
*             �㷨�ٶ� WIN64
*             RAW LZ4 [   2271498] compress use us[    7700759.320]us [        283.281]Mb/s decompress use [    2053244.006]us [       1062.453]Mb/s.
*             ZCE ZLZ [   2297174] compress use us[    7838718.628]us [        278.295]Mb/s decompress use [    3052662.310]us [        714.614]Mb/s.
*             ZCE LZ4 [   2314403] compress use us[    7617242.962]us [        286.387]Mb/s decompress use [    2221799.686]us [        981.851]Mb/s.
*             �㷨�ٶ� LINUX64����������������ݲ����ã������Ҫ���ٶȶԱ��ˡ�
*             RAW LZ4 [   2271498] compress use us[   15232820.350]us [        143.209]Mb/s decompress use [    5388874.147]us [        404.811]Mb/s.
*             ZCE ZLZ [   2297174] compress use us[   16014259.992]us [        136.221]Mb/s decompress use [    6448855.066]us [        338.273]Mb/s.
*             ZCE LZ4 [   2314403] compress use us[   16264991.011]us [        134.121]Mb/s decompress use [    5559644.472]us [        392.377]Mb/s.
*
*             64λ�£�ѹ����ѹ�ٶȻ������LZ4 10%�����Ҹ��˸о�LZ4�Ľ�ѹ��ȫ������Ĳ�������
*
*             �Ľ��㷨����죬������һֱ�������������Ĵ�ʱ����������ʱ��������
*
*             ɽ�� - ����ʢ
*
*             Խ��ɽ��  ��Ȼ�Ѱ���ͷ
*             �ੲ���  ʱ������İ���
*             ��δ��Ը���Ų���
*             �Ͱ��Լ��ȸ㶪
*             Խ��ɽ��  �ŷ������˵Ⱥ�
*             �ੲ��� ��Ҳ����������
*             Ϊ�μǲ�����һ����˭����ӵ��
*             ��ʲôʱ��
*
*
*/

#ifndef ZCE_LIB_BYTES_COMPRESS_H_
#define ZCE_LIB_BYTES_COMPRESS_H_

#include "zce_bytes_base.h"

namespace zce
{

template < typename COMPRESS_STRATEGY >
class ZCE_Compress
{
public:
    /*!
    * @brief      �õ�ѹ��������ڴ�BUFFER��С
    * @return     int  ==0��ʾ�ɹ�������ʧ�ܣ�һ��ԭ����ԭ�ĳ��ȹ���
    * @param      original_size      ԭ�ĵĳ���
    * @param      need_cmpbuf_size   �����ѹ��buffer�ĳ��ȣ�һ����ԭ�ĳ������ڲ��ᾡ����֤����̵ĳߴ硣
    */
    inline static int need_compressed_bufsize(size_t original_size,
                                              size_t *need_cmpbuf_size)
    {
        //��Լ���Դ���2G�����ݣ�
        return COMPRESS_STRATEGY::need_comp_size(original_size,
                                                 need_cmpbuf_size);
    }

    /*!
    * @brief      ��ѹ����ĵ���������õ�ԭ�ĵĳߴ磬
    * @return     int           ѹ�����ݸ�ʽ����
    * @param      original_buf  ѹ��������buffer
    * @param      original_size ���ز�����ԭ�ĵĳߴ�
    */
    static int get_original_size(const unsigned char *compressed_buf,
                                 size_t *original_size)
    {
        uint32_t srclen_type = *compressed_buf & 0x7;
        size_t need_srclen = 0;
        if (srclen_type == 0x1)
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
        //�����ܳ����������
        else
        {
            return -1;
        }
        *original_size = need_srclen;
        return 0;
    }

    /*!
    * @brief      ʵʱѹ��һ��������
    * @return     int              ==0��ʾ�ɹ�������ʧ�ܣ�ʧ��ԭ��һ�����ڴ治��������ԭ�Ĺ���
    * @param[in]     original_buf     ԭ�ģ�
    * @param[in]     original_size    ԭ�ĵĳ���
    * @param[out]    compressed_buf   ѹ�����ڴ棬
    * @param[in,out] compressed_size  ѹ���ĳߴ磬���봫������������ʱ��ʾcompressed_buf���ȣ�����need_compressdbuf_size�ķ���ֵ
    *                                 ������ʱ������ʹ�õ�compressed_buf�ĳ���
    */
    int compress(const unsigned char *original_buf,
                 size_t original_size,
                 unsigned char *compressed_buf,
                 size_t *compressed_size)
    {
        ZCE_ASSERT(original_buf && compressed_buf && original_size > 0);

        //���׼����ѹ���ռ䲻����
        int ret = 0;
        size_t need_compbuf_size = 0;
        ret = need_compressed_bufsize(original_size, &need_compbuf_size);
        if (ret != 0
            || *compressed_size < need_compbuf_size
            || original_size <= 0)
        {
            ZCE_ASSERT(false);
            return -1;
        }

        unsigned char *head_pos = compressed_buf;
        *head_pos = 0;

        //ע�Ᵽ�������ȫ���õ�LE
        //ͷ����1�ֽ�ѡ�1��1�ֽ�ԭ�����ֶΣ�
        size_t head_size = 0;
        if (original_size <= 0xFF)
        {
            head_size = 2;
            *head_pos |= 0x1;
            *(head_pos + 1) = (uint8_t)(original_size);
        }
        //ͷ����1�ֽ�ѡ�1��2�ֽ�ԭ�����ֶΣ�
        else if (original_size <= 0xFFFF)
        {
            head_size = 3;
            *head_pos |= 0x2;
            ZLEUINT16_TO_BYTE((head_pos + 1), ((uint16_t)(original_size)));
        }
        //ͷ����1�ֽ�ѡ�1��4�ֽڱ�ʶԭ�����ֶΣ�
        else  if (original_size <= COMPRESS_STRATEGY::LZ_MAX_ORIGINAL_SIZE)
        {
            head_size = 5;
            *head_pos |= 0x4;
            ZLEUINT32_TO_BYTE((head_pos + 1), ((uint32_t)(original_size)));
        }
        else
        {
            ZCE_ASSERT(false);
        }

        bool if_compress = false;
        size_t compressed_data_len = 0;

        //С��������ȾͲ�Ҫѹ���ˣ��˷�����
        if (original_size > ZCE_LZ_MIN_PROCESS_LEN)
        {
            compress_fmt_.compress_core(original_buf,
                                        original_size,
                                        compressed_buf + head_size,
                                        &compressed_data_len);

            //����������ѹ��
            if (compressed_data_len < original_size)
            {
                if_compress = true;
            }
        }

        //�����Ƿ������ѹ�������û��ѹ����Դ����ȫ����������
        if (if_compress)
        {
            *compressed_size = compressed_data_len + head_size;
            *head_pos |= 0x80;
        }
        else
        {
            *compressed_size = original_size + head_size;
            //�ֽ��Ƕ���ĵط�������ʹ��memcpy��
            ::memcpy(compressed_buf + head_size, original_buf, original_size);
        }

        return 0;
    }

    /*!
    * @brief      ʵʱ��ѹ�ڴ�����
    * @return     int             ==0����ʾ�ɹ�������ʧ�ܣ�ʧ��ԭ�����ѹ�����ݴ��󣬿ռ䲻���ȣ�
    * @param      compressed_buf  ��ѹ��������
    * @param      compressed_size ��ѹ�������ݵĳߴ�
    * @param      original_buf    ԭ��
    * @param      original_size   ԭ�ĵĴ�С
    */
    int decompress(const unsigned char *compressed_buf,
                   size_t compressed_size,
                   unsigned char *original_buf,
                   size_t *original_size)
    {
        ZCE_ASSERT(original_buf && compressed_buf && compressed_size >= 5);
        if (compressed_size < 5)
        {
            return -1;
        }
        const unsigned char *head_pos = compressed_buf;
        uint32_t srclen_type = *head_pos & 0x7;
        size_t need_srclen = 0;
        size_t head_size = 0;
        if (srclen_type == 0x1)
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
        //�����ܳ����������
        else
        {
            return -1;
        }

        //����Ŀռ䲻��
        if (need_srclen > *original_size)
        {
            return -1;
        }

        *original_size = need_srclen;
        bool if_compressed = (*head_pos & 0x80) ? true : false;
        if (if_compressed)
        {
            return compress_fmt_.decompress_core(compressed_buf + head_size,
                                                 compressed_size - head_size,
                                                 original_buf,
                                                 need_srclen);
        }
        //���û�о���ѹ��
        else
        {
            if (compressed_size != need_srclen + head_size)
            {
                return -1;
            }
            //�ֽڶ���ĵط�����ֱ����memcpy��
            ::memcpy(original_buf, compressed_buf + head_size, need_srclen);
            return 0;
        }

        //return 0;
    }

public:

    //����β������һЩ�ֽڲ����д�����Ҫ��Ϊ�˼��٣���Ϊ����ܶ�ط���ʹ��uint64_t���д���
    static const size_t ZCE_LZ_MIN_PROCESS_LEN = sizeof(uint64_t) * 2;

protected:

    COMPRESS_STRATEGY  compress_fmt_;
};




//=====================================================================================================
///ZLZ�㷨�ǲ���ģ��LZ4�Ĵ��룬����һЩ��ʽ�仯��
///��������Ϊ�ҵ��㷨Ӧ�ø���һЩ��
class ZLZ_Compress_Format
{
public:

    ZLZ_Compress_Format();
    ~ZLZ_Compress_Format();

    //ѹ�����Ĵ���
    void compress_core(const unsigned char alignas(sizeof(size_t))* original_buf,
                       size_t original_size,
                       unsigned char alignas(sizeof(size_t)) *compressed_buf,
                       size_t *compressed_size);

    //��ѹ���Ĵ���
    int decompress_core(const unsigned char *compressed_buf,
                        size_t compressed_size,
                        unsigned char *original_buf,
                        size_t original_size);

    //��Ҫ��ѹ����buffer����
    inline static int need_comp_size(size_t original_size,
                                     size_t *need_cmpbuf_size)
    {
        if (original_size > LZ_MAX_ORIGINAL_SIZE)
        {
            return -1;
        }
        *need_cmpbuf_size = ((original_size) + ((original_size) / 0xFFF7 + 1) * 3 + 64);
        return 0;
    }

public:
    //���ѹ����������֧�ֵ���󳤶ȣ�
    //��Ϊ��3���ֽڱ�ʾ����Լ��64K��ѹ�����ݣ�����ͷ����β���Ĵ���
    const static size_t LZ_MAX_ORIGINAL_SIZE = 0xFFFCFF00;



protected:

    //HASH TABLE��ָ��
    uint32_t *hash_lz_offset_ = nullptr;
};

//ֱ�ӵ�ZLZ��typedef��ʹ��zce::ZLZ_Compress::compress ,decompress�����Ϳ�����ɹ���
typedef ZCE_Compress<zce::ZLZ_Compress_Format> ZLZ_Compress;




//=====================================================================================================

//ZEN ZLZ �Ƿ��Ľ��õ����㷨��
class LZ4_Compress_Format
{
public:

    LZ4_Compress_Format();
    ~LZ4_Compress_Format();

    //ѹ�����Ĵ���
    void compress_core(const unsigned char *original_buf,
                       size_t original_size,
                       unsigned char *compressed_buf,
                       size_t *compressed_size);
    //��ѹ���Ĵ���
    int decompress_core(const unsigned char *compressed_buf,
                        size_t compressed_size,
                        unsigned char *original_buf,
                        size_t original_size);

    //��Ҫ��ѹ����buffer����
    inline static int need_comp_size(size_t original_size,
                                     size_t *need_cmpbuf_size)
    {
        if (original_size > LZ_MAX_ORIGINAL_SIZE)
        {
            return -1;
        }
        *need_cmpbuf_size = ((original_size) + ((original_size) / 0xFF + 1) + 64);
        return 0;
    }

    //���ѹ����������֧�ֵ���󳤶ȣ�
    //LZ4�㷨����һ���ֽڱ�ʾ255���ֽ�����û��ѹ����
    const static size_t LZ_MAX_ORIGINAL_SIZE = 0xFEFEFE00;

protected:

    //HASH TABLE��ָ��
    uint32_t *hash_lz_offset_ = nullptr;
};

//ֱ�ӵ�ZEN LZ4��typedef
typedef ZCE_Compress<LZ4_Compress_Format> LZ4_Compress;

//=====================================================================================================

};//end of zce

#endif

