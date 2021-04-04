/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_hash_expire.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2005��10��26��
* @brief      ���HASH TABLE��LIST�Ĺ��ܵ�HASH��������LRU��̭��ʱ�򷽱��
*             �ṩHASH�Ĳ�ѯ���ܣ�ͬʱ�ṩLRU����̭����
*
* @details
*
*
*
* @note
*
*/



#ifndef ZCE_SHM_LRU_HASH_TABLE_H_
#define ZCE_SHM_LRU_HASH_TABLE_H_

#include "zce_shm_predefine.h"


namespace zce
{


template < class _value_type,
           class _key_type,
           class _hash_fun,
           class _extract_key,
           class _equal_key,
           class _washout_fun > class shm_hashtable_expire;

//LRU HASH ������
template < class _value_type,
           class _key_type,
           class _hashfun,
           class _extract_key,
           class _equal_key,
           class _washout_fun >
class _hashtable_expire_iterator
{
protected:

    //HASH TABLE�Ķ���
    typedef shm_hashtable_expire < _value_type,
            _key_type,
            _hashfun,
            _extract_key,
            _equal_key,
            _washout_fun  > _lru_hashtable;

    //���������
    typedef _hashtable_expire_iterator < _value_type,
            _key_type,
            _hashfun,
            _extract_key,
            _equal_key,
            _washout_fun > iterator;

protected:
    //���к�
    size_t                 serial_;
    //
    _lru_hashtable        *lruht_instance_;


public:
    _hashtable_expire_iterator():
        serial_(0),
        lruht_instance_(NULL)
    {
    }

    _hashtable_expire_iterator(size_t serial, _lru_hashtable *lru_ht_inst):
        serial_(serial),
        lruht_instance_(lru_ht_inst)
    {
    }

    ~_hashtable_expire_iterator()
    {
    }

    _value_type &operator*() const
    {
        return lruht_instance_->value_base_ [serial_];
    }

    _value_type *operator->() const
    {
        return lruht_instance_->value_base_ + serial_;
    }

    //����ֻ�ṩǰ���������������Ϊʹ�ÿ���ʹ��LIST��֤�����ĸ�Ч�����ֲ��У�
    //����Ҫ�ṩ����ĺ���
    //ǰ�������
    iterator &operator++()
    {
        size_t oldseq = serial_;
        serial_ = *(lruht_instance_->hash_index_base_ + serial_);

        //�������ڵ���ĩλ�Ľڵ�
        if (serial_ == _shm_memory_base::_INVALID_POINT)
        {
            //˳��Index��ѯ.
            size_t bucket = lruht_instance_->bkt_num_value(*(lruht_instance_->value_base_ + oldseq));

            //
            while (serial_ == _shm_memory_base::_INVALID_POINT && ++bucket < lruht_instance_->capacity() )
            {
                serial_ = *(lruht_instance_->hash_factor_base_ + bucket);
            }
        }

        return *this;
    }

    //�����������++������ֻ��Hash���ݵ��������ε����������ܸ���
    //�����һ��������һ����KEY����ô�ͳ�Ϊ�¸����ݵĵ�������
    //�����Ϊend
    //��ʵҲ�ͱ����Լ�����һ��㡣
    iterator &goto_next_equal()
    {
        size_t oldseq = serial_;
        serial_ = *(lruht_instance_->hash_index_base_ + serial_);

        //�������ڵ㲻��ĩλ�Ľڵ�
        if (serial_ != _shm_memory_base::_INVALID_POINT)
        {
            _extract_key get_key;
            _equal_key   equal_key;

            if ( false == equal_key(get_key(*(lruht_instance_->value_base_ + oldseq)),
                                    get_key(*(lruht_instance_->value_base_ + serial_))) )
            {
                serial_ = _shm_memory_base::_INVALID_POINT;
            }
        }
        else
        {
        }

        return *this;
    }

    //ǰ�������
    iterator operator++(int)
    {
        iterator tmp = *this;
        ++*this;
        return tmp;
    }
    //
    bool operator==(const iterator &it) const
    {
        if (lruht_instance_ == it.lruht_instance_ &&
            serial_ == it.serial_ )
        {
            return true;
        }

        return false;
    }
    //
    bool operator!=(const iterator &it) const
    {
        return !(*this == it);
    }

    //������žͿ����ٸ���ģ��ʵ���������ҵ���Ӧ����,����ʹ��ָ��
    size_t getserial() const
    {
        return serial_;
    }
};


//ͷ����LRU_HASH��ͷ���ṹ������LRUHASH�ڴ��ǰ��
class _hashtable_expire_head
{
protected:
    _hashtable_expire_head():
        size_of_mmap_(0),
        num_of_node_(0),
        sz_freenode_(0),
        sz_usenode_(0),
        sz_useindex_(0)
    {
    }
    ~_hashtable_expire_head()
    {
    }

public:
    //�ڴ����ĳ���
    size_t           size_of_mmap_;

    //NODE,INDEX������,INDEX�ĸ�����NODE�Ľڵ����Ϊ1:1,
    size_t           num_of_node_;

    //FREE��NODE����
    size_t           sz_freenode_;
    //USE��NODE����
    size_t           sz_usenode_;

