#ifndef ZCE_LIB_BYTES_SERIALIZATION_H_
#define ZCE_LIB_BYTES_SERIALIZATION_H_

#include "zce_bytes_base.h"
#include "zce_log_logging.h"

//===========================================================================================================
//�����봦����࣬д�����Ĵ�����

class ZCE_Serialize_Write;

//�������������ݵ�һЩ��
template<typename val_type >
class ZCE_ClassSerialize_WriteHelp
{
public:
    void write_help(ZCE_Serialize_Write *ssave, const val_type &val);
};

template<>
class ZCE_ClassSerialize_WriteHelp<std::string>
{
public:
    void write_help(ZCE_Serialize_Write *ssave, const std::string &val);
};

template<typename vector_type >
class ZCE_ClassSerialize_WriteHelp<std::vector<vector_type> >
{
public:
    void write_help(ZCE_Serialize_Write *ssave, const std::vector<vector_type> &val);
};

template<typename list_type >
class ZCE_ClassSerialize_WriteHelp<std::list<list_type> >
{
public:
    void write_help(ZCE_Serialize_Write *ssave, const std::list<list_type> &val);
};

template<typename key_type, typename data_type >
class ZCE_ClassSerialize_WriteHelp<std::map<key_type, data_type> >
{
public:
    void write_help(ZCE_Serialize_Write *ssave, const std::map<key_type, data_type> &val);
};


/*!
* @brief      �����ݽ��б��봦����࣬�����ݱ������
*
* @note       û�п��Ƕ�������⣬
*             BTW������д�룬���ǲ������������������Ŭ���������㸺�������
*/
class ZCE_Serialize_Write
{
public:

    ///���캯��
    ZCE_Serialize_Write(char *write_buf, size_t buf_len);

    ~ZCE_Serialize_Write();

public:

    ///���ص�ǰ���Ƿ�������
    bool is_good()
    {
        return is_good_;
    }

    ///���ô����־
    void set_bad()
    {
        is_good_ = false;
    }

    ///����,���ÿ�ʼλ�ú�good��־λ
    void reset()
    {
        write_pos_ = write_buf_;
        is_good_ = true;
    }

    ///�Ѿ�д��ĳ���
    size_t write_len()
    {
        return write_pos_ - write_buf_;
    }

    ///����ö��ֵ,����SFINA��ԭ�򣬽�������
    template<typename val_type  >
    void write(const typename std::enable_if<std::is_enum<val_type>::value, val_type>::type &val)
    {
        return save_enum(val);
    }
    template<typename enum_type >
    void save_enum(const enum_type &val)
    {
        write_arithmetic(static_cast<const int &>(val));
    }

    ///������ֵ����
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

    ///��������
    template<typename val_type >
    typename std::enable_if<std::is_array<val_type>::value >::type write(const val_type &val)
    {
        // consider alignment
        std::size_t count = sizeof(val) / (
                                static_cast<const char *>(static_cast<const void *>(&val[1]))
                                - static_cast<const char *>(static_cast<const void *>(&val[0]))
                            );
        return write_array(val, count);
    }
    template<typename array_type >
    void write_array(const array_type *ary, size_t count)
    {
        //��ʵ������ע�͵������������һ�㣬����֪��Ϊɶ�и澯��������
        //ZCE_ASSERT(count < std::numeric_limits<unsigned int>::max());
        ZCE_ASSERT(count < 0xFFFFFFFFll);
        this->write_arithmetic(static_cast<unsigned int>(count));
        for (size_t i = 0; i < count && is_good_; ++i)
        {
            this->write(*(ary + i));
        }
        return;
    }

    ///�ػ������ַ������м���
    void write_array(const char *ary, size_t count)
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
            memcpy(write_pos_, ary, count);
            write_pos_ += count;
        }
    }

    ///�ַ����ػ�
    void write_array(const unsigned char *ary, size_t count)
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
            memcpy(write_pos_, ary, count);
            write_pos_ += count;
        }
    }

    ///�����࣬���Ҫ�ø�����ʵ��һЩƫ�ػ�������
    template<typename val_type >
    typename std::enable_if<std::is_class<val_type>::value >::type write(const val_type &val)
    {
        ZCE_ClassSerialize_WriteHelp<val_type> ssave;
        ssave.write_help(this, val);
        return;
    }

    ///ʹ��& ��������д�����ݣ�
    template<typename val_type>
    ZCE_Serialize_Write &operator &(const val_type &val)
    {
        this->write(val);
        return *this;
    }

protected:

    ///״̬�Ƿ���ȷ�����д��λ�ó����������Ľ�β������Ϊfalse
    bool is_good_;

    ///�������ݣ���д��buffer��
    char *write_buf_;
    ///buf�ĳ���
    size_t buf_len_;

    ///buf������λ�ã������жϼӿ촦��
    char *end_pos_;
    ///��ǰд���posλ�ã�
    char *write_pos_;
};

//�����࣬write_help ������ʵ��

//�ñ���class��������� base templates ʵ��
template<typename val_type>
void ZCE_ClassSerialize_WriteHelp<val_type>::write_help(ZCE_Serialize_Write *ssave,
                                                        const val_type &val)
{
    val.serialize(ssave);
}

//���ڱ���vector ����������ػ�
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
    for (size_t i = 0; i < v_size && ssave->is_good(); ++i, ++iter)
    {
        ssave->write(*iter);
    }
    return;
}

