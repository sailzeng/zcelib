/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_avltree.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2006��1��16��
* @brief      ϣ��AVLTree��Ҫ����ɿ��������MAP,SET,��MMAP��
*
* @details
*
* @note       ������������д��������2014��3��8����Ҳû���������ʣ�
*             ��Ҫԭ���Ƕ�AVL��ɾ������һֱû�����Ŀ������Խ����һֱ�����ˡ�
*             07���ʱ��scottxu����������ǹƥ��Ѻ�����㶨�ˣ��������
*             ��һֱû��������֮�أ��Ǻǡ�Ҳ�������һ��ǻ�ʵ�ֵġ�
*
*             һ�������ڰ칫�ң������װ�ᡣ���а����Լ���2B��
*
*             ��ʼ���ʱ�����濴��Scott�ĺ������ʵ�֣����ֵ��ʻ����ԣ�
*             ����ֱ��������ģ�ͬʱ����������룬Ӧ�ÿ��Բο��ã�
*             ����֣�д�������ƵĻ���ɾ��node���룬�����ٷ���2��������
*             ��Ϊ�ܶ����ӵķ�������̫���ף�������ΰ�Ҫɾ���Ľڵ㽻����Ҷ��
*             �ڵ����з���ɵ��
*             ����AVL��ɾ���󣬽ڵ�ĵ���Ҳ��ֹ��4�ֱ任��
*             http://www.cnblogs.com/fullsail/p/3729015.html
*
*/

#ifndef ZCE_LIB_SHM_AVL_TREE_H_
#define ZCE_LIB_SHM_AVL_TREE_H_

#include "zce_shm_predefine.h"

namespace zce
{




template<class _value_type, class _key_type, class _extract_key, class _compare_key> class shm_avl_tree;

///AVL TREE��ͷ��������
class _shm_avl_tree_head
{
protected:
    _shm_avl_tree_head()
        : size_of_mmap_(0)
        , num_of_node_(0)
        , sz_free_node_(0)
        , sz_use_node_(0)
    {
    }
    ~_shm_avl_tree_head()
    {
    }

public:
    //�ڴ����ĳ���
    size_t               size_of_mmap_;
    //NODE������
    size_t               num_of_node_;
    //FREE��NODE����
    size_t               sz_free_node_;
    //USE��NODE����
    size_t               sz_use_node_;
};


//AVL tree�������Ľڵ�
class _shm_avl_tree_index
{

public:
    _shm_avl_tree_index()
        : parent_(_shm_memory_base::_INVALID_POINT)
        , left_(_shm_memory_base::_INVALID_POINT)
        , right_(_shm_memory_base::_INVALID_POINT)
        , balanced_(0)
    {
    }

    _shm_avl_tree_index(const size_t &p, const size_t &l, const size_t &r, int8_t hb)
        : parent_(p)
        , left_(l)
        , right_(r)
        , balanced_(hb)
    {
    }

    ~_shm_avl_tree_index()
    {
    }

public:
    ///���ڵ�
    size_t       parent_;
    ///������
    size_t       left_;
    ///������
    size_t       right_;
    ///ƽ��
    int32_t      balanced_;
};


//AVL tree�ĵ�����
template < class _value_type,
           class _key_type,
           class _extract_key,
           class _compare_key >
class _shm_avl_tree_iterator
{
    typedef _shm_avl_tree_iterator<_value_type, _key_type, _extract_key, _compare_key> iterator;

    typedef shm_avl_tree<_value_type, _key_type, _extract_key, _compare_key> shm_avl_tree_t;


    //��������ȡ�����еĶ���
    typedef ptrdiff_t difference_type;
    typedef _value_type *pointer;
    typedef _value_type &reference;
    typedef _value_type value_type;
    typedef std::bidirectional_iterator_tag iterator_category;

public:
    //���캯��
    _shm_avl_tree_iterator(size_t seq, shm_avl_tree_t *instance)
        : serial_(seq)
        , avl_tree_inst_(instance)
    {
    }

    _shm_avl_tree_iterator()
        : serial_(_shm_memory_base::_INVALID_POINT),
          avl_tree_inst_(NULL)
    {
    }

    ~_shm_avl_tree_iterator()
    {
    }

    //��ʼ��
    void initialize(size_t seq, shm_avl_tree_t *instance)
    {
        serial_ = seq;
        avl_tree_inst_ = instance;
    }

    //������žͿ����ٸ���ģ��ʵ���������ҵ���Ӧ����,����ʹ��ָ��
    size_t getserial() const
    {
        return serial_;
    }

    bool operator==(const iterator &x) const
    {
        return (serial_ == x.serial_ && avl_tree_inst_ == x.avl_tree_inst_);
    }
    bool operator!=(const iterator &x) const
    {
        return !(*this == x);
    }

    _value_type &operator*() const
    {
        return *(operator->());
    }

    //�ڶ��̵߳Ļ������ṩ������ͷ����ǲ���ȫ��,û�м���,�ϲ��Լ���֤
    _value_type *operator->() const
    {
        return avl_tree_inst_->getdatabase() + serial_;
    }

    iterator &operator++()
    {
        increment();
        return *this;
    }
    iterator operator++(int)
    {
        iterator tmp = *this;
        increment();
        return tmp;
    }

    iterator &operator--()
    {
        decrement();
        return *this;
    }
    iterator operator--(int)
    {
        iterator tmp = *this;
        decrement();
        return tmp;
    }

