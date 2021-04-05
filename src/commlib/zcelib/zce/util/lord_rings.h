/*!
* @copyright  2004-2019  Apache License, Version 2.0 FULLSAIL
* @filename   zce_boost_lord_rings.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2006��3��11��
* @brief      ħ�䣬һ��ѭ��������ݵĶ���,���ڸ��ֻ����������У����Զ�����Ҳ������չ����
*             ��LIST�����ô��ǿռ���һ��ʼ����õģ�����Ҫ���new���ٶȷɿ죬��������
*
* @details    ��˵�����������ʵ��BOOSTû�а�ëǮ��ϵ��
*             BOOSTҲ�����Ƶ�ʵ�� circular_buffer��������ط�û��͵ʦ����;ͬ����ѣ�
*             ���Կ�����������������£��Ƿ񸲸�,��һ��Ӧ�õĳ��������ӣ���Ϸ���������û�
*             ��1s������У�Ȼ��㲥�������������󳤶�200���������������ĸ���ǰ��ģ�
*             ����������β���������Ǻ�
*
* @note       ԭ��ȡ��cyc_deque������һ���Linker������˵˵��д�����Ƶ���������rings��
*             �Ҿ���������ֱȽϿᣬ����ѡ��������֣�Խ��Խ�����������á�
*             2011��10��7�� ����һ��������ԭ����ʵ�����ÿռ�+1��ʶǰ�պ󿪵Ŀռ䣬������
*             �Ǽ�����size��¼������Ҫ������1���ռ��ʶ�󿪿ռ��ˡ�ͬʱ�����˵�������
*
*             ฽�㣬����ؼң�ǣ���ҵ��֣�����Щ����
*/

#ifndef ZCE_LIB_BOOST_LORD_RINGS_H_
#define ZCE_LIB_BOOST_LORD_RINGS_H_

/*!
* @namespace zce ��Ҫ����ģ��������ֿռ䣬Ϊ�˺�stdͳһ��ģ����һ������Сд
*/

namespace zce
{

/*!
* @tparam    _value_type ��������
* @brief     ħ��ĵĵ���������ħ����������typedef��Ϊiterator��
* @note      ��סlordrings�ĵ�������vector����Σ�գ���Ϊ���Ŀռ���ѭ�����õģ��κ�һ��push,pop���������õ�����ʧЧ��
*            ����ʹ�õ�ʱ���ģ�
*            ���˾��õ����������ò�����Ϊħ���ʹ����Ҫͷβ���������ñ���ʹ�á�
*/
template <class _value_type >
class _lordrings_iterator
{
protected:

    //���������
    typedef _lordrings_iterator<_value_type > iterator;


    //��������ȡ�����еĶ���
    typedef ptrdiff_t difference_type;
    typedef _value_type *pointer;
    typedef _value_type &reference;
    typedef _value_type value_type;
    typedef std::random_access_iterator_tag iterator_category;

public:
    ///���캯��
    _lordrings_iterator():
        rings_start_(0),
        rings_capacity_(0),
        rings_value_base_(NULL),
        serial_(0)
    {
    }
    ///���캯��
    _lordrings_iterator(size_t rings_start,
                        size_t rings_capacity,
                        _value_type *rings_value_base,
                        size_t serial
                       ):
        rings_start_(rings_start),
        rings_capacity_(rings_capacity),
        rings_value_base_(rings_value_base),
        serial_(serial)
    {
    }
    ///��������
    ~_lordrings_iterator()
    {
    }

    ///*���������
    _value_type &operator*() const
    {
        return *(rings_value_base_ + (rings_start_ +  serial_) % rings_capacity_);
    }

    ///->����
    _value_type *operator->() const
    {
        return rings_value_base_ + (rings_start_ +  serial_) % rings_capacity_;
    }

    ///ǰ�������
    iterator &operator++()
    {
        serial_++;
        return *this;
    }

    ///ǰ�������
    iterator operator++(int)
    {
        iterator tmp = *this;
        ++*this;
        return tmp;
    }


    ///ǰ�������
    iterator &operator--()
    {
        serial_--;
        return *this;
    }

    ///ǰ�������
    iterator operator--(int)
    {
        iterator tmp = *this;
        --*this;
        return tmp;
    }

    ///�������ж����
    bool operator==(const iterator &it) const
    {
        if (rings_value_base_ == it.rings_value_base_
            && rings_start_ == it.rings_start_
            && rings_capacity_ == it.rings_capacity_
            && serial_ == it.serial_ )
        {
            return true;
        }

        return false;
    }

    ///�������ж������
    bool operator!=(const iterator &it) const
    {
        return !(*this == it);
    }