template<typename key_type, typename data_type >
void ZCE_ClassSerialize_WriteHelp<std::map<key_type, data_type> >::write_help(ZCE_Serialize_Write *ssave,
                                                                              const std::map<key_type, data_type> &val)
{
    size_t v_size = val.size();
    ZCE_ASSERT(v_size < 0xFFFFFFFFll);
    ssave->write_arithmetic(static_cast<unsigned int>(v_size));
    typename std::map<key_type, data_type>::const_iterator iter = val.begin();
    for (size_t i = 0; i < v_size && ssave->is_good(); ++i, ++iter)
    {
        ssave->write(iter->first);
        ssave->write(iter->second);
    }
    return;
}

//===========================================================================================================

class ZCE_Serialize_Read;
//���������ȡ���ݵ�һЩ��
template<typename val_type >
class ZCE_ClassSerialize_ReadHelp
{
public:
    void read_help(ZCE_Serialize_Read *sload, val_type &val);
};

template<>
class ZCE_ClassSerialize_ReadHelp<std::string>
{
public:
    void read_help(ZCE_Serialize_Read *sload, std::string &val);
};

template<typename vector_type >
class ZCE_ClassSerialize_ReadHelp<std::vector<vector_type> >
{
public:
    void read_help(ZCE_Serialize_Read *sload, std::vector<vector_type> &val);
};

template<typename list_type >
class ZCE_ClassSerialize_ReadHelp<std::list<list_type> >
{
public:
    void read_help(ZCE_Serialize_Read *sload, std::list<list_type> &val);
};

template<typename key_type, typename data_type >
class ZCE_ClassSerialize_ReadHelp<std::map<key_type, data_type> >
{
public:
    void read_help(ZCE_Serialize_Read *sload, std::map<key_type, data_type> &val);
};

/*!
* @brief      �����ݽ��н����봦����࣬����������ݣ�
*
* @note       ��ȡ���ڱ߽�����һЩ��ȫ���������������ݾ�����������
*/
class ZCE_Serialize_Read
{
    friend class ZCE_ClassSerialize_ReadHelp<std::string>;
public:

    /*!
    * @brief      ���캯��
    * @param      read_buf ��������ݣ���������ݽ��иĶ�
    * @param      buf_len  ���ݵĳ���
    */
    ZCE_Serialize_Read(const char *read_buf, size_t buf_len);

    ///��������
    ~ZCE_Serialize_Read();

public:
    ///���ص�ǰ���Ƿ�������BTW�����ǲ��������������Ŭ���������㸺�������
    bool is_good()
    {
        return is_good_;
    }

    const char *read_pos()
    {
        return read_pos_;
    }

    ///���ô����־
    void set_bad()
    {
        is_good_ = false;
    }

    ///���ã����ÿ�ʼλ�ú�good��־λ
    void reset()
    {
        read_pos_ = read_buf_;
        is_good_ = true;
    }

    ///�Ѿ���ȡ�ĳ���
    size_t read_len()
    {
        return read_pos_ - read_buf_;
    }

    ///����ö��ֵ
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

    ///������ֵ����
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

    ///д������
    template<typename val_type >
    typename std::enable_if<std::is_array<val_type>::value>::type read(val_type &val)
    {
        // consider alignment
        std::size_t ary_count = sizeof(val) / (
                                    static_cast<const char *>(static_cast<const void *>(&val[1]))
                                    - static_cast<const char *>(static_cast<const void *>(&val[0]))
                                );
        size_t load_count;
        return read_array(val, ary_count, load_count);
    }
    template<typename array_type >
    void read_array(array_type ary, size_t ary_count, size_t &load_count)
    {
        //��ȡ���鳤��
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
    ///�ػ������ַ������м���
    void read_array(char *ary, size_t ary_count, size_t &load_count)
    {
        unsigned int ui_load_count;
        this->read_arithmetic(ui_load_count);
        load_count = ui_load_count;

        if (!is_good_ || load_count > ary_count || read_pos_ + load_count > end_pos_)
        {
            is_good_ = false;
            return;
        }

        memcpy(ary, read_pos_, load_count);
        read_pos_ += load_count;
    }

    ///�ػ�
    void read_array(unsigned char *ary, size_t ary_count, size_t &load_count)
    {
        unsigned int ui_load_count;
        this->read_arithmetic(ui_load_count);
        load_count = ui_load_count;

        if (!is_good_ || load_count > ary_count || read_pos_ + load_count > end_pos_)
        {
            is_good_ = false;
            return;
        }

        memcpy(ary, read_pos_, load_count);
        read_pos_ += load_count;
    }

    ///�����࣬���Ҫ�ø�����ʵ��һЩƫ�ػ�������
    template<typename val_type >
    typename std::enable_if<std::is_class<val_type>::value>::type read(val_type &val)
    {
        ZCE_ClassSerialize_ReadHelp<val_type> sload;
        sload.read_help(this, val);
        return;
    }

    ///ʹ��&��������д�����ݣ�
    template<typename val_type>
    ZCE_Serialize_Read &operator &(val_type &val)
    {
        this->read(val);
        return *this;
    }

protected:

    ///״̬�Ƿ���ȷ�����д��λ�ó����������Ľ�β������Ϊfalse
    bool is_good_;

    ///��ȡ����BUFFER
    const char *read_buf_;
    ///BUF�ĳ���
    size_t buf_len_;

    ///BUFFER������λ��
    const char *end_pos_;
    ///��ǰ��ȡ��λ��
    const char *read_pos_;

};

//�����࣬save_help ����
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


template<typename key_type, typename data_type >
void ZCE_ClassSerialize_ReadHelp<std::map<key_type, data_type> >::read_help(ZCE_Serialize_Read *sload,
                                                                            std::map<key_type, data_type> &val)
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