    //ʹ�õ�INDEX����,�����˽�ʵ�ʿ����ĸ��ر���
    size_t           sz_useindex_;
};







/*!
@brief      ����ʱ�������һ���ڴ棨�����ڴ棩ʹ�õ�Hash table��

@tparam     _value_type
@tparam     _key_type
@tparam     _hash_fun
@tparam     _extract_key
@tparam     _equal_key
@tparam     _washout_fun
@note
*/
template < class _value_type,
           class _key_type,
           class _hash_fun = smem_hash<_key_type>,
           class _extract_key = smem_identity<_value_type>,
           class _equal_key = std::equal_to<_key_type>,
           class _washout_fun = _default_washout_fun<_value_type> >
class shm_hashtable_expire : public  _shm_memory_base
{
public:
    //���������
    typedef _hashtable_expire_iterator < _value_type,
            _key_type,
            _hash_fun,
            _extract_key,
            _equal_key,
            _washout_fun > iterator;

    friend class _hashtable_expire_iterator < _value_type,
               _key_type,
               _hash_fun,
               _extract_key,
               _equal_key,
               _washout_fun >;

protected:
    //
    static const size_t  LIST_ADD_NODE_NUMBER = 2;

protected:
    //
    _hashtable_expire_head *lru_hash_head_;

    //Hash���ӵ�BASE
    size_t                 *hash_factor_base_;
    //Hash������,hash��������,ע��д�ò�������Լ����ǲ�����.
    size_t                 *hash_index_base_;


    //LIST������
    _shm_list_index       *lst_index_base_;
    //FREE�ڵ�����Ŀ�ʼ
    _shm_list_index       *lst_free_node_;
    //USE�ڵ�����Ŀ�ʼ
    _shm_list_index       *lst_use_node_;

    //���ȼ�������ָ��,��32λ�����ݱ������ȼ�
    unsigned int           *priority_base_;
    //������ָ��
    _value_type            *value_base_;


protected:

    shm_hashtable_expire():
        lru_hash_head_(NULL),
        hash_factor_base_(NULL),
        hash_index_base_(NULL),
        lst_index_base_(NULL),
        lst_free_node_(NULL),
        lst_use_node_(NULL),
        priority_base_(NULL),
        value_base_(NULL)
    {
    }

public:

    ~shm_hashtable_expire()
    {
    }

protected:



public:


    /*!
    * @brief      �ڴ����Ĺ���Ϊ define��,index��,data��,��������Ҫ�ĳ���,
    * @return     size_t   ����ĳߴ�
    * @param      req_num  �����NODE����
    * @param      real_num ʵ�ʷ����NODE����
    * @note       ע�ⷵ�ص���ʵ��INDEX����,��ȡһ������
    */
    static size_t getallocsize(size_t req_num, size_t &real_num)
    {
        //ȡ��һ�������������һ���Ŵ��������
        zce::hash_prime(req_num, real_num);
        size_t sz_alloc =  0;
        //
        sz_alloc += sizeof(_hashtable_expire_head);
        sz_alloc += sizeof(size_t) * real_num;
        sz_alloc += sizeof(size_t) * real_num;
        //
        sz_alloc += sizeof(_shm_list_index) * (real_num + LIST_ADD_NODE_NUMBER);
        sz_alloc += sizeof(unsigned int) * (real_num);
        sz_alloc += sizeof(_value_type) * (real_num);
        return sz_alloc;
    }

    //��ʼ��
    static shm_hashtable_expire < _value_type,
           _key_type,
           _hash_fun,
           _extract_key,
           _equal_key,
           _washout_fun > *
           initialize(size_t req_num, size_t &real_num, char *pmmap, bool if_restore = false)
    {
        assert(pmmap != NULL && req_num > 0);
        //����
        size_t sz_mmap = getallocsize(req_num, real_num);
        _hashtable_expire_head *hashhead =  reinterpret_cast< _hashtable_expire_head * >(pmmap);

        //����ǻָ�,���ݶ����ڴ���,
        if ( true == if_restore )
        {


            //���һ�»ָ����ڴ��Ƿ���ȷ,
            //������Ļָ�������ȡ�����ķ������ı��ˣ�
            if (sz_mmap != hashhead->size_of_mmap_ ||
                real_num != hashhead->num_of_node_ )
            {

                //һ������²�һ�·���NULL����ʶ�ָ�ʧ�ܣ�
#if ALLOW_RESTORE_INCONFORMITY != 1
                return NULL;
#else
                ZCE_LOG(RS_ALERT, "Expire hash node initialize number[%lu|%lu] and restore number [%lu|%lu] "
                        "is different,but user defind ALLOW_RESTORE_INCONFORMITY == 1.Please notice!!! ",
                        sz_mmap,
                        real_num,
                        hashhead->size_of_mmap_,
                        hashhead->num_of_node_);
#endif
            }
        }
        else
        {
            //��¼��ʼ���ߴ�
            hashhead->size_of_mmap_ = sz_mmap;
            hashhead->num_of_node_ = real_num;
        }


        shm_hashtable_expire< _value_type, _key_type, _hash_fun, _extract_key, _equal_key, _washout_fun > *instance
            = new shm_hashtable_expire< _value_type, _key_type, _hash_fun, _extract_key, _equal_key, _washout_fun>();

        instance->smem_base_ = pmmap;
        char *tmp_base = instance->smem_base_;
        instance->lru_hash_head_ = reinterpret_cast<_hashtable_expire_head *>(tmp_base);
        tmp_base = tmp_base + sizeof(_hashtable_expire_head);
        instance->hash_factor_base_ = reinterpret_cast<size_t *>(tmp_base);
        tmp_base = tmp_base + sizeof(size_t) * real_num;
        instance->hash_index_base_ = reinterpret_cast<size_t *>(tmp_base);

        tmp_base = tmp_base + sizeof(size_t) * real_num;
        instance->lst_index_base_ = reinterpret_cast<_shm_list_index *>(tmp_base);
        tmp_base = tmp_base + sizeof(_shm_list_index) * (real_num + LIST_ADD_NODE_NUMBER);
        instance->lst_use_node_ = instance->lst_index_base_ + real_num;
        instance->lst_free_node_ = instance->lst_index_base_ + real_num + 1;

        instance->priority_base_ = reinterpret_cast<unsigned int *>(tmp_base);
        tmp_base = tmp_base + sizeof(unsigned int) * (real_num );
        instance->value_base_ = reinterpret_cast<_value_type *>(tmp_base);

        if ( false == if_restore )
        {
            //�����ʼ�����е��ڴ�,���еĽڵ�ΪFREE
            instance->clear();
        }
        //��ʵ����ǻָ�����Ӧ�ü��һ�����еĶ���

        //�����չ�
        return instance;
    }

