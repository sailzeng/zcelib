#ifndef ZCE_LIB_BYTES_SERIALIZATION_H_
#define ZCE_LIB_BYTES_SERIALIZATION_H_

#include "zce/bytes/bytes_common.h"
#include "zce/logger/logging.h"

//===========================================================================================================
//流编码处理的类，写入流的处理类

class ZCE_Serialize_Write;

//辅助处理保存数据的一些类
template<typename val_type >
class ZCE_ClassSerialize_WriteHelp
{
public:
    void write_help(ZCE_Serialize_Write *ssave,const val_type &val);
};

template<>
class ZCE_ClassSerialize_WriteHelp<std::string>
{
public:
    void write_help(ZCE_Serialize_Write *ssave,const std::string &val);
};

template<typename vector_type >
class ZCE_ClassSerialize_WriteHelp<std::vector<vector_type> >
{
public:
    void write_help(ZCE_Serialize_Write *ssave,const std::vector<vector_type> &val);
};

template<typename list_type >
class ZCE_ClassSerialize_WriteHelp<std::list<list_type> >
{
public:
    void write_help(ZCE_Serialize_Write *ssave,const std::list<list_type> &val);
};

template<typename key_type,typename data_type >
class ZCE_ClassSerialize_WriteHelp<std::map<key_type,data_type> >
{
public:
    void write_help(ZCE_Serialize_Write *ssave,const std::map<key_type,data_type> &val);
};

/*!
* @brief      对数据进行编码处理的类，将数据变成流，
*
* @note       没有考虑对齐等问题，
*             BTW：对于写入，我们不在溢出保护上做过多努力，那是你负责的事情
*/
class ZCE_Serialize_Write
{
public:

    ///构造函数
    ZCE_Serialize_Write(char *write_buf,size_t buf_len);

    ~ZCE_Serialize_Write();

public:

    ///返回当前类是否正常，
    bool is_good()
    {
        return is_good_;
    }

    ///设置错误标志
    void set_bad()
    {
        is_good_ = false;
    }

    ///重置,重置开始位置和good标志位
    void reset()
    {
        write_pos_ = write_buf_;
        is_good_ = true;
    }

    ///已经写入的长度
    size_t write_len()
    {
        return write_pos_ - write_buf_;
    }

    ///保存枚举值,利用SFINA的原则，进行重载
    template<typename val_type  >
    void write(const typename std::enable_if<std::is_enum<val_type>::value,val_type>::type &val)
    {
        return save_enum(val);
    }
    template<typename enum_type >
    void save_enum(const enum_type &val)
    {
        write_arithmetic(static_cast<const int &>(val));
    }

    ///保存数值类型
    template<typename val_type >
    typename std::enable_if<std::is_arithmetic<val_type>::value >::type write(const val_type &val)
    {
        return write_arithmetic(val);
    }
    void write_arithmetic(const bool &val);
    void write_arithmetic(const char &val);
    void write_arithmetic(const unsigned char &val);
    void write_arithmetic(const short &val);
    void write_arithmetic(const unsigned short &val);
    void write_arithmetic(const int &val);
    void write_arithmetic(const unsigned int &val);
    void write_arithmetic(const int64_t &val);
    void write_arithmetic(const uint64_t &val);
    void write_arithmetic(const float &val);
    void write_arithmetic(const double &val);

    ///保存数组
    template<typename val_type >
    typename std::enable_if<std::is_array<val_type>::value >::type write(const val_type &val)
    {
        // consider alignment
        std::size_t count = sizeof(val) / (
            static_cast<const char *>(static_cast<const void *>(&val[1]))
            - static_cast<const char *>(static_cast<const void *>(&val[0]))
            );
        return write_array(val,count);
    }
    template<typename array_type >
    void write_array(const array_type *ary,size_t count)
    {
        //其实用下面注释的这个代码会更酷一点，但不知道为啥有告警，放弃，
        //ZCE_ASSERT(count < std::numeric_limits<unsigned int>::max());
        ZCE_ASSERT(count < 0xFFFFFFFFll);
        this->write_arithmetic(static_cast<unsigned int>(count));
        for (size_t i = 0; i < count && is_good_; ++i)
        {
            this->write(*(ary + i));
        }
        return;
    }

