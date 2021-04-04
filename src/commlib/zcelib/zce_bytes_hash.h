/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_data_proc_hash.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013��2��16��
* @brief
*             ���ݵ�HASH����ĺ�����������MD5��SHA1��SHA256��CRC32�ȣ�
*             �����Ͱ���MD4,SHA224��Щ������Ŀǰ�Ŀ�����������ܼ򵥣��������Ҫ��
*             ����һ����
*             ����ҪҪ���ϣ�����������ż��ĳɷ��ˣ�Ϊ�˲�д�ظ����룬�������ģ��
*             �����㷨��
*             �Ҹ�˵�ҵĴ���ʵ�ָ�˵��ĿǰMD5��SHA1�㷨��д����������һ�ף�֮һ����
*             �����ҿ�����MD5,SHA1��BLOCK���ݴ������ˣ��Ŷ����֡�
*             ��ʵMD5��SHA�����㷨�ķ���������һ�£����ǽ�������Ϣ���з��飬ÿ���
*             ��N���ֽڣ�Ȼ�������������ժҪ���Ӵգ����㣬Ȼ�󽫽��������һ������
*             ���㣬���һ������һ����һ����ʶ����(0x80)���������油��0�������볤��.
*
* @details    ������㷨�ο���rhash����⣬�ô��Ǻ��淢�֣���ʵ�����ο��������׿�
*             ���淢�������⣬��ĳЩ�����ݵø�����׿�ĳЩ�̶Ȼ��á�
*             ��Ȼ����bugҲ�����٣�����������룬ĳЩ�ط��ֽ��������ȡ�
*             ������Ϊ�ϼ������������Ѹ�ٸ���һЩ���룬������֣��ϵ�ѽ�����Ƕ���
*             ���㣬��ϡ���Ϳ�Ŀ���1��࣬�����������㷨�ٶ��֡�
*             ���ܽ���Щ�����������Կ�����ԭ��������£�
*             �����ҵ�����Բο��Ĵ����м��ף������Կ������ɲ�𣩣�RSA�Ĵ��룬
*             openssl�Ĵ���ȣ���Щ���뵱����ƹ������Ժܶ���ѧ�ң���ѧ�Һܶ�ʱ��
*             д�Ĵ��벻�߱��ɶ��ԣ�����󲿷��㷨�����xxx_update��������final
*             �����ֵ��ã���������̫�����ˣ�˼ά���Զ�·������Ŀǰ�Ĵ����������Щ
*             �����ϸĽ��ġ��ܶද�ָĵ���Ҳû������������⣬�Ͷ����֣�����ܶ��
*             �뷴���������ִ�ë��õ��ɻ�
*             �������ڻ������ֽ�����ƶ���һ�֣�BE����������ĸĽ����̣��ֽ������
*             �����������ǣ��������ͱ���е����ˡ�
*             ���⣬�ܶ����˵��������MD5,SHA1�㷨��˵�����ܺ��죬��Ӧ������ѧ��
*             �������SHA1��ÿ�δ���Ŀ�BLOCKֻ��һ�仰��������MD5һ������ʵ��
*             �أ�SHA1�㷨��������ݶ�����BE����ģ����һ������ҲҪ����BE��ʽ����
*             ��MD5�㷨�ڲ�������LE����ЩҲ���������ʹ�ࡣ
*
*             BTW: rhashlib���õ�Э����MIT���ڴ��ٴθ�лԭ�������ߣ�
*             ����ά�������α��������ǳ���
*
*             ��ĳ�������Ͻ���MD5��SHA1,SHA256�㷨�Ĵ����ظ��Ⱥܸߣ�crypt++�����ģ���
*             ����������һ������Ĵ����ظ�
*             http://www.cnblogs.com/fullsail/archive/2013/02/22/2921505.html
*             ��ʱ�ĵ��еĴ��뻹����C���룬������Ѿ�ģ����Ի�����Ҳ��crypto++���
*             ������㷨����������Ϳ���ժҪ�㷨һ��������ԭ��������������ˣ�
*
*             ����һЩ�򵥵����ܲ���benchmark���ݣ�270���ֽڵ��ַ�����ִ��
*             10000000�Σ�VC++ 2010, Realse �汾 O2
*             Ϊ�˷�ֹ�ڲ��Ż���ÿ�β����ַ������ݶ���һ������лairfu��������ô��
*             �������Ż����⡣
*             bkdr_hash 3.514  seconds.  768.355M/s
*             crc32     5.433  seconds.  496.963M/s
*             md5       10.137 seconds.  266.351M/s
*             sha1      17.173 seconds.  157.224M/s
*
*             �����ա� �����
*             ���˾����������
*             ���˾�����������
*             ���˾��������������ûȤζ
*             ������ ���� ����Ӧ����
*             ��Ը�� һ����Զ�����
*             ���˾����������
*             ���˾�����������
*             ���˾��������������ûȤζ
*             ������ ���� ����Ӧ����
*             ��Ը�� һ����Զ�����
*             Ŷ~~
*             # һ��֮�ж���תת �ǻῴ���
*             ����ʱ��Ҳ�Թ�����һ������ûЭ��
*             ��ĳ�� ����С����
*             ���������༸����������ҹ����
*             һ��֮������������ҲҪ�߹�
*             �Ӻ�ʱ����������Ҹ������ҵ��ĺ�
*             �����֮�� ȼ�������
*             ����� ǧɽҲ����̤��
*             ����� ���ᴵ��
*             ���������Ļ���������ף������
*             ������ ��������
*             ������ÿ��ϣ�����˻� ��Ҫմʪ��
*             ���˾����������
*             ���˾�����������
*             ���˾��������������ûȤζ
*             ������ ���� ����Ӧ����
*             ��Ը�� һ����Զ�����
*             ��Ը�� һ����Զ�����
*             Ŷ~~
*             # һ��֮�ж���תת �ǻῴ���
*             ����ʱ��Ҳ�Թ�����һ������ûЭ��
*             ��ĳ�� ����С����
*             ���������༸����������ҹ����
*             һ��֮������������ҲҪ�߹�
*             �Ӻ�ʱ����������Ҹ������ҵ��ĺ�
*             �����֮�� ȼ�������
*             ����� ǧɽҲ����̤��
*             ����� ���ᴵ��
*             ���������Ļ���������ף������
*             ������ ��������
*             ������ÿ��ϣ�����˻� ��Ҫմʪ��
*             ���˾����������
*             ���˾�����������
*             ���˾��������������ûȤζ
*             ������ ���� ����Ӧ����
*             ��Ը�� һ����Զ�����
*             ���˾����������
*             ���˾�����������
*             ���˾��������������ûȤζ
*             ������ ���� ����Ӧ����
*             ��Ը�� һ����Զ�����
*             ���˾����������
*             ���˾�����������
*             ���˾��������������ûȤζ
*             ������ ���� ����Ӧ����
*             ��Ը�� һ����Զ�����
*             ���˾����������
*             ���˾�����������
*             ���˾��������������ûȤζ
*             ������ ���� ����Ӧ����
*             ��Ը�� һ����Զ�����
*             ���˾����������
*             ���˾�����������
*             ���˾��������������ûȤζ
*             ������ ���� ����Ӧ����
*             ��Ը�� һ����Զ�����
*             Ŷ~~
*/