    //�����ʼ�����е��ڴ�,���еĽڵ�ΪFREE
    void clear()
    {
        //����ؼ�Node,�Լ���س���,��ʼ���е�������free.
        lru_hash_head_->sz_freenode_ = lru_hash_head_->num_of_node_;
        lru_hash_head_->sz_usenode_ = 0;
        lru_hash_head_->sz_useindex_ = 0;

        //���������ж�����ΪNULL,��ָ�붼ָ���Լ�,�����һ��С����,
        //����Խ�����Ϊ��˫�������ͷָ��,(��ʵҲ��βָ��).
        lst_use_node_->idx_next_ = lru_hash_head_->num_of_node_ ;
        lst_use_node_->idx_prev_ = lru_hash_head_->num_of_node_ ;

        lst_free_node_->idx_next_ = lru_hash_head_->num_of_node_ + 1;
        lst_free_node_->idx_prev_ = lru_hash_head_->num_of_node_ + 1;

        _shm_list_index *pindex = lst_index_base_;

        //��ʼ��free������
        for (size_t i = 0; i < lru_hash_head_->num_of_node_ ; ++i )
        {

            //
            hash_factor_base_[i] = _INVALID_POINT;
            hash_index_base_[i] = _INVALID_POINT;
            priority_base_[i] = 0;

            pindex->idx_next_ = (i + 1) ;
            pindex->idx_prev_ = (i - 1) ;

            //�����е�������FREENODE������
            if (0 == i)
            {
                pindex->idx_prev_ = lst_free_node_->idx_next_;
                lst_free_node_->idx_next_ = 0;
            }

            if (i == lru_hash_head_->num_of_node_ - 1)
            {
                pindex->idx_next_ = lst_free_node_->idx_prev_;
                lst_free_node_->idx_prev_ = lru_hash_head_->num_of_node_ - 1;
            }

            pindex++;
        }
    }

protected:

    //����һ��NODE,�����FREELIST��ȡ��
    size_t create_node(const _value_type &val, unsigned int priority )
    {
        //���û�пռ���Է���
        if (lru_hash_head_->sz_freenode_ == 0)
        {
            return _INVALID_POINT;
        }

        //������ȡ1������
        size_t newnode = lst_free_node_->idx_next_;

        lst_free_node_->idx_next_ = (lst_index_base_ + newnode)->idx_next_;
        //lst_free_node_->idx_next_�Ѿ�������һ��λ����
        (lst_index_base_ + lst_free_node_->idx_next_)->idx_prev_ = (lst_index_base_ + newnode)->idx_prev_;

        //ע��num_of_node_��λ����usenode
        lst_index_base_[newnode].idx_next_ = lst_use_node_->idx_next_;
        lst_index_base_[newnode].idx_prev_ = lst_index_base_[lst_use_node_->idx_next_].idx_prev_;
        lst_index_base_[lst_use_node_->idx_next_].idx_prev_ = newnode;
        lst_use_node_->idx_next_ = newnode;

        //placement new��¼���ݺ����ȼ�
        new (value_base_ + newnode)  _value_type( val);
        priority_base_[newnode] = priority;

        lru_hash_head_->sz_usenode_  ++;
        lru_hash_head_->sz_freenode_ --;

        //�����ɴ�����ôû��
        assert(lru_hash_head_->sz_usenode_ + lru_hash_head_->sz_freenode_ == lru_hash_head_->num_of_node_);
        return newnode;
    }