    ///����ʵ��operator++������һ�����Լ���(�ȽϺ�������)�Ľڵ�
    void increment()
    {
        if ((avl_tree_inst_->index_base_ + serial_)->right_ != _shm_memory_base::_INVALID_POINT)
        {
            //��������ӽڵ㣬�������ߣ�Ȼ��һֱ���������ߵ��׼���
            serial_ = (avl_tree_inst_->index_base_ + serial_)->right_;

            while ((avl_tree_inst_->index_base_ + serial_)->left_ != _shm_memory_base::_INVALID_POINT)
            {
                serial_ = (avl_tree_inst_->index_base_ + serial_)->left_;
            }
        }
        else
        {
            //���û�����ӽڵ㣬�ҵ����ڵ㣬�����ǰ�ڵ���ĳ�����ӽڵ㣬��һֱ���ݵ���Ϊ���ӽڵ�Ϊֹ
            size_t y = (avl_tree_inst_->index_base_ + serial_)->parent_;

            while (serial_ == (avl_tree_inst_->index_base_ + y)->right_)
            {
                serial_ = y;
                y = (avl_tree_inst_->index_base_ + y)->parent_;
            }

            //����ʱ�����ӽڵ㲻���ڸ��ڵ㣬�򸸽ڵ㼴�ǣ�������ǵ�ǰ�ڵ�
            if ((avl_tree_inst_->index_base_ + serial_)->right_ != y)
            {
                serial_ = y;
            }
        }
    }

    //����ʵ��operator--������һ�����Լ�С(�ȽϺ�������)�Ľڵ�
    void decrement()
    {
        //�����END��serial_����head_index_���±꣬��ô��right_����endǰ����Ǹ�
        if (avl_tree_inst_->avl_tree_head_->num_of_node_ == serial_)
        {
            serial_ = (avl_tree_inst_->index_base_ + serial_)->right_;
        }
        //��������ӽڵ�
        else if ((avl_tree_inst_->index_base_ + serial_)->left_ != _shm_memory_base::_INVALID_POINT)
        {
            //��yָ�����ӽڵ㣬�ҵ�y�����ӽڵ㣬�����ߵ��׼���
            size_t y = (avl_tree_inst_->index_base_ + serial_)->left_;

            while ((avl_tree_inst_->index_base_ + y)->right_ != _shm_memory_base::_INVALID_POINT)
            {
                y = (avl_tree_inst_->index_base_ + y)->right_;
            }

            serial_ = y;
        }
        else
        {
            //�ҳ����ڵ㣬�����ǰ�ڵ��Ǹ����ӽڵ㣬��һֱ���ݣ�ֱ������Ϊ���ӽڵ㣬����ĸ��ڵ㼴��
            size_t y = (avl_tree_inst_->index_base_ + serial_)->parent_;

            while (serial_ == (avl_tree_inst_->index_base_ + y)->left_)
            {
                serial_ = y;
                y = (avl_tree_inst_->index_base_ + y)->parent_;
            }

            serial_ = y;
        }
    }


protected:
    //���к�
    size_t          serial_;
    //RBtree��ʵ��ָ��
    shm_avl_tree_t  *avl_tree_inst_;

};


/*!
* @brief      AVL Tree����������������Ĵ���ȣ�
*             AVL�ĸ�����Ĵ������ܶ���Ϊһ�㣬������룬ɾ���ĺ�ʱ���� O(LOG2N)
*             �ļ���
* @tparam     _value_type   ��������
* @tparam     _key_type     KEY������
* @tparam     _extract_key  �����_value_type�л�ȡ_key_type�ķ���
* @tparam     _compare_key  �ȽϷ���
*/
template < class _value_type,
           class _key_type,
           class _extract_key = smem_identity<_value_type>,
           class _compare_key = std::less<_key_type> >
class shm_avl_tree : public _shm_memory_base
{
public:
    //�����Լ�
    typedef shm_avl_tree < _value_type,
            _key_type,
            _extract_key,
            _compare_key > self;

    //���������
    typedef _shm_avl_tree_iterator < _value_type,
            _key_type,
            _extract_key,
            _compare_key > iterator;

    //��������Ԫ
    friend class _shm_avl_tree_iterator<_value_type, _key_type, _extract_key, _compare_key>;



public:

    //����ڹ����ڴ�ʹ��,û��new,����ͳһ��initialize ��ʼ��
    //�������,��������,���ǲ�������
    shm_avl_tree<_value_type, _key_type, _extract_key, _compare_key >(size_t numnode, void *pmmap, bool if_restore)
        : _shm_memory_base(NULL)
        , index_base_(NULL)
        , data_base_(NULL)
    {
    }

    shm_avl_tree<_value_type, _key_type, _extract_key, _compare_key >()
        : _shm_memory_base(NULL)
    {
    }

    ~shm_avl_tree<_value_type, _key_type, _extract_key, _compare_key >()
    {
    }

    //ֻ����,��ʵ��,���ⷸ��
    const self &operator=(const self &others);

    //�õ������Ļ�����ַ
    inline _shm_avl_tree_index *getindexbase()
    {
        return index_base_;
    }

