/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_uuid_64_generator.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013��2��4��
* @brief      Universally Unique Identifier ��дUUID��֧��2��UUID
*             64bit��UUID���������׼����Ҫ����һЩ������Ҫһ���̵�ID��ʾ�����⻷��
*             64λ��Ψһ��ʾ��
*             128Ϊ��UUID��������UUID��
*
*             ����ط���Ҫ��Ϊ���ṩ�����һ�ַ���������һ��Ψһ��ʶ��
*             ����Ϸ��Ŀ�飬��Ҫ������Ϸ���߱�ʶ�ȷ�Χ��Ҫ�����Ķ�������Ҫ����
*             ����ĳ����Ʒ����ת�����Ҹ��˶Ե���ʹ�����������ΪID��Ч���ֱ��������
*             Ч����������
*
* @details    ֧�����ֲ�����ʽ�����������ʱ��Ϊ�����ķ�ʽ
*
* @note
*
* ����
* �ʣ����� ��������
*
* �ҷԸ����ҵ������������ǣ������
* ����û�������ҵĻ����ұ��Լ��ߵ����
* �����ð���������ȥ��
* Զ�������˺Ž�����������������ʲô��˼��
* ��˵��֪������ʲôҲû������ʲôҲû������
*
* �ڴ��ſڣ�����ס�ң�
* �ʣ������������Ķ�ȥ�أ��ҵ����ˣ���
* ���Ҳ�֪��������˵����ֻ���뿪������뿪�����
* �뿪�����ǰ�ߣ���ǰ�ߣ�������ҴﵽĿ���Ψһ�취����
* ����ô��֪������Ŀ���ˣ������ʡ�
* ���ǵġ��һش�
* ���Ҹոո������ˣ��뿪������뿪�����������ҵ�Ŀ�ꡣ��
* ������û�д��Ͽ����أ�����˵����ʲô������Ҳ��Ҫ������˵��
* ����;����ô�������������һ·���ҵò���������
* ����һ���ᣬ���ġ�
*
* ʲô����Ҳ���ܴ���ң�
* ���˵��ǣ������һ�Σ�����û�о�ͷ���ó̰�����
*
*/
#ifndef _ZCE_LIB_UUID_GENERATOR_H_
#define _ZCE_LIB_UUID_GENERATOR_H_

#include "zce_boost_random.h"

/************************************************************************************************************
Class           : ZCE_UUID64
************************************************************************************************************/

#pragma pack(push,1)

/*!
* @brief      UUID64��һ�ֱ�ʾ����
*             ZCE_UUID64_16_48 ��1��16bit����ֵ+48λ����ֵ
*             ��ʾUUID64����������һЩ���ⳡ�ϲ���Ψһ��ʾ��
*/
struct ZCE_UUID64_16_48
{
    uint64_t       data1_: 16;
    uint64_t       data2_: 48;
};

/*!
* @brief      64λ��UUID���ڲ��ֳ�3�ֱ�ʾ������Ҳ�ṩ�˱Ƚ�==��<�Ȳ���
*             @li 64λ������
*             @li 16λ����+48Ϊ������
*             @li 2��32λ������
*
*/
class ZCE_UUID64
{
public:
    ///���캯��
    ZCE_UUID64();
    ///��������
    ~ZCE_UUID64();

    /// < ���������
    bool operator < (const ZCE_UUID64 &others) const;
    /// == �����
    bool operator == (const ZCE_UUID64 &others) const;

    /// ת�Ƴ�һ��uint64_t�Ľṹ
    operator uint64_t();

    /// ת��Ϊ�ַ���
    const char *to_string(char *buffer, size_t buf_len, size_t &use_buf) const;


public:

    ///�ṹ���ݵļ��ֲ�ͬ�ı�﷽ʽ
    union
    {
        ///����64�ֽڵı�ʾ����
        uint64_t          u_uint64_;
        ///2��32�ֽڵı�ʾ����
        uint32_t          u_2uint32_[2];
        ///16bit+48Bit�ı�ʾ����
        ZCE_UUID64_16_48  u_16_48_;
    };

public:

