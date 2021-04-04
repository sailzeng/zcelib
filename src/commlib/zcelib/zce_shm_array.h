/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_vector.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2005��10��21��
* @brief      �ڹ����ڴ���ʹ�õ�vector����ʵ�������飨�������ˣ�
*             ����û�ж�̬��չ���ԣ�������£���Ϊ�ұϾ���һ���ڴ�����
*             �ɻ��չ�����飬��������ģ�
*             2014�꣬�����µ�C++11�Ĺ淶�������ֶ�������Ϊarray�ˣ�
*             array�ǲ�����չ�ģ�
* @details
*
*/

#ifndef ZCE_LIB_SHM_ARRAY_H_
#define ZCE_LIB_SHM_ARRAY_H_

#include "zce_shm_predefine.h"

namespace zce
{

///�����ڴ�vectorͷ��������
class _shm_array_head
{
protected:

    //ͷ�����캯��
    _shm_array_head():
        size_of_mmap_(0),
        num_of_node_(0),
        num_of_use_(0)
    {
    }
    //��������
    ~_shm_array_head()
    {
    }

public:

    ///�ڴ����ĳ���
    size_t               size_of_mmap_;
    ///���������
    size_t               num_of_node_;

    ///��ʾĿǰʹ�õĽ�����
    size_t               num_of_use_;

};

/*!
* @brief      �����ڴ���ʹ�õ�vector�����׼򻯰汾
* @tparam     _value_type  ��������
*/
template <class _value_type> class shm_array:
    public _shm_memory_base
{
public:

    ///���������,�����
    typedef _value_type *iterator;

protected:

    //ֻ����,��ʵ��,
    const shm_array<_value_type> &operator=(const shm_array<_value_type> &others);

    ///Ĭ�Ϲ��캯��,���ǲ�������
    shm_array():
        _shm_memory_base(NULL),
        data_base_(NULL)
    {
    }

public:

    /*!
    * @brief      ���캯�������ݲ������г�ʼ����
    * @return     void
    * @param      numnode
    * @param      pmmap
    * @param      if_restore
    */
    shm_array(const size_t numnode, char *pmmap, bool if_restore = false):
        _shm_memory_base(pmmap),
        data_base_(NULL)
    {
        initialize(numnode, pmmap, if_restore);
    }
    ///��������
    ~shm_array()
    {
    }
public:

    ///�ڴ����Ĺ���Ϊ ������,data��,��������Ҫ�ĳ���,
    static size_t getallocsize(const size_t numnode)
    {
        return  sizeof(_shm_array_head)  + sizeof(_value_type) * (numnode ) ;
    }

    ///��ʼ��
    static shm_array<_value_type> *initialize(const size_t numnode, char *pmmap, bool if_restore = false)
    {

        _shm_array_head *aryhead  = reinterpret_cast<_shm_array_head *>(pmmap);

        //����ǻָ�,���ݶ����ڴ���,
        if (if_restore == true)
        {
            //���һ�»ָ����ڴ��Ƿ���ȷ,
            if (getallocsize(numnode) != aryhead->size_of_mmap_ ||
                numnode != aryhead->num_of_node_ )
            {
                return NULL;
            }
        }

        //��ʼ���ߴ�
        aryhead->size_of_mmap_ = getallocsize(numnode);
        aryhead->num_of_node_ = numnode;

        shm_array<_value_type> *instance = new shm_array<_value_type>();

        //���е�ָ�붼�Ǹ��ӻ���ַ����õ���,���ڷ������,ÿ�γ�ʼ�������¼���
        instance->smem_base_ = pmmap;
        instance->array_head_ = aryhead;
        instance->data_base_  = reinterpret_cast<_value_type *>(pmmap + sizeof(_shm_array_head) );

        if (if_restore == false)
        {
            instance->clear();
        }

        //�����չ�
        return instance;
    }

    ///
    void clear()
    {
        array_head_->num_of_use_ = 0;
    }

    ///��[]�������ݣ�Խ�����Լ�����
    _value_type &operator[](size_t n)
    {
        return *(data_base_ + n);
    }

    ///��ʼλ�õĵ�����
    iterator begin()
    {
        return data_base_;
    }
    ///����λ�õĵ�����
    iterator end()
    {
        return data_base_ + array_head_->num_of_use_;
    }

    ///�Ƿ�Ϊ���Ϳ�
    bool empty() const
    {
        return (array_head_->num_of_use_ == 0);
    }
    ///
    bool full() const
    {
        return (array_head_->num_of_use_ == array_head_->num_of_node_);
    }

    ///�������ÿռ䣬�����������С
    void resize(size_t num)
    {
#if defined _DEBUG || defined DEBUG
        assert(num <= array_head_->num_of_node_);
#endif

        //���������ռ䣬��������
        if (num > array_head_->num_of_use_)
        {
            //����Ĭ�ϵ�����
            for (size_t i = array_head_->num_of_use_; i < num; ++i)
            {
                new (data_base_ + i) _value_type();
            }
        }
        //�������С�ռ�,�������ݣ���������
        else if (num < array_head_->num_of_use_)
        {
            //����Ĭ�ϵ�����,����
            for (size_t i = num; i < array_head_->num_of_use_; ++i)
            {
                (data_base_ + i)->~_value_type();
            }
        }
        //���ڣ�ʲô������
        else
        {

        }

        array_head_->num_of_use_ = num;
    }

    //ʹ���˵Ŀռ������
    size_t size() const
    {
        return array_head_->num_of_use_;
    }
    //����������ӵ�����
    size_t capacity() const
    {
        return array_head_->num_of_node_;
    }
    //ʣ��ռ������
    size_t sizefreenode() const
    {
        return array_head_->num_of_node_ - array_head_->num_of_use_;
    }

    ///�ؼ�λ��
    _value_type &front()
    {
        return data_base_;
    }
    ///
    _value_type &back()
    {
        return *(data_base_ + ( array_head_->num_of_use_ - 1));
    }

    ///����������
    bool push_back(const _value_type &val)
    {
        if (array_head_->num_of_use_ == array_head_->num_of_node_)
        {
            return false;
        }

        //ʹ��placement new ���ƶ���
        new (data_base_ + array_head_->num_of_use_) _value_type(val);

        ++(array_head_->num_of_use_);

        return true;
    }

    ///�Ӻ���ɾ������
    bool pop_back()
    {
        if ( array_head_->num_of_use_ == 0 )
        {
            return false;
        }

        //��ʽ������������
        (data_base_ + array_head_->num_of_use_)->~_value_type();

        --(array_head_->num_of_use_);
        return true;
    }

protected:
    ///
    _shm_array_head   *array_head_ = nullptr;
    ///��������ʼָ��,
    _value_type       *data_base_ = nullptr;

};

};

#endif //ZCE_LIB_SHM_ARRAY_H_

