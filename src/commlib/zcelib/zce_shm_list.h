/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_list.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2008��1��18��
* @brief      ���Է��ڹ����ڴ�����������������Էţ����ҿ��������ָ�����������
*             STL��Ϊ���������ġ�������
*
* @details    д��ʱ��ο��˺�ݵġ�STLԴ�������������飬
*             ���鿴����һЩ���õ�˵����@ref zce_shm_predefine.h �ļ����棬
*             ���Լ��Ķ���
*
* @note       frost�̵���˵����Ѿ�Ͽ�ȥ̸���������������������׸裬
* ������¼д��17��Ĵ��룬����о��е����ƣ�����ժ����
*
* ��������ʲô������Ī��ε
* ��ʵһ���˵�����Ҳ����̫��
* ż����ЩСС�ı���
* �������Ҳ��������
* ��ʹ�µ���ʹ���˻�
* Ҳ���������Լ���ÿ�
* ���� ��֪�����Ѿ�ϰ�߻��ǰ�
* ��������ֵ�����
* �ǲ��ǻ���Ȼ����
* �ۿ���һ����Ҫ��ȥ
* �ҵĴ��컹û����
* ��Ϊ�β�����ͷȥ
* �����Լ�ȥ�������
* �㳢���Ų�¶�ۼ�
* �����Ұ���ĵ���
* ����Ϊֵ��Ŭ����
* ������֮��ľ���
* � ��һ�� ������Щֵ�û���
*
* �ٶ԰��Ů���У�����Ա˵�������Ѿ���顣
*/

#ifndef ZCE_LIB_SHARE_MEM_LIST_H_
#define ZCE_LIB_SHARE_MEM_LIST_H_

#include "zce_shm_predefine.h"

