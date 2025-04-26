/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_data_proc_encode.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2020年
* @brief      序列化的辅助类，
*
* @details    你可以通过定义函数，辅助完成结构的序列化
*             支持int，double，数组，list，vector，map等。
* struct A
* {
*     int f1;
*     double f2;
*     std::string f3_;
*
*     template<typename serialize_type>
*     void serialize(serialize_type& ss, unsigned int ver = 0)
*     {
*         ss&f1_;
*         ss&f2_;
*         ss&f3_;
*     }
* };
*
*
*
*/

#pragma once

#include "zce/bytes/bytes_common.h"
#include "zce/buffer/string_buffer.h"
#include "zce/util/mpl.h"

//===========================================================================================================
//流编码处理的类，写入流的处理类

namespace zce::ser
{
class encode;
class decode;

template <typename T>
concept HasMemberSerialize = requires(T obj, encode & en, decode & de)
{
    { obj.template serialize<encode&>(en, (uint32_t)0) };
    { obj.template serialize<decode&>(de, (uint32_t)0) };
};

// Add a global serialize function template to resolve the error.
template <typename ST, typename T>
void serialize([[maybe_unused]] ST& serializer,
               [[maybe_unused]] T& obj,
               [[maybe_unused]] uint32_t  version = 0)
{
    // This is a placeholder function to resolve the error.
    // Actual serialization logic should be implemented in the class or global function.
    std::cout << "Global serialize function not implemented for this type: "
        << typeid(T).name() << std::endl;
    assert(false && "Global serialize function not implemented for this type.");
}

template <typename T>
concept HasGlobalSerialize = requires(encode & en, decode & de, T & obj)
{
    { zce::ser::serialize<encode&>(en, obj, (uint32_t)0) };
    { zce::ser::serialize<decode&>(de, obj, (uint32_t)0) };
};

/*!
* @brief      对数据进行编码处理的类，将数据变成流，
*
* @note       没有考虑对齐等问题，
*             BTW：对于写入，我们不在溢出保护上做过多努力，那是你负责的事情
*/
class encode
{
public:

    ///构造函数
    encode(char* write_buf, size_t buf_len);
    ~encode() = default;

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

    void write(const bool& val);
    void write(const char& val);
    void write(const unsigned char& val);
    void write(const short& val);
    void write(const unsigned short& val);
    void write(const int& val);
    void write(const unsigned int& val);
    void write(const long& val);
    void write(const unsigned long& val);
    void write(const long long& val);
    void write(const unsigned long long& val);
    void write(const float& val);
    void write(const double& val);

    void write(const std::string& val);
    void write(const std::string_view& val);

    template<typename T>
    void write(const T& val)
    {
        if constexpr (std::is_array<T>::value)
        {
            std::size_t count = sizeof(val) / (
                static_cast<const char*>(static_cast<const void*>(&val[1]))
                - static_cast<const char*>(static_cast<const void*>(&val[0]))
                );
            write_array(val, count);
        }
        else if constexpr (std::is_enum<T>::value)
        {
            write_enum(val.data(), val.size());
        }
        else if constexpr (std::is_class<T>::value)
        {
            write_class(val);
        }
        else
        {
            assert(false);
        }
    }

    template<typename ET >
    void write_enum(const ET& val)
    {
        write(static_cast<const int&>(val));
    }

    template<typename AT >
    void write_array(const AT* ary, size_t count)
    {
        //其实用下面注释的这个代码会更酷一点，但不知道为啥有告警，放弃，
        //ZCE_ASSERT(count < std::numeric_limits<unsigned int>::max());
        assert(count < 0xFFFFFFFFll);
        this->write(static_cast<unsigned int>(count));
        for (size_t i = 0; i < count && is_good_; ++i)
        {
            this->write(*(ary + i));
        }
        return;
    }

    ///特化，对字符串进行加速
    void write_array(const char* ary, size_t count)
    {
        assert(count < 0xFFFFFFFFll);
        this->write(static_cast<unsigned int>(count));
        if (is_good_)
        {
            if (write_pos_ + count > end_pos_)
            {
                is_good_ = false;
                return;
            }
            memcpy(write_pos_, ary, count);
            write_pos_ += count;
        }
    }

