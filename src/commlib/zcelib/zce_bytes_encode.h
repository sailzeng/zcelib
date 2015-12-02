/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_data_proc_encode.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013年2月16日
* @brief      几种数据格式的转换算法，BASE16，BASE64等，
*
* @details
*
* http://news.cnblogs.com/n/171287/
*
* 配的上人生歌手的这个名号人屈指可数，陈升算的上一个。
*
* 老爹的故事
* 词/曲：陈升
* 电梯门口的老爹 活在辉煌的从前
* 扶着那双徐蚌会战瘸了的腿
* 孩子们都不了解 讨厌老爹真多嘴
* 他说现在的年轻人 跟他们比差得远
* 我在老弟你这样的年纪
* 南北已经走了好几回
* 好不好 哪天你来听我把故事说完
* 好不好 好不好
* 门口扫地的老爹 杵着酒瓶想从前
* 说着永远都学不好的台湾话
* 他说老弟你吃饱了没 没事陪我聊聊天
* 张着嘴巴 眼眶里面含住眼泪对我说
* 以前的我带兵最英勇
* 别当我是抹布的英雄
* 好不好 哪天你来听我把故事说完
* 好不好 好不好 Oh　
* 我把我的岁月都虚掷磋跎
* 好不好 好不好
* 好不好 哪天你来听我把故事说完
* 好不好 好不好 Oh　
*
*/

#ifndef ZCE_LIB_BYTES_ENCODE_H_
#define ZCE_LIB_BYTES_ENCODE_H_

#include "zce_bytes_base.h"

namespace ZCE_LIB
{

#ifndef ZCE_BASE64_ENCODE_BUFFER
#define ZCE_BASE64_ENCODE_BUFFER(in_len)  ( 4 * (((inlen) + 2) / 3) + 1 )
#endif

/*!
@brief      对一个内存块进行BASE64编码，末尾添加'/0'
@return        int     返回0表示成功，否则标识出错，一般出错原因是out的buffer长度不够
@param[in]     in      要进行编码的内存
@param[in]     inlen   要进行编码的内存长度，
@param[out]    out     编码后的输出的内存，保证长度大于 ( 4 * (((inlen) + 2) / 3) + 1 )
@param[in,out] out_len 返回编码后的长度，
@note       注意这儿主要是按照RFC 1421和 RFC 2045。 而在电子邮件中，根据RFC 822规定，
            每76个字符，还需要加上一个回车换行。
            这儿么没有（注意是没有）按照电子邮件（RFC 822）的要求进行增加回车操作。
*/
int base64_encode(const unsigned char *in,
                  size_t in_len,
                  unsigned char *out,
                  size_t *out_len);

#ifndef ZCE_BASE64_DECODE_BUFFER
#define ZCE_BASE64_DECODE_BUFFER(in_len)  ( (in_len) / 4 *3 )
#endif

/*!
@brief      对一个内存块进行base64的解码
@return     int    返回0表示成功，返回-1表示错误，最大可能是编码错误
@param[in]     in      要进行解码的内存
@param[in]     in_len  要进行解码的内存长度，
@param[out]    out     解码码后的输出的内存，（保证长度为大于(inlen)/4*3），
@param[in,out] out_len 返回编码后的长度，为 in_len/4*3 - N (N为1或者2)
*/
int base64_decode(const unsigned char *in,
                  size_t in_len,
                  unsigned char *out,
                  size_t *out_len);

/*!
@brief      对一个内存块进行BASE16编码，末尾添加'/0'
@return        int     返回0表示成功，否则标识出错，一般出错原因是out的buffer长度不够
@param[in]     in      要进行编码的内存
@param[in]     in_len  要进行编码的内存长度，
@param[out]    out     编码后的输出的内存，保证长度大于in_len *2 +1
@param[in,out] out_len 返回编码后的长度，为 in_len *2
*/
int base16_encode(const unsigned char *in,
                  size_t in_len,
                  unsigned char *out,
                  size_t *out_len);

/*!
@brief      对一个内存块进行base16的解码
@return        int    返回0表示成功，返回-1表示错误，最大可能是编码错误,或者out的buffer空间不够
@param[in]     in      要进行解码的内存
@param[in]     inlen   要进行解码的内存长度，
@param[out]    out     解码码后的输出的内存，（保证长度大于(inlen) / 2）
@param[in,out] out_len 返回编码后的长度，为in_len/2
*/
int base16_decode(const unsigned char *in,
                  size_t in_len,
                  unsigned char *out,
                  size_t *out_len);

};