    //�õ��������Ļ�������
    inline  _value_type *getdatabase()
    {
        return data_base_;
    }

protected:
    //����һ��NODE,�����FREELIST��ȡ��
    size_t create_node(const _value_type &val)
    {
        //���û�пռ���Է���
        if (avl_tree_head_->sz_free_node_ == 0)
        {
            return _INVALID_POINT;
        }

        //������ȡ1������
        size_t new_node = free_index_->right_;
        free_index_->right_ = (index_base_ + new_node)->right_;
        avl_tree_head_->sz_free_node_--;
        avl_tree_head_->sz_use_node_++;

        //��ʼ��
        (index_base_ + new_node)->parent_ = _INVALID_POINT;
        (index_base_ + new_node)->left_ = _INVALID_POINT;
        (index_base_ + new_node)->right_ = _INVALID_POINT;
        (index_base_ + new_node)->balanced_ = 0;

        new (data_base_ + new_node)_value_type(val);

        return new_node;
    }

    //�ͷ�һ��NODE,����黹��FREELIST
    void destroy_node(size_t pos)
    {
        size_t freenext = free_index_->right_;

        (index_base_ + pos)->right_ = freenext;
        free_index_->right_ = pos;

        (index_base_ + pos)->left_ = _INVALID_POINT;
        (index_base_ + pos)->parent_ = _INVALID_POINT;
        (index_base_ + pos)->balanced_ = 0;

        avl_tree_head_->sz_free_node_++;
        avl_tree_head_->sz_use_node_--;

        //������ʽ����������
        (data_base_ + pos)->~_value_type();
    }

public:

    //�ڴ����Ĺ���Ϊ ͷ��������,index��,data��,��������Ҫ�ĳ���,
    static size_t getallocsize(const size_t numnode)
    {
        return  sizeof(_shm_avl_tree_head)
                + sizeof(_shm_avl_tree_index) * (numnode + ADDED_NUM_OF_INDEX)
                + sizeof(_value_type) * numnode;
    }

    //��ʼ��
    static self *initialize(const size_t numnode, char *pmmap, bool if_restore = false)
    {
        //assert(pmmap!=NULL && numnode >0 );
        _shm_avl_tree_head *avl_tree_head = reinterpret_cast<_shm_avl_tree_head *>(pmmap);

        //����ǻָ�,���ݶ����ڴ���,
        if (true == if_restore)
        {
            //���һ�»ָ����ڴ��Ƿ���ȷ,
            if (getallocsize(numnode) != avl_tree_head->size_of_mmap_ ||
                numnode != avl_tree_head->num_of_node_)
            {
                return NULL;
            }
        }

        //��ʼ���ߴ�
        avl_tree_head->size_of_mmap_ = getallocsize(numnode);
        avl_tree_head->num_of_node_ = numnode;

        self *instance = new self();

        //���е�ָ�붼�Ǹ��ӻ���ַ����õ���,���ڷ������,ÿ�γ�ʼ�������¼���
        instance->smem_base_ = pmmap;
        //ͷ��
        instance->avl_tree_head_ = avl_tree_head;
        //������
        instance->index_base_ = reinterpret_cast<_shm_avl_tree_index *>(
                                    pmmap +
                                    sizeof(_shm_avl_tree_head));
        //������
        instance->data_base_ = reinterpret_cast<_value_type *>(
                                   pmmap +
                                   sizeof(_shm_rb_tree_head) +
                                   sizeof(_shm_avl_tree_index) * (numnode + ADDED_NUM_OF_INDEX));

        //��ʼ��free_index_,head_index_
        instance->head_index_ = reinterpret_cast<_shm_avl_tree_index *>(
                                    pmmap +
                                    sizeof(_shm_avl_tree_head) +
                                    sizeof(_shm_avl_tree_index) * (numnode));
        instance->free_index_ = reinterpret_cast<_shm_avl_tree_index *>(
                                    pmmap +
                                    sizeof(_shm_avl_tree_head) +
                                    sizeof(_shm_avl_tree_index) * (numnode + 1));

        if (false == if_restore)
        {
            //�����ʼ�����е��ڴ�,���еĽڵ�ΪFREE
            instance->clear();
        }

        return instance;
    }

    //�����ʼ�����е��ڴ�,���еĽڵ�ΪFREE
    void clear()
    {
        //����2���ؼ�Node,�Լ���س���,��ʼ���е�������free.
        avl_tree_head_->sz_free_node_ = avl_tree_head_->num_of_node_;
        avl_tree_head_->sz_use_node_ = 0;

        //������ΪNULL,��ָ�붼ָ���Լ�
        head_index_->parent_ = _INVALID_POINT;
        head_index_->right_ = avl_tree_head_->num_of_node_;
        head_index_->left_ = avl_tree_head_->num_of_node_;
        head_index_->balanced_ = 0;

        //
        free_index_->left_ = _INVALID_POINT;
        free_index_->parent_ = _INVALID_POINT;
        free_index_->balanced_ = 0;

        //��right_������FREE NODE���б�
        free_index_->right_ = 0;

        //��ʼ��free������
        _shm_avl_tree_index *pindex = index_base_;
        for (size_t i = 0; i < avl_tree_head_->num_of_node_; ++i)
        {
            pindex->right_ = (i + 1);

            //������FREENODE������
            if (i == avl_tree_head_->num_of_node_ - 1)
            {
                pindex->right_ = avl_tree_head_->num_of_node_ + 1;
            }

            pindex++;
        }
    }

    //�ҵ���һ���ڵ�
    iterator begin()
    {
        return iterator(head_index_->left_, this);
    };