#ifndef ZCE_LIB_BYTES_HASH_H_
#define ZCE_LIB_BYTES_HASH_H_

#include "zce_bytes_base.h"

//=====================================================================================================

/*!
* @brief      64�ֽ�ΪBLOCK��HASH�㷨�Ĵ��������Ļ��࣬������ص�context�ṹ��
*             ����Ǵ�MD���㷨ϵ�г������㷨��BLOCK������64�ģ������������������չ����
*             ����MD4,MD5,SHA0,SHA1,SHA256,SHA224,
*             BLOCK��128��SHA512��Щ�಻�ܴ����������չ�õ�
* @tparam     result_size    HASH����ĳߴ磬�ֽ�
* @tparam     little_endian  ����㷨�Ƿ����Сͷ���룬�������true��������ǣ���false��
* @tparam     HASH_STRATEGY  �㷨�Ĳ�����
*/
template <size_t result_size, bool little_endian, typename HASH_STRATEGY>
class ZCE_HashFun_Block64
{
public:

    ///������ݳ���
    static const size_t HASH_RESULT_SIZE   = result_size;
    ///ÿ�δ����BLOCK�Ĵ�С
    static const size_t PROCESS_BLOCK_SIZE = 64;

    //�㷨�������ģ�����һЩ״̬���м����ݣ����
    typedef struct hash_fun_ctx
    {
        ///��������ݵ��ܳ���
        uint64_t length_;
        ///��û�д�������ݳ���
        uint64_t unprocessed_;
        ///(�м�)����������ĵط�
        uint32_t hash_[HASH_RESULT_SIZE / 4];
    } hash_fun_ctx;