//===========================================================================================================
//流编码处理的类

/*!
* @brief      对数据进行编码处理的类，将数据变成流，
*
* @note       没有考虑对齐等问题，
*             BTW：对于写入，我们不在溢出保护上做过多努力，那是你负责的事情
*/
class ZCE_Serialized_Save  
{
public:

    ///构造函数
    ZCE_Serialized_Save(char *write_buf, size_t buf_len);

    ~ZCE_Serialized_Save();

public:

    ///返回当前类是否正常，
    inline bool is_good();

    ///设置错误标志
    inline void set_bad();

    ///重置
    void reset();



    ///写入一个数据,只有特化处理函数，
    template<typename val_type>
    bool save(const val_type /*val*/)
    {
        return false;
    }

    ///写入一个vector,注意，只支持到32位的个数的vector
    template<typename  vector_type>
    bool save(std::vector<vector_type> &val)
    {
        size_t v_size = vector_data.size();
        this->save<unsigned int>(v_size);
        for (size_t i = 0; i < v_size; ++i)
        {
            this->save<vector_type>(vector_data[i]);
        }
        if (false == is_good_)
        {
            return false;
        }
        return true;
    }

    template<> bool save(int val)
    {
        const size_t SIZE_OF_VALUE = sizeof(int);
        if (write_pos_ + SIZE_OF_VALUE > end_pos_)
        {
            is_good_ = false;
            return is_good_;
        }
        ZBEUINT32_TO_BYTE(write_pos_, val);
        write_pos_ += SIZE_OF_VALUE;

        return is_good_;
    }
    template<> bool save(unsigned int val)
    {
        const size_t SIZE_OF_VALUE = sizeof(unsigned int);
        if (write_pos_ + SIZE_OF_VALUE > end_pos_)
        {
            is_good_ = false;
            return is_good_;
        }
        ZBEUINT32_TO_BYTE(write_pos_, val);
        write_pos_ += SIZE_OF_VALUE;

        return is_good_;
    }
    template<> bool save(float val)
    {
        const size_t SIZE_OF_VALUE = sizeof(float);
        if (write_pos_ + SIZE_OF_VALUE > end_pos_)
        {
            is_good_ = false;
            return is_good_;
        }
        ZFLOAT_TO_BYTE(write_pos_, val);
        write_pos_ += SIZE_OF_VALUE;

        return is_good_;
    }
    template<> bool save(double val)
    {
        const size_t SIZE_OF_VALUE = sizeof(double);
        if (write_pos_ + SIZE_OF_VALUE > end_pos_)
        {
            is_good_ = false;
            return is_good_;
        }
        ZDOUBLE_TO_BYTE(write_pos_, val);
        write_pos_ += SIZE_OF_VALUE;

        return is_good_;
    }

    ///使用<<操作符号写入数据，主要是便于外部数据统一使用<<操作符.外部可以用这样的函数
    ///ZCE_Serialized_Save& operator <<(ZCE_Serialized_Save &dr_encode,const val_type &val);
    ///bool operator <<(ZCE_Serialized_Save &dr_encode,const val_type &val);
    template<typename val_type>
    ZCE_Serialized_Save &operator <<(val_type val);

    ///写入一个数组
    template<typename ary_type>
    bool save_array(const ary_type *ary, size_t ary_size);

    
    template<typename vector_type>
    bool save_vector(const std::vector<vector_type> &vector_data);

protected:

    ///状态是否正确，如果写入位置超出缓冲区的结尾，会置为false
    bool is_good_;

    ///保存数据，被写的buffer，
    char *write_buf_;
    ///buf的长度
    size_t buf_len_;

    ///buf的最后的位置，用于判断加快处理
    char *end_pos_;
    ///当前写入的pos位置，
    char *write_pos_;
};


//返回当前类是否正常，
inline bool ZCE_Serialized_Save::is_good()
{
    return is_good_;
}

