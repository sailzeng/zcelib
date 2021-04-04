/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_hash_table.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2005��10��26��
* @brief      �ڹ����ڴ���ʹ�õ�hashtable
*             Ϊ�˼�͵����������Ƶ�Hashtable�����Դ����ͬ��key��ֻҪ�����
*             insert_equal ���ຯ���Ϳ����ˣ�������STL������������һ����ȥ��װ
*
* @details    STL��HASHTABLE̫TMD�ľ���!,���㷨����װ,�ر��Ƿ�װ����ֱ��������ˮ!.
*
*             2010��4�£�
*             ��������������࣬�ͺ���һЩ��װ˼·����һ�£�ȥ���˼����ⲿ��װ�ķ�װ
*             ȥ����_shm_hashtable_node ,_shm_hashtable_index�����Ա��ִ���
*             һ�£����⣬�����ܵĽǶȽ�����һ�ο���ת����
*
*/

#ifndef ZCE_LIB_SMEM_HASH_TABLE_H_
#define ZCE_LIB_SMEM_HASH_TABLE_H_

#include "zce_shm_predefine.h"

//ԭ�����������ṹ������ɾ����ԭ����2��
//������д�Ĵ���û�������Ƶķ�����
//д�ָ���ʱ�����������һ��ṹҪ�鷳һ�㣬

//hashtable�Ľڵ�,ȥ����ԭ���Ǳ����ο������죬
//template <class _value_type> class _shm_hashtable_node
//{
//public:
//    //NODE�����ָ��,
//    size_t        next_;
//    //ValueType����
//    _value_type   hash_data_;
//

namespace zce
{

template < class _value_type,
           class _key_type,
           class _hash_fun,
           class _extract_key,
           class _equal_key > class shm_hashtable;

///

/*!
* @brief      hash_table�ĵ�����.
*             ֻ�ṩ�˵����������++��
*             ģ������ҾͲ����ͣ�������hash_tableһ�£��㿴�����˵���ѡ�
*/
template <class _value_type, class _key_type, class _hash_fun, class _extract_key, class _equal_key>
class _shm_hashtable_iterator
{
protected:

    ///hash_type���壬����ʹ��
    typedef shm_hashtable<_value_type, _key_type, _hash_fun, _extract_key, _equal_key> shm_hashtable_t;
    ///���������壬���������ʹ��
    typedef _shm_hashtable_iterator<_value_type, _key_type, _hash_fun, _extract_key, _equal_key> iterator;

public:

    ///���캯��
    _shm_hashtable_iterator(size_t seq, shm_hashtable_t *instance):
        serial_(seq),
        ht_instance_(instance)
    {
    }

    ///Ĭ�Ϲ��캯��
    _shm_hashtable_iterator()
    {
    }

    ///������žͿ����ٸ���ģ��ʵ���������ҵ���Ӧ����,����ʹ��ָ��
    size_t getserial() const
    {
        return serial_;
    }

    ///�ṩ���������Ϊ�˼ӿ�ܶຯ�����ٶ�,
    size_t getnext()
    {
        return *(ht_instance_->next_index_ + serial_);
    }

    ///
    bool operator==(const iterator &x) const
    {
        return (serial_ == x.serial_ && ht_instance_ == x.ht_instance_);
    }
    ///
    bool operator!=(const iterator &x) const
    {
        return !(*this == x);
    }

    ///�������
    _value_type &operator*() const
    {
        return *(ht_instance_->data_base_ + serial_);
    }
    ///�ڶ��̵߳Ļ������ṩ������ͷ����ǲ���ȫ��,��û�м���,ԭ����˵��
    _value_type *operator->() const
    {
        //
        return &(operator*());
    }

