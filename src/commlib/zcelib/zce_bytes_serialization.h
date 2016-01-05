#ifndef ZCE_LIB_BYTES_SERIALIZATION_H_
#define ZCE_LIB_BYTES_SERIALIZATION_H_

#include "zce_bytes_base.h"

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


#endif //ZCE_LIB_BYTES_SERIALIZATION_H_