    typedef hash_fun_ctx context;

    /*!
    * @brief      �ڲ��������������ݵ�ǰ�沿��(>PROCESS_BLOCK_SIZE 65�ֽڵĲ���)��ÿ�����һ��PROCESS_BLOCK_SIZE�ֽڵ�block�ͽ����Ӵմ���
    * @param[out] ctx  �㷨��context�����ڼ�¼һЩ����������ĺͽ��
    * @param[in]  buf  ��������ݣ�
    * @param[in]  buf_size ��������ݳ���
    */
    static void process(context *ctx, const unsigned char *buf, size_t buf_size)
    {
        //Ϊʲô����=����Ϊ��ĳЩ�����£����Զ�ε���zce_md5_update����������������뱣֤ǰ��ĵ��ã�ÿ�ζ�û��unprocessed_
        ctx->length_ += buf_size;

        //ÿ������Ŀ鶼��PROCESS_BLOCK_SIZE�ֽ�
        while (buf_size >= PROCESS_BLOCK_SIZE)
        {
            HASH_STRATEGY::process_block(ctx->hash_, reinterpret_cast<const uint32_t *>(buf));
            buf  += PROCESS_BLOCK_SIZE;
            buf_size -= PROCESS_BLOCK_SIZE;
        }

        ctx->unprocessed_ = buf_size;
    }

    /*!
    * @brief      �����㷨Ҫ��͵�ǰ�������ֽ������ͣ�����N��uint32_t���ݵĿ���������
    *             ����������к͵�ǰ�������ֽ������Ͳ����ϣ������SWAP
    * @param      dst      Ŀ��ָ��
    * @param      src      ������Դͷָ��
    * @param      length   ���ȣ�ע�����ֽڳ���
    */
    inline static void endian_copy(void *dst, const void *src, size_t length)
    {
        //�����㷨�Ĵ�СͷҪ���Լ���ǰCPU�Ĵ�Сͷ����������
        bool if_le = little_endian;
#if ZCE_BYTES_ORDER == ZCE_LITTLE_ENDIAN
        //�����������Сͷ�������㷨����Ҫ��Сͷ�ģ�ֱ��ʹ��ָ��
        if (if_le)
        {
            ::memcpy(dst, src, length);
        }
        //�����������Сͷ�����㷨����Ҫ���ͷ�ģ������е�uint32_t����swap
        else
        {
            ::memcpy(dst, src, length);
            //���е����ݷ�ת
            for (size_t i = 0; i < length / 4; ++i)
            {
                ((uint32_t *)dst)[i] = ZCE_SWAP_UINT32(((uint32_t *)dst)[i]);
            }
        }
#else
        //��������Ǵ�ͷ�����㷨�ڲ�Ҫ���������Сͷ���������е�uint32_t����swap
        if (if_le)
        {
            ::memcpy(dst, src, length);
            //���е����ݷ�ת
            for (size_t i = 0; i < length / 4; ++i)
            {
                ((uint32_t *)dst)[i] = ZCE_SWAP_UINT32(((uint32_t *)dst)[i]);
            }
        }
        //��������Ǵ�ͷ�����㷨�ڲ�Ҫ��������Ǵ�ͷ����ֱ��ʹ��ָ��
        else
        {
            ::memcpy(dst, src, length);
        }
#endif
    }