    ///���������
    iterator &operator++()
    {
        size_t oldseq = serial_;
        serial_ = *(ht_instance_->next_index_ + serial_);

        //�������ڵ���ĩλ�Ľڵ�
        if (serial_ == _shm_memory_base::_INVALID_POINT)
        {
            //˳��Index��ѯ.
            size_t bucket = ht_instance_->bkt_num_value(*(ht_instance_->data_base_ + oldseq));

            while (serial_ == _shm_memory_base::_INVALID_POINT && ++bucket < ht_instance_->capacity() )
            {
                serial_ = *(ht_instance_->index_base_ + bucket);
            }
        }

        return *this;
    }

    ///iter++,
    iterator operator++(int)
    {
        iterator tmp = *this;
        ++(*this);
        return tmp;
    }

protected:
    ///���к�
    size_t           serial_;
    ///HashTable��ָ��,
    shm_hashtable_t *ht_instance_;
};

/*!
* @brief shm_hash_table��ͷ���������Ľṹ
*        ��¼��С��ʹ�������
*/
class _shm_hash_table_head
{
protected:
    _shm_hash_table_head():
        size_of_mmap_(0),
        num_of_node_(0),
        sz_freenode_(0),
        sz_usenode_(0),
        sz_useindex_(0),
        free_headnode_(0)
    {
    }
    ~_shm_hash_table_head()
    {
    }

public:
    ///�ڴ����ĳ���
    size_t                             size_of_mmap_;

    ///NODE,INDEX������,INDEX�ĸ�����NODE�Ľڵ����Ϊ1:1,
    size_t                             num_of_node_;

    ///FREE��NODE����
    size_t                             sz_freenode_;
    ///USE��NODE����
    size_t                             sz_usenode_;

    ///ʹ�õ�INDEX����,�����˽�ʵ�ʿ����ĸ��ر���
    size_t                             sz_useindex_;

    ///FREE NODE��ͷָ��
    size_t                             free_headnode_;

};

/*!
* @brief      HashTABLE�Ķ���,����HashTable,�ŷ��ַ�װ̫̫����,����STL��,����Щ�������Թ��¾�
*             ����,�����ϵ�������Ϊ1,��������һЩ,���ǿ���STL��hasttable�ڸ��Ӵﵽ1������,
*             ����Ĳ���˳���Ҳ�ϲ��,������STL��˳���
* @tparam     _value_type  �ڵ��ʵ��ֵ�ͱ�
* @tparam     _key_type    �ڵ�ļ����ͱ�
* @tparam     _hash_fun    _key_type��hash����,���ߺ�������
* @tparam     _extract_key �ӽڵ��еõ���ֵ�ķ���,���ߺ�������
* @tparam     _equal_key    �Ƚϼ�ֵ�Ƿ���ȵķ���,���ߺ�������
* @note
*/
template < class _value_type,
           class _key_type,
           class _hash_fun = smem_hash<_key_type>,
           class _extract_key = smem_identity<_value_type>,
           class _equal_key = std::equal_to<_key_type> >
class shm_hashtable : public _shm_memory_base
{
public:
    //�����Լ�
    typedef shm_hashtable<_value_type, _key_type, _hash_fun, _extract_key, _equal_key> self;

    //���������
    typedef _shm_hashtable_iterator<_value_type, _key_type, _hash_fun, _extract_key, _equal_key> iterator;

    //��Ԫ
    friend class _shm_hashtable_iterator<_value_type, _key_type, _hash_fun, _extract_key, _equal_key>;

public:
    //����ڹ����ڴ�ʹ��,û��new,����ͳһ��initialize ��ʼ��
    //�������,��������,���ǲ�������
    shm_hashtable<_value_type, _key_type, _hash_fun, _extract_key, _equal_key >(size_t numnode, void *pmmap, bool if_restore):
        _shm_memory_base(NULL),
        hash_head_(NULL),
        index_base_(NULL),
        data_base_(NULL)
    {
    }

    shm_hashtable<_value_type, _key_type, _hash_fun, _extract_key, _equal_key >():
        _shm_memory_base(NULL)
    {
    }

    ~shm_hashtable<_value_type, _key_type, _hash_fun, _extract_key, _equal_key >()
    {
    }