    //�ͷ�һ��NODE,����黹��FREELIST,����������Ǽ�
    void destroy_node(size_t pos)
    {
        size_t freenext = lst_free_node_->idx_next_;

        size_t pos_next = lst_index_base_[pos].idx_next_ ;
        size_t pos_prev = lst_index_base_[pos].idx_prev_ ;
        lst_index_base_[pos_next].idx_prev_ = pos_prev;
        lst_index_base_[pos_prev].idx_next_ = pos_next;

        (lst_index_base_ + pos)->idx_next_ = freenext;
        (lst_index_base_ + pos)->idx_prev_ = (lst_index_base_ + freenext)->idx_prev_;

        lst_free_node_->idx_next_ = pos;

        (lst_index_base_ + freenext)->idx_prev_ = pos;
        lru_hash_head_->sz_usenode_  --;
        lru_hash_head_->sz_freenode_ ++;

        hash_index_base_[pos] = _INVALID_POINT;


        //������ʽ����������
        (value_base_ + pos)->~_value_type();
        priority_base_[pos] = 0;

        assert(lru_hash_head_->sz_usenode_ + lru_hash_head_->sz_freenode_ == lru_hash_head_->num_of_node_);
    }


    //��value��ȡֵ
    size_t bkt_num_value(const _value_type &obj) const
    {
        _extract_key get_key;
        return static_cast<size_t>(bkt_num_key(get_key(obj)));
    }

    //Ϊʲô������������ĺ���,�Լ�����һ��,
    //���صĻ������_value_type��_key_typeһ�����͵��ſް� ---inmore
    size_t bkt_num_key(const _key_type &key) const
    {
        _hash_fun hash_fun;
        return static_cast<size_t>(hash_fun(key) % lru_hash_head_->num_of_node_);
    }

public:


    //�õ���ʼ�ĵ�������λ��
    iterator begin()
    {
        for (size_t i = 0; i < lru_hash_head_->num_of_node_; ++i)
        {
            if (*(hash_factor_base_ + i) != _INVALID_POINT)
            {
                return iterator(*(hash_factor_base_ + i), this);
            }
        }

        return end();
    }

    //�õ�����λ��
    iterator end()
    {
        return iterator(_INVALID_POINT, this);
    }
    //��ǰʹ�õĽڵ�����
    size_t size() const
    {
        return lru_hash_head_->sz_usenode_;
    }
    //�õ�����
    size_t capacity() const
    {
        return lru_hash_head_->num_of_node_;
    }
    //
    bool empty() const
    {
        return (lru_hash_head_->sz_freenode_ == lru_hash_head_->num_of_node_);
    }
    //�Ƿ�ռ��Ѿ�����
    bool full() const
    {
        return (lru_hash_head_->sz_freenode_ == 0);
    }


    /*!
    * @brief      ����ڵ�
    * @return     std::pair<iterator, bool>  ���صĵ�������bool,
    * @param      val      ����Ľڵ�
    * @param      priority ���ȼ����ԣ������뵱ǰʱ����Ϊ��������̭ʱ��С��ĳ��ֵ
    *                      ����̭�ķ�������,����Ҫ��֤���洫������ֵ����  ��Ϊʲ
    *                      ô��ֱ����time(NULL),�Ǹ������������,
    * @note       ����Ὣ��������ݷ��������̭�ĵط�
    */
    std::pair<iterator, bool> insert_unique(const _value_type &val,
                                            unsigned int priority  /*=reinterpret_cast<unsigned int>(time(NULL))*/  )
    {
        size_t idx = bkt_num_value(val);
        size_t first = hash_factor_base_[idx];

        //ʹ������������,һ���൥������һ���Ƿ����?
        _extract_key get_key;
        _equal_key   equal_key;

        while (first != _INVALID_POINT )
        {

            //����ҵ���ͬ��Key����
            if (equal_key((get_key(value_base_[first])), (get_key(val))) == true )
            {
                return std::pair<iterator, bool>(iterator(first, this), false);
            }

            first = hash_index_base_[ first ];
        }

        //û���ҵ�,����������
        size_t newnode = create_node(val, priority);

        //�ռ䲻��,
        if (newnode == _INVALID_POINT)
        {
            return std::pair<iterator, bool>(iterator(_INVALID_POINT, this), false);
        }

        //����������
        hash_index_base_[newnode]  = hash_factor_base_[idx];
        hash_factor_base_[idx] = newnode;


        return std::pair<iterator, bool>(iterator(newnode, this), true);
    }


    //����ڵ�,�������
    //���ȼ����ԣ������뵱ǰʱ����Ϊ������
    std::pair<iterator, bool> insert_equal(const _value_type &val,
                                           unsigned int priority /*=reinterpret_cast<unsigned int>(time(NULL))*/ )
    {
        size_t idx = bkt_num_value(val);
        size_t first = hash_factor_base_[idx];

        //ʹ������������,һ���൥������һ���Ƿ����?
        _extract_key get_key;
        _equal_key   equal_key;

        while (first != _INVALID_POINT )
        {
            //����ҵ���ͬ��Key����
            if (equal_key((get_key(value_base_[first])), (get_key(val))) == true )
            {
                break;
            }

            first = hash_index_base_[ first ];
        }

        //û���ҵ�,����������
        size_t newnode = create_node(val, priority);

        //�ռ䲻��,
        if (newnode == _INVALID_POINT)
        {
            return std::pair<iterator, bool>(iterator(_INVALID_POINT, this), false);
        }

        //û���ҵ���ͬKEY������
        if (first == _INVALID_POINT)
        {
            //����������ײ��Ϳ�����
            hash_index_base_[newnode]  = hash_factor_base_[idx];
            hash_factor_base_[idx] = newnode;
        }
        //����ҵ�����ͬ��KEY�ڵ�
        else
        {
            //�ŵ�����ڵ�ĺ���
            hash_index_base_ [newnode] = hash_index_base_[first];
            hash_index_base_[first] = newnode;
        }

        return std::pair<iterator, bool>(iterator(newnode, this), true);
    }

