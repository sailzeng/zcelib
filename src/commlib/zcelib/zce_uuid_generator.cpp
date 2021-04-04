#include "zce_predefine.h"
#include "zce_log_logging.h"
#include "zce_uuid_generator.h"

/************************************************************************************************************
Class           : ZCE_UUID64
************************************************************************************************************/
ZCE_UUID64::ZCE_UUID64():
    u_uint64_(0)
{
}

ZCE_UUID64::~ZCE_UUID64()
{
}

// < �����
bool ZCE_UUID64::operator < (const ZCE_UUID64 &others) const
{
    if (this->u_uint64_ < others.u_uint64_)
    {
        return true;
    }

    return false;
}

// == �����
bool ZCE_UUID64::operator == (const ZCE_UUID64 &others) const
{
    if (this->u_uint64_ == others.u_uint64_)
    {
        return true;
    }

    return false;
}

//ת��Ϊ�ַ���
const char *ZCE_UUID64::to_string(char *buffer, size_t buf_len, size_t &use_buf) const
{

    //������ݵ�BUFFER�ռ䲻����ֱ�ӷ���NULL
    if (buf_len < LEN_OF_ZCE_UUID64_STR + 1)
    {
        return NULL;
    }

    int ret = snprintf(buffer, buf_len, "%08x-%08x", this->u_2uint32_[1], this->u_2uint32_[0]);
    if (ret < 0 || ret > static_cast<int>( buf_len) )
    {
        return NULL;
    }
    use_buf = LEN_OF_ZCE_UUID64_STR;
    return buffer;
}

// ת�Ƴ�һ��uint64_t�Ľṹ
ZCE_UUID64::operator uint64_t ()
{
    return this->u_uint64_;
}

/************************************************************************************************************
Class           : ZCE_UUID64_Generator
************************************************************************************************************/

//ʵ��ָ��
ZCE_UUID64_Generator *ZCE_UUID64_Generator::instance_ = NULL;

//���캯��
ZCE_UUID64_Generator::ZCE_UUID64_Generator():
    generator_type_(UUID64_GENERATOR_INVALID)
{
}
ZCE_UUID64_Generator::~ZCE_UUID64_Generator()
{
}

//����������ӣ�ע�����������һ���ֲ�ʽϵͳ�������ӣ�
//ʱ��δ���ر����Ҳ����IP��ַ����õ�һ�����Ӹ���һЩ
void ZCE_UUID64_Generator::random_seed(uint32_t seed)
{
    generator_type_ = UUID64_GENERATOR_RANDOM;
    mt_19937_random_.srand(seed);
    mt_11213b_random_.srand(seed);
}

//����һ��UUID64
ZCE_UUID64 ZCE_UUID64_Generator::random_gen()
{
    ZCE_ASSERT(UUID64_GENERATOR_RANDOM == generator_type_);

    //�ֱ���2��������������õ������
    ZCE_UUID64 uuid64_ret;

    uuid64_ret.u_2uint32_[0] = mt_19937_random_.rand();
    uuid64_ret.u_2uint32_[1] = mt_11213b_random_.rand();

    return uuid64_ret;
}

//��ʱ��Ϊ�����ĳ�ʼ����
void ZCE_UUID64_Generator::time_radix(uint16_t identity, uint32_t radix )
{
    generator_type_ = UUID64_GENERATOR_TIME;

    time_radix_seed_.u_16_48_.data1_ = identity;
    time_radix_seed_.u_16_48_.data2_ = static_cast<uint64_t>(radix) * 0x10;
}

//��ʱ��Ϊ��������UUID64
ZCE_UUID64 ZCE_UUID64_Generator::timeradix_gen()
{
    ZCE_ASSERT(UUID64_GENERATOR_TIME == generator_type_);

    ++time_radix_seed_.u_16_48_.data2_;
    return time_radix_seed_;
}

/************************************************************************************************************
Class           : ZCE_UUID128
************************************************************************************************************/
ZCE_UUID128::ZCE_UUID128()
{
    u_4uint32_[0] = 0;
    u_4uint32_[1] = 0;
    u_4uint32_[2] = 0;
    u_4uint32_[3] = 0;
}

ZCE_UUID128::~ZCE_UUID128()
{
}