    ///字符串特化
    void write_array(const unsigned char* ary, size_t count)
    {
        assert(count < 0xFFFFFFFFll);
        this->write(static_cast<unsigned int>(count));
        if (is_good_)
        {
            if (write_pos_ + count > end_pos_)
            {
                is_good_ = false;
                return;
            }
            memcpy(write_pos_, ary, count);
            write_pos_ += count;
        }
    }

    template<typename CT>
    void write_class(const CT& val)
    {
        //std::cout << typeid(CT).name() << std::endl;
        //std::cout << zce::is_container<CT>::value << std::endl;
        //std::cout << zce::has_key_type<CT>::value << std::endl;
        //std::cout << zce::is_single_type_container<CT>::value << std::endl;
        //std::cout << zce::is_associative_container<CT>::value << std::endl;

        if constexpr (zce::is_single_type_container<CT>::value)
        {
            size_t v_size = val.size();
            assert(v_size < 0xFFFFFFFFll);
            write((unsigned int)v_size);
            typename CT::const_iterator iter = val.begin();
            for (size_t i = 0; i < v_size && is_good(); ++i, ++iter)
            {
                write(*iter);
            }
        }
        else if constexpr (zce::is_associative_container<CT>::value)
        {
            size_t v_size = val.size();
            assert(v_size < 0xFFFFFFFFll);
            write((unsigned int)v_size);
            typename CT::const_iterator iter = val.begin();
            for (size_t i = 0; i < v_size && is_good(); ++i, ++iter)
            {
                write(iter->first);
                write(iter->second);
            }
        }
        else if constexpr (HasMemberSerialize<CT>)
        {
            const_cast<CT&>(val).serialize(*this);
        }
        else if constexpr (HasGlobalSerialize<CT>)
        {
            zce::ser::serialize(*this, const_cast<CT&>(val));
        }
        else
        {
            //其他的类型，直接用默认的处理
            write(val);
        }
    }

    template<class BC, class DC>
    void base_class(const DC& val)
    {
        if constexpr (std::is_base_of<BC, DC>::value)
        {
            write_class(static_cast<const BC&>(val));
        }
        else
        {
            assert(false);
        }
    }

    ///使用& 操作符号写入数据，
    template<typename T>
    encode& operator &(const T& val)
    {
        this->write(val);
        return *this;
    }

    template<typename T>
    encode& ptr(T* p, size_t ary_sz)
    {
        this->write_array(p, ary_sz);
        return *this;
    }
protected:

    ///状态是否正确，如果写入位置超出缓冲区的结尾，会置为false
    bool is_good_ = true;

    ///保存数据，被写的buffer，
    char* write_buf_ = nullptr;
    ///buf的长度
    size_t buf_len_ = 0;

    ///buf的最后的位置，用于判断加快处理
    char* end_pos_ = nullptr;
    ///当前写入的pos位置，
    char* write_pos_ = nullptr;
};

//===========================================================================================================

/*!
* @brief      对数据进行解码码处理的类，将流变成数据，
*
* @note       读取对于边界有有一些安全处理，避免输入数据就有问题的情况
*/
class decode
{
public:

    /*!
    * @brief      构造函数
    * @param      read_buf 输入的数据，不会对数据进行改动
    * @param      buf_len  数据的长度
    */
    decode(const char* read_buf, size_t buf_len);
    ///析构函数
    ~decode() = default;

public:
    ///返回当前类是否正常，BTW：我们不在溢出保护上做努力，那是你负责的事情
    bool is_good()
    {
        return is_good_;
    }

    const char* read_pos()
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
    typename std::enable_if<std::is_enum<val_type>::value>::type read(val_type& val)
    {
        return load_enum(val);
    }

    template<typename T>
    T read_to(const char* str)
    {
        T to_val;
        read(to_val);
        return to_val;
    }

    ///保存数值类型
    void read(bool& val);
    void read(char& val);
    void read(unsigned char& val);
    void read(short& val);
    void read(unsigned short& val);
    void read(int& val);
    void read(unsigned int& val);
    void read(long& val);
    void read(unsigned long& val);
    void read(long long& val);
    void read(unsigned long long& val);
    void read(float& val);
    void read(double& val);
    void read(std::string& val);
    void read(zce::string_buf& val);