    ///������žͿ����ٸ���ģ��ʵ���������ҵ���Ӧ����,����ʹ��ָ��
    size_t getserial() const
    {
        return serial_;
    }

protected:
    ///ħ��Ŀǰ��ʼ��λ�ã�
    size_t                 rings_start_;
    ///ħ�������
    size_t                 rings_capacity_;
    ///���ݵ�ָ�룬
    _value_type           *rings_value_base_;

    //���к�,������ǰ�������ӵľ������ֵ
    size_t                 serial_;
};

/*!
* @tparam    _value_type ��������
* @brief     ħ��.ѭ�����������Զ���չ���������ĸ��ǵ�һ����
*/
template<class _value_type >
class lordrings
{
public:
    //���������
    typedef _lordrings_iterator<_value_type > iterator;

public:
    ///���캯��������������,initialize
    lordrings():
        cycdeque_start_(0),
        cycdeque_size_(0),
        cycdeque_capacity_(0),
        value_ptr_(NULL)
    {
    }

    ///���캯����ͬʱ��ɳ�ʼ��,������ȫ û�б�Ҫ����,initialize
    lordrings(size_t max_len):
        cycdeque_start_(0),
        cycdeque_size_(0),
        cycdeque_capacity_(max_len),
        value_ptr_(NULL)
    {
        assert(max_len > 0);
        value_ptr_ = new _value_type[cycdeque_capacity_];
    }

    ///�����������ͷſռ�
    ~lordrings()
    {
        if (value_ptr_)
        {
            delete[] value_ptr_;
            value_ptr_ = NULL;
        }
    }

    ///��ʼ�����������͹��캯���ɵ��������һ����ֻ�Ƕ���һ��ԭ�������ݾ������
    void initialize(size_t max_len)
    {
        assert(max_len > 0);

        cycdeque_start_ = 0;
        cycdeque_size_ = 0;
        cycdeque_capacity_ = max_len;

        //�����ֳ�
        if (value_ptr_)
        {
            delete[] value_ptr_;
            value_ptr_ = NULL;
        }

        value_ptr_ = new _value_type[cycdeque_capacity_];
    }

    ///��������ɣ����٣�����fini��ʲô����д��
    void finish()
    {
        cycdeque_start_ = 0;
        cycdeque_size_ = 0;
        cycdeque_capacity_ = 0;

        //�����ֳ�
        if (value_ptr_)
        {
            delete[] value_ptr_;
            value_ptr_ = NULL;
        }
    }

    ///����ܵ���
    void clear()
    {
        cycdeque_start_ = 0;
        cycdeque_size_ = 0;
    }

    ///�ߴ�ռ�
    inline size_t size() const
    {
        return cycdeque_size_;
    }
    ///���ؿ��пռ�Ĵ�С
    inline size_t freesize() const
    {
        return cycdeque_capacity_ - cycdeque_size_ ;
    }

    ///���ض��е�����
    inline size_t capacity() const
    {
        return cycdeque_capacity_;
    }

    ////����Ƿ��Ѿ�����
    inline bool full() const
    {
        //�Ѿ��õĿռ��������
        if (cycdeque_size_  == cycdeque_capacity_ )
        {
            return true;
        }

        return false;
    }

    ///�ж϶����Ƿ�Ϊ��
    inline bool empty() const
    {
        //�ռ����0
        if (cycdeque_size_ == 0)
        {
            return true;
        }

        return false;
    }

    ///���·���һ���ռ�,
    bool resize(size_t new_max_size)
    {
        assert(new_max_size > 0);

        size_t deque_size = size();

        //���ԭ���ĳߴ�����µĳߴ磬�޷���չ
        if ( deque_size > new_max_size )
        {
            return false;
        }

        _value_type *new_value_ptr = new _value_type[new_max_size];

        //���ԭ��������,�������µ�������
        if (value_ptr_ != NULL)
        {
            for (size_t i = 0; i < deque_size && i < new_max_size; ++i)
            {
                new_value_ptr[i] = value_ptr_[(cycdeque_start_ + i) % cycdeque_capacity_];
            }

            delete[] value_ptr_;
            value_ptr_ = NULL;
        }

        //���������ڲ�����
        cycdeque_start_ = 0;
        cycdeque_capacity_ = new_max_size ;
        //cycdeque_size_ ����

        value_ptr_ = new_value_ptr;

        return true;
    }