namespace zce
{

//============================================================================================

template <class _value_type> class smem_list;

/*!
@brief      �������ķ�װ��˫���������Ϊsmem_list �ṩ��������
            �ڲ�ͨ�����кţ��Լ�����ָ��Խ��е��������еȵȡ�
@tparam     _value_type �����������������
*/
template <class _value_type> class _shm_list_iterator
{
    typedef _shm_list_iterator<_value_type> iterator;

    //��������ȡ�����еĶ���
    typedef ptrdiff_t difference_type;
    typedef _value_type *pointer;
    typedef _value_type &reference;
    typedef _value_type value_type;
    typedef std::bidirectional_iterator_tag iterator_category;

public:

    /*!
    @brief      ���캯��
    @param      seq      ���
    @param      instance LIST��ʵ��
    @note
    */
    _shm_list_iterator<_value_type>(size_t seq, smem_list<_value_type> *instance):
        serial_(seq),
        list_instance_(instance)
    {
    }

    ///���캯��
    _shm_list_iterator<_value_type>():
        serial_(_shm_memory_base::_INVALID_POINT),
        list_instance_(NULL)
    {
    }
    ///��������
    ~_shm_list_iterator<_value_type>()
    {
    }

    ///��ʼ����
    void initialize(size_t seq, smem_list<_value_type> *instance)
    {
        serial_ = seq;
        list_instance_ = instance;
    }

    ///������žͿ����ٸ���ģ��ʵ���������ҵ���Ӧ����,����ʹ��ָ��
    size_t getserial() const
    {
        return serial_;
    }

    ///���������еȣ�
    bool operator==(const iterator &x) const
    {
        return (serial_ == x.serial_ && list_instance_ == x.list_instance_ );
    }

    /// ���������ж�����
    bool operator!=(const iterator &x) const
    {
        return !(*this == x);
    }

    ///�������
    _value_type &operator*() const
    {
        return *(operator->());
    }
    //�ڶ��̵߳Ļ������ṩ������ͷ����ǲ���ȫ��,��û�м���,ԭ����˵��
    _value_type *operator->() const
    {
        //
        return list_instance_->getdatabase() + serial_ ;
    }

    ///++iter�������������ƶ�����
    iterator &operator++()
    {
        serial_ = (list_instance_->getindexbase() + serial_)->idx_next_;
        return *this;
    }

    ///iter++������
    iterator operator++(int)
    {
        iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    /// --iter����
    iterator &operator--()
    {
        serial_ = (list_instance_->getindexbase() + serial_)->idx_prev_;
        return *this;
    }

    /// iter--����
    iterator operator--(int)
    {
        iterator tmp = *this;
        --(*this);
        return tmp;
    }

protected:

    //���кţ�����������±�
    size_t                  serial_;
    //��Ӧ��list����ָ��
    smem_list<_value_type> *list_instance_;
};

//============================================================================================

/*!
@brief      LIST��������ͷ�������ڴ�Ŵ˴Σ�����������������
            ʹ�õ�NODE������FREE��NODE������
*/
class _shm_list_head
{
protected:

    ///���캯��
    _shm_list_head():
        size_of_mmap_(0),
        num_of_node_(0),
        size_free_node_(0),
        size_use_node_(0)
    {
    }

    //��������
    ~_shm_list_head()
    {
    }

    //�Ҳ��ܶ�ģ������Ԫ�����ˣ���ʼ���ų�����
public:

    ///�ڴ����ĳ���
    size_t               size_of_mmap_;
    ///NODE������
    size_t               num_of_node_;

    ///FREE��NODE����
    size_t               size_free_node_;
    ///USE��NODE����
    size_t               size_use_node_;
};

//============================================================================================

/*!
@brief      ˫�����������ڹ����ڴ棨��ͨ�ڴ�Ҳ�У���ʹ�ã�
            Ҳ���Զ���̹�������Ȼͬ����������飬���Լ����ǣ�

            ������Ҫ�ĵ�ַ�ռ��С˵����ÿ��node������Ҫ2��shm_index_t��С�Ŀռ䣬

@tparam     _value_type Ԫ������
*/
template <class _value_type> class smem_list :
    public _shm_memory_base
{
public:

    ///���������
    typedef _shm_list_iterator<_value_type> iterator;

    //ĳЩ�����ṩ����������
    friend class _shm_list_iterator<_value_type>;

public:

    ///����ڹ����ڴ�ʹ��,û��new,����ͳһ��initialize ��ʼ��
    ///�������,��������,���ǲ�������
    smem_list<_value_type>(size_t numnode, void *pmmap, bool if_restore):
        _shm_memory_base(pmmap),
        list_head_(NULL),
        index_base_(NULL),
        data_base_(NULL),
        freenode_(NULL),
        usenode_(NULL)
    {
    }

    smem_list<_value_type>():
        _shm_memory_base(NULL),
        list_head_(NULL),
        index_base_(NULL),
        data_base_(NULL),
        freenode_(NULL),
        usenode_(NULL)
    {
    }

    ~smem_list<_value_type>()
    {
    }

    //ֻ����,��ʵ��,
    const smem_list<_value_type> &operator=(const smem_list<_value_type> &others);

protected:

    //�õ������Ļ�����ַ
    inline _shm_list_index *getindexbase()
    {
        return index_base_;
    }
    //�õ��������Ļ�������
    inline  _value_type *getdatabase()
    {
        return data_base_;
    }

    //����һ��NODE,�����FREELIST��ȡ��
    size_t create_node(const _value_type &val)
    {
        //���û�пռ���Է���
        if (list_head_->size_free_node_ == 0)
        {
            return _INVALID_POINT;
        }

        //������ȡ1������
        size_t node = freenode_->idx_next_;

        //��FREE������ժ����
        freenode_->idx_next_ = (index_base_ + node)->idx_next_;
        (index_base_ + freenode_->idx_next_)->idx_prev_ = (index_base_ + node)->idx_prev_;

        //��placement new��������
        new (data_base_ + node) _value_type(val) ;

        list_head_->size_use_node_  ++;
        list_head_->size_free_node_ --;

        //assert(list_head_->szusenode_ + list_head_->szfreenode_ == list_head_->numofnode_);

        return node;
    }
    //�ͷ�һ��NODE,����黹��FREELIST
    void destroy_node(size_t pos)
    {
        size_t freenext = freenode_->idx_next_;

        (index_base_ + pos)->idx_next_ = freenext;
        (index_base_ + pos)->idx_prev_ = (index_base_ + freenext)->idx_prev_;

        freenode_->idx_next_ = pos;

        (index_base_ + freenext)->idx_prev_ = pos;

        //������ʽ����������
        (data_base_ + pos)->~_value_type();

        list_head_->size_use_node_  --;
        list_head_->size_free_node_ ++;

        //assert(list_head_->szusenode_ + list_head_->szfreenode_ == list_head_->numofnode_);
    }

public:

    //�ڴ����Ĺ���Ϊ ������,index��,data��,��������Ҫ�ĳ���,
    static size_t getallocsize(const size_t numnode)
    {
        return  sizeof(_shm_list_head)  + sizeof(_shm_list_index) * (numnode + ADDED_NUM_OF_INDEX) + sizeof(_value_type) * numnode ;
    }

    smem_list<_value_type> *getinstance()
    {
        return this;
    }

    //��ʼ��
    static smem_list<_value_type> *initialize(const size_t numnode, char *pmmap, bool if_restore = false)
    {
        //assert(pmmap!=NULL && numnode >0 );
        _shm_list_head *listhead = reinterpret_cast<_shm_list_head *>(pmmap);

        //����ǻָ�,���ݶ����ڴ���,
        if (if_restore == true)
        {
            //���һ�»ָ����ڴ��Ƿ���ȷ,
            if (getallocsize(numnode) != listhead->size_of_mmap_ ||
                numnode != listhead->num_of_node_ )
            {
                return NULL;
            }
        }

        //��ʼ���ߴ�
        listhead->size_of_mmap_ = getallocsize(numnode);
        listhead->num_of_node_ = numnode;

        smem_list<_value_type> *instance = new smem_list<_value_type>();

        //���е�ָ�붼�Ǹ��ӻ���ַ����õ���,���ڷ������,ÿ�γ�ʼ�������¼���
        instance->smem_base_ = pmmap;
        instance->list_head_ = listhead;
        instance->index_base_ = reinterpret_cast<_shm_list_index *>(pmmap + sizeof(_shm_list_head));
        instance->data_base_  = reinterpret_cast<_value_type *>(pmmap + sizeof(_shm_list_head) + sizeof(_shm_list_index) * (numnode + ADDED_NUM_OF_INDEX) );

        //�������һ�����FREENODE,USENODE��ʹ��
        instance->freenode_   = reinterpret_cast<_shm_list_index *>(pmmap + sizeof(_shm_list_head) + sizeof(_shm_list_index) * (numnode ));
        instance->usenode_    = reinterpret_cast<_shm_list_index *>(pmmap + sizeof(_shm_list_head) + sizeof(_shm_list_index) * (numnode + 1));

        //
        if (if_restore == false)
        {
            //�����ʼ�����е��ڴ�,���еĽڵ�ΪFREE
            instance->clear();
        }

        assert(listhead->size_use_node_ + listhead->size_free_node_ == listhead->num_of_node_);

        //�����չ�
        return instance;
    }

    //�����ʼ�����е��ڴ�,���еĽڵ�ΪFREE
    void clear()
    {
        //����2���ؼ�Node,�Լ���س���,��ʼ���е�������free.
        list_head_->size_free_node_ = list_head_->num_of_node_;
        list_head_->size_use_node_ = 0;

        //���������ж�����ΪNULL,��ָ�붼ָ���Լ�,�����һ��С����,
        //����Խ�����Ϊ��˫�������ͷָ��,(��ʵҲ��βָ��).
        freenode_->idx_next_ = list_head_->num_of_node_ ;
        freenode_->idx_prev_ = list_head_->num_of_node_ ;

        usenode_->idx_next_ = list_head_->num_of_node_ + 1;
        usenode_->idx_prev_ = list_head_->num_of_node_ + 1;

        _shm_list_index *pindex = index_base_;

        //��ʼ��free������
        for (size_t i = 0; i < list_head_->num_of_node_ ; ++i )
        {

            pindex->idx_next_ = (i + 1) ;
            pindex->idx_prev_ = (i - 1) ;

            //�����е�������FREENODE������
            if (0 == i)
            {
                pindex->idx_prev_ = freenode_->idx_next_;
                freenode_->idx_next_ = 0;
            }

            if (i == list_head_->num_of_node_ - 1)
            {
                pindex->idx_next_ = freenode_->idx_prev_;
                freenode_->idx_prev_ = list_head_->num_of_node_ - 1;
            }

            pindex++;
        }
    }

    //
    iterator begin()
    {
        return iterator(usenode_->idx_next_, this);
    };
    //����Ӧ����ǰ�պ󿪵�,usenode_��ΪΪ���һ��index
    iterator end()
    {
        return iterator(list_head_->num_of_node_ + 1, this);
    }

    //�Բ���,�Ҳ��ṩ���������,STL�ķ��������̫������,�������Ҫ�ñ��붨��
    //������Լ���end,--���ڰ�.

    //
    bool empty()
    {
        if (list_head_->size_free_node_ == list_head_->num_of_node_)
        {
            return true;
        }

        return false;
    }
    //�ڲ�������ǰ����,����������
    bool full()
    {
        if (list_head_->size_free_node_ == 0 )
        {
            return true;
        }

        return false;
    };

protected:
    //ͨ��ƫ�����кŲ���,��������ʹ��,���Ƿǳ���ȫ,FREENODEҲ����POS��.
    //���������POS�ڵ��ǰ��
    size_t insert(size_t pos, const _value_type &val)
    {
        size_t node = create_node(val);

        if (node == _INVALID_POINT)
        {
            return _INVALID_POINT;
        }

        //���½��ҽӵ�������
        (index_base_ + node)->idx_next_ = pos;
        (index_base_ + node)->idx_prev_ = (index_base_ + pos)->idx_prev_;

        (index_base_ + (index_base_ + pos)->idx_prev_)->idx_next_ = node;
        (index_base_ + pos)->idx_prev_ = node;

        return node;
    }

public:

    //ͨ������������,�Ƽ�ʹ���������,
    //����������������ڵ��ǰ��
    std::pair<iterator, bool> insert(const iterator &pos, const _value_type &val)
    {
        size_t tmp = insert(pos.getserial(), val);

        //����ʧ��
        if (_INVALID_POINT == tmp)
        {
            return std::pair<iterator, bool>(end(), false);
        }
        else
        {
            return std::pair<iterator, bool>(iterator(tmp, this), true);
        }
    }

protected:

    //ͨ��ƫ�����к�ɾ��,Σ�պ���,�Լ���װ��ȷʹ��
    size_t erase(size_t pos)
    {
        size_t nextnode = (index_base_ + pos)->idx_next_;
        size_t prevnode = (index_base_ + pos)->idx_prev_;

        (index_base_ + prevnode)->idx_next_ = nextnode;
        (index_base_ + nextnode)->idx_prev_ = prevnode;

        destroy_node(pos);

        return nextnode;
    }

public:
    //ͨ��������ɾ��
    iterator erase(const iterator &pos)
    {
        size_t tmp = erase(pos.getserial());
        return iterator(tmp, this);
    }

    //���˵�����,��Щ������Ȼ��˼�,�벻����
    bool push_front(const _value_type &x)
    {
        std::pair<iterator, bool> tmp = insert(begin(), x);
        return tmp.second;
    }

    bool push_back(const _value_type &x)
    {
        std::pair<iterator, bool> tmp = insert(end(), x);
        return tmp.second;
    }

    void pop_front()
    {
        erase(begin());
    }
    void pop_back()
    {
        iterator tmp = end();
        erase(--tmp);
    }

    //��[first,last)�Ķ����ƶ���pos��λ��,ע��:pos,first,last������ͬһ�����������.!!!
    //���ⲻҪ�н���.
    void transfer(const iterator &pos, const iterator &first, const iterator &last)
    {
        //ʵ�ڲ�Ը����,
        if (pos != last && pos != first )
        {
            size_t sqpos   = pos.getserial();
            size_t sqfirst = first.getserial();
            size_t sqlast = last.getserial();
            size_t sqpos_prev =  (index_base_ + sqpos)->idx_prev_;
            size_t sqfirst_prev = (index_base_ + sqfirst)->idx_prev_;
            size_t sqlast_prev = (index_base_ + sqlast)->idx_prev_;

            (index_base_ + sqlast)->idx_prev_ = sqfirst_prev;
            (index_base_ + sqfirst_prev)->idx_next_  = sqlast;
            (index_base_ + sqfirst)->idx_prev_ = sqpos_prev;
            (index_base_ + sqpos_prev)->idx_next_ = sqfirst;
            (index_base_ + sqpos)->idx_prev_ = sqlast_prev;
            (index_base_ + sqlast_prev)->idx_next_ = sqpos;
        }
    }

    void move_begin(const iterator &first, const iterator &last)
    {
        transfer(begin(), first, last);
    }

    void move_end(const iterator &first, const iterator &last)
    {
        transfer(end(), first, last);
    }

    void move_begin(const iterator &itr)
    {
        move_begin(itr, iterator((index_base_ + itr.getserial())->idx_next_, this));
    }

    void move_end(const iterator &itr)
    {
        move_end(itr, iterator((index_base_ + itr.getserial())->idx_next_, this));
    }

    //�����������Ѿ��е�Ԫ�ظ���
    size_t size()
    {
        return list_head_->size_use_node_;
    }
    //����������ӵ�����
    size_t capacity()
    {
        return list_head_->num_of_node_;
    }
    //
    size_t sizefreenode()
    {
        return list_head_->size_free_node_;
    }

    void dump()
    {
        iterator iter_tmp = begin();
        iterator iter_end = end();

        for (; iter_tmp != iter_end; ++iter_tmp)
        {
            /*std::cout<<"getserial:"<<static_cast<unsigned int>(iter_tmp.getserial())
                << "idx_next_:"<<static_cast<unsigned int>((index_base_ + iter_tmp.getserial())->idx_next_ )
                << "idx_prev_:"<<static_cast<unsigned int>((index_base_ + iter_tmp.getserial())->idx_prev_ )<<std::endl;*/
        }
    }

protected:

    //index��Ҫ������������,һ����ʾ
    static const size_t ADDED_NUM_OF_INDEX = 2;

protected:

    //���е�ָ�붼�Ǹ��ӻ���ַ����õ���,���ڷ������,ÿ�γ�ʼ�������¼���

    //LIST��ͷ����ָ��
    _shm_list_head      *list_head_;
    //����������ָ��,
    _shm_list_index     *index_base_;
    //��������ʼָ��,
    _value_type          *data_base_;

    //FREE NODE��ͷָ��,N+1������λ��ʾ
    _shm_list_index     *freenode_;
    //USE NODE��ͷָ��,N+2������λ��ʾ
    _shm_list_index     *usenode_;
};

};

#endif //ZCE_LIB_SHARE_MEM_LIST_H_