    //����Ӧ����ǰ�պ󿪵�,ͷ�ڵ���Ϊ���һ��index
    iterator end()
    {
        return iterator(avl_tree_head_->num_of_node_, this);
    }

    //���нڵ㶼��free���ϼ��ǿ�
    bool empty()
    {
        if (avl_tree_head_->sz_free_node_ == avl_tree_head_->num_of_node_)
        {
            return true;
        }

        return false;
    }

    //�ڲ�������ǰ����,����������
    bool full()
    {
        if (avl_tree_head_->sz_free_node_ == 0)
        {
            return true;
        }

        return false;
    };

    size_t size() const
    {
        return avl_tree_head_->sz_use_node_;
    }

    size_t capacity() const
    {
        return avl_tree_head_->num_of_node_;
    }

    //���еĽڵ����
    size_t sizefree()
    {
        return avl_tree_head_->sz_free_node_;
    }

public:

    //�����������δ���ȫ���궨��ģ���������һ�£����û���inline���㹻�ˡ�
    //��Ͼ����ô����ó�ª�����ˡ�������Щ�����ĳ���Ӧ���ǿ��Ա�inline�ġ�

    inline size_t  &header() const
    {
        return avl_tree_head_->num_of_node_;
    }

    inline size_t  &root() const
    {
        return head_index_->parent_;
    }

    inline size_t  &leftmost() const
    {
        return head_index_->left_;
    }

    inline size_t  &rightmost() const
    {
        return head_index_->right_;
    }

    inline size_t  &left(size_t x)
    {
        return (index_base_ + x)->left_;
    }

    inline size_t  &right(size_t x)
    {
        return (index_base_ + x)->right_;
    }

    inline size_t  &parent(size_t x)
    {
        return (index_base_ + x)->parent_;
    }

    inline int32_t &balanced(size_t x)
    {
        return (index_base_ + x)->balanced_;
    }

    inline const _value_type  &value(size_t x)
    {
        return *(data_base_ + x);
    }

    inline const _key_type  &key(size_t x)
    {
        return _extract_key()(value(x));
    }

    //ȡ����ֵ
    size_t minimum(size_t x)
    {
        while (left(x) != _INVALID_POINT)
        {
            x = left(x);
        }

        return x;
    }

    //ȡ��Сֵ
    size_t maximum(size_t x)
    {
        while (right(x) != _INVALID_POINT)
        {
            x = right(x);
        }

        return x;
    }

protected:


    /*!
    * @brief      �����Ĳ����������������ɵ�
    * @return     std::pair<iterator, bool> ���صĲ���ṹ�������������ͽ��
    * @param      x   �����,�󲿷�ʱ��Ϊ_INVALID_POINT
    * @param      y   �����ĸ��ڵ�
    * @param      val ���������
    */
    std::pair<iterator, bool> _insert(size_t x, size_t y, const _value_type &val)
    {
        //����һ���ռ�
        size_t z = create_node(val);
        //�չ��ռ䲻�㣬�޷����룬����end,false��pair
        if (_INVALID_POINT == z)
        {
            return std::pair<iterator, bool>(end(), false);
        }

        //�Ѵ˶��������ȥ�����ҵ������ֶ���

        //���1.�������root�ڵ㣬2.�������ڵ㲻�ǿսڵ㣬3.����Ƚ�ΪTRUE
        if (y == header() || x != _INVALID_POINT || _compare_key()(_extract_key()(val), key(y)))
        {
            left(y) = z;

            if (y == header())
            {
                root() = z;
                rightmost() = z;
            }
            //���Y����Сֵ�������Сֵ��ΪY
            else if (y == leftmost())
            {
                leftmost() = z;
            }
        }
        else
        {
            right(y) = z;

            if (y == rightmost())
            {
                rightmost() = z;
            }
        }

        parent(z) = y;
        left(z) = _INVALID_POINT;
        right(z) = _INVALID_POINT;

        //������Ǹ��ڵ㣬���ǽ���ƽ�����
        if (y != header())
        {
            _balance_adjust(z, true);
        }

        return   std::pair<iterator, bool>(iterator(z, this), true);
    }



