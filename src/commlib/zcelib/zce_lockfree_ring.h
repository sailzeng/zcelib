/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_lockfree_deque.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2019��8��7��
* @brief      
*
*
* @details    
*
*
* @note
*
*/




#ifndef ZCE_LIB_LOCKFREE_RINGS_H_
#define ZCE_LIB_LOCKFREE_RINGS_H_

#include "zce_shm_predefine.h"


namespace zce::lockfree
{

/*!
* @tparam    _value_type ��������
* @brief     ħ��.ѭ�����������Զ���չ���������ĸ��ǵ�һ����
*/
template<class _value_type >
class rings
{

public:
    ///���캯��������������,initialize
    rings():
        rings_start_(0),
        rings_size_(0),
        rings_capacity_(0),
        value_ptr_(NULL)
    {
    }

    ///���캯����ͬʱ��ɳ�ʼ��,������ȫ û�б�Ҫ����,initialize
    rings(size_t max_len):
        rings_start_(0),
        rings_size_(0),
        rings_capacity_(max_len),
        value_ptr_(NULL)
    {
        assert(max_len>0);
        initialize(max_len);

    }

    ///�����������ͷſռ�
    ~rings()
    {
        finalize();
    }

    ///��ʼ�����������͹��캯���ɵ��������һ����ֻ�Ƕ���һ��ԭ�������ݾ������
    ///initialize ��������
    void initialize(size_t max_len)
    {
        assert(max_len>0);

        rings_start_=0;
        rings_size_=0;
        rings_capacity_=max_len;

        //�����ֳ�
        if(value_ptr_)
        {
            free(value_ptr_);
            value_ptr_=NULL;
        }
        //����new�������Ĺ��캯��
        value_ptr_=(_value_type*)malloc(sizeof(_value_type)*capacity_);
    }

    ///��������ɣ�����
    void finalize()
    {
        rings_start_=0;
        rings_size_=0;
        rings_capacity_=0;

        //�����ֳ�
        if(value_ptr_)
        {
            size_t sz= rings_size_.load();
            for(; read<sz; ++read)
            {
                value_ptr_[read].~_value_type();
            }

            free(value_ptr_);
            value_ptr_=NULL;
        }
    }

    ///����ܵ���
    void clear()
    {
        rings_start_=0;
        rings_size_=0;
    }

    ///�ߴ�ռ�
    inline size_t size() const
    {
        return rings_size_;
    }
    ///���ؿ��пռ�Ĵ�С
    inline size_t freesize() const
    {
        return rings_capacity_-rings_size_;
    }

    ///���ض��е�����
    inline size_t capacity() const
    {
        return rings_capacity_;
    }

    ////����Ƿ��Ѿ�����
    inline bool full() const
    {
        //�Ѿ��õĿռ��������
        if(rings_size_==rings_capacity_)
        {
            return true;
        }

        return false;
    }

    ///�ж϶����Ƿ�Ϊ��
    inline bool empty() const
    {
        //�ռ����0
        if(rings_size_==0)
        {
            return true;
        }

        return false;
    }

    ///���·���һ���ռ�,
    bool resize(size_t new_max_size)
    {
        assert(new_max_size>0);

        size_t deque_size=size();

        //���ԭ���ĳߴ�����µĳߴ磬�޷���չ
        if(deque_size>new_max_size)
        {
            return false;
        }

        _value_type* new_value_ptr=new _value_type[new_max_size];

        //���ԭ��������,�������µ�������
        if(value_ptr_!=NULL)
        {
            for(size_t i=0; i<deque_size&&i<new_max_size; ++i)
            {
                new_value_ptr[i]=value_ptr_[(rings_start_+i)%rings_capacity_];
            }

            delete[] value_ptr_;
            value_ptr_=NULL;
        }

        //���������ڲ�����
        rings_start_=0;
        rings_capacity_=new_max_size;
        //cycdeque_size_ ����

        value_ptr_=new_value_ptr;

        return true;
    }

    ///��һ�����ݷ�����е�β��,��������Ѿ�����,����Խ�lay_over������λtrue,����ԭ�е�����
    bool push_back(const _value_type& value_data,bool lay_over=false)
    {
        //����Ѿ�����
        if(full())
        {
            //�����Ҫ���ǣ����ش���
            if(lay_over==false)
            {
                return false;
            }
            //���Ҫ����
            else
            {
                //�����һ��λ�ø��ǣ����ҵ�����ʼ�ͽ���λ��
                value_ptr_[(rings_start_+rings_size_)%rings_capacity_]=value_data;
                rings_start_=(rings_start_+1)%rings_capacity_;

                return true;
            }
        }

        //ֱ�ӷ��ڶ�β

        value_ptr_[(rings_start_+rings_size_)%rings_capacity_]=value_data;
        ++rings_size_;

        return true;
    }

    ///��һ�����ݷ�����е�β��,��������Ѿ�����,����Խ�lay_over������λtrue,����ԭ�е�����
    bool push_front(const _value_type& value_data,bool lay_over=false)
    {
        //����Ѿ�����
        if(full())
        {
            //�����Ҫ���ǣ����ش���
            if(lay_over==false)
            {
                return false;
            }
            //���Ҫ����
            else
            {
                //����һ��λ�õ������ǣ����ҵ�����ʼ�ͽ���λ��
                rings_start_=(rings_start_>0)?rings_start_-1:rings_capacity_-1;
                value_ptr_[rings_start_]=value_data;

                //���ǣ��ߴ�Ҳ���õ���

                return true;
            }
        }

        //ֱ�ӷ��ڶ�β
        rings_start_=(rings_start_>0)?rings_start_-1:rings_capacity_-1;
        value_ptr_[rings_start_]=value_data;

        ++rings_size_;

        return true;
    }

    ///�Ӷ��е�ǰ��pop���ҵõ�һ������
    bool pop_front(_value_type& value_data)
    {
        //����ǿյķ��ش���
        if(empty())
        {
            return false;
        }

        value_data=value_ptr_[rings_start_];
        rings_start_=(rings_start_+1)%rings_capacity_;

        --rings_size_;

        return true;
    }

    ///�Ӷ���ǰ��popһ������
    bool pop_front()
    {
        //����ǿյķ��ش���
        if(empty())
        {
            return false;
        }

        rings_start_=(rings_start_+1)%rings_capacity_;
        --rings_size_;

        return true;
    }

    ///�Ӷ��е�β��pop���ҵõ�һ������
    bool pop_back(_value_type& value_data)
    {
        //����ǿյķ��ش���
        if(empty())
        {
            return false;
        }

        value_data=value_ptr_[(rings_start_+rings_size_)%rings_capacity_];
        --rings_size_;
        return true;
    }

    ///�Ӷ��е�β��popһ������
    bool pop_back()
    {
        //
        if(size()==0)
        {
            return false;
        }
        --rings_size_;
        return true;
    }




protected:

    ///ѭ�����е���ʼλ��
    std::atomic<size_t> rings_start_;

    ///ѭ�����еĳ��ȣ�
    ///û���ý���Ϊֹ�Ƿ�����㣬 ����λ��ͨ��(rings_start_+cycdeque_size_)%rings_capacity_�õ���˼·��Ȼ��ǰ�����
    std::atomic<size_t> rings_size_;

    ///���еĳ��ȣ�
    size_t rings_capacity_;
    ///������ݵ�ָ��
    _value_type* value_ptr_;

};


};

#endif //ZCE_LIB_LOCKFREE_RINGS_H_