    //��ѯ��Ӧ��Key�Ƿ���,���ص�����
    iterator find(const _key_type &key)
    {
        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[ idx];
        //ʹ������������,һ���൥������һ���Ƿ����?
        _extract_key get_key;
        _equal_key   equal_key;

        while (first != _INVALID_POINT && !equal_key(get_key(value_base_[first]), key))
        {
            first = hash_index_base_[ first ];
        }

        return iterator(first, this);
    }

    //
    iterator find_value(const _value_type &val)
    {
        _extract_key get_key;
        return find(get_key(val));
    }

    //�õ�ĳ��KEY��Ԫ�ظ������е��൱�ڲ�ѯ����
    size_t count(const _key_type &key)
    {
        size_t equal_count = 0;
        size_t idx = bkt_num_key(key);
        //���������ҵ���һ��
        size_t first = hash_factor_base_[ idx];

        //ʹ������������,һ���൥������һ���Ƿ����?
        _extract_key get_key;
        _equal_key   equal_key;

        //���б��м��ѯ
        while (first != _INVALID_POINT )
        {
            //����ҵ���ͬ��Key
            if (equal_key(get_key(value_base_[first]), key) == true )
            {
                ++equal_count;
            }

            first = hash_index_base_[ first ];
        }

        return equal_count;
    }

    //�õ�ĳ��VALUE��Ԫ�ظ������е��൱�ڲ�ѯ����
    size_t count_value(const _value_type &val)
    {
        _extract_key get_key;
        return count( get_key(val));
    }

    /*!
    * @brief
    * @return     bool
    * @param      key
    */
    bool erase_unique(const _key_type &key)
    {
        size_t idx = bkt_num_key(key);
        //���������ҵ���һ��
        size_t first = hash_factor_base_[ idx];
        size_t prev = first;

        //ʹ������������,һ���൥������һ���Ƿ����?
        _extract_key get_key;
        _equal_key   equal_key;

        //
        while (first != _INVALID_POINT )
        {
            //����ҵ���ͬ��Key
            if (equal_key(get_key(value_base_[first]), key) == true )
            {
                if (first == hash_factor_base_[ idx])
                {
                    hash_factor_base_[ idx] = hash_index_base_[first];
                }
                else
                {
                    hash_index_base_[prev] = hash_index_base_[first];
                }

                //���տռ�
                destroy_node(first);

                return true;
            }

            prev = first;
            first = hash_index_base_[ first ];
        }

        return false;
    }


    /*!
    * @brief      ʹ�õ�����ɾ��,������Ч���Բ��ü�д��
    * @return     bool ����ֵ
    * @param      it   ɾ���ĵ�����
    */
    bool erase(const iterator &it)
    {
        _extract_key get_key;
        size_t idx = bkt_num_key(get_key(*it));
        size_t first = hash_factor_base_[ idx];
        size_t prev = first;
        size_t itseq = it.getserial();

        //
        while (first != _INVALID_POINT )
        {
            if (first == itseq )
            {
                if (first == hash_factor_base_[ idx])
                {
                    hash_factor_base_[ idx] = hash_index_base_[first];
                }
                else
                {
                    hash_index_base_[prev] = hash_index_base_[first];
                }

                //���տռ�
                destroy_node(first);

                return true;
            }

            prev = first;
            first = hash_index_base_[ first ];
        }

        return false;
    }

    //ɾ��ĳ��ֵ
    bool erase_unique_value(const _value_type &val )
    {
        _extract_key get_key;
        return erase_unique( get_key(val));
    }


    /*!
    * @brief      ɾ��������ȵ�KEY������,��insert_equal���ʹ�ã�����ɾ���˼�������
    * @return     size_t
    * @param      key
    * @note
    */
    size_t erase_equal(const _key_type &key)
    {
        size_t erase_count = 0;
        size_t idx = bkt_num_key(key);
        //���������ҵ���һ��
        size_t first = hash_factor_base_[ idx];
        size_t prev = first;

        //ʹ������������,һ���൥������һ���Ƿ����?
        _extract_key get_key;
        _equal_key   equal_key;

        //ѭ����ѯ
        while (first != _INVALID_POINT )
        {
            //����ҵ���ͬ��Key
            if (equal_key(get_key(value_base_[first]), key) == true )
            {
                if (first == hash_factor_base_[ idx])
                {
                    hash_factor_base_[ idx] = hash_index_base_[first];
                }
                else
                {
                    hash_index_base_[prev] = hash_index_base_[first];
                }

                //ɾ���������prev���õ�����first����ƶ�
                size_t del_pos = first;
                first = hash_index_base_[ first ];
                //���տռ�
                destroy_node(del_pos);
                ++erase_count;
            }
            else
            {
                //����Ѿ�ɾ�������˳�ѭ������Ϊ���е�KEY��ͬ�Ķ�������һ��ɾ��Ҳ��һ��ɾ����.
                if (erase_count > 0)
                {
                    break;
                }

                prev = first;
                first = hash_index_base_[ first ];
            }
        }

        return erase_count;
    }