    template<typename T>
    void read(T& val)
    {
        if constexpr (std::is_array<T>::value)
        {
            std::size_t ary_count = sizeof(val) / (
                static_cast<const char*>(static_cast<const void*>(&val[1]))
                - static_cast<const char*>(static_cast<const void*>(&val[0]))
                );
            size_t read_count;
            return read_array(val, ary_count, read_count);
        }
        else if constexpr (std::is_enum<T>::value)
        {
            read_enum(val.data(), val.size());
        }
        else if constexpr (std::is_class<T>::value)
        {
            read_class(val);
        }
        else
        {
            assert(false);
        }
    }

    template<typename ET >
    void read_enum(const ET& val)
    {
        read(static_cast<int&>(val));
    }

    template<typename CT>
    void read_class(CT& val)
    {
        if constexpr (zce::is_single_type_container<CT>::value)
        {
            uint32_t v_size = 0;
            read(v_size);
            bool is_ok = is_good();
            for (size_t i = 0; i < v_size && is_ok; ++i)
            {
                typename CT::value_type ve;
                read(ve);
                is_ok = is_good();
                if (is_ok)
                {
                    val.push_back(ve);
                }
            }
        }
        else if constexpr (zce::is_associative_container<CT>::value)
        {
            uint32_t v_size = 0;
            read(v_size);
            bool is_ok = is_good();
            for (size_t i = 0; i < v_size && is_good(); ++i)
            {
                typename CT::key_type vk;
                typename CT::mapped_type vv;
                read(vk);
                read(vv);
                is_ok = is_good();
                if (is_ok)
                {
                    val[vk] = vv;
                }
            }
        }
        else if constexpr (HasMemberSerialize<CT>)
        {
            val.serialize(*this);
        }
        else if constexpr (HasGlobalSerialize<CT>)
        {
            zce::ser::serialize(*this, val);
        }
        else
        {
            //其他的类型，直接用默认的处理
            read(val);
        }
    }

    /**
     * @brief 从BUF种读取array_type类型的队列数据
     * @tparam array_type 队列类型
     * @param ary 队列
     * @param ary_count 队列数量
     * @param read_count 读取的数量
    */
    template<typename AT >
    void read_array(AT ary, size_t ary_count, size_t& read_count)
    {
        //读取数组长度
        uint32_t ui_load_count = 0;
        this->read(ui_load_count);
        read_count = ui_load_count;
        //
        if (!is_good_ || read_count > ary_count || read_pos_ + read_count * sizeof(ary[0]) > end_pos_)
        {
            is_good_ = false;
            return;
        }
        for (size_t i = 0; i < read_count && is_good_; ++i)
        {
            this->read(*(ary + i));
        }
        return;
    }
    ///特化，对字符串进行加速
    void read_array(char* ary, size_t ary_count, size_t& load_count)
    {
        uint32_t ui_load_count;
        this->read(ui_load_count);
        load_count = ui_load_count;

        if (!is_good_ || load_count > ary_count || read_pos_ + load_count > end_pos_)
        {
            is_good_ = false;
            return;
        }

        memcpy(ary, read_pos_, load_count);
        read_pos_ += load_count;
    }

    ///特化
    void read_array(unsigned char* ary, size_t ary_count, size_t& load_count)
    {
        uint32_t ui_load_count;
        this->read(ui_load_count);
        load_count = ui_load_count;

        if (!is_good_ || load_count > ary_count || read_pos_ + load_count > end_pos_)
        {
            is_good_ = false;
            return;
        }

        memcpy(ary, read_pos_, load_count);
        read_pos_ += load_count;
    }

    template<class BC, class DC>
    void base_class(DC& val)
    {
        if constexpr (std::is_base_of<BC, DC>::value)
        {
            read_class(static_cast<BC&>(val));
        }
        else
        {
            assert(false);
        }
    }

    ///使用&操作符号写入数据，
    template<typename T>
    decode& operator &(T& val)
    {
        this->read(val);
        return *this;
    }

    template<typename T>
    decode& ptr(T* p, size_t ary_sz)
    {
        size_t load_sz = 0;
        this->read_array(p, ary_sz, &load_sz);
        return *this;
    }

    const char* read_pos() const
    {
        return read_pos_;
    }

protected:

    ///状态是否正确，如果写入位置超出缓冲区的结尾，会置为false
    bool is_good_ = true;

    ///读取流的BUFFER
    const char* read_buf_ = nullptr;
    ///BUF的长度
    size_t buf_len_ = 0;

    ///BUFFER结束的位置
    const char* end_pos_ = nullptr;
    ///当前读取的位置
    const char* read_pos_ = nullptr;
};
}
