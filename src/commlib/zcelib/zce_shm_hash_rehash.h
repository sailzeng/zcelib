/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_hash_rehash.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2010��04��6��
* @brief      ���HASH�����ͻ��HASHʵ�֣�ͬʱҲ�ǰ�ȫ��HASH,���Ŀ���������HASH
*             ӳ�乲���ڴ��ļ����Բ���COPY�ķ�ʽ����,
*             ����㷨�����ô�������ȫ����������ŵģ����Զ����ݽ���ֱ�ӵı��ݺͲ���,
*             �����Ҽ�����һ��SAFE��ʾ
*             HASHȡģ���ܴ����Ҳ���24��,Ϊʲô�����أ���Ϊ������һЩ���ԣ���HASH��
*             ������24������ʱ����һ�γ��ֳ�ͻ������ĸ����ʴ�Լ��85%����
*             http://blog.csdn.net/fullsail/article/details/6328702
*
* @details    ����㷨�Ļ���˼·���Լ�ͨ�ļ���ͬ��,��лicezhuang��mikewei��
*             spraydong��ͬ�£�����������һ������HASH�ķ�ʽ�����ͻ���㷨����Ϊ�˱�
*             ���鷳������Ȼ������ӵ��100%�İ�Ȩ������������������ģ�壬������������
*             ������з�װ, ���������˶�̬HASH����������ѡ��Ĭ��ֵ����ȣ�
*             �����HASH�����ͻ��˼·�ڡ����ݽṹ����ε��������Ҳ���漰��ֻ�ǵ�����ȫ
*             ���������
*             ����㷨�Ĳ���֮�����ԣ��ڼ�������£�����ܳ����޷����ص������
*             ���Ǽ�ͨ��ͬ��˵���ǲ��ԣ������㷨�ĸ��������ܴﵽ95%,������������Ǳ�
*             ֤��80%�ĸ���������ʱ���������,�����ǵ��㷨��32�ף�
*             ����Թ���һ�£�����㷨���ֳ�ͻ�����Ӧ���Ƿǳ��͵�
*             ����Ҫע����ǣ�������˲�����д���������ݣ������ǻ���ֿ���������ֻ�а�
*             �ص�����,����޷�����,������������û�����������������ݻ��ǿ���ʹ�õģ���
*             ���Ǻô�,
* @note       �����һЩ��ʷ��
*             1.Date  : 2010��04��12��
*             Author  : Sailzeng
*             Modification  : ���뼶����ɣ���ʼ����,
*
*             1.Date  : 2010��04��12��
*             Author  : Sailzeng
*             Modification  : ���ܲ��Խ���Ա�HASH�Ŀ�����ʽ�Ͷ��HASH��ʽ,���
*             HASH������Ҫ�����٣���1s�����ǧ��β�ѯ���ǿ��Ե�,
*
*             2.Date  : 2010��04��20��
*             Author  : Sailzeng
*             Modification  :�������������һ����̭�㷨�ģ���Ϊ˼ǰ��󣬳����ڴ�
*             ��ļ���ȡģ�Ĺ����н�����̭���������岻��,������kliuͬѧ��ս������˼
*             ����һ�£���ʹ���е�������Լ1�ڸ���ȫ��ɨ��Ҫ��̭�����ݵ�ʱ��Ҳ�ò���
*             1s���������ô�ں��ɱ��𣿼�Ȼ����ս���Ҿ��ṩһ���ɡ�����Ҳ���ֵ�����
*             ������,���⣬Ϊʲô�Ҳ�����¯����,��Ϊ�����Ͷ�������,
*
*             2.Date  : 2010��04��21��
*             Author  : Sailzeng
*             Modification  :˼���������һ��ǽ�_equal��ɾ���ˣ���ΪREHASH����
*             insert_equal֧�ֲ����ã�Ϊʲô�أ��������֧����ȵĶ������������
*             ���б�ĳ���,�ṩ������������������к�YY��������һ��Ҫ�ã����ȸ���
*             ����������Լ��Ĵ������ǻ����ص���ʵ��ʹ�ģ������Ĵ��뱻ɾ���ˣ�����ª
*             �Ķ���������Ҳ���ǶԵ���
*/