    /*!
    * @brief      ����,����������ݹҵ�LIST���ʼ,��̭ʹ��expire,disuse
    * @return     bool
    * @param      key
    * @param      priority ���ȼ���������ʹ�õ�ǰ��ʱ��
    */
    bool active_unique(const _key_type &key,
                       unsigned int priority /*=static_cast<unsigned int>(time(NULL))*/ )
    {
        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[ idx];
        //ʹ������������,һ���൥������һ���Ƿ����?
        _extract_key get_key;
        _equal_key   equal_key;

        //���б��м��ѯ
        while (first != _INVALID_POINT )
        {
            //����ҵ���ͬ��Key
            if (equal_key(get_key(value_base_[first]), key) == true )
            {
                priority_base_[first] = priority;

                size_t first_prv = lst_index_base_[first].idx_prev_;
                size_t first_nxt = lst_index_base_[first].idx_next_;
                //��ԭ���ĵط�ȡ����
                lst_index_base_[first_prv].idx_next_ = lst_index_base_[first].idx_next_;
                lst_index_base_[first_nxt].idx_prev_ = lst_index_base_[first].idx_prev_;

                //����ͷ��
                lst_index_base_[first].idx_next_ = lst_use_node_->idx_next_;
                lst_index_base_[lst_use_node_->idx_next_].idx_prev_ = first;
                lst_index_base_[first].idx_prev_ = lru_hash_head_->num_of_node_;
                lst_use_node_->idx_next_ = first;

                return true;

            }

            first = hash_index_base_[ first ];
        }

        return false;
    }



    /*!
    * @brief      ͨ��VALUE���ͬʱ��ֵ�滻�����µ�����VALUE��
    *             ���ȼ���������ʹ�õ�ǰ��ʱ�䣬MAPʹ�ã�
    * @return     bool     �Ƿ񼤻�ɹ�
    * @param      val      ֵ
    * @param      priority ���ȼ�
    * @note       LRU�������һ��ֵ��ʹ�ú󣬿�����Ϊ�Ǽ����һ�Σ�
    */
    bool active_unique_value(const _value_type &val,
                             unsigned int priority /*=static_cast<unsigned int>(time(NULL))*/ )
    {
        _extract_key get_key;
        _equal_key   equal_key;

        _key_type key = get_key(val);
        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[idx];
        //ʹ������������,һ���൥������һ���Ƿ����?



        //���б��м��ѯ
        while (first != _INVALID_POINT )
        {
            //����ҵ���ͬ��Key
            if (equal_key(get_key(value_base_[first]), key) == true )
            {
                priority_base_[first] = priority;
                value_base_[first] = val;
                size_t first_prv = lst_index_base_[first].idx_prev_;
                size_t first_nxt = lst_index_base_[first].idx_next_;
                //��ԭ���ĵط�ȡ����
                lst_index_base_[first_prv].idx_next_ = lst_index_base_[first].idx_next_;
                lst_index_base_[first_nxt].idx_prev_ = lst_index_base_[first].idx_prev_;

                //����ͷ��
                lst_index_base_[first].idx_next_ = lst_use_node_->idx_next_;
                lst_index_base_[lst_use_node_->idx_next_].idx_prev_ = first;
                lst_index_base_[first].idx_prev_ = lru_hash_head_->num_of_node_;
                lst_use_node_->idx_next_ = first;

                return true;

            }

            first = hash_index_base_[ first ];
        }

        return false;
    }

    //����������ͬ��KEY,����������ݹҵ�LIST���ʼ,��̭ʹ��expire
    size_t active_equal(const _key_type &key,
                        unsigned int priority /*=static_cast<unsigned int>(time(NULL))*/ )
    {
        size_t active_count = 0;

        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[ idx];
        //ʹ������������,һ���൥������һ���Ƿ����?
        _extract_key get_key;
        _equal_key   equal_key;

        //���б��м��ѯ
        while (first != _INVALID_POINT )
        {
            //����ҵ���ͬ��Key
            if (equal_key(get_key(value_base_[first]), key) == true )
            {
                priority_base_[first] = priority;

                size_t first_prv = lst_index_base_[first].idx_prev_;
                size_t first_nxt = lst_index_base_[first].idx_next_;
                //��ԭ���ĵط�ȡ����
                lst_index_base_[first_prv].idx_next_ = lst_index_base_[first].idx_next_;
                lst_index_base_[first_nxt].idx_prev_ = lst_index_base_[first].idx_prev_;

                //����ͷ��
                lst_index_base_[first].idx_next_ = lst_use_node_->idx_next_;
                lst_index_base_[first].idx_prev_ = lst_index_base_[lst_use_node_->idx_next_].idx_prev_;
                lst_index_base_[lst_use_node_->idx_next_].idx_prev_ = first;
                lst_use_node_->idx_next_ = first;
            }
            else
            {
                //����Ѿ������ݣ��˳�ѭ������Ϊ���е�KEY��ͬ�Ķ�������һ��ɾ��Ҳ��һ��ɾ����.
                if (active_count > 0)
                {
                    break;
                }
            }

            first = hash_index_base_[ first ];

        }

        return active_count;
    }