    /*!
    * @brief      �ڲ��������������ݵ�ĩβ���֣�����Ҫƴ�����1��������������Ҫ�����BLOCK������0x80�����ϳ��Ƚ��д���
    * @param[in]  ctx    �㷨��context�����ڼ�¼һЩ����������ĺͽ��
    * @param[in]  buf    ���������
    * @param[in]  buf_size   ����buffer�ĳ���
    * @param[out] result ���صĽ����
    */
    static void finalize(context *ctx,
                         const unsigned char *buf,
                         size_t buf_size,
                         unsigned char result[HASH_RESULT_SIZE])
    {
        uint32_t message[PROCESS_BLOCK_SIZE / 4] = {0};

        //����ʣ������ݣ�����Ҫƴ�����1��������������Ҫ����Ŀ飬ǰ����㷨��֤�ˣ����һ����϶�С��64���ֽ�
        memset(message, 0, PROCESS_BLOCK_SIZE);
        if (ctx->unprocessed_)
        {
            memcpy(message, buf + buf_size - ctx->unprocessed_, static_cast<size_t>( ctx->unprocessed_));
        }

        //ÿ������Ŀ鶼��64�ֽ�

        //�õ�0x80Ҫ����ڵ�λ�ã���uint32_t �����У���
        uint32_t index = (uint32_t)((ctx->length_ & 63) >> 2);
        uint32_t shift = (uint32_t)((ctx->length_ & 3) * 8);

        //���0x80��ȥ�����Ұ����µĿռ䲹��0
        message[index]   &= ~(0xFFFFFFFF << shift);
        message[index++] ^= 0x80 << shift;

        //������block���޷����������ĳ����޷����ɳ���64bit����ô�ȴ������block
        if (index > 14)
        {
            while (index < 16)
            {
                message[index++] = 0;
            }

            HASH_STRATEGY::process_block(ctx->hash_, message);

            index = 0;
        }

        //��0
        while (index < 14)
        {
            message[index++] = 0;
        }

        //���泤�ȣ�ע����bitλ�ĳ���,����������ҿ��������˰��죬
        uint64_t data_len = (ctx->length_) << 3;

        //ע��MD5�㷨Ҫ���64bit�ĳ�����СͷLITTLE-ENDIAN���룬ע������ıȽ���!=
        bool if_le = little_endian;
#if ZCE_BYTES_ORDER == ZCE_LITTLE_ENDIAN
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

        message[14] = (uint32_t) (data_len & 0x00000000FFFFFFFFULL);
        message[15] = (uint32_t) ((data_len & 0xFFFFFFFF00000000ULL) >> 32);

        HASH_STRATEGY::process_block(ctx->hash_, message);

        //ע������Сͷ���ģ��ڴ�ͷ������Ҫ����ת��
        endian_copy(result, ctx->hash_, HASH_RESULT_SIZE);
    }

    //��Ҫʵ���㷨��Ҫʵ�ֵ�����������
public:

    /*!
    * @brief      ��ʼ��context������
    * @param      ctx  context ��ָ��
    */
    //static void initialize(context *ctx);

    /*!
    * @brief      ��64���ֽڣ�16��uint32_t���������ժҪ���Ӵգ�����
    * @param      state ��Ŵ����hash���ݽ��,
    * @param      block Ҫ�����block��64���ֽڣ�16��uint32_t������  �������û�жԱ������ת��
    */
    //static void process_block(uint32_t state[HASH_RESULT_SIZE / 4],
    //    const uint32_t block[PROCESS_BLOCK_SIZE / 4]);

};

//=====================================================================================================
//MD5�Ĵ�������࣬��Ҫ��ʵ���㷨������ֱ��ʹ�ã�
//������ϣ����protected��friends ��Ԫ�����ⲿ�ܸ�֪��Щ�����ģ�����Ԫ�Զ�ģ���֧�ֻ���������ֻ��public��
class ZCE_Hash_MD5 : public ZCE_HashFun_Block64<16, true, ZCE_Hash_MD5>
{

public:

    static void initialize(context  *ctx);

    static void process_block(uint32_t state[HASH_RESULT_SIZE / 4],
                              const uint32_t block[PROCESS_BLOCK_SIZE / 4]);

};