    ///UUID����ַ�������󳤶ȣ�������'\0',��ʽXXXXXXXX-XXXXXXXX
    static const size_t   LEN_OF_ZCE_UUID64_STR = 17;

};

#pragma pack(pop)

/************************************************************************************************************
Class           : ZCE_UUID64_Generator
************************************************************************************************************/

/*!
* @brief      UUID64�Ĳ��������ṩ�������������ʱ��Ϊ��������������ķ���
*             @li ���������������һ�������uint64_t������,��Ȼ��ҷ�������ظ��ɺܵ�
*             @li ��ʱ��Ϊ�����Ĳ���������16λ��ֵ���Լ����壬����48λ��������ʱ��Ϊ����
*                 ��Ȼ�����ÿ�β������ۼ�һ�Σ����ַ����ʺ�С��ģ�ķֲ���16λ��ʾ�ֲ���
*                 ��������ÿ���������Լ�����UUID�������ֻҪ1s������UUID������������16
*                 λ���ͱȽϺܰ�ȫ���������ʼ��Ӽ��Ӧ���� sonicmao������Ը���UUID����
*                 ��ʲô����������������
*
*/
class ZCE_UUID64_Generator
{
    //UUID����������
    enum UUID64_GENERATOR_TYPE
    {
        ///��Ч�Ĳ�����ʽ
        UUID64_GENERATOR_INVALID,

        ///��������ķ�������
        UUID64_GENERATOR_RANDOM,
        ///���¼���Ϊ��������
        UUID64_GENERATOR_TIME,
    };

public:

    /*!
    * @brief      ���캯��
    */
    ZCE_UUID64_Generator();

    /*!
    * @brief      ��������
    */
    ~ZCE_UUID64_Generator();

public:

    /*!
    * @brief      �����������,�������������
    *             �ͻ�ȷ�ϲ�����ʽ��UUID64_GENERATOR_RANDOM�����ܸ����ˡ�
    * @param      seed �����������,
    */
    void random_seed(uint32_t seed);
    /*!
    @brief      ͨ�����������һ��UUID64
    @return     ZCE_UUID64 ������UUID
    */
    ZCE_UUID64 random_gen();

    /*!
    * @brief      ��ʱ��Ϊ�����ĳ�ʼ����radixһ������õ�ǰʱ��time_t
    *             ������������󣬾ͻ�ȷ�ϲ�����ʽ��UUID64_GENERATOR_TIME�����ܸ����ˡ�
    * @param      identity ��ǰ��Ψһ��ʾ�����������ID����Ϣ
    * @param      radix    ʱ�����
    */
    void time_radix(uint16_t identity, uint32_t radix = static_cast<uint32_t> (time(NULL)));

    /*!
    * @brief      ��ʱ��Ϊ��������UUID64
    * @return     ZCE_UUID64 ������UUID64
    */
    ZCE_UUID64 timeradix_gen();

protected:

    ///������ʵ��ָ��
    static ZCE_UUID64_Generator *instance_;

protected:

    /*!
    * @brief      ���Ӻ���
    * @return     ZCE_UUID64_Generator* ���ص�ʵ��ָ��
    */
    static ZCE_UUID64_Generator *instance();
    /*!
    * @brief      ����ʵ��ָ��
    */
    static void clean_instance();

protected:

    ///������ʹ��ʲô������ʽ
    UUID64_GENERATOR_TYPE         generator_type_;

    ///���������1
    zce::random_mt19937       mt_19937_random_;
    ///���������2
    zce::random_mt11213b      mt_11213b_random_;

    ///�����������
    ZCE_UUID64                    time_radix_seed_;


};







/************************************************************************************************************
Class           : ZCE_UUID128
************************************************************************************************************/
///32bit����+32����+64λ�����ı�ʾ����
struct ZCE_UUID128_32_32_64
{

    //
    uint32_t       data1_;
    uint32_t       data2_;
    uint64_t       data3_;
};

///��׼��UUID�ĸ�ʽ
struct ZCE_UUID128_32_16_16_16_48
{
    uint64_t       data1_ : 32;
    uint64_t       data2_ : 16;
    uint64_t       data3_ : 16;
    uint64_t       data4_ : 16;
    uint64_t       data5_ : 48;
};