    //��̭���ڵ�����,����LIST�м�������ǰ��չ���ʵ������ģ���Ҫ���㴫������ȼ������ʱ��
    //С�ڵ���������ȼ������ݽ�����̭
    size_t expire(unsigned int expire_time)
    {
        //��β����ʼ��飬
        size_t list_idx = lst_use_node_->idx_prev_;
        size_t expire_num = 0;

        while (list_idx != lru_hash_head_->num_of_node_)
        {
            //С�ڵ���
            if (priority_base_[list_idx] <= expire_time)
            {
                size_t del_iter = list_idx;
                _washout_fun wash_fun;
                wash_fun(value_base_[del_iter]);
                ++expire_num;
                //
                iterator iter_tmp(del_iter, this);
                erase(iter_tmp);
            }
            else
            {
                break;
            }

            //���ɾ���ˣ����Ǽ���һ��
            list_idx = lst_use_node_->idx_prev_;
        }

        return expire_num;
    }

    //ϣ����̭��disuse_num�����ݣ�
    //���disuse_eaqul == ture����ɾ�������ɾ�����Ǹ����ȼ���ȵ�����Ԫ��
    //disuse_eaqul���Ա�֤���ݵ�������̭������һ��KEY�Ĳ����������ڴ棬һ���ֲ���
    size_t disuse(size_t disuse_num, bool disuse_eaqul)
    {
        //��β����ʼ��飬
        size_t list_idx = lst_use_node_->idx_prev_;
        size_t fact_del_num = 0;
        unsigned int disuse_priority = 0;

        for (size_t i = 0; i < disuse_num && list_idx != lru_hash_head_->num_of_node_; ++i)
        {
            size_t del_iter = list_idx;
            _washout_fun wash_fun;
            wash_fun(value_base_[del_iter]);
            ++fact_del_num;
            //
            iterator iter_tmp(del_iter, this);
            erase(iter_tmp);
            disuse_priority = priority_base_[list_idx];
            //
            //���ɾ���ˣ����Ǽ���һ��
            list_idx = lst_use_node_->idx_prev_;
        }

        if ( true == disuse_eaqul && disuse_num > 0)
        {
            fact_del_num += expire(disuse_priority);
        }

        return fact_del_num;
    }

    //��ע�����¸�һ�����ݴ�һ�����ȼ���ǩ����̭ʹ�ú���washout
    bool mark_unique(const _key_type &key, unsigned int priority)
    {
        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[ idx];
        //ʹ������������,һ���൥������һ���Ƿ����?
        _extract_key get_key;
        _equal_key   equal_key;

        //���б��м��ѯ
        while (first != _INVALID_POINT )
        {
            //����ҵ���ͬ��Key
            if (equal_key(get_key(value_base_[first]), key) == true )
            {
                priority_base_[first] = priority;

                return true;
            }

            first = hash_index_base_[ first ];
        }

        return false;
    }

    //����value�����ȼ����£����¸�һ�����ݴ�һ�����ȼ���ǩ��ͬʱ��ֵ�滻��
    bool mark_value(const _value_type &val, unsigned int priority )
    {
        //ʹ������������,һ���൥������һ���Ƿ����?
        _extract_key get_key;
        _equal_key   equal_key;

        _key_type key = get_key(val);


        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[ idx];

        //���б��м��ѯ
        while (first != _INVALID_POINT )
        {
            //����ҵ���ͬ��Key
            if (equal_key(get_key(value_base_[first]), key) == true )
            {
                priority_base_[first] = priority;
                value_base_[first] = val;
                return true;
            }

            first = hash_index_base_[ first ];
        }

        return false;
    }

    //��ע������ȵ����ݣ����¸�һ�����ݴ�һ�����ȼ���ǩ����̭ʹ�ú���washout
    bool mark_equal(const _key_type &key, unsigned int priority)
    {
        size_t mark_count = 0;

        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[ idx];
        //ʹ������������,һ���൥������һ���Ƿ����?
        _extract_key get_key;
        _equal_key   equal_key;

        //���б��м��ѯ
        while (first != _INVALID_POINT )
        {
            //����ҵ���ͬ��Key
            if (equal_key(get_key(value_base_[first]), key) == true )
            {
                priority_base_[first] = priority;
            }
            else
            {
                //����Ѿ������ݣ��˳�ѭ������Ϊ���е�KEY��ͬ�Ķ�������һ��ɾ��Ҳ��һ��ɾ����.
                if (mark_count > 0)
                {
                    break;
                }
            }

            first = hash_index_base_[ first ];

        }

        return mark_count;
    }