//=====================================================================================================
//SHA1�Ĵ��������
class ZCE_Hash_SHA1 : public ZCE_HashFun_Block64<20, false, ZCE_Hash_SHA1>
{
public:

    static void initialize(context  *ctx);

    static void process_block(uint32_t state[HASH_RESULT_SIZE / 4],
                              const uint32_t block[PROCESS_BLOCK_SIZE / 4]);
};

//=====================================================================================================

//SHA256��SHA2���㷨����
class ZCE_Hash_SHA256 : public ZCE_HashFun_Block64<32, false, ZCE_Hash_SHA256>
{
public:

    static void initialize(context  *ctx);

    static void process_block(uint32_t state[HASH_RESULT_SIZE / 4],
                              const uint32_t block[PROCESS_BLOCK_SIZE / 4]);

};

//=====================================================================================================

///CRC32 ��Ҫ��Ϊ�˷�������ļ�CRC32��ʵ�ֵģ���ʵ���ֻ�Ǽ����ڴ�CRCֵ��������һ������ʵ�ּ�
class ZCE_Hash_CRC32
{
public:
    //������ݳ���
    static const size_t HASH_RESULT_SIZE   = 4;
    //ÿ�δ����BLOCK�Ĵ�С,��ʵ����CRC32���ֵû��ɶ�ã���Ҫ�����ļ�����
    static const size_t PROCESS_BLOCK_SIZE = 256;

    typedef uint32_t context;

    static void process(context *ctx, const unsigned char *buf, size_t buf_size);

    static void finalize(context *ctx,
                         const unsigned char *buf,
                         size_t buf_size,
                         unsigned char result[HASH_RESULT_SIZE]);

    static void initialize(context  *ctx);

};

//=====================================================================================================