///΢���GUID�ĸ�ʽ
struct ZCE_UUID128_32_16_16_64
{
    uint64_t       data1_ : 32;
    uint64_t       data2_ : 16;
    uint64_t       data3_ : 16;
    uint64_t       data4_;

};

/*!
* @brief      UUID128��128λ��Ψһ��ʶ��
*             ��5�б�ʾ������
*             @li 16���ֽ�
*             @li 4��32Ϊ������
*             @li 32λ����+32λ����+64λ�����ı�ʾ����
*             @li 32λ����+16λ����+16λ����+16λ����+48λ��������׼��UUID��ʾ����
*             @li 32λ����+16λ����+16λ����+64λ������΢���UUID��ʾ������GUID��
*
*/
class ZCE_UUID128
{
public:
    ///���캯��
    ZCE_UUID128();
    ///��������
    ~ZCE_UUID128();

    /// < ���������
    bool operator < (const ZCE_UUID128 &others) const;
    /// == �����
    bool operator == (const ZCE_UUID128 &others) const;

    /// ��UUID8-4-4-4-12�ĸ�ʽ����ת��Ϊ�ַ���
    const char *to_string(char *buffer, size_t buf_len, size_t &use_buf) const;

public:
    ///UUID���ַ�����ʾ�ĳ���
    static const size_t LEN_OF_ZCE_UUID128_STR = 36;

public:

    ///5�����ݱ�ʾ����
    union
    {
        ///16���ֽڵ����
        uint8_t                      u_16uint8_[16];
        ///4��32Ϊ�ֽڵ����
        uint32_t                     u_4uint32_[4];
        ///32λ����+32λ����+64λ����
        ZCE_UUID128_32_32_64         u_32_32_64_;
        ///��׼��UUID�ı�ʶ����
        ZCE_UUID128_32_16_16_16_48   u_32_16_16_16_48_;
        ///΢���GUID�ı�ʶ����
        ZCE_UUID128_32_16_16_64      u_32_16_16_64_;

    };


};



/************************************************************************************************************
Class           : ZCE_UUID128_Generator UUID�ķ�����
************************************************************************************************************/

/*!
* @brief      ZCE_UUID128_Generator UUID�ķ�����
*
*/
class ZCE_UUID128_Generator
{
    //
    enum UUID128_GENERATOR_TYPE
    {
        //
        UUID128_GENERATOR_INVALID,

        ///��������ķ�������
        UUID128_GENERATOR_RANDOM,
        ///���¼���Ϊ��������
        UUID128_GENERATOR_TIME,
    };

public:

    ///���캯��
    ZCE_UUID128_Generator();
    ///��������
    ~ZCE_UUID128_Generator();

public:

    /*!
    * @brief      ע�������������
    * @param      seed ����
    */
    void random_seed(uint32_t seed);
    /*!
    * @brief      ����һ��UUID128��
    * @return     ZCE_UUID128
    */
    ZCE_UUID128 random_gen();

    /*!
    * @brief      ��ʱ��Ϊ�����ĳ�ʼ����radixһ������õ�ǰʱ��time_t����ȻҲ���Բ��ã���������һ����ʶ��
    * @param      identity
    * @param      radix
    */
    void time_radix(uint32_t identity, uint32_t radix = static_cast<uint32_t> (time(NULL)));

    /*!
    * @brief      ��ʱ��Ϊ��������UUID64
    * @return     ZCE_UUID128
    */
    ZCE_UUID128 timeradix_gen();

protected:

    ///������ʹ��ʲô������ʽ
    UUID128_GENERATOR_TYPE        generator_type_;

    ///���������1
    zce::random_mt19937       mt_19937_random_;
    ///���������2
    zce::random_mt11213b      mt_11213b_random_;

    ///
    ZCE_UUID128                   time_radix_seed_;

protected:

    //������ʵ��ָ��
    static ZCE_UUID128_Generator *instance_;

};

#endif //# ZCE_LIB_UUID_64_GENERATOR_H_