    //��ע������ȵ����ݡ���������е��Ť���Ҳ�ϲ��������ΪVALUEû�У�û��������
    //size_t mark_value_equal(const _value_type &val, unsigned int priority )
    //{
    //    return mark(_extract_key(val),priority);
    //}


    //��̭���ȼ����͵�����,LIST�м���������������Ҳ����.ֻ��̭num_wash����������
    //��������̭�Ǵ�ͷ����ʼ���о�����̫�á�
    void washout(unsigned int wash_priority, size_t num_wash)
    {
        size_t list_idx = lst_use_node_->idx_next_;
        size_t num_del = 0;

        //��ΪNULL������ɾ���ĸ���û�дﵽ����ͷ����ʼ�Ƿ����?�Ҳ�ȷ�����������ṩһ������
        while ( list_idx != lru_hash_head_->num_of_node_ && num_del < num_wash)
        {
            //������ȼ�С����̭ϵ��
            if (priority_base_[list_idx] < wash_priority)
            {
                ++num_del;
                size_t del_iter = list_idx;
                list_idx = lst_index_base_[list_idx].idx_next_;
                //
                _washout_fun wash_fun;
                wash_fun(value_base_[del_iter]);
                //
                iterator iter_tmp(del_iter, this);
                erase(iter_tmp);
            }
            else
            {
                continue;
            }
        }
    }
};



/************************************************************************************************************
template           : shm_hashset_expire
************************************************************************************************************/
template < class _value_type,
           class _hash_fun = smem_hash<_value_type>,
           class _equal_key = std::equal_to<_value_type>,
           class _washout_fun = _default_washout_fun<_value_type> >
class shm_hashset_expire :
    public shm_hashtable_expire < _value_type,
    _value_type,
    _hash_fun,
    smem_identity<_value_type>,
    _equal_key,
    _washout_fun >
{

protected:

    //����ڹ����ڴ�ʹ��,û��new,����ͳһ��initialize ��ʼ��
    //�������,��������,���ǲ�������
    shm_hashset_expire<_value_type, _hash_fun, _equal_key, _washout_fun >(size_t numnode, void *pmmap, bool if_restore):
        shm_hashtable_expire<_value_type, _value_type, _hash_fun, smem_identity<_value_type>, _equal_key, _washout_fun>(numnode, pmmap, if_restore)
    {
        initialize(numnode, pmmap, if_restore);
    }

    ~shm_hashset_expire<_value_type, _hash_fun, _equal_key, _washout_fun>()
    {
    }

public:
    static shm_hashset_expire< _value_type, _hash_fun, _equal_key, _washout_fun > *
    initialize(size_t &numnode, char *pmmap, bool if_restore = false)
    {
        return reinterpret_cast<shm_hashset_expire< _value_type, _hash_fun, _equal_key, _washout_fun >*>(
                   shm_hashtable_expire<_value_type, _value_type, _hash_fun, smem_identity<_value_type>, _equal_key, _washout_fun>::initialize(numnode, pmmap, if_restore));
    }
};



/************************************************************************************************************
template           : smem_hashmap_expire
************************************************************************************************************/
template < class _key_type,
           class _value_type,
           class _hash_fun = smem_hash<_key_type>,
           class _extract_key = mmap_select1st <std::pair <_key_type, _value_type> >,
           class _equal_key = std::equal_to<_key_type>,
           class _washout_fun = _default_washout_fun<_value_type> >
class smem_hashmap_expire :
    public shm_hashtable_expire < std::pair <_key_type, _value_type>,
    _key_type,
    _hash_fun,
    _extract_key,
    _equal_key,
    _washout_fun >
{


protected:

    //����ڹ����ڴ�ʹ��,û��new,����ͳһ��initialize ��ʼ��
    //�������,��������,���ǲ�������
    smem_hashmap_expire<_key_type, _value_type, _hash_fun, _extract_key, _equal_key, _washout_fun >(size_t numnode, void *pmmap, bool if_restore):
        shm_hashtable_expire< std::pair <_key_type, _value_type>, _key_type, _extract_key, _equal_key, _washout_fun >(numnode, pmmap, if_restore)
    {
        initialize(numnode, pmmap, if_restore);
    }

    ~smem_hashmap_expire<_key_type, _value_type, _hash_fun, _extract_key, _equal_key, _washout_fun >()
    {
    }

public:
    static smem_hashmap_expire< _key_type, _value_type, _hash_fun, _extract_key, _equal_key, _washout_fun  > *
    initialize(size_t &numnode, char *pmmap, bool if_restore = false)
    {
        return reinterpret_cast<smem_hashmap_expire< _key_type, _value_type, _hash_fun, _extract_key, _equal_key, _washout_fun >*>(
                   shm_hashtable_expire< std::pair <_key_type, _value_type>, _key_type, _hash_fun, _extract_key, _equal_key, _washout_fun >::initialize(numnode, pmmap, if_restore));
    }
    //[]�����������ŵ��ȱ�㣬
    _value_type &operator[](const _key_type &key)
    {
        return (find(key)).second;
    }
};

};




#endif //#ifndef ZCE_SHM_LRU_HASH_TABLE_H_ 