namespace zce
{

//=====================================================================================================
//��ͳ��HASH�㷨������

/*!
* @brief      ����һ���ڴ����ݵ�HASHֵ���������ظ��Լ������Ǿ��������ģ�棬������ʵ�ַ�ʽ
* @tparam     HASH_STRATEGY ģ�����������һ��������HASH���㷨�����ԣ�ZCE_Hash_SHA1��ZCE_Hash_MD5��
* @return     unsigned char * ���صõ��Ľ��
* @param[in]  buf    �����BUFFER
* @param[in]  buf_size   BUFFER�ĳ���
* @param[out] result ���صĽ������ָ��
*/
template<typename HASH_STRATEGY>
unsigned char *hash_fun(const unsigned char *buf,
                        size_t buf_size,
                        unsigned char result[HASH_STRATEGY::HASH_RESULT_SIZE])
{
    ZCE_ASSERT(result != NULL);

    typename HASH_STRATEGY::context ctx;
    HASH_STRATEGY::initialize(&ctx);
    HASH_STRATEGY::process(&ctx, buf, buf_size);
    HASH_STRATEGY::finalize(&ctx, buf, buf_size, result);
    return result;
}

/*!
* @brief      ����һ���ڴ����ݵ�HASHֵ��һ�����͵Ĳ���ģʽ
* @tparam     HASH_STRATEGY   ģ�����������һ��������HASH���㷨�����ԣ�ZCE_Hash_SHA1��ZCE_Hash_MD5��
* @tparam     BUFFER_MULTIPLE ��������һ��һ�δ���һ���飬�����ͨ������������ö�ȡ��BUFFER�������ı�����
*                             �����ڵ������IO�����CPU���㣩
* @return     int       �ɹ�����0��ʧ�ܣ��ļ��޷��򿪣��޷���ȡ������-1
* @param      file_name �ļ�����
* @param      result    ���صĽ������ָ��
* @note
*/
template<typename HASH_STRATEGY, size_t BUFFER_MULTIPLE>
int hash_file(const char *file_name,
              unsigned char result[HASH_STRATEGY::HASH_RESULT_SIZE])
{

    //���ļ�
    ZCE_HANDLE  fd = zce::open(file_name, O_RDONLY);
    if (ZCE_INVALID_HANDLE == fd)
    {
        return -1;
    }

    //��ȡ�ļ��ߴ磬�г��ȿ��Ա�����ʱ�������ȡ���ļ����Ⱥͻ�����ȣ�Ҫ��ȡһ�ε��鷳��
    size_t file_size = 0;
    int ret = zce::filesize(fd, &file_size);
    if ( 0 != ret)
    {
        return -1;
    }

    //ÿ�ξ�����ȡ640K���ݣ�ע�����buffer������PROCESS_BLOCK_SIZE�ֽڵ�N���
    const size_t READ_LEN = HASH_STRATEGY::PROCESS_BLOCK_SIZE * BUFFER_MULTIPLE;
    char *read_buf = new char [READ_LEN];

    typename HASH_STRATEGY::context ctx;
    HASH_STRATEGY::initialize(&ctx);

    ssize_t read_len = 0;

    do
    {
        //��ȡ����
        read_len = zce::read(fd, read_buf, READ_LEN);
        if (read_len < 0)
        {
            delete [] read_buf;
            zce::close(fd);
            return -1;
        }
        HASH_STRATEGY::process(&ctx, (unsigned char *) read_buf, read_len);

    }
    while ( (file_size -= read_len) > 0 );

    HASH_STRATEGY::finalize(&ctx, (unsigned char *) read_buf, read_len, result);
    delete [] read_buf;
    zce::close(fd);

    return 0;
}

/*!
* @brief      ��ĳ���ڴ���MD5��д���������Ҫ����Ϊ�˷�����ʹ��
* @return     unsigned char* ���صĵĽ����
* @param[in]  buf    ��MD5���ڴ�BUFFERָ��
* @param[in]  buf_size   BUFFER����
* @param[out] result ���
*/
inline unsigned char *md5(const unsigned char *buf,
                          size_t buf_size,
                          unsigned char result[ZCE_Hash_MD5::HASH_RESULT_SIZE])
{
    return zce::hash_fun<ZCE_Hash_MD5>(buf, buf_size, result);
}

/*!
* @brief      ��ĳ���ļ���MD5ֵ���ڲ����ļ���Ƭ��������õ�MD5,���������ʹ�ļ��ܴ�Ҳû�й�ϵ��
*             ����32λϵͳ�������ļ�����������
* @return     int         ����0��ʶ�ɹ�
* @param      file_name   �ļ�·��������
* @param      result      ���صĽ��
*/
inline int md5_file(const char *file_name,
                    unsigned char result[ZCE_Hash_MD5::HASH_RESULT_SIZE])
{
    return zce::hash_file<ZCE_Hash_MD5, 10240>(file_name, result);
}

/*!
* @brief      ���ڴ��BUFFER��SHA1ֵ
* @return     unsigned char* ���صĵĽ��
* @param[in]  buf    ��SHA1���ڴ�BUFFERָ��
* @param[in]  buf_size   BUFFER����
* @param[out] result ���
*/
inline unsigned char *sha1(const unsigned char *buf,
                           size_t buf_size,
                           unsigned char result[ZCE_Hash_SHA1::HASH_RESULT_SIZE])
{
    return zce::hash_fun<ZCE_Hash_SHA1>(buf, buf_size, result);
}

/*!
* @brief      ��ĳ���ļ���SHA1ֵ���ڲ����ļ���Ƭ��������õ�SHA1,���������ʹ�ļ��ܴ�Ҳû�й�ϵ��
*             ����32λϵͳ�������ļ�����������
* @return     int         ����0��ʶ�ɹ�
* @param[in]  file_name   �ļ�·��������
* @param[out] result      ���صĽ��
*/
inline int sha1_file(const char *file_name,
                     unsigned char result[ZCE_Hash_SHA1::HASH_RESULT_SIZE])
{
    return zce::hash_file<ZCE_Hash_SHA1, 10240>(file_name, result);
}

/*!
* @brief      ���ڴ��BUFFER��SHA256ֵ
* @return     unsigned char* ���صĵĽ��
* @param[in]  buf    ��SHA1���ڴ�BUFFERָ��
* @param[in]  buf_size   BUFFER����
* @param[out] result ���
*/
inline unsigned char *sha256(const unsigned char *buf,
                             size_t buf_size,
                             unsigned char result[ZCE_Hash_SHA256::HASH_RESULT_SIZE])
{
    return zce::hash_fun<ZCE_Hash_SHA256>(buf, buf_size, result);
}

/*!
* @brief      ��ĳ���ļ���SHA256ֵ���ڲ����ļ���Ƭ��������õ�SHA256,���������ʹ�ļ��ܴ�Ҳû�й�ϵ��
*             ����32λϵͳ�������ļ�����������
* @return     int         ����0��ʶ�ɹ�
* @param[in]  file_name   �ļ�·��������
* @param[out] result      ���صĽ��
*/
inline int sha256_file(const char *file_name,
                       unsigned char result[ZCE_Hash_SHA256::HASH_RESULT_SIZE])
{
    return zce::hash_file<ZCE_Hash_SHA256, 10240>(file_name, result);
}

/*!
* @brief      ��һ��buffer��CRC32ֵ����������һЩҪ���ٶȵļ�У��
* @return     uint32_t ������õ�CRC32ֵ
* @param[in]  crcinit  CRC�ĳ�ʼֵ�����ڸ�����֤,�����2���ط���֤����ʼֵҪһ��
* @param[in]  buf      ��CRC���ڴ�BUFFER
* @param[in]  buf_size     BUFFER����
*/
uint32_t crc32(uint32_t crcinit,
               const unsigned char *buf,
               size_t buf_size);

/*!
* @brief      ��ʼֵΪ0��CRC32����
* @return     uint32_t   ���ص�CRC32ֵ
* @param[in]  buf        ����CRC32���ڴ�
* @param[in]  buf_size   �ڴ泤��
*/
inline uint32_t crc32(const unsigned char *buf,
                      size_t buf_size)
{
    return crc32(0, buf, buf_size);
}

/*!
* @brief      ��ʼֵΪ0�ļ����ļ���CRC32�ĺ���
* @return     int        ����0��ʾ�ɹ��������ʾ��ȡ�ļ�ʧ��
* @param[in]  file_name  �ļ�����
* @param[out] result     ���ص�CRC32ֵ
*/
inline  int crc32_file(const char *file_name,
                       uint32_t *result)
{
    return zce::hash_file<ZCE_Hash_CRC32, 10240>(file_name, (unsigned char *)result);
}

uint16_t crc16(uint16_t crcinit,
               const unsigned char *buf,
               size_t buf_size);

inline uint16_t crc16(const unsigned char *buf,
                      size_t buf_size)
{
    return crc16(0, buf, buf_size);
}

//=====================================================================================================
///�����ַ�����HASH�㷨������
///��ѡ����㷨Ҳ��BKDRHash AP Hash,DJBHash,JS Hash
///ǰ���Ѿ����˺ܶ๤�����Ҿ��ǳ������ˣ�����Ȥ��ȥ�о�һЩ��Щ���£�
///http://blog.csdn.net/liuben/article/details/5050697
///http://www.byvoid.com/blog/string-hash-compare/
///��������ó��棬��ô��ѡ��BKDRHash�ɣ�
///�ҵ���΢�ĸĽ��ǽ�unsigned int ��Ϊ��size_t,
///char *  ��Ϊ��unsigned char * ��ͬʱ�����˳��Ȳ�������Ӧ���������ݣ�

/*!
* @brief      �����ַ�����HASH�㷨������ BKDR �㷨 Hash Function����Щ����û����inline
*             ��ԭ���ǣ���VS2010��Release�汾��O2�£�����û�н���inline�Ż�����
* @return     size_t  ���ص�HASHֵ��ʹ��size_t�ڲ�ͬ�����²�ͬ,����inline���Ż��������Ҫ��
* @param[in]  str     �ַ���
* @param[in]  str_len �ַ�������
*/
size_t bkdr_hash(const unsigned char *str, size_t str_len);

/// AP Hash Function
size_t ap_hash(const unsigned char *str, size_t str_len);

/// JS Hash Function
size_t js_hash(const unsigned char *str, size_t str_len);

/// DJB Hash Function
size_t djb_hash(const unsigned char *str, size_t str_len);

//=====================================================================================================

};

#endif //ZCE_LIB_BYTES_HASH_H_