inline void ZCE_Serialized_Save::set_bad()
{
    is_good_ = false;
}


template<typename vector_type>
bool ZCE_Serialized_Save::save_vector(const std::vector<vector_type> &vector_data)
{
    size_t v_size = vector_data.size();
    this->save<unsigned int>(v_size);
    for (size_t i = 0; i < v_size; ++i)
    {
        this->save<vector_type>(vector_data[i]);
    }
    if (false == is_good_)
    {
        return false;
    }
    return true;
}

//
template<typename val_type>
ZCE_Serialized_Save &ZCE_Serialized_Save::operator << (val_type val)
{
    this->save<val_type>(val);
    return *this;
}


/*!
* @brief      对数据进行解码码处理的类，将流变成数据，
*
* @note       读取对于边界有有一些安全处理，避免输入数据就有问题的情况
*/
class ZCE_Serialized_Load
{
public:


    /*!
    * @brief      构造函数
    * @param      read_buf 输入的数据，不会对数据进行改动
    * @param      buf_len  数据的长度
    */
    ZCE_Serialized_Load(const char *read_buf, size_t buf_len);

    ///析构函数
    ~ZCE_Serialized_Load();

public:
    ///返回当前类是否正常，BTW：我们不在溢出保护上做努力，那是你负责的事情
    inline bool is_good();

    ///设置错误标志
    inline void set_bad();

    ///重置开始位置和good标志位
    void reset();

    ///写入一个数据,只有特化处理函数，
    template<typename val_type>
    bool read(val_type *val);

    ///使用>>操作符号写入数据，主要是便于外部数据统一使用<<操作符,外部可以用这样的函数
    ///ZCE_Serialized_Load& operator >>(ZCE_Serialized_Load &dr_encode,val_type *val);
    ///bool operator >>(ZCE_Serialized_Load &dr_encode,val_type *val);
    template<typename val_type>
    ZCE_Serialized_Load &operator >>(val_type *val);

    /*!
    * @brief      读取一个数组
    * @tparam        ary_type 数组数据类型
    * @return        ZCE_Serialized_Save& 返回自己的引用
    * @param[out]    ary       数组类型
    * @param[in,out] ary_size  数组长度,输入输出参数，输入表示ary的长度，输出表示实际读取的长度
    */
    template<typename ary_type>
    bool read_array(ary_type *ary, size_t *ary_size);


    /*!
    * @brief      读出一个vector
    * @tparam     vector_type vector数据类型
    * @return     ZCE_Serialized_Load& 返回自己的引用
    * @param[out]    vector_data  读取返回的vector
    * @param[in,out] ary_size     输入表示预期的最大vector size，输出表示实际尺寸
    */
    template<typename vector_type>
    bool read_vector(std::vector<vector_type> *vector_data, size_t *ary_size);

protected:

    ///状态是否正确，如果写入位置超出缓冲区的结尾，会置为false
    bool is_good_;

    ///读取流的BUFFER
    const char *read_buf_;
    ///BUF的长度
    size_t buf_len_;

    ///BUFFER结束的位置
    const char *end_pos_;
    ///当前读取的位置
    const char *read_pos_;

};

inline bool ZCE_Serialized_Load::is_good()
{
    return is_good_;
}

inline void ZCE_Serialized_Load::set_bad()
{
    is_good_ = false;
}

template<typename vector_type>
bool ZCE_Serialized_Load::read_vector(std::vector<vector_type> *vector_data, size_t *vector_size)
{
    unsigned int read_ary_size = 0;
    if (!this->read<unsigned int>(&read_ary_size))
    {
        return false;
    }
    if (read_ary_size > *vector_size)
    {
        is_good_ = false;
        return false;
    }
    vector_size = read_ary_size;
    vector_data->resize(read_ary_size);

    vector_type read_data;
    for (unsigned int i = 0; i < read_ary_size; ++i)
    {
        this->read<vector_type>(read_data);
        (*vector_data)[i] = read_data;
    }
    if (false == is_good_)
    {
        return false;
    }
    return true;
}


template<typename val_type>
ZCE_Serialized_Load &ZCE_Serialized_Load::operator >> (val_type *val)
{
    this->read<val_type>(val);
    return *this;
}


#endif