    //ֻ����,��ʵ��,
    const self &operator=(const self &others);

protected:

    //����һ��NODE,�����FREELIST��ȡ��
    size_t create_node(const _value_type &val)
    {
        //���û�пռ���Է���
        if (hash_head_->sz_freenode_ == 0)
        {
            return _INVALID_POINT;
        }

        //������ȡ��һ�����нڵ�
        size_t new_node = hash_head_->free_headnode_;
        hash_head_->free_headnode_ = *(next_index_ + new_node);
        -- hash_head_->sz_freenode_ ;
        ++ hash_head_->sz_usenode_ ;

        //��placement new���и�ֵ
        new (data_base_ + new_node)_value_type(val);

        return new_node;
    }

    //�ͷ�һ��NODE,����黹��FREELIST,����������Ǽ�
    void destroy_node(size_t pos)
    {
        size_t freenext = hash_head_->free_headnode_;
        *(next_index_ + pos) = freenext;
        hash_head_->free_headnode_ = pos;

        ++ hash_head_->sz_freenode_ ;
        -- hash_head_->sz_usenode_ ;

        //������ʽ����������
        (data_base_ + pos)->~_value_type();
    }

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
        zce::hash_prime(req_num, real_num);
        return  sizeof(_shm_hash_table_head)  +
                sizeof(size_t) * (real_num) +
                sizeof(_value_type) * real_num +
                sizeof(size_t) * (real_num);
    }