    /*!
    * @brief      ����ƽ��������ڲ�������
    * @param[in]  z ����Ľڵ�λ��
    * @param[in]  if_inssert �Ƿ��ǲ���������е����������ɾ����������дfalse
    */
    void _balance_adjust(size_t z, bool if_inssert)
    {
        //��ʵ����ط�ֱ��ʹ�ó�������������һ��,������û����ö�ٻ��ߺ�

        //�ҵ���С�Ĳ�ƽ��ĵ�,

        size_t s = parent(z);
        size_t t = z, u = 0;
        int32_t mod_balance = 0;
        for (; s != header(); t = s, s = parent(s))
        {

            if (if_inssert)
            {
                mod_balance = (t == left(s)) ? 1 : -1;
            }
            else
            {
                mod_balance = (t == left(s)) ? -1 : 1;
            }

            //�����ƽ��ģ��޸�ƽ��������������ϸɻ�
            if (0 == balanced(s))
            {
                balanced(s) += mod_balance;

                //����ǲ��룬ԭ���ڵ���ƽ��ģ��������ϣ������ɾ����ԭ���ڵ���ƽ��ģ�����Ϊֹ
                if (if_inssert)
                {
                    continue;
                }
                else
                {
                    break;
                }

            }
            //�������ԭ���Ͳ�ƽ�⣬�ҵ���С�Ĳ�ƽ������������ת������ƽ��
            else
            {
                balanced(s) += mod_balance;
                //���ݲ�ƽ����������������ʲô������ת
                if (2 == balanced(s))
                {
                    //���ǵ�ɾ��������������������ֱ����t
                    u = left(s);
                    int32_t u_b = balanced(u);
                    if (-1 == u_b )
                    {
                        size_t u_r = right(u);
                        _lr_rotate(s, u, right(u));
                        s = u_r;

                    }
                    else if (1 == u_b )
                    {
                        _ll_rotate(s, u);
                        s = u;
                    }
                    else
                    {
                        _ll_rotate(s, u);
                        //u_b == 0 ֻ������ɾ�����������,�������������󣬸߶Ȳ��仯��
                        break;
                    }
                }
                else if (-2 == balanced(s))
                {
                    u = right(s);
                    int32_t u_b = balanced(u);
                    if (1 == u_b)
                    {
                        size_t u_l = left(u);
                        _rl_rotate(s, u, left(u));
                        s = u_l;
                    }
                    else if (-1 == u_b)
                    {
                        _rr_rotate(s, u);
                        s = u;
                    }
                    else
                    {
                        _rr_rotate(s, u);
                        //u_b == 0 ֻ������ɾ�����������
                        break;
                    }
                }

                //����ɾ����˵����ת����������������ĸ߶Ȼ��б仯������Ҫ����
                if (if_inssert)
                {
                    break;
                }
                else
                {
                    continue;
                }
            }
        }
        return;
    }

    /*!
    * @brief      LL��ת��
    * @param      a   ���ڵ㣬��С�Ĳ�ƽ�����ĸ��ڵ�
    * @param      b  ��ߵ��ӽڵ�
    */
    void _ll_rotate(size_t a, size_t b)
    {
        size_t gf = parent(a);
        size_t b_r = right(b);
        parent(a) = b;
        left(a) = b_r;
        if (_INVALID_POINT != b_r)
        {
            parent(b_r) = a;
        }
        right(b) = a;
        parent(b) = gf;


        //����ƽ������
        if ( 1 == balanced(b) )
        {
            balanced(a) = 0;
            balanced(b) = 0;
        }
        //���ڲ���LL��balanced(b)ֻ���ܵ���1��������ɾ��balanced(b) �����ܵ���0
        else
        {
            balanced(a) = 1;
            balanced(b) = -1;
        }

        //����p�ĸ��ڵ����������������ָ���µ������¸�
        if (gf == header())
        {
            root() = b;
        }
        else
        {
            if (left(gf) == a)
            {
                left(gf) = b;
            }
            else
            {
                right(gf) = b;
            }
        }
    }

    /*!
    * @brief      LR��ת
    * @param      a   ���ڵ㣬��С�Ĳ�ƽ�����ĸ��ڵ�
    * @param      b   a�����ӽڵ�
    * @param      c   b�����ӽڵ�
    */
    void _lr_rotate(size_t a, size_t b, size_t c)
    {
        size_t gf = parent(a);
        size_t c_l = left(c), c_r = right(c);
        parent(a) = c;
        left(a) = c_r;
        if (_INVALID_POINT != c_r )
        {
            parent(c_r) = a;
        }
        parent(b) = c;
        right(b) = c_l;
        if (_INVALID_POINT != c_l)
        {
            parent(c_l) = b;
        }
        left(c) = b;
        right(c) = a;
        parent(c) = gf;

        //����C��״̬����ƽ������
        if (1 == balanced(c) )
        {
            balanced(a) = -1;
            balanced(b) = 0;
            balanced(c) = 0;
        }
        else if (-1 == balanced(c) )
        {
            balanced(a) = 0;
            balanced(b) = 1;
            balanced(c) = 0;
        }
        else
        {
            balanced(a) = 0;
            balanced(b) = 0;
            balanced(c) = 0;
        }

        //����p�ĸ��ڵ����������������ָ���µ������¸�
        if (gf == header())
        {
            root() = c;
        }
        else
        {
            if (left(gf) == a)
            {
                left(gf) = c;
            }
            else
            {
                right(gf) = c;
            }
        }
    }

    /*!
    * @brief      RR��ת��
    * @param      a  ���ڵ㣬��С�Ĳ�ƽ�����ĸ��ڵ�
    * @param      b  �ұߵ��ӽڵ�
    */
    void _rr_rotate(size_t a, size_t b)
    {
        size_t gf = parent(a);
        size_t b_l = left(b);
        parent(a) = b;
        right(a) = b_l;
        if (_INVALID_POINT != b_l)
        {
            parent(b_l) = a;
        }
        left(b) = a;
        parent(b) = gf;

        //����ƽ������
        if ( -1 == balanced(b) )
        {
            balanced(a) = 0;
            balanced(b) = 0;
        }
        //���ڲ���RR��ת��balanced(b)ֻ���ܵ���-1��������ɾ��balanced(b) �����ܵ���0
        else
        {
            balanced(a) = -1;
            balanced(b) = 1;
        }

        //����p�ĸ��ڵ����������������ָ���µ������¸�
        if (gf == header())
        {
            root() = b;
        }
        else
        {
            if (left(gf) == a)
            {
                left(gf) = b;
            }
            else
            {
                right(gf) = b;
            }
        }

    }