    ///特化，对字符串进行加速
    void write_array(const char *ary,size_t count)
    {
        ZCE_ASSERT(count < 0xFFFFFFFFll);
        this->write_arithmetic(static_cast<unsigned int>(count));
        if (is_good_)
        {
            if (write_pos_ + count > end_pos_)
            {
                is_good_ = false;
                return;
            }
            memcpy(write_pos_,ary,count);
            write_pos_ += count;
        }
    }

    ///字符串特化
    void write_array(const unsigned char *ary,size_t count)
    {
        ZCE_ASSERT(count < 0xFFFFFFFFll);
        this->write_arithmetic(static_cast<unsigned int>(count));
        if (is_good_)
        {
            if (write_pos_ + count > end_pos_)
            {
                is_good_ = false;
                return;
            }
            memcpy(write_pos_,ary,count);
            write_pos_ += count;
        }
    }

    ///保存类，这儿要用辅助类实现一些偏特化的能力
    template<typename val_type >
    typename std::enable_if<std::is_class<val_type>::value >::type write(const val_type &val)
    {
        ZCE_ClassSerialize_WriteHelp<val_type> ssave;
        ssave.write_help(this,val);
        return;
    }

    ///使用& 操作符号写入数据，
    template<typename val_type>
    ZCE_Serialize_Write &operator &(const val_type &val)
    {
        this->write(val);
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

//辅助类，write_help 函数的实现

//用保存class辅助处理的 base templates 实现
template<typename val_type>
void ZCE_ClassSerialize_WriteHelp<val_type>::write_help(ZCE_Serialize_Write *ssave,
                                                        const val_type &val)
{
    val.serialize(ssave);
}

//用于保存vector 辅助处理的特化
template<typename vector_type>
void ZCE_ClassSerialize_WriteHelp<std::vector<vector_type> >::write_help(ZCE_Serialize_Write *ssave,
                                                                         const std::vector<vector_type> &val)
{
    size_t v_size = val.size();
    ZCE_ASSERT(v_size < 0xFFFFFFFFll);
    ssave->write_arithmetic(static_cast<unsigned int>(v_size));
    for (size_t i = 0; i < v_size && ssave->is_good(); ++i)
    {
        ssave->write<vector_type>(val[i]);
    }
    return;
}

template<typename list_type>
void ZCE_ClassSerialize_WriteHelp<std::list<list_type> >::write_help(ZCE_Serialize_Write *ssave,
                                                                     const std::list<list_type> &val)
{
    size_t v_size = val.size();
    ZCE_ASSERT(v_size < 0xFFFFFFFFll);
    ssave->write_arithmetic(static_cast<unsigned int>(v_size));
    typename std::list<list_type>::const_iterator iter = val.begin();
    for (size_t i = 0; i < v_size && ssave->is_good(); ++i,++iter)
    {
        ssave->write(*iter);
    }
    return;
}

template<typename key_type,typename data_type >
void ZCE_ClassSerialize_WriteHelp<std::map<key_type,data_type> >::write_help(ZCE_Serialize_Write *ssave,
                                                                             const std::map<key_type,data_type> &val)
{
    size_t v_size = val.size();
    ZCE_ASSERT(v_size < 0xFFFFFFFFll);
    ssave->write_arithmetic(static_cast<unsigned int>(v_size));
    typename std::map<key_type,data_type>::const_iterator iter = val.begin();
    for (size_t i = 0; i < v_size && ssave->is_good(); ++i,++iter)
    {
        ssave->write(iter->first);
        ssave->write(iter->second);
    }
    return;
}

//===========================================================================================================

class ZCE_Serialize_Read;
//辅助处理读取数据的一些类
template<typename val_type >
class ZCE_ClassSerialize_ReadHelp
{
public:
    void read_help(ZCE_Serialize_Read *sload,val_type &val);
};

template<>
class ZCE_ClassSerialize_ReadHelp<std::string>
{
public:
    void read_help(ZCE_Serialize_Read *sload,std::string &val);
};

template<typename vector_type >
class ZCE_ClassSerialize_ReadHelp<std::vector<vector_type> >
{
public:
    void read_help(ZCE_Serialize_Read *sload,std::vector<vector_type> &val);
};

template<typename list_type >
class ZCE_ClassSerialize_ReadHelp<std::list<list_type> >
{
public:
    void read_help(ZCE_Serialize_Read *sload,std::list<list_type> &val);
};

template<typename key_type,typename data_type >
class ZCE_ClassSerialize_ReadHelp<std::map<key_type,data_type> >
{
public:
    void read_help(ZCE_Serialize_Read *sload,std::map<key_type,data_type> &val);
};

/*!
* @brief      对数据进行解码码处理的类，将流变成数据，
*
* @note       读取对于边界有有一些安全处理，避免输入数据就有问题的情况
*/
class ZCE_Serialize_Read
{
    friend class ZCE_ClassSerialize_ReadHelp<std::string>;
public:

    /*!
    * @brief      构造函数
    * @param      read_buf 输入的数据，不会对数据进行改动
    * @param      buf_len  数据的长度
    */
    ZCE_Serialize_Read(const char *read_buf,size_t buf_len);

    ///析构函数
    ~ZCE_Serialize_Read();

public:
    ///返回当前类是否正常，BTW：我们不在溢出保护上做努力，那是你负责的事情
    bool is_good()
    {
        return is_good_;
    }

    const char *read_pos()
    {
        return read_pos_;
    }

    ///设置错误标志
    void set_bad()
    {
        is_good_ = false;
    }

    ///重置，重置开始位置和good标志位
    void reset()
    {
        read_pos_ = read_buf_;
        is_good_ = true;
    }

    ///已经读取的长度
    size_t read_len()
    {
        return read_pos_ - read_buf_;
    }

    ///保存枚举值
    template<typename val_type  >
    typename std::enable_if<std::is_enum<val_type>::value>::type read(val_type &val)
    {
        return load_enum(val);
    }
    template<typename enum_type >
    void load_enum(const enum_type &val)
    {
        read_arithmetic(static_cast<int &>(val));
    }

    ///保存数值类型
    template<typename val_type >
    typename std::enable_if<std::is_arithmetic<val_type>::value>::type read(val_type &val)
    {
        return read_arithmetic(val);
    }
    void read_arithmetic(bool &val);
    void read_arithmetic(char &val);
    void read_arithmetic(unsigned char &val);
    void read_arithmetic(short &val);
    void read_arithmetic(unsigned short &val);
    void read_arithmetic(int &val);
    void read_arithmetic(unsigned int &val);
    void read_arithmetic(int64_t &val);
    void read_arithmetic(uint64_t &val);
    void read_arithmetic(float &val);
    void read_arithmetic(double &val);

    ///写入数组
    template<typename val_type >
    typename std::enable_if<std::is_array<val_type>::value>::type read(val_type &val)
    {
        // consider alignment
        std::size_t ary_count = sizeof(val) / (
            static_cast<const char *>(static_cast<const void *>(&val[1]))
            - static_cast<const char *>(static_cast<const void *>(&val[0]))
            );
        size_t load_count;
        return read_array(val,ary_count,load_count);
    }
    template<typename array_type >
    void read_array(array_type ary,size_t ary_count,size_t &load_count)
    {
        //读取数组长度
        unsigned int ui_load_count;
        this->read_arithmetic(ui_load_count);
        load_count = ui_load_count;
        //
        if (!is_good_ || load_count > ary_count || read_pos_ + load_count * sizeof(ary[0]) > end_pos_)
        {
            is_good_ = false;
            return;
        }
        for (size_t i = 0; i < load_count && is_good_; ++i)
        {
            this->read(*(ary + i));
        }
        return;
    }
    ///特化，对字符串进行加速
    void read_array(char *ary,size_t ary_count,size_t &load_count)
    {
        unsigned int ui_load_count;
        this->read_arithmetic(ui_load_count);
        load_count = ui_load_count;

        if (!is_good_ || load_count > ary_count || read_pos_ + load_count > end_pos_)
        {
            is_good_ = false;
            return;
        }

        memcpy(ary,read_pos_,load_count);
        read_pos_ += load_count;
    }

    ///特化
    void read_array(unsigned char *ary,size_t ary_count,size_t &load_count)
    {
        unsigned int ui_load_count;
        this->read_arithmetic(ui_load_count);
        load_count = ui_load_count;

        if (!is_good_ || load_count > ary_count || read_pos_ + load_count > end_pos_)
        {
            is_good_ = false;
            return;
        }

        memcpy(ary,read_pos_,load_count);
        read_pos_ += load_count;
    }

    ///加载类，这儿要用辅助类实现一些偏特化的能力
    template<typename val_type >
    typename std::enable_if<std::is_class<val_type>::value>::type read(val_type &val)
    {
        ZCE_ClassSerialize_ReadHelp<val_type> sload;
        sload.read_help(this,val);
        return;
    }

    ///使用&操作符号写入数据，
    template<typename val_type>
    ZCE_Serialize_Read &operator &(val_type &val)
    {
        this->read(val);
        return *this;
    }

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

//辅助类，save_help 函数
template<typename val_type>
void ZCE_ClassSerialize_ReadHelp<val_type>::read_help(ZCE_Serialize_Read *sload,
                                                      val_type &val)
{
    val.serialize(sload);
}

template<typename vector_type>
void ZCE_ClassSerialize_ReadHelp<std::vector<vector_type> >::read_help(ZCE_Serialize_Read *sload,
                                                                       std::vector<vector_type> &val)
{
    unsigned int v_size = 0;
    sload->read_arithmetic(v_size);
    bool is_ok = sload->is_good();
    for (size_t i = 0; i < v_size && is_ok; ++i)
    {
        vector_type ve;
        sload->read(ve);
        is_ok = sload->is_good();
        if (is_ok)
        {
            val.push_back(ve);
        }
    }
    return;
}

template<typename list_type>
void ZCE_ClassSerialize_ReadHelp<std::list<list_type> >::read_help(ZCE_Serialize_Read *sload,
                                                                   std::list<list_type> &val)
{
    size_t v_size = val.size();
    sload->read_arithmetic(v_size);
    bool is_ok = sload->is_good();
    for (size_t i = 0; i < v_size && is_ok; ++i)
    {
        list_type le;
        sload->read(le);
        is_ok = sload->is_good();
        if (is_ok)
        {
            val.push_back(le);
        }
    }
    return;
}

template<typename key_type,typename data_type >
void ZCE_ClassSerialize_ReadHelp<std::map<key_type,data_type> >::read_help(ZCE_Serialize_Read *sload,
                                                                           std::map<key_type,data_type> &val)
{
    size_t v_size = val.size();
    sload->read_arithmetic(v_size);
    bool is_ok = sload->is_good();
    for (size_t i = 0; i < v_size && is_ok; ++i)
    {
        key_type ke;
        data_type de;
        sload->read(ke);
        sload->read(de);
        is_ok = sload->is_good();
        if (is_ok)
        {
            val[ke] = de;
        }
    }
    return;
}

#endif //ZCE_LIB_BYTES_SERIALIZATION_H_