#ifndef ZCE_SHM_HASH_TABLE_REHASH_H_
#define ZCE_SHM_HASH_TABLE_REHASH_H_

#include "zce_shm_predefine.h"

namespace zce
{

//Ϊ�˷�����룬Ԥ�ȶ���һ��
template < class _value_type,
           class _key_type,
           class _hash_fun,
           class _extract_key,
           class _equal_key,
           class _washout_fun > class shm_hash_rehash;

//SAFE HASH ��������ע�������SAFE HASH������SAFE iterator���Ұ�HASH��ǰ�洿���Ƿ��㼸���ļ�
//����һ��
template < class _value_type,
           class _key_type,
           class _hashfun,
           class _extract_key,
           class _equal_key,
           class _washout_fun >
class _hash_rehash_iterator
{
protected:

    //HASH TABLE�Ķ���
    typedef shm_hash_rehash < _value_type,
            _key_type,
            _hashfun,
            _extract_key,
            _equal_key,
            _washout_fun > _hashtable_rehash;

    //���������
    typedef _hash_rehash_iterator < _value_type,
            _key_type,
            _hashfun,
            _extract_key,
            _equal_key,
            _washout_fun > iterator;

protected:
    //���к�
    size_t                 serial_;
    //
    _hashtable_rehash     *ht_rehash_instance_;

public:
    _hash_rehash_iterator():
        serial_(_shm_memory_base::_INVALID_POINT),
        ht_rehash_instance_(NULL)
    {
    }

    _hash_rehash_iterator(size_t serial, _hashtable_rehash *ht_safe_inst):
        serial_(serial),
        ht_rehash_instance_(ht_safe_inst)
    {
    }

    ~_hash_rehash_iterator()
    {
    }

    _value_type &operator*() const
    {
        return ht_rehash_instance_->value_base_ [serial_];
    }

    _value_type *operator->() const
    {
        return ht_rehash_instance_->value_base_ + serial_;
    }