    /*!
    * @brief      ��ʼ��
    * @return     self*
    * @param      req_num   �����NODE����
    * @param      real_num
    * @param      pmmap
    * @param      if_restore
    */
    static self *initialize(size_t req_num, size_t &real_num, char *pmmap, bool if_restore = false)
    {
        assert(pmmap != NULL && req_num > 0);
        //����
        size_t sz_mmap = getallocsize(req_num, real_num);

        _shm_hash_table_head *hashhead =  reinterpret_cast< _shm_hash_table_head * >(pmmap);

        //����ǻָ�,���ݶ����ڴ���,
        if (if_restore == true)
        {
            //���һ�»ָ����ڴ��Ƿ���ȷ,
            if (sz_mmap != hashhead->size_of_mmap_ ||
                real_num != hashhead->num_of_node_ )
            {
                //һ������²�һ�·���NULL����ʶ�ָ�ʧ�ܣ�
#if ALLOW_RESTORE_INCONFORMITY != 1
                return NULL;
#else
                ZCE_LOG(RS_ALERT, "Hash Table node initialize number[%lu|%lu] and restore number [%lu|%lu] "
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
            //��ʼ���ߴ�
            hashhead->size_of_mmap_ = sz_mmap;
            hashhead->num_of_node_ = real_num;
        }


        shm_hashtable< _value_type, _key_type, _hash_fun, _extract_key, _equal_key  > *instance
            = new shm_hashtable< _value_type, _key_type, _hash_fun, _extract_key, _equal_key  >();

        //���е�ָ�붼�Ǹ��ӻ���ַ����õ���,���ڷ������,ÿ�γ�ʼ�������¼���

        //������Щָ��ĵ�ַ����Ҫ��Ϊ�˷������Ĵ���
        instance->smem_base_ = pmmap;
        char *tmp_base = instance->smem_base_;

        instance->hash_head_ = hashhead;
        tmp_base = tmp_base + sizeof(_shm_hash_table_head);

        instance->index_base_ = reinterpret_cast<size_t *>(tmp_base );
        tmp_base = tmp_base +  sizeof(size_t) * (real_num );

        instance->data_base_ = reinterpret_cast< _value_type *>(tmp_base );
        tmp_base = tmp_base +  sizeof(_value_type) * (real_num );

        instance->next_index_ = reinterpret_cast< size_t *>(tmp_base );

        if (if_restore == false)
        {
            //�����ʼ�����е��ڴ�,���еĽڵ�ΪFREE
            instance->clear();
        }

        //�����չ�
        return instance;
    }

    //�����ʼ�����е��ڴ�,���еĽڵ�ΪFREE
    void clear()
    {
        //����ؼ�Node,�Լ���س���,��ʼ���е�������free.
        hash_head_->sz_freenode_ = hash_head_->num_of_node_;
        hash_head_->sz_usenode_ = 0;
        hash_head_->sz_useindex_ = 0;

        //��ʼ��free������
        for (size_t i = 0; i < hash_head_->num_of_node_ ; ++i )
        {
            index_base_[i] = _INVALID_POINT;
        }

        //����FREELIST�ĵ���NODE,
        hash_head_->free_headnode_ = 0;

        //
        for (size_t i = 0; i < hash_head_->num_of_node_ ; ++i )
        {
            next_index_[i] = i + 1;

            if (i == hash_head_->num_of_node_ - 1)
            {
                next_index_[i] = _INVALID_POINT;
            }
        }
    }

    //��value��ȡֵ
    size_t bkt_num_value(const _value_type &obj) const
    {
        _extract_key get_key;
        return static_cast<size_t>(bkt_num_key(get_key(obj)));
    }
    //Ϊʲô������������ĺ���,�Լ�����һ��,
    size_t bkt_num_key(const _key_type &key) const
    {
        _hash_fun hash_fun;
        return static_cast<size_t>(hash_fun(key) % hash_head_->num_of_node_);
    }

    //HashTable��Begin ���Ǹ�Ч����,������̫��ʹ��
    iterator begin()
    {
        for (size_t i = 0; i < hash_head_->num_of_node_; ++i)
        {
            if (*(index_base_ + i) != _INVALID_POINT)
            {
                return iterator(*(index_base_ + i), this);
            }
        }

        return end();
    }
    //����Чָ��
    iterator end()
    {
        return iterator(_INVALID_POINT, this);
    }

    //
    bool empty()
    {
        if (hash_head_->sz_freenode_ == hash_head_->num_of_node_)
        {
            return true;
        }

        return false;
    }
    //�ڲ�������ǰ����,����������
    bool full()
    {
        if (hash_head_->sz_freenode_ == 0 )
        {
            return true;
        }

        return false;
    }

    //��ѯ��Ӧ��Key�Ƿ���
    iterator find(const _key_type &key)
    {
        size_t idx = bkt_num_key(key);
        size_t first = *(index_base_ +  idx);
        //ʹ������������,һ���൥������һ���Ƿ����?
        _extract_key get_key;
        _equal_key   equal_key;

        //
        while (first != _INVALID_POINT && !equal_key(get_key(*(data_base_ + first )), key))
        {
            first = *(next_index_ + first );
        }

        return iterator(first, this);
    }
    //
    iterator find_value(const _value_type &val)
    {
        _extract_key get_key;
        return find(get_key(val));
    }

    //
    _value_type &find_or_insert(const _value_type &val)
    {
        iterator iter = find_value(val);

        if (iter == end())
        {
            std::pair<iterator, bool> pair_iter = insert_unique(val);
            return (*(pair_iter.first));
        }

        return *iter;
    }


    /*!
    * @brief      ����ڵ�,�����������ͬ�ڵ�Ĳ���
    * @return     std::pair<iterator, bool> iteratorΪ���صĵ�������boolΪ�Ƿ����ɹ���
    * @param      val ���������
    */
    std::pair<iterator, bool> insert_unique(const _value_type &val)
    {
        size_t idx = bkt_num_value(val);
        size_t first_idx = *(index_base_ +  idx);

        //ʹ������������,һ���൥������һ���Ƿ����?
        _extract_key get_key;
        _equal_key   equal_key;

        size_t nxt_idx = first_idx;

        while (nxt_idx != _INVALID_POINT )
        {
            //����ҵ���ͬ��Key����
            if (equal_key((get_key(*(data_base_ + nxt_idx ))), (get_key(val))) == true )
            {
                return std::pair<iterator, bool>(iterator(nxt_idx, this), false);
            }

            nxt_idx = *(next_index_ + nxt_idx );
        }

        //û���ҵ�,����������
        size_t newnode = create_node(val);
        //�ռ䲻��,
        if (newnode == _INVALID_POINT)
        {
            return std::pair<iterator, bool>(iterator(_INVALID_POINT, this), false);
        }

        //����������
        *(next_index_ + newnode) = *(index_base_ + idx);
        *(index_base_ + idx) = newnode;

        //�����һ��λ�þͲ�����Ч��INDEX
        if (first_idx == _INVALID_POINT)
        {
            //��¼ʹ����һ������
            ++(hash_head_->sz_useindex_);
        }

        return std::pair<iterator, bool>(iterator(newnode, this), true);
    }


    /*!
    * @brief      ����ڵ�,������ȣ�KEY���Ľڵ����,
    * @return     std::pair<iterator, bool> iteratorΪ���صĵ�������boolΪ�Ƿ����ɹ���
    * @param      val ���������
    */
    std::pair<iterator, bool> insert_equal(const _value_type &val)
    {
        size_t idx = bkt_num_value(val);
        size_t first_idx = *(index_base_ +  idx);

        //ʹ������������,һ���൥������һ���Ƿ����?
        _extract_key get_key;
        _equal_key   equal_key;

        size_t nxt_idx = first_idx;

        while (nxt_idx != _INVALID_POINT )
        {
            //����ҵ���ͬ��Key����,�Ὣ��ͬ�����ݷ���һ�𣬱��ڴ���
            if (equal_key((get_key(*(data_base_ + nxt_idx ))), (get_key(val))) == true )
            {
                break;
            }

            nxt_idx = *(next_index_ + nxt_idx );
        }

        //û���ҵ�,����������
        size_t newnode = create_node(val);

        //�ռ䲻��,
        if (newnode == _INVALID_POINT)
        {
            return std::pair<iterator, bool>(iterator(_INVALID_POINT, this), false);
        }

        //û���ҵ���ͬKEY������
        if (nxt_idx == _INVALID_POINT)
        {
            //����������ײ��Ϳ�����
            (next_index_ + newnode) = *(index_base_ + idx);
            *(index_base_ + idx) = newnode;
        }
        //����ҵ�����ͬ��KEY�ڵ�
        else
        {
            //�ŵ�����ڵ�ĺ���
            *(next_index_ + newnode) = *(next_index_ + nxt_idx );
            *(next_index_ + nxt_idx ) = newnode;
        }

        //�����һ��λ�þͲ�����Ч��INDEX,��¼ʹ����INDEX
        if (first_idx == _INVALID_POINT)
        {
            ++(hash_head_->sz_useindex_);
        }

        return std::pair<iterator, bool>(iterator(newnode, this), true);
    }


    /*!
    * @brief      �õ�KEY��ͬ��Ԫ�ظ������е��൱�ڲ�ѯ����
    * @return     size_t ����
    * @param      key    ��ѯ��key��
    */
    size_t count(const _key_type &key)
    {
        size_t equal_count = 0;
        size_t idx = bkt_num_key(key);
        //���������ҵ���һ��
        size_t first = *(index_base_ +  idx);

        //ʹ������������,һ���൥������һ���Ƿ����?
        _extract_key get_key;
        _equal_key   equal_key;

        //
        while (first != _INVALID_POINT )
        {
            //����ҵ���ͬ��Key
            if (equal_key(get_key(*(data_base_ + first )), key) == true )
            {
                ++equal_count;
            }

            first = (next_index_ + first );
        }

        return equal_count;
    }


    /*!
    * @brief      ����keyɾ��,
    * @return     bool �Ƿ�ɾ���ɹ�
    * @param      key ɾ�����ݵ�key
    */
    bool erase_unique(const _key_type &key)
    {
        size_t idx = bkt_num_key(key);
        //���������ҵ���һ��
        size_t first = *(index_base_ +  idx);
        size_t prev = first;

        //ʹ������������,һ���൥������һ���Ƿ����?
        _extract_key get_key;
        _equal_key   equal_key;

        //
        while (first != _INVALID_POINT )
        {
            //����ҵ���ͬ��Key
            if (equal_key(get_key(*(data_base_ + first )), key) == true )
            {
                if (first == *(index_base_ + idx))
                {
                    *(index_base_ + idx) = *(next_index_ + first );
                }
                else
                {
                    *(next_index_ + prev ) = *(next_index_ + first );
                }

                destroy_node(first);

                //���INDEX�Ѿ���ɾ���ˣ�ȡ����¼
                if ( *(index_base_ + idx) == _shm_memory_base::_INVALID_POINT )
                {
                    --(hash_head_->sz_useindex_);
                }

                return true;
            }

            prev = first;
            first = *(next_index_ + first );
        }

        return false;
    }


    /*!
    * @brief      ʹ�õ�����ɾ��,������Ч���Բ��ü�д��
    * @return     bool �Ƿ�ɾ���ɹ�
    * @param      it ɾ�����ݵĵ�������
    */
    bool erase(const iterator &it)
    {
        _extract_key get_key;
        size_t idx = bkt_num_key(get_key(*it));
        size_t first = *(index_base_ +  idx);
        size_t prev = first;
        size_t itseq = it.getserial();

        //
        while (first != _INVALID_POINT )
        {
            if (first == itseq )
            {
                if (first == *(index_base_ + idx))
                {
                    *(index_base_ + idx) = *(next_index_ + first );
                }
                else
                {
                    *(next_index_ + prev ) = *(next_index_ + first );
                }

                destroy_node(first);

                //���INDEX�Ѿ���ɾ���ˣ�ȡ����¼
                if ( *(index_base_ + idx) == _shm_memory_base::_INVALID_POINT )
                {
                    --(hash_head_->sz_useindex_);
                }

                return true;
            }

            prev = first;
            first = *(next_index_ + first );
        }

        return false;
    }


    //ɾ��������ȵ�KEY������,��insert_equal���ʹ�ã�����ɾ���˼�������
    size_t erase_equal(const _key_type &key)
    {
        size_t erase_count = 0;
        size_t idx = bkt_num_key(key);
        //���������ҵ���һ��
        size_t first = *(index_base_ +  idx);
        size_t prev = first;

        //ʹ������������,һ���൥������һ���Ƿ����?
        _extract_key get_key;
        _equal_key   equal_key;

        //
        while (first != _INVALID_POINT )
        {
            //����ҵ���ͬ��Key
            if (equal_key(get_key(*(data_base_ + first )), key) == true )
            {
                if (first == *(index_base_ + idx))
                {
                    *(index_base_ + idx) = *(next_index_ + first );
                }
                else
                {
                    *(next_index_ + prev ) = *(next_index_ + first );
                }

                //ɾ���������prev���õ�����first����ƶ�
                size_t del_pos = first;
                first = *(next_index_ + first );

                destroy_node(del_pos);

                //���INDEX�Ѿ���ɾ���ˣ�ȡ����¼
                if ( *(index_base_ + idx) == _shm_memory_base::_INVALID_POINT )
                {
                    --(hash_head_->sz_useindex_);
                }

                ++erase_count;
            }
            else
            {
                //����Ѿ�ɾ�������˳�ѭ������Ϊ���е�KEY��ͬ�Ķ�������һ��ɾ��Ҳ��һ��ɾ����.
                if (erase_count > 0)
                {
                    break;
                }

                //û���ҵ�����������ƶ�
                prev = first;
                first = *(next_index_ + first );
            }
        }

        return erase_count;
    }


    //�����������Ѿ��е�Ԫ�ظ���
    size_t size()
    {
        return hash_head_->sz_usenode_;
    }

    //����NODE���ӵ�����
    size_t capacity()
    {
        return hash_head_->num_of_node_;
    }

    //���еĽڵ����
    size_t sizefreenode()
    {
        return hash_head_->sz_freenode_;
    }

    //ʹ�õ������ĸ���
    size_t sizeuseindex()
    {
        return hash_head_->sz_useindex_;
    }

protected:

    _shm_hash_table_head         *hash_head_;

    ///���е�ָ�붼�Ǹ��ݻ���ַ����õ���,����������Ҫ���ڷ������,
    ///ÿ�γ�ʼ�������¼���,���Բ����е�ַ��������⣬������Щֵ�Ǹ���ӵ�У�������ģ�
    //����������,
    size_t                       *index_base_;

    //��������ʼָ��,
    _value_type                  *data_base_;

    //���Ԫ�غ�����Ŀ���������
    size_t                       *next_index_;
};

template<class _value_type, class _hash_fun = smem_hash<_value_type>, class _equal_key = std::equal_to<_value_type> >
class shm_hashset :
    public shm_hashtable< _value_type, _value_type, _hash_fun, smem_identity<_value_type>, _equal_key  >
{
public:
    //���������
    //typedef shm_hashtable< _value_type,_value_type ,_hash_fun, _extract_key,_equal_key  >::iterator iterator;
protected:

    //����ڹ����ڴ�ʹ��,û��new,����ͳһ��initialize ��ʼ��
    //�������,��������,���ǲ�������
    shm_hashset<_value_type, _hash_fun, _equal_key >(size_t numnode, void *pmmap, bool if_restore):
        shm_hashtable<_value_type, _value_type, _hash_fun, smem_identity<_value_type>, _equal_key>(numnode, pmmap, if_restore)
    {
        initialize(numnode, pmmap, if_restore);
    }

    ~shm_hashset<_value_type, _hash_fun, _equal_key >()
    {
    }

public:
    static shm_hashset< _value_type, _hash_fun, _equal_key  > *
    initialize(size_t &numnode, char *pmmap, bool if_restore = false)
    {
        return reinterpret_cast<shm_hashset< _value_type, _hash_fun, _equal_key  >*>(
                   shm_hashtable<_value_type, _value_type, _hash_fun, smem_identity<_value_type>, _equal_key>::initialize(numnode, pmmap, if_restore));
    }
};

//HASH MAP
template<class _key_type, class _value_type, class _hash_fun = smem_hash<_key_type>, class _extract_key = mmap_select1st <std::pair <_key_type, _value_type> >, class _equal_key = std::equal_to<_key_type> >
class shm_hashmap :
    public shm_hashtable< std::pair <_key_type, _value_type>, _key_type, _hash_fun, _extract_key, _equal_key  >
{

protected:

    //����ڹ����ڴ�ʹ��,û��new,����ͳһ��initialize ��ʼ��
    //�������,��������,���ǲ�������
    shm_hashmap<_key_type, _value_type, _hash_fun, _extract_key, _equal_key >(size_t numnode, void *pmmap, bool if_restore):
        shm_hashtable< std::pair <_key_type, _value_type>, _key_type, _extract_key, _equal_key  >(numnode, pmmap, if_restore)
    {
        initialize(numnode, pmmap, if_restore);
    }

    ~shm_hashmap<_key_type, _value_type, _hash_fun, _extract_key, _equal_key >()
    {
    }
public:
    static shm_hashmap< _key_type, _value_type, _hash_fun, _extract_key, _equal_key  > *
    initialize(size_t &numnode, char *pmmap, bool if_restore = false)
    {
        return reinterpret_cast<shm_hashmap< _key_type, _value_type, _hash_fun, _extract_key, _equal_key  >*>(
                   shm_hashtable< std::pair <_key_type, _value_type>, _key_type, _hash_fun, _extract_key, _equal_key>::initialize(numnode, pmmap, if_restore));
    }
    //[]�����������ŵ��ȱ�㣬
    _value_type &operator[](const _key_type &key)
    {
        return (find(key)).second;
    }
};

};

#endif //ZCE_LIB_SMEM_HASH_TABLE_H_