// < �����
bool ZCE_UUID128::operator < (const ZCE_UUID128 &others) const
{
    if (u_4uint32_[3] < others.u_4uint32_[3])
    {
        return true;
    }
    else if (u_4uint32_[3] > others.u_4uint32_[3])
    {
        return false;
    }

    //else (u_4uint32_[3] == others.u_4uint32_[3])

    if (u_4uint32_[2] < others.u_4uint32_[2])
    {
        return true;
    }
    else if (u_4uint32_[2] > others.u_4uint32_[2])
    {
        return false;
    }

    //else (u_4uint32_[2] == others.u_4uint32_[2])

    if (u_4uint32_[1] < others.u_4uint32_[1])
    {
        return true;
    }
    else if (u_4uint32_[1] > others.u_4uint32_[1])
    {
        return false;
    }

    //else (u_4uint32_[1] == u_4uint32_[1])

    if (u_4uint32_[0] < others.u_4uint32_[0])
    {
        return true;
    }
    else
    {
        return false;
    }
}

// == �����
bool ZCE_UUID128::operator == (const ZCE_UUID128 &others) const
{
    if (this->u_4uint32_[0] == others.u_4uint32_[0] &&
        this->u_4uint32_[1] == others.u_4uint32_[1] &&
        this->u_4uint32_[2] == others.u_4uint32_[2] &&
        this->u_4uint32_[3] == others.u_4uint32_[3])
    {
        return true;
    }

    return false;
}

//ת��Ϊ�ַ���,������õĸ�ʽ�Ǳ�׼��8-4-4-4-12��������GUID��8-4-4-16�ĸ�ʽ
const char *ZCE_UUID128::to_string(char *buffer, size_t buf_len, size_t &use_buf) const
{

    //������ݵ�BUFFER�ռ䲻�����ɴ�ʲô������,ֱ�ӷ���NULL,����Ҫ����'\0'
    if (buf_len < LEN_OF_ZCE_UUID128_STR + 1)
    {
        return NULL;
    }

    //���8-4-4-4-12�ĸ�ʽ
    int ret = snprintf(buffer, buf_len, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                       this->u_16uint8_[15],
                       this->u_16uint8_[14],
                       this->u_16uint8_[13],
                       this->u_16uint8_[12],
                       this->u_16uint8_[11],
                       this->u_16uint8_[10],
                       this->u_16uint8_[9],
                       this->u_16uint8_[8],
                       this->u_16uint8_[7],
                       this->u_16uint8_[6],
                       this->u_16uint8_[5],
                       this->u_16uint8_[4],
                       this->u_16uint8_[3],
                       this->u_16uint8_[2],
                       this->u_16uint8_[1],
                       this->u_16uint8_[0]);

    if (ret < 0 || ret > static_cast<int>(buf_len))
    {
        return NULL;
    }
    use_buf = LEN_OF_ZCE_UUID128_STR;
    return buffer;
}

/************************************************************************************************************
Class           : ZCE_UUID128_Generator
************************************************************************************************************/

//ʵ��ָ��
ZCE_UUID128_Generator *ZCE_UUID128_Generator::instance_ = NULL;

//���캯��
ZCE_UUID128_Generator::ZCE_UUID128_Generator() :
    generator_type_(UUID128_GENERATOR_INVALID)
{
}
ZCE_UUID128_Generator::~ZCE_UUID128_Generator()
{
}

//����������ӣ�ע�����������һ���ֲ�ʽϵͳ�������ӣ�
//ʱ��δ���ر����Ҳ����IP��ַ����õ�һ�����Ӹ���һЩ
void ZCE_UUID128_Generator::random_seed(uint32_t seed)
{
    generator_type_ = UUID128_GENERATOR_RANDOM;
    mt_19937_random_.srand(seed);
    mt_11213b_random_.srand(seed);
}

//����һ��UUID64
ZCE_UUID128 ZCE_UUID128_Generator::random_gen()
{
    ZCE_ASSERT(UUID128_GENERATOR_RANDOM == generator_type_);

    //�ֱ���2��������������õ������
    ZCE_UUID128 uuid64_ret;

    uuid64_ret.u_4uint32_[0] = mt_19937_random_.rand();
    uuid64_ret.u_4uint32_[1] = mt_19937_random_.rand();
    uuid64_ret.u_4uint32_[2] = mt_11213b_random_.rand();
    uuid64_ret.u_4uint32_[3] = mt_11213b_random_.rand();

    return uuid64_ret;
}

//��ʱ��Ϊ�����ĳ�ʼ����
void ZCE_UUID128_Generator::time_radix(uint32_t identity, uint32_t radix)
{
    generator_type_ = UUID128_GENERATOR_TIME;

    time_radix_seed_.u_32_32_64_.data1_ = identity;
    time_radix_seed_.u_32_32_64_.data1_ = radix;
}

//��ʱ��Ϊ��������UUID64
ZCE_UUID128 ZCE_UUID128_Generator::timeradix_gen()
{
    ZCE_ASSERT(UUID128_GENERATOR_TIME == generator_type_);

    ++time_radix_seed_.u_32_32_64_.data3_;
    return time_radix_seed_;
}