    //����ֻ�ṩǰ���������������Ϊʹ�ÿ���ʹ��LIST��֤�����ĸ�Ч�����ֲ��У�
    //����Ҫ�ṩ����ĺ���
    //ǰ�������
    iterator &operator++()
    {
        _extract_key get_key;
        _equal_key   equal_key;
        size_t old_serial = serial_ + 1;

        for (; old_serial < ht_rehash_instance_->hash_safe_head_->num_of_node_ ; ++old_serial)
        {
            //�������Чֵ���󣬶�����Ԫ�ؽ��и�ֵ��Ϊʲôʹ��placement new����Ϊ��������һ��
            //�������һ����Чֵ
            if (false == equal_key( get_key (*(ht_rehash_instance_->value_base_ + old_serial)),
                                    get_key(ht_rehash_instance_->invalid_data_)) )
            {
                serial_ = old_serial;
                break;
            }
        }

        //���û�з�������
        if (old_serial == ht_rehash_instance_->hash_safe_head_->num_of_node_)
        {
            serial_ = _shm_memory_base::_INVALID_POINT;
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
        if (ht_rehash_instance_ == it.ht_rehash_instance_ &&
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

//===================================================================================================
//ÿ����16,24,32��
//Ŀǰʹ��24�У���Խ�࣬��ͻ���ܻ�ԽС
static const size_t DEF_PRIMES_LIST_NUM = 24;

static const size_t MIN_PRIMES_LIST_NUM = 4;

static const size_t MAX_PRIMES_LIST_NUM = 64;


//ͷ����LRU_HASH��ͷ���ṹ������LRUHASH�ڴ��ǰ��
class _hashtable_rehash_head
{
protected:
    //������������
    _hashtable_rehash_head():
        size_of_mmap_(0),
        num_of_node_(0),
        sz_freenode_(0),
        sz_usenode_(0),
        row_primes_ary_(0),
        expire_start_(0)
    {
        //��0
        memset(primes_ary_, 0, sizeof(primes_ary_));
    }
    ~_hashtable_rehash_head()
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

    //�����б��������
    size_t           row_primes_ary_;

    //
    size_t           primes_ary_[MAX_PRIMES_LIST_NUM];

    //��̭��ʵ��λ�ã�Ϊ�˱���ÿ����̭����һ���ط���ʼ
    size_t           expire_start_;

};



//=============================================================================================


template < class _value_type,
           class _key_type,
           class _hash_fun = smem_hash<_key_type>,
           class _extract_key = smem_identity<_value_type>,
           class _equal_key = std::equal_to<_key_type>,
           class _washout_fun = _default_washout_fun<_value_type> >
class shm_hash_rehash : public  _shm_memory_base
{
public:

    //���������
    typedef _hash_rehash_iterator < _value_type,
            _key_type,
            _hash_fun,
            _extract_key,
            _equal_key,
            _washout_fun > iterator;

    //�����Լ�
    typedef shm_hash_rehash < _value_type,
            _key_type,
            _hash_fun,
            _extract_key,
            _equal_key,
            _washout_fun > self;

    //��������������Ԫ
    friend class _hash_rehash_iterator < _value_type,
               _key_type,
               _hash_fun,
               _extract_key,
               _equal_key,
               _washout_fun >;

protected:

    //��Ч�����ݣ�����������������KEY����Զ������ֵģ�һ���Ǳ���0������-1
    //����ط���ʵ��͵���ı��֣����ڼ򻯴���ĳ���ռ䣬�Ƿ���Ч
    _value_type              invalid_data_;

    //���ڹ����ڴ�ͷ����ָ��
    _hashtable_rehash_head  *hash_safe_head_;

    //������ָ��
    _value_type             *value_base_;

    //���ȼ�������ָ��,��32λ�����ݱ������ȼ�
    unsigned int            *priority_base_;

protected:

    shm_hash_rehash():
        hash_safe_head_(NULL),
        value_base_(NULL),
        priority_base_(NULL)
    {
    }

public:

    ~shm_hash_rehash()
    {
    }

protected:

    //��value��ȡֵ
    size_t bkt_num_value(const _value_type &obj, size_t one_primes) const
    {
        _extract_key get_key;
        return static_cast<size_t>(bkt_num_key(get_key(obj), one_primes));
    }

    //Ϊʲô������������ĺ���,�Լ�����һ��,
    //���صĻ������_value_type��_key_typeһ�����͵��ſް� ---inmore
    size_t bkt_num_key(const _key_type &key, size_t one_primes) const
    {
        _hash_fun hash_fun;
        return static_cast<size_t>(hash_fun(key) % one_primes);
    }

    //ȡ����Ч���ݣ��ڵ�����ʹ�õ�ʱ����Ҫ
    _value_type get_invalid_data()
    {
        return invalid_data_;
    }

    //�����Լ����ڲ��ĳ�ʼ������
    static self  *initialize_i(size_t row_number,
                               const size_t primes_list[],
                               size_t num_node,
                               size_t sz_alloc,
                               char *pmmap,
                               const _value_type &invalid_data,
                               bool if_expire,
                               bool if_restore)
    {

        self *instance = new shm_hash_rehash < _value_type,
        _key_type,
        _hash_fun,
        _extract_key,
        _equal_key > ();

        instance->smem_base_ = pmmap;
        char *tmp_base = instance->smem_base_;
        instance->hash_safe_head_ = reinterpret_cast<_hashtable_rehash_head *>(tmp_base);
        tmp_base = tmp_base + sizeof(_hashtable_rehash_head);

        instance->value_base_ = reinterpret_cast<_value_type *>(tmp_base);
        tmp_base = tmp_base + (sizeof(_value_type) * (num_node));

        //��ʼ���ߴ�,����ǻָ����ղ��Ѿ��ȽϹ���
        instance->hash_safe_head_->size_of_mmap_ = sz_alloc;
        instance->hash_safe_head_->num_of_node_ = num_node;

        //ÿ�ζ�������̭��ʼ�ĵط���¼Ϊ0
        instance->hash_safe_head_->expire_start_ = 0;

        if (if_expire)
        {
            instance->priority_base_ = reinterpret_cast<unsigned int *>(tmp_base);
            tmp_base = tmp_base + (sizeof(unsigned int) * (num_node));
        }
        else
        {
            //����ǿ��һ�γ�NULL
            instance->priority_base_ = NULL;
        }

        instance->invalid_data_ = invalid_data;

        //��������д��ȥ
        instance->hash_safe_head_->row_primes_ary_ = row_number;

        for (size_t y = 0; y < row_number; ++y)
        {
            instance->hash_safe_head_->primes_ary_[y] = primes_list[y];
        }
        //�Ѷ���ĵط���д��0
        for (size_t y = row_number; y < MAX_PRIMES_LIST_NUM; ++y)
        {
            instance->hash_safe_head_->primes_ary_[y] = 0;
        }

        //��������лָ�����
        if (if_restore == false)
        {
            //�����ʼ�����е��ڴ�,�еĽڵ�ΪFREE
            instance->clear(if_expire);
        }

        return instance;
    }

public:


    /*!
    * @brief      ������Ҫ����NODE������������Ҫ����Ŀռ��С���Լ���Ӧ������������Ϣ
    * @return     size_t ����ֵΪ��Ҫ����Ŀռ��С
    * @param[in]  req_num   ��ʾ����Ҫ�Ľڵ������
    * @param[out] real_num  ���ʵ�ʷ���Ľڵ�������ע�ⷵ�ص���ʵ��INDEX����,������һЩ
    * @param[out] prime_ary ���صģ��������飬���ڶ��REHASH����
    * @param[in]  if_expire �Ƿ�ʹ�ó�ʱ����
    * @param[in]  row_prime_ary ������ֵ��������,�������һ���ط�����Ϊ�˷���Ĭ��ֵ
    * @note       �ڴ����Ĺ���Ϊ define��,index��,data��,��������Ҫ�ĳ���,
    */
    static size_t getallocsize(size_t req_num,
                               size_t &real_num,
                               size_t prime_ary[],
                               bool if_expire,
                               size_t row_prime_ary = DEF_PRIMES_LIST_NUM)
    {
        ZCE_ASSERT(row_prime_ary >= MIN_PRIMES_LIST_NUM && row_prime_ary <= MAX_PRIMES_LIST_NUM);

        zce::hash_prime_ary(req_num, real_num, row_prime_ary, prime_ary);

        size_t sz_alloc =  0;
        //
        sz_alloc += sizeof(_hashtable_rehash_head);

        //�����ṹ������
        sz_alloc += sizeof(_value_type) * (real_num);

        if (if_expire)
        {
            sz_alloc += (sizeof(unsigned int) * (real_num));
        }

        return sz_alloc;
    }


    /*!
    * @brief      ����ֵΪ��Ҫ����Ŀռ��С������������������У�����������õ��ռ�,
    * @return     size_t         ����ֵΪ��Ҫ����Ŀռ��С
    * @param[in]  row_prime_ary  �����������г���,
    * @param[in]  primes_list    �����������У�
    * @param[in]  if_expire      �Ƿ�ʱ����
    * @param[out] node_count     ���ز���,�ܽڵ����,
    * @note
    */
    static size_t getallocsize(size_t row_prime_ary,
                               const size_t primes_list[],
                               bool if_expire,
                               size_t &node_count)
    {
        //�б����󳤶Ȳ��ܴ���MAX_PRIMES_LIST_ELEMENT
        ZCE_ASSERT(row_prime_ary <= DEF_PRIMES_LIST_NUM);

        //����������
        node_count = 0;

        for (size_t i = 0; i < row_prime_ary; ++i)
        {
            node_count += primes_list[i];
        }

        ZCE_ASSERT(node_count > 0);

        //
        size_t sz_alloc =  0;

        //����ռ䣬����ͷ����������,�ṹ���������ȼ��ռ�
        sz_alloc += sizeof(_hashtable_rehash_head);

        sz_alloc += sizeof(_value_type) * (node_count);

        if (if_expire)
        {
            sz_alloc += sizeof(unsigned int) * (node_count);
        }

        return sz_alloc;
    }


    /*!
    * @brief      ��ʼ�������غ��������ָ�룬�Ժ��ͨ�����ָ�������Ϊʲô��ֱ���ù��캯���أ��Һ��ѻش𣬿�����ͨ��getallocsizeһ����У�
    * @return     shm_hash_rehash < _value_type, _key_type, _hash_fun, _extract_key, _equal_key >*
    * @param      req_num        ��ʾ����Ҫ�Ľڵ����������Ҫ�ż���Ԫ�ظ��㣬
    * @param      real_num       ע����������᷵��һ��ʵ���ҷ�����ٳߴ����
    * @param      pmmap          ���ݽ����Ŀռ�ָ�룬�ռ�Ĵ�Сͨ��getallocsize�õ�.
    * @param      invalid_data   һ����Ч����������ֵ����Ϊ�����ø��㿪��һ���ط�
    *                            ��¼ĳ�������Ƿ�ʹ����.�����һὫ���е����ݶ���ʼ
    *                            ����Ч�ṹ����Ч�ṹ���һ���������ռ�û��ʹ��
    * @param      if_expire      �Ƿ�Ҫʹ����̭����,��������ã��ռ���Ը���СһЩ
    * @param      if_restore     �Ƿ��Ǵ�һ���ڴ��лָ��ռ䣬���繲���ڴ�֮��Ļָ�
    * @note       �Ƽ�ʹ���������,����������Ҫ�ٺܶ�
    */
    static self *initialize(size_t req_num,
                            size_t &real_num,
                            char *pmmap,
                            const _value_type &invalid_data,
                            bool if_expire,
                            size_t row_prime_ary = DEF_PRIMES_LIST_NUM,
                            bool if_restore = false)
    {
        ZCE_ASSERT(pmmap != NULL && req_num > 0);

        //����,������ĳߴ磬������һ�����ʵĿռ�
        size_t prime_ary[MAX_PRIMES_LIST_NUM];
        size_t sz_alloc = getallocsize(req_num, real_num, prime_ary, if_expire, row_prime_ary);

        _hashtable_rehash_head *hashhead =  reinterpret_cast< _hashtable_rehash_head * >(pmmap);

        //����ǻָ�,���ݶ����ڴ���,�����ݽ��м��
        if (if_restore == true)
        {
            //�Ƿ�Ҫ�������������ڵı��Ƿ�һ����?����ʱѡ���˽��м�飬
            //�������������������ʹ��������ľ���ı��ˣ����ǿ����ù����ڴ��м�����ݼ����ܣ���ѡ�����Ͻ���������

            //���һ�»ָ����ڴ��Ƿ���ȷ,
            if ( sz_alloc != hashhead->size_of_mmap_ ||
                 req_num != hashhead->num_of_node_ )
            {
                return NULL;
            }

            //�����б�ĸ���Ӧ��һ��
            if (hashhead->row_primes_ary_ != row_prime_ary)
            {
                return NULL;
            }

            //HASH�б�ĸ�����Ӧ�õ���Ԫ�ص��ܺͣ��ǲ���Ӧ�ý�ÿ�����ݶ��ó����Ƚ�һ����
            size_t num_node_count = 0;

            for (size_t p = 0; p < hashhead->row_primes_ary_; ++p)
            {
                num_node_count += hashhead->primes_ary_[p];
            }

            if (num_node_count != hashhead->num_of_node_)
            {
                return NULL;
            }

            //������������Ƿ�һ��
            for (size_t y = 0; y < row_prime_ary; ++y)
            {
                if (hashhead->primes_ary_[y] != prime_ary[y])
                {
                    return NULL;
                }
            }

        }

        //�����չ�
        return initialize_i(row_prime_ary,
                            prime_ary,
                            req_num,
                            sz_alloc,
                            pmmap,
                            invalid_data,
                            if_expire,
                            if_restore);

    }

    //�����ʼ�����е��ڴ�,���еĽڵ�ΪFREE
    void clear(bool if_expire)
    {
        //����ؼ�Node,�Լ���س���,��ʼ���е�������free.
        hash_safe_head_->sz_freenode_ = hash_safe_head_->num_of_node_;
        hash_safe_head_->sz_usenode_ = 0;

        //��ʼ��free�������������е����ݳ�ʼ������Чֵ�����ڱ�ʾһ����¼�Ƿ�������
        for (size_t i = 0; i < hash_safe_head_->num_of_node_ ; ++i )
        {
            //�������Чֵ���󣬶�����Ԫ�ؽ��и�ֵ��Ϊʲôʹ��placement new����Ϊ��������һ��
            new (value_base_ + i)_value_type(invalid_data_);
        }

        //���Ҫ��¼��̭��Ϣ
        if (if_expire)
        {
            memset(priority_base_, 0, (sizeof(unsigned int) * (hash_safe_head_->num_of_node_)));
        }
    }

    //��Ҳ���Դ���һ���������У���Ϊ���ж���HASHȡģ����������,
    static self *initialize(size_t primes_number,
                            size_t primes_list[],
                            char *pmmap,
                            const _value_type &invalid_data,
                            bool if_expire,
                            bool if_restore = false)
    {
        assert(pmmap != NULL );

        _hashtable_rehash_head *hashhead =  reinterpret_cast< _hashtable_rehash_head * >(pmmap);

        size_t node_count = 0;
        size_t sz_alloc = getallocsize(primes_number, primes_list, if_expire, node_count);

        //����ǻָ�,���ݶ����ڴ���,�����ݽ��м��
        if (if_restore == true)
        {
            //�Ƿ�Ҫ�������������ڵı��Ƿ�һ����?����ʱѡ���˽��м�飬
            //�������������������ʹ��������ľ���ı��ˣ����ǿ����ù����ڴ��м�����ݼ����ܣ���ѡ�����Ͻ���������

            //���һ�»ָ����ڴ��Ƿ���ȷ,
            if (sz_alloc = hashhead->size_of_mmap_ ||
                           node_count != hashhead->num_of_node_ )
            {
                return NULL;
            }

            //�����б�ĸ���Ӧ��һ��
            if ( hashhead->row_primes_ary_ != primes_number )
            {
                return NULL;
            }

            //������������Ƿ�һ��
            for (size_t y = 0; y < primes_number; ++y)
            {
                if (hashhead->primes_ary_[y] != primes_list[y])
                {
                    return NULL;
                }
            }
        }

        //�����չ�
        return initialize_i(primes_number,
                            primes_list,
                            node_count,
                            sz_alloc,
                            pmmap,
                            invalid_data,
                            if_expire,
                            if_restore);
    }

public:

    //�õ���ʼ�ĵ�������λ�ã���ʵ������Ч������ѽ�����ǣ�
    iterator begin()
    {
        _extract_key get_key;
        _equal_key   equal_key;

        //�ҵ���һ���Ѿ���д���ݵĶ�����Ϊbegin
        for (size_t i = 0; i < hash_safe_head_->num_of_node_; ++i)
        {
            //�������һ����Чֵ
            if (false == equal_key(get_key(*(value_base_ + i)), get_key(invalid_data_)) )
            {
                return iterator( i, this);
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
        return hash_safe_head_->sz_usenode_;
    }
    //�õ�����
    size_t capacity() const
    {
        return hash_safe_head_->num_of_node_;
    }
    //�Ƿ�Ϊ��
    bool empty() const
    {
        return (hash_safe_head_->sz_freenode_ == hash_safe_head_->num_of_node_);
    }
    //�Ƿ�ռ��Ѿ�����
    bool full() const
    {
        return (hash_safe_head_->sz_freenode_ == 0);
    }

    //����ڵ�
    std::pair<iterator, bool> insert(const _value_type &val )
    {
        //ʹ�ú�������,һ���൥������һ���Ƿ����?
        _extract_key get_key;
        _equal_key   equal_key;

        size_t idx_count = 0;
        size_t idx_no_use = _INVALID_POINT;

        //ѭ������N��ȡģ������
        for (size_t i = 0; i < hash_safe_head_->row_primes_ary_; ++i )
        {
            size_t idx = bkt_num_value(val, hash_safe_head_->primes_ary_[i]);
            idx_count += idx;

            //����ҵ���ͬ��Key����,������������岻������
            if (equal_key((get_key(value_base_[idx_count])), (get_key(val))) )
            {
                return std::pair<iterator, bool>(iterator(idx_count, this), false);
            }

            //�����һ����Ч���ݣ���ʾһ����λ��
            if (equal_key((get_key(value_base_[idx_count])), (get_key(invalid_data_))) == true)
            {
                if (idx_no_use == _INVALID_POINT)
                {
                    idx_no_use = idx_count;
                    break;
                }
            }

            //׼������һ��������������
            idx_count += (hash_safe_head_->primes_ary_[i] - idx);
        }

        //���ÿһ�ж�Ӧ��λ�ö�����åռ����,����һ������ĵ�����end,����ǰ�Σ��ռ�Σ����
        if (_INVALID_POINT == idx_no_use )
        {
            return std::pair<iterator, bool>(end(), false);
        }

        //ʹ��placement new���и�ֵ
        new (value_base_ + idx_no_use)_value_type(val);

        ++(hash_safe_head_->sz_usenode_);
        --(hash_safe_head_->sz_freenode_);

        return std::pair<iterator, bool>(iterator(idx_no_use, this), true);
    }

    //�����ȼ��Ĳ��룬��ʼ��ʼ����ʱ�����if_expire == true
    //@const _value_type &val ���������
    //@unsigned int priority  �����������ȼ���
    //@unsigned int expire_priority = static_cast<unsigned int>(-1)����̭�����ȼ���Ĭ��Ϊ���ֵ�������в���̭�������������djiang�ĺý���
    std::pair<iterator, bool> insert(const _value_type &val,
                                     unsigned int priority,
                                     unsigned int expire_priority = 0)
    {

        //ʹ�ú�������,һ���൥������һ���Ƿ����?
        _extract_key get_key;
        _equal_key   equal_key;

        size_t idx_count = 0;
        size_t idx_no_use = _INVALID_POINT;

        //ѭ������N��ȡģ������
        for (size_t i = 0; i < hash_safe_head_->row_primes_ary_; ++i )
        {
            size_t idx = bkt_num_value(val, hash_safe_head_->primes_ary_[i]);
            idx_count += idx;

            //����ҵ���ͬ��Key����,������������岻������
            if (equal_key((get_key(value_base_[idx_count])), (get_key(val))) )
            {
                return std::pair<iterator, bool>(iterator(idx_count, this), false);
            }

            //�����һ����Ч���ݣ���ʾһ����λ��
            if (equal_key((get_key(value_base_[idx_count])), (get_key(invalid_data_))) == true)
            {
                if (idx_no_use == _INVALID_POINT)
                {
                    idx_no_use = idx_count;
                    break;
                }
            }
            //������λ���Ѿ������ݣ����Լ��һ����������Ƿ����
            else
            {
                //����Ǹ���djiang�Ľ�����е�һ�����书�ܣ��ڲ����ʱ�����̭�����ڵ����ݣ�
                //������ȼ�ʱ���Ѿ����ڣ��Ϳ���ֱ���������λ��
                if (priority_base_[idx_count] <= expire_priority)
                {
                    idx_no_use = idx_count;
                    break;
                }
            }

            //׼������һ��������������
            idx_count += (hash_safe_head_->primes_ary_[i] - idx);
        }

        //���ÿһ�ж�Ӧ��λ�ö�����åռ����,����һ������ĵ�����end,����ǰ�Σ��ռ�Σ����
        if (_INVALID_POINT == idx_no_use )
        {
            return std::pair<iterator, bool>(end(), false);
        }

        //ʹ��placement new���и�ֵ
        new (value_base_ + idx_no_use)_value_type(val);

        ++(hash_safe_head_->sz_usenode_);
        --(hash_safe_head_->sz_freenode_);

        //�������ɹ��˼�¼���ȼ�
        priority_base_[idx_no_use] = priority;

        return std::pair<iterator, bool>(iterator(idx_no_use, this), true);
    }

    //��ѯ��Ӧ��Key�Ƿ���,���ص�����
    //����ط���һ������,����ط����صĵ�����++�����ܸ����ҵ���ͬ��key������,��������HASHʵ�����������
    iterator find(const _key_type &key)
    {
        //ʹ������������,һ���൥������һ���Ƿ����?
        _extract_key get_key;
        _equal_key   equal_key;

        size_t idx_count = 0;

        //ѭ������N��ȡģ������
        for (size_t i = 0; i < hash_safe_head_->row_primes_ary_; ++i )
        {
            //��valȡ��key��ȡģ
            size_t idx = bkt_num_key(key, hash_safe_head_->primes_ary_[i]);
            idx_count += idx;

            //����ҵ���ͬ��Key����,������������岻������
            if (equal_key((get_key(value_base_[idx_count])), key) )
            {
                return iterator(idx_count, this);
            }

            //׼������һ��������������
            idx_count += (hash_safe_head_->primes_ary_[i] - idx);
        }

        return end();
    }

    //
    iterator find_value(const _value_type &val)
    {
        _extract_key get_key;
        return find(get_key(val));
    }

    bool erase(const _key_type &key)
    {
        iterator iter_temp = find(key);

        if (iter_temp == end())
        {
            return false;
        }

        return erase(iter_temp);
    }

    //ʹ�õ�����ɾ��,������Ч���Բ��ü�д��
    bool erase(const iterator &it_del)
    {
        //�Ҳ�������ļ��
        if (it_del != end())
        {
            //������������,ע���������ֻ��ͨ��ָ����У�����int��֮�������
            (value_base_ + it_del.getserial())->~_value_type();
            //�ָ�����Ч����
            new (value_base_ + it_del.getserial() )_value_type(invalid_data_);
            --(hash_safe_head_->sz_usenode_);
            ++(hash_safe_head_->sz_freenode_);

            return true;
        }

        return false;
    }

    //ɾ��ĳ��ֵ
    bool erase_value(const _value_type &val )
    {
        _extract_key get_key;
        return erase( get_key(val));
    }

    //����,����������ݹҵ�LIST���ʼ,��̭ʹ��expire,disuse
    //���ȼ���������ʹ�õ�ǰ��ʱ��
    bool active(const _key_type &key,
                unsigned int priority /*=static_cast<unsigned int>(time(NULL))*/ )
    {
        iterator  iter_tmp = find(key);

        //����ҵ��˶�Ӧ��KEY
        if ( end()  != iter_tmp )
        {
            priority_base_[iter_tmp.getserial()] = priority;
            return true;
        }

        return false;
    }

    //
    bool active_value(const _value_type &val,
                      unsigned int priority /*=static_cast<unsigned int>(time(NULL))*/ )
    {
        _extract_key get_key;
        return active(get_key(val), priority);
    }

    //��̭���ڵ�����,����LIST�м�������ǰ��չ���ʵ������ģ���Ҫ���㴫������ȼ������ʱ��
    //С�ڵ���������ȼ������ݽ�����̭
    //hope_expire_num��ʾ��ϣ��ɾ�����ٸ�ֵ��Ĭ��Ϊ���ֵ,ȫ����̭
    size_t expire(unsigned int expire_time, size_t hope_expire_num = static_cast<size_t>(-1))
    {
        //��β����ʼ��飬
        size_t expire_num = 0;
        size_t i = 0;

        for (; i < hash_safe_head_->num_of_node_ && expire_num < hope_expire_num ; ++i)
        {
            size_t del_iter = ( hash_safe_head_->expire_start_ + i) % hash_safe_head_->num_of_node_;

            //С�ڵ���
            if (priority_base_[del_iter] <= expire_time)
            {

                _washout_fun wash_fun;
                wash_fun(value_base_[del_iter]);
                ++expire_num;
                //
                iterator iter_tmp(del_iter, this);
                erase(iter_tmp);
            }
        }

        //��һ������ĵط��Ͽ�ʼ
        hash_safe_head_->expire_start_ = ( hash_safe_head_->expire_start_ + i) % hash_safe_head_->num_of_node_;

        return expire_num;
    }

};

};

#endif //#ifndef ZCE_SHM_HASH_TABLE_REHASH_H_