    ///��һ�����ݷ�����е�β��,��������Ѿ�����,����Խ�lay_over������λtrue,����ԭ�е�����
    bool push_back(const _value_type &value_data, bool lay_over = false)
    {
        //����Ѿ�����
        if ( full() )
        {
            //�����Ҫ���ǣ����ش���
            if (lay_over == false)
            {
                return false;
            }
            //���Ҫ����
            else
            {
                //�����һ��λ�ø��ǣ����ҵ�����ʼ�ͽ���λ��
                value_ptr_[ (cycdeque_start_ + cycdeque_size_ ) % cycdeque_capacity_] = value_data;
                cycdeque_start_ = (cycdeque_start_ + 1 ) % cycdeque_capacity_;

                return true;
            }
        }

        //ֱ�ӷ��ڶ�β

        value_ptr_[(cycdeque_start_ + cycdeque_size_ ) % cycdeque_capacity_] = value_data;
        ++cycdeque_size_;

        return true;
    }

    ///��һ�����ݷ�����е�β��,��������Ѿ�����,����Խ�lay_over������λtrue,����ԭ�е�����
    bool push_front(const _value_type &value_data, bool lay_over = false)
    {
        //����Ѿ�����
        if ( full() )
        {
            //�����Ҫ���ǣ����ش���
            if (lay_over == false)
            {
                return false;
            }
            //���Ҫ����
            else
            {
                //����һ��λ�õ������ǣ����ҵ�����ʼ�ͽ���λ��
                cycdeque_start_ = (cycdeque_start_ > 0) ? cycdeque_start_ - 1 : cycdeque_capacity_ - 1;
                value_ptr_[cycdeque_start_] = value_data;

                //���ǣ��ߴ�Ҳ���õ���

                return true;
            }
        }

        //ֱ�ӷ��ڶ�β
        cycdeque_start_ = (cycdeque_start_ > 0) ? cycdeque_start_ - 1 : cycdeque_capacity_ - 1;
        value_ptr_[cycdeque_start_] = value_data;

        ++cycdeque_size_;

        return true;
    }

    ///�Ӷ��е�ǰ��pop���ҵõ�һ������
    bool pop_front(_value_type &value_data)
    {
        //����ǿյķ��ش���
        if ( empty() )
        {
            return false;
        }

        value_data = value_ptr_[cycdeque_start_];
        cycdeque_start_ = (cycdeque_start_ + 1 ) % cycdeque_capacity_;

        --cycdeque_size_;

        return true;
    }

    ///�Ӷ���ǰ��popһ������
    bool pop_front()
    {
        //����ǿյķ��ش���
        if (empty())
        {
            return false;
        }

        cycdeque_start_ = (cycdeque_start_ + 1 ) % cycdeque_capacity_;
        --cycdeque_size_;

        return true;
    }

    ///�Ӷ��е�β��pop���ҵõ�һ������
    bool pop_back(_value_type &value_data)
    {
        //����ǿյķ��ش���
        if ( empty())
        {
            return false;
        }

        value_data = value_ptr_[(cycdeque_start_ + cycdeque_size_ ) % cycdeque_capacity_];
        --cycdeque_size_;
        return true;
    }

    ///�Ӷ��е�β��popһ������
    bool pop_back()
    {
        //
        if (size() == 0)
        {
            return false;
        }

        --cycdeque_size_;

        return true;
    }

    ///[]�����±궨λ,ID��ҪԽ�磬�Լ���֤����û��ȤΪ���ʲô
    _value_type &operator[](size_t id)
    {
        return value_ptr_[(cycdeque_start_ + id) % cycdeque_capacity_];
    }

    ///[]�����±궨λ,ID��ҪԽ�磬�Լ���֤����û��ȤΪ���ʲô
    const _value_type &operator[](size_t id) const
    {
        return value_ptr_[(cycdeque_start_ + id) % cycdeque_capacity_];
    }

    ///����start�ĵ���������ʼ�������к�Ϊ0��λ��
    iterator begin()
    {
        return iterator(cycdeque_start_, cycdeque_capacity_, value_ptr_, 0);
    }

    ///����end�ĵ���������ʼ�������к�Ϊcycdeque_size_��λ��
    iterator end()
    {
        return iterator(cycdeque_start_, cycdeque_capacity_, value_ptr_, cycdeque_size_);
    }

protected:

    ///ѭ�����е���ʼλ��
    size_t                 cycdeque_start_;

    ///ѭ�����еĳ��ȣ�����λ��ͨ��cycdeque_start_+cycdeque_size_%cycdeque_capacity_�õ���˼·��Ȼ��ǰ�����
    size_t                 cycdeque_size_;
    ///���еĳ��ȣ�
    size_t                 cycdeque_capacity_;
    ///������ݵ�ָ��
    _value_type           *value_ptr_;

};

};

#endif //#ifndef ZCE_LIB_BOOST_LORD_RINGS_H_

