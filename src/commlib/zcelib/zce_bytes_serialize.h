#ifndef ZCE_LIB_BYTES_SERIALIZATION_H_
#define ZCE_LIB_BYTES_SERIALIZATION_H_

#include "zce_bytes_base.h"
#include "zce_trace_log_debug.h"

//===========================================================================================================
//流编码处理的类

class ZCE_Serialized_Save;

//辅助处理保存数据的一些类
template<typename val_type >
class ZCE_ClassSave_Help
{
public:
    void save_help(ZCE_Serialized_Save *ssave, const val_type &val);
};

template<>
class ZCE_ClassSave_Help<std::string>
{
public:
    void save_help(ZCE_Serialized_Save *ssave, const std::string &val);
};

template<typename vector_type >
class ZCE_ClassSave_Help<std::vector<vector_type> >
{
public:
    void save_help(ZCE_Serialized_Save *ssave, const std::vector<vector_type> &val);
};

template<typename list_type >
class ZCE_ClassSave_Help<std::list<list_type> >
{
public:
    void save_help(ZCE_Serialized_Save *ssave, const std::list<list_type> &val);
};

template<typename key_type,typename data_type >
class ZCE_ClassSave_Help<std::map<key_type,data_type> >
{
public:
    void save_help(ZCE_Serialized_Save *ssave, const std::map<key_type, data_type> &val);
};


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
    inline bool is_good()
    {
        return is_good_;
    }

    ///设置错误标志
    inline void set_bad()
    {
        is_good_ = false;
    }

    ///重置
    void reset();

    ///写入一个数据,只有特化处理函数，
    template<typename val_type>
    bool save(const val_type &/*val*/)
    {
        return false;
    }

    ///保存枚举值
    template<typename val_type  >
    void save(const typename std::enable_if<std::is_enum<val_type>::value, val_type>::type &val)
    {
        return save_enum(val);
    }
    template<typename enum_type >
    void save_enum(const enum_type &val)
    {
        save_arithmetic(static_cast<const int &>(val));
    }

    ///保存数值类型
    template<typename val_type >
    void save(const typename std::enable_if<std::is_arithmetic<val_type>::value, val_type>::type &val)
    {
        return save_arithmetic(val);
    }
    void save_arithmetic(const bool &val);
    void save_arithmetic(const char &val);
    void save_arithmetic(const unsigned char &val);
    void save_arithmetic(const short &val);
    void save_arithmetic(const unsigned short &val);
    void save_arithmetic(const int &val);
    void save_arithmetic(const unsigned int &val);
    void save_arithmetic(const int64_t &val);
    void save_arithmetic(const uint64_t &val);
    void save_arithmetic(const float &val);
    void save_arithmetic(const double &val);

    ///保存数组
    template<typename val_type >
    void save(const typename std::enable_if<std::is_array<val_type>::value, val_type>::type &val)
    {
        // consider alignment
        std::size_t count = sizeof(val) / (
            static_cast<const char *>(static_cast<const void *>(&val[1]))
            - static_cast<const char *>(static_cast<const void *>(&val[0]))
            );
        return save_array(val, count);
    }
    template<typename array_type >
    void save_array(const array_type *ary, size_t count)
    {
        //ZCE_ASSERT(count < std::numeric_limits<unsigned int>::max());
        ZCE_ASSERT(count < 0xFFFFFFFFll);
        this->save_arithmetic(static_cast<unsigned int>(count));
        for (size_t i = 0; i < count && is_good_;++i)
        {
            this->save(*(ary+i));
        }
        return;
    }
    ///特化，对字符串进行加速
    template<>
    void save_array(const char *ary, size_t count)
    {
        ZCE_ASSERT(count < 0xFFFFFFFFll);
        this->save_arithmetic(static_cast<unsigned int>(count));
        if (is_good_)
        {
            if (write_pos_ + count < end_pos_)
            {
                is_good_ = false;
                return;
            }
            memcpy(write_pos_, ary, count);
            write_pos_ += count;
        }
    }
    template<>
    void save_array(const unsigned char *ary, size_t count)
    {
        ZCE_ASSERT(count < 0xFFFFFFFFll);
        this->save_arithmetic(static_cast<unsigned int>(count));
        if (is_good_)
        {
            if (write_pos_ + count < end_pos_)
            {
                is_good_ = false;
                return;
            }
            memcpy(write_pos_, ary, count);
            write_pos_ += count;
        }
    }

    ///保存类，这儿要用一些偏特化的能力
    template<typename val_type >
    void save(const typename std::enable_if<std::is_class<val_type>::value, val_type>::type &val)
    {
        ZCE_ClassSave_Help<val_type> ssave;
        ssave.save_help<val_type>(this, val);
        return;
    }

    ///使用<<操作符号写入数据，主要是便于外部数据统一使用<<操作符.外部可以用这样的函数
    ///ZCE_Serialized_Save& operator <<(ZCE_Serialized_Save &dr_encode,const val_type &val);
    ///bool operator <<(ZCE_Serialized_Save &dr_encode,const val_type &val);
    template<typename val_type>
    ZCE_Serialized_Save & operator &(val_type val)
    {
        this->save<val_type>(val);
        return *this;
    }

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

//辅助类，save_help 函数
//这个类必须写了虚化
template<typename val_type>
void ZCE_ClassSave_Help<val_type>::save_help(ZCE_Serialized_Save *ssave,
    const val_type &val)
{
    val.serialize(ssave);
}

void ZCE_ClassSave_Help<std::string>::save_help(ZCE_Serialized_Save *ssave, 
    const std::string &val)
{
    ssave->save_array(val.c_str(), val.length());
}

template<typename vector_type>
void ZCE_ClassSave_Help<std::vector<vector_type> >::save_help(ZCE_Serialized_Save *ssave, 
    const std::vector<vector_type> &val)
{
    size_t v_size = val.size();
    this->save_arithmetic<unsigned int>(v_size);
    for (size_t i = 0; i < v_size && is_good_; ++i)
    {
        this->save<vector_type>(val[i]);
    }
    return;
}

template<typename list_type>
void ZCE_ClassSave_Help<std::list<list_type> >::save_help(ZCE_Serialized_Save *ssave,
    const std::list<list_type> &val)
{
    size_t v_size = val.size();
    this->save_arithmetic<unsigned int>(v_size);
    for (size_t i = 0; i < v_size && is_good_; ++i)
    {
        this->save<list_type>(val[i]);
    }
    return;
}


template<typename key_type, typename data_type >
void ZCE_ClassSave_Help<std::map<key_type, data_type> >::save_help(ZCE_Serialized_Save *ssave,
    const std::map<key_type, data_type> &val)
{
    size_t v_size = val.size();
    this->save_arithmetic<unsigned int>(v_size);
    for (size_t i = 0; i < v_size && is_good_; ++i)
    {
        this->save<list_type>(val[i]);
    }
    return;
}

//===========================================================================================================

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