    void _rl_rotate(size_t a, size_t b, size_t c)
    {
        size_t gf = parent(a);
        size_t c_l = left(c), c_r = right(c);
        parent(a) = c;
        right(a) = c_l;
        if (_INVALID_POINT != c_l )
        {
            parent(c_l) = a;
        }
        parent(b) = c;
        left(b) = c_r;
        if ( _INVALID_POINT != c_r)
        {
            parent(c_r) = b;
        }
        left(c) = a;
        right(c) = b;
        parent(c) = gf;

        //����C��״̬����ƽ������
        if (balanced(c) == 1)
        {
            balanced(a) = 0;
            balanced(b) = -1;
            balanced(c) = 0;
        }
        else if (balanced(c) == -1)
        {
            balanced(a) = 1;
            balanced(b) = 0;
            balanced(c) = 0;
        }
        else
        {
            balanced(a) = 0;
            balanced(b) = 0;
            balanced(c) = 0;
        }

        //����C�ĸ��ڵ����������������ָ���µ������¸�
        if (gf == header())
        {
            root() = c;
        }
        else
        {
            if (left(gf) == a)
            {
                left(gf) = c;
            }
            else
            {
                right(gf) = c;
            }
        }
    }


    /*!
    * @brief      ������ɾ������ʵ�֣��о�AVL��ɾ�����Բ���
    * @param      x Ϊɾ����λ��
    * @param      y ΪX�ĸ��ڵ�
    */
    void _erase(size_t x, size_t y)
    {

        //��leftmost��rightmost���д���
        if (x == leftmost())
        {
            iterator iter(x, this);
            ++iter;
            leftmost() = iter.getserial();
        }
        if (x == rightmost())
        {
            iterator iter(x, this);
            if (iter != begin())
            {
                --iter;
                rightmost() = iter.getserial();
            }
            else
            {
                rightmost() = avl_tree_head_->num_of_node_;
            }

        }

        size_t a = x, a_p = y, a_l = left(a), a_r = right(a), b = 0, b_p  = 0;
        //Ҫ��A���½�����ѡ�������ӽ��Ľڵ�B�滻����������������һֱ���ұߵĽڵ㣬
        //������������һֱ����ߵĽڵ㣬ֱ��A��Ҷ�ӽڵ�
        while (a_l != _INVALID_POINT || a_r != _INVALID_POINT)
        {
            if (a_l != _INVALID_POINT)
            {
                b = a_l;
                while (_INVALID_POINT != right(b) )
                {
                    b = right(b);
                }
            }
            else
            {
                b = a_r;
                while (_INVALID_POINT != left(b))
                {
                    b = left(b);
                }
            }
            b_p = parent(b);
            //��A��B���н���
            _exchange(a, a_p, b, b_p);

            a_l = left(a);
            a_r = right(a);
            a_p = parent(a);

        }

        //��ƽ�����
        _balance_adjust(a, false);

        size_t last_p = parent(x);
        if (last_p != header())
        {
            if (left(last_p) == a)
            {
                left(last_p) = _INVALID_POINT;
            }
            else
            {
                right(last_p) = _INVALID_POINT;
            }
        }
        else
        {
            root() = _INVALID_POINT;
        }
        //
        destroy_node(x);

        return;
    }

    /*!
    * @brief      ��a,b�����ڵ��󽻻���ע���������ֻ����н��������ῼ��ƽ���
    * @param      a   a �ڵ㣬a��b�ĸ��ڵ���߸����ȵĵļ��㣬
    * @param      a_p a����
    * @param      b   b�ڵ㣬
    * @param      b_p b����
    */
    void _exchange(size_t a, size_t a_p, size_t b, size_t b_p)
    {
        size_t a_l = left(a);
        size_t a_r = right(a);
        uint32_t a_b = balanced(a);

        size_t b_l = left(b);
        size_t b_r = right(b);
        uint32_t b_b = balanced(b);


        if (a_p != header())
        {
            if (left(a_p) == a)
            {
                left(a_p) = b;
            }
            else
            {
                right(a_p) = b;
            }
        }
        else
        {
            root() = b;
        }

        parent(b) = a_p;

        if (a_l == b)
        {
            left(b) = a;
        }
        else
        {
            left(b) = a_l;
        }
        if (a_r == b)
        {
            right(b) = a;
        }
        else
        {
            right(b) = a_r;
        }


        balanced(b) = a_b;

        parent(a) = b_p;
        left(a) = b_l;
        right(a) = b_r;
        balanced(a) = b_b;

        if ( a_l != _INVALID_POINT)
        {
            if (a_l != b)
            {
                parent(a_l) = b;
            }
            else
            {
                parent(a) = b;
            }
        }
        if ( a_r != _INVALID_POINT)
        {
            if (a_r != b)
            {
                parent(a_r) = b;
            }
            else
            {
                parent(a) = b;
            }
        }
        if (b_l != _INVALID_POINT)
        {
            parent(b_l) = a;
        }
        if (b_r != _INVALID_POINT)
        {
            parent(b_r) = a;
        }
    }

public:

    /*!
    * @brief      �����ظ�key����Ĳ��뺯����Multimap��Multimap�����
    *             ��Ϊ�ռ�����������Է��ص������л�����bool��pair
    * @return     std::pair<iterator, bool>  ���ص�iteratorΪ��������boolΪ�Ƿ����ɹ�
    * @param      v        �����_value_type������
    */
    std::pair<iterator, bool> insert_equal(const _value_type &v)
    {
        //����������ˣ�Ҳ����ʧ��
        if (avl_tree_head_->sz_free_node_ == 0)
        {
            return std::pair<iterator, bool>(end(), false);
        }

        size_t y = header();
        size_t x = root();

        //���뵽һ���սڵ���
        while (x != _INVALID_POINT)
        {
            y = x;
            x = _compare_key()(_extract_key()(v), key(x)) ? left(x) : right(x);
        }

        return _insert(x, y, v);
    }

    /*!
    * @brief      �ظ�key������ʧ�ܵĲ��뺯����Map��Sap�����
    * @return     std::pair<iterator, bool> ���ص�iteratorΪ��������boolΪ�Ƿ����ɹ�
    * @param      v �����_value_type������
    */
    std::pair<iterator, bool> insert_unique(const _value_type &v)
    {
        //����������ˣ�Ҳ����ʧ��
        if (avl_tree_head_->sz_free_node_ == 0)
        {
            return std::pair<iterator, bool>(end(), false);
        }

        size_t y = header();
        size_t x = root();
        bool comp = true;

        //����Ƚ�(������<)����true,�����󣬷���(>=)�����ң�
        while (x != _INVALID_POINT)
        {
            y = x;
            comp = _compare_key()(_extract_key()(v), key(x));
            x = comp ? left(x) : right(x);
        }

        iterator j = iterator(y, this);

        if (comp)
        {
            if (j == begin())
            {
                return _insert(x, y, v);
            }
            else
            {
                --j;
            }
        }

        if (_compare_key()(key(j.getserial()), _extract_key()(v)))
        {
            return _insert(x, y, v);
        }

        //����Ȳ���>,�ֲ���<����ô����==,��ô���ش���
        return std::pair<iterator, bool>(j, false);
    }

    /*!
    * @brief      ͨ��������ɾ��һ���ڵ�
    * @return     void ע�⣬΢�������������񷵻�һ����������
    * @param      pos ɾ���ĵ�����
    */
    void erase(const iterator &pos)
    {
        //x,Ϊɾ����λ�ã�yΪX�ĸ��ڵ㣬z����Ϊ�滻x�Ľڵ�
        size_t x = pos.getserial();
        size_t y = parent(x);
        return _erase(x, y);
    }

    //ͨ����ʼ������ɾ��һ�νڵ�
    size_t erase(iterator first, iterator last)
    {
        size_t erase_count = 0;

        //��������ļ���
        if (first == begin() && last == end())
        {
            erase_count = size();
            clear();
        }
        else
        {
            //
            while (first != last)
            {
                ++erase_count;
                //ע������ط��õ���erase(first++)����first
                erase(first++);
            }
        }
        return erase_count;
    }

    //ͨ��keyɾ���ڵ㣬MAPʹ��
    size_t erase_unique(const _key_type &k)
    {
        //�ȳ����ҵ��ڵ�
        iterator find_iter = find(k);
        if (find_iter == end())
        {
            return 0;
        }
        erase(find_iter);
        return 1;
    }

    //ͨ��valueɾ���ڵ㣬SETʹ��
    size_t erase_unique_value(const _value_type &v)
    {
        _extract_key get_key;
        return erase_unique(get_key(v));
    }

    //ͨ��keyɾ���ڵ㣬MULTIMAP��
    size_t erase_equal(const _key_type &k)
    {
        iterator it_l = lower_bound(k);
        iterator it_u = upper_bound(k);
        return erase(it_l, it_u);
    }

    //ͨ��ֵɾ���ڵ㣬MULTISET��
    size_t erase_equal_value(const _value_type &v)
    {
        _extract_key get_key;
        return erase_equal(get_key(v));
    }

    //�ҵ���һ��keyֵ��ͬ�Ľڵ�
    iterator lower_bound(const _key_type &k)
    {
        size_t y = header();
        size_t x = root();

        while (x != _INVALID_POINT)
        {
            //�����������������д��벻һ����ע���Ⱥ�Ƚ�
            if (!_compare_key()(key(x), k))
            {
                y = x;
                x = left(x);
            }
            else
            {
                x = right(x);
            }
        }

        return iterator(y, this);
    }

    //�ҵ����һ��keyֵ��ͬ�Ľڵ�
    iterator upper_bound(const _key_type &k)
    {
        size_t y = header();
        size_t x = root();

        while (x != _INVALID_POINT)
        {
            //�����������������д��벻һ����ע���Ⱥ�ȽϹ�ϵ
            if (_compare_key()(k, key(x)))
            {
                y = x;
                x = left(x);
            }
            else
            {
                x = right(x);
            }
        }

        return iterator(y, this);
    }

    //��key��ͬ�Ľڵ�
    iterator find(const _key_type &k)
    {
        size_t y = header();
        size_t x = root();

        while (x != _INVALID_POINT)
        {
            if (!_compare_key()(key(x), k))
            {
                y = x;
                x = left(x);
            }
            else
            {
                x = right(x);
            }
        }

        iterator j = iterator(y, this);
        //ע�����ε���_compare_key�ıȽϲ���˳���
        return (j == end() || _compare_key()(k, key(j.getserial()))) ? end() : j;
    }

    //��value��ͬ�Ľڵ�
    iterator find_value(const _value_type &v)
    {
        _extract_key get_key;
        return find(get_key(v));
    }

    //��value��ͬ�Ľڵ㣬��δ�ҵ������
    _value_type &find_or_insert(const _value_type &v)
    {
        iterator iter = find_value(v);

        if (iter == end())
        {
            std::pair<iterator, bool> pair_iter = insert(v);
            return (*(pair_iter.first));
        }

        return *iter;
    }

    //���Դ��룬���_value_type������ ��ʱ����Ч��������Ч
    void debug_note(size_t x, typename std::enable_if<std::is_integral<_value_type>::value >::type * = 0)
    {
        std::cout << "Note :" << std::setw(6) << x
                  << " Data:" << std::setw(8) << data_base_[x]
                  << " parent:" << std::setw(6) << (int) parent(x)
                  << " left:" << std::setw(6) << (int) left(x)
                  << " right:" << std::setw(6) << (int) right(x)
                  << " balanced:" << balanced(x)
                  << std::endl;
    }

    //������νṹ�Ƿ�ƽ��
    bool check_balanced(size_t x)
    {
        int32_t x_b = balanced(x);
        size_t x_lh = height(left(x));
        size_t x_rh = height(right(x));
        if (x_b != (int32_t)( x_lh - x_rh))
        {
            return false;
        }
        return true;
    }

    //�õ�ĳ���ڵ�ĸ߶�
    size_t height(size_t x)
    {
        if (x == _INVALID_POINT)
        {
            return 0;
        }
        size_t l_h = 0, r_h = 0;
        l_h = height(left(x)) + 1;
        r_h = height(right(x)) + 1;

        return l_h > r_h ? l_h : r_h;
    }

protected:
    //index��Ҫ������������,һ����ͷָ�룬һ���ǿսڵ��ͷָ��
    static const size_t ADDED_NUM_OF_INDEX = 2;

protected:
    ///RBTreeͷ��
    _shm_avl_tree_head            *avl_tree_head_;

    ///���е�ָ�붼�Ǹ��ݻ���ַ����õ���,���ڷ������,ÿ�γ�ʼ�������¼���
    ///����������,
    _shm_avl_tree_index            *index_base_;

    ///��������ʼָ��,
    _value_type                   *data_base_;

    ///ͷ�ڵ��ͷָ��,N+1������λ��ʾ
    _shm_avl_tree_index            *head_index_;

    ///�սڵ��ͷָ��,N+2������λ��ʾ����������right�ڵ������ӣ��ѿսڵ㴮������
    _shm_avl_tree_index            *free_index_;

};

//��AVL Treeʵ��SET��������multiset��set��ͨ����ͨ��insert�Լ�����
template < class _value_type,
           class _compare_key = std::less<_value_type> >
class mmap_avl_set :
    public shm_avl_tree< _value_type, _value_type, smem_identity<_value_type>, _compare_key >
{
protected:
    //����ڹ����ڴ�ʹ��,û��new,����ͳһ��initialize ��ʼ��
    //�������,��������,���ǲ�������
    mmap_avl_set<_value_type, _compare_key >(size_t numnode, void *pmmap, bool if_restore) :
        shm_avl_tree<_value_type, _value_type, smem_identity<_value_type>, _compare_key>(numnode, pmmap, if_restore)
    {
        initialize(numnode, pmmap, if_restore);
    }

    ~mmap_avl_set<_value_type, _compare_key >()
    {
    }

public:

    static mmap_avl_set< _value_type, _compare_key  > *
    initialize(size_t &numnode, char *pmmap, bool if_restore = false)
    {
        return reinterpret_cast<mmap_set< _value_type, _compare_key  >*>(
                   shm_avl_tree < _value_type,
                   _value_type,
                   smem_identity<_value_type>,
                   _compare_key >::initialize(numnode, pmmap, if_restore));
    }
};

//��AVL Treeʵ��MAP��������multiset��set��ͨ����ͨ��insert�Լ�����
template < class _key_type,
           class _value_type,
           class _extract_key = mmap_select1st <std::pair <_key_type, _value_type> >,
           class _compare_key = std::less<_value_type>  >
class mmap_avl_map :
    public shm_avl_tree< std::pair <_key_type, _value_type>, _key_type, _extract_key, _compare_key  >
{
protected:
    //����ڹ����ڴ�ʹ��,û��new,����ͳһ��initialize ��ʼ��
    //�������,��������,���ǲ�������
    mmap_avl_map<_key_type, _value_type, _extract_key, _compare_key >(size_t numnode, void *pmmap, bool if_restore) :
        shm_avl_tree< std::pair <_key_type, _value_type>, _key_type, _extract_key, _compare_key  >(numnode, pmmap, if_restore)
    {
        initialize(numnode, pmmap, if_restore);
    }

    ~mmap_avl_map<_key_type, _value_type, _extract_key, _compare_key >()
    {
    }
public:
    static mmap_avl_map< _key_type, _value_type, _extract_key, _compare_key  > *
    initialize(size_t &numnode, char *pmmap, bool if_restore = false)
    {
        return reinterpret_cast < mmap_avl_map < _key_type,
               _value_type,
               _extract_key,
               _compare_key  > * > (
                   shm_avl_tree < std::pair < _key_type,
                   _value_type >,
                   _key_type,
                   _extract_key,
                   _compare_key >::initialize(numnode, pmmap, if_restore));
    }
    //[]�����������ŵ��ȱ�㣬����ʹ��
    _value_type &operator[](const _key_type &key)
    {
        return (find_or_insert(std::pair<_key_type, _value_type >(key, _value_type()))).second;
    }
};


};

#endif //ZCE_LIB_SHM_AVL_TREE_H_

