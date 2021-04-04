/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_rbtree.h
* @author     scottxu(���)
* @version
* @date       2007��7��24��
* @brief      RBTree��Ҫ����ɿ��������MAP,SET,MultiMap,MultiSet��MMAP��
*             ʵ����RBTree�Լ�MAP,SET,MultiMap,MultiSet
*
* @details
*
*
*
* @note       ����֤�̰߳�ȫ���ϲ��Լ�����֤
*
*/


#ifndef ZCE_LIB_SHM_RB_TREE_H_
#define ZCE_LIB_SHM_RB_TREE_H_

#include "zce_shm_predefine.h"

namespace zce
{

enum RB_TREE_COLOR
{
    //��ڵ�
    RB_TREE_RED   = 0,
    //�ڽڵ�
    RB_TREE_BLACK = 1,
};

//
typedef char  color_type;

template<class _value_type, class _key_type, class _extract_key, class _compare_key> class shm_rb_tree;

//RB TREE��ͷ��������
class _shm_rb_tree_head
{
protected:
    _shm_rb_tree_head()
        : size_of_mmap_(0)
        , num_of_node_(0)
        , sz_free_node_(0)
        , sz_use_node_(0)
    {
    }
    ~_shm_rb_tree_head()
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

//RBtree�������Ľڵ�
class _shm_rb_tree_index
{

public:
    _shm_rb_tree_index()
        : parent_(_shm_memory_base::_INVALID_POINT)
        , left_(_shm_memory_base::_INVALID_POINT)
        , right_(_shm_memory_base::_INVALID_POINT)
        , color_(RB_TREE_RED)
    {
    }

    _shm_rb_tree_index(const size_t &p, const size_t &l, const size_t &r, char cl)
        : parent_(p)
        , left_(l)
        , right_(r)
        , color_(cl)
    {
    }

    ~_shm_rb_tree_index()
    {
    }

public:
    //���ڵ�
    size_t       parent_;
    //������
    size_t       left_;
    //������
    size_t       right_;
    //��ɫ
    color_type   color_;

};

//RBtree�ĵ�����
template <class _value_type, class _key_type, class _extract_key, class _compare_key> class _shm_rb_tree_iterator
{
    typedef _shm_rb_tree_iterator<_value_type, _key_type, _extract_key, _compare_key> iterator;

    typedef shm_rb_tree<_value_type, _key_type, _extract_key, _compare_key> shm_rb_tree_t;


    //��������ȡ�����еĶ���
    typedef ptrdiff_t difference_type;
    typedef _value_type *pointer;
    typedef _value_type &reference;
    typedef _value_type value_type;
    typedef std::bidirectional_iterator_tag iterator_category;

public:
    _shm_rb_tree_iterator(size_t seq, shm_rb_tree_t *instance)
        : serial_(seq)
        , rb_tree_instance_(instance)
    {
    }

    _shm_rb_tree_iterator()
        : serial_(_shm_memory_base::_INVALID_POINT),
          rb_tree_instance_(NULL)
    {
    }

    ~_shm_rb_tree_iterator()
    {
    }

    //��ʼ��
    void initialize(size_t seq, shm_rb_tree_t *instance)
    {
        serial_ = seq;
        rb_tree_instance_ = instance;
    }

    //������žͿ����ٸ���ģ��ʵ���������ҵ���Ӧ����,����ʹ��ָ��
    size_t getserial() const
    {
        return serial_;
    }

    bool operator==(const iterator &x) const
    {
        return (serial_ == x.serial_ && rb_tree_instance_ == x.rb_tree_instance_ );
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
        return rb_tree_instance_->getdatabase() + serial_;
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

    //����ʵ��operator++������һ���ڵ�
    void increment()
    {
        if ( (rb_tree_instance_->index_base_ + serial_)->right_ != _shm_memory_base::_INVALID_POINT )
        {
            //��������ӽڵ㣬�������ߣ�Ȼ��һֱ���������ߵ��׼���
            serial_ = (rb_tree_instance_->index_base_ + serial_)->right_;

            while ( (rb_tree_instance_->index_base_ + serial_)->left_ != _shm_memory_base::_INVALID_POINT )
            {
                serial_ = (rb_tree_instance_->index_base_ + serial_)->left_;
            }
        }
        else
        {
            //���û�����ӽڵ㣬�ҵ����ڵ㣬�����ǰ�ڵ���ĳ�����ӽڵ㣬��һֱ���ݵ���Ϊ���ӽڵ�Ϊֹ
            size_t y = (rb_tree_instance_->index_base_ + serial_)->parent_;

            while ( serial_ == (rb_tree_instance_->index_base_ + y)->right_ )
            {
                serial_ = y;
                y = (rb_tree_instance_->index_base_ + y)->parent_;
            }

            //����ʱ�����ӽڵ㲻���ڸ��ڵ㣬�򸸽ڵ㼴�ǣ�������ǵ�ǰ�ڵ�
            if ( (rb_tree_instance_->index_base_ + serial_)->right_ != y )
            {
                serial_ = y;
            }
        }
    }

    //����ʵ��operator--������һ���ڵ�
    void decrement()
    {
        //����Ǻ�ڵ㣬�Ҹ��ڵ�ĵĸ��ڵ�����Լ�
        if ( (rb_tree_instance_->index_base_ + serial_)->color_ == RB_TREE_RED &&
             (rb_tree_instance_->index_base_ + ((rb_tree_instance_->index_base_ + serial_)->parent_))->parent_ == serial_ )
        {
            //���ӽڵ㼴��
            serial_ = (rb_tree_instance_->index_base_ + serial_)->right_;
        }
        //��������ӽڵ�
        else if ( (rb_tree_instance_->index_base_ + serial_)->left_ != _shm_memory_base::_INVALID_POINT )
        {
            //��yָ�����ӽڵ㣬�ҵ�y�����ӽڵ㣬�����ߵ��׼���
            size_t y = (rb_tree_instance_->index_base_ + serial_)->left_;

            while ( (rb_tree_instance_->index_base_ + y)->right_ != _shm_memory_base::_INVALID_POINT )
            {
                y = (rb_tree_instance_->index_base_ + y)->right_;
            }

            serial_ = y;
        }
        else
        {
            //�ҳ����ڵ㣬�����ǰ�ڵ��Ǹ����ӽڵ㣬��һֱ���ݣ�ֱ������Ϊ���ӽڵ㣬����ĸ��ڵ㼴��
            size_t y = (rb_tree_instance_->index_base_ + serial_)->parent_;

            while ( serial_ == (rb_tree_instance_->index_base_ + y)->left_ )
            {
                serial_ = y;
                y = (rb_tree_instance_->index_base_ + y)->parent_;
            }

            serial_ = y;
        }
    }

protected:
    //���к�
    size_t          serial_;
    //RBtree��ʵ��ָ��
    shm_rb_tree_t  *rb_tree_instance_;

};



/*!
* @brief
*
* @tparam     _value_type   �ڵ��ʵ��ֵ�ͱ�
* @tparam     _key_type     �ڵ�ļ����ͱ�
* @tparam     _extract_key  �ӽڵ��еõ���ֵ�ķ���,���ߺ�������
* @tparam     _compare_key  �Ƚϼ�ֵ��С�ķ���,���ߺ�������
* @note
*/
template < class _value_type,
           class _key_type,
           class _extract_key = smem_identity<_value_type>,
           class _compare_key = std::less<_key_type> >
class shm_rb_tree : public _shm_memory_base
{
public:
    //�����Լ�
    typedef shm_rb_tree<_value_type, _key_type, _extract_key, _compare_key> self;

    //���������
    typedef _shm_rb_tree_iterator<_value_type, _key_type, _extract_key, _compare_key> iterator;

    //��������Ԫ
    friend class _shm_rb_tree_iterator<_value_type, _key_type, _extract_key, _compare_key>;

protected:
    //����ڹ����ڴ�ʹ��,û��new,����ͳһ��initialize ��ʼ��
    //�������,��������,���ǲ�������
    shm_rb_tree<_value_type, _key_type, _extract_key, _compare_key >(size_t numnode, void *pmmap, bool if_restore)
        : _shm_memory_base(NULL)
        , index_base_(NULL)
        , data_base_(NULL)
    {
    }

    shm_rb_tree<_value_type, _key_type, _extract_key, _compare_key >()
        : _shm_memory_base(NULL)
    {
    }
public:

    ~shm_rb_tree<_value_type, _key_type, _extract_key, _compare_key >()
    {
    }

    //ֻ����,��ʵ��,���ⷸ��
    const self &operator=(const self &others);

    //�õ������Ļ�����ַ
    inline _shm_rb_tree_index *getindexbase()
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
        if (rb_tree_head_->sz_free_node_ == 0)
        {
            return _INVALID_POINT;
        }

        //������ȡ1������
        size_t new_node = free_index_->right_;
        free_index_->right_ = (index_base_ + new_node)->right_;
        rb_tree_head_->sz_free_node_--;
        rb_tree_head_->sz_use_node_++;

        //��ʼ��
        (index_base_ + new_node)->parent_ = _INVALID_POINT;
        (index_base_ + new_node)->left_ = _INVALID_POINT;
        (index_base_ + new_node)->right_ = _INVALID_POINT;
        (index_base_ + new_node)->color_ = RB_TREE_RED;

        new (data_base_ + new_node)_value_type(val);

        return new_node;
    }

    //�ͷ�һ��NODE,����黹��FREELIST
    void destroy_node(size_t pos)
    {
        size_t freenext = free_index_->right_;
        (index_base_ + pos)->right_ = freenext;
        free_index_->right_ = pos;
        rb_tree_head_->sz_free_node_++;
        rb_tree_head_->sz_use_node_--;

        //������ʽ����������
        (data_base_ + pos)->~_value_type();
    }

public:

    //�ڴ����Ĺ���Ϊ ������,index��,data��,��������Ҫ�ĳ���,
    static size_t getallocsize(const size_t numnode)
    {
        return  sizeof(_shm_rb_tree_head)  +
                sizeof(_shm_rb_tree_index) * (numnode + ADDED_NUM_OF_INDEX) +
                sizeof(_value_type) * numnode ;
    }

    //��ʼ��
    static self *initialize(const size_t numnode, char *pmmap, bool if_restore = false)
    {
        //assert(pmmap!=NULL && numnode >0 );
        _shm_rb_tree_head *rb_tree_head = reinterpret_cast<_shm_rb_tree_head *>(pmmap);

        //����ǻָ�,���ݶ����ڴ���,
        if ( true == if_restore)
        {
            //���һ�»ָ����ڴ��Ƿ���ȷ,
            if (getallocsize(numnode) != rb_tree_head->size_of_mmap_ ||
                numnode != rb_tree_head->num_of_node_ )
            {
                return NULL;
            }
        }

        //��ʼ���ߴ�
        rb_tree_head->size_of_mmap_ = getallocsize(numnode);
        rb_tree_head->num_of_node_ = numnode;

        self *instance = new self();

        //���е�ָ�붼�Ǹ��ӻ���ַ����õ���,���ڷ������,ÿ�γ�ʼ�������¼���
        instance->smem_base_ = pmmap;
        instance->rb_tree_head_ = rb_tree_head;
        instance->index_base_ = reinterpret_cast<_shm_rb_tree_index *>(pmmap + sizeof(_shm_rb_tree_head));
        instance->data_base_  = reinterpret_cast<_value_type *>(pmmap + sizeof(_shm_rb_tree_head) + sizeof(_shm_rb_tree_index) * (numnode + ADDED_NUM_OF_INDEX) );

        //��ʼ��free_index_,head_index_
        instance->head_index_ = reinterpret_cast<_shm_rb_tree_index *>(pmmap + sizeof(_shm_rb_tree_head) + sizeof(_shm_rb_tree_index) * (numnode ));
        instance->free_index_ = reinterpret_cast<_shm_rb_tree_index *>(pmmap + sizeof(_shm_rb_tree_head) + sizeof(_shm_rb_tree_index) * (numnode + 1));

        if ( false == if_restore)
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
        rb_tree_head_->sz_free_node_ = rb_tree_head_->num_of_node_;
        rb_tree_head_->sz_use_node_ = 0;

        //������ΪNULL,��ָ�붼ָ���Լ�
        head_index_->parent_ = _INVALID_POINT;
        head_index_->right_  = rb_tree_head_->num_of_node_;
        head_index_->left_   = rb_tree_head_->num_of_node_;
        head_index_->color_  = RB_TREE_RED;

        _shm_rb_tree_index *pindex = index_base_;

        free_index_->right_ = 0;

        //��ʼ��free������
        for (size_t i = 0; i < rb_tree_head_->num_of_node_ ; ++i )
        {
            pindex->right_ = (i + 1) ;

            //������FREENODE������
            if (i == rb_tree_head_->num_of_node_ - 1)
            {
                pindex->right_ = rb_tree_head_->num_of_node_ + 1;
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
        return iterator(rb_tree_head_->num_of_node_, this);
    }

    //���нڵ㶼��free���ϼ��ǿ�
    bool empty()
    {
        if (rb_tree_head_->sz_free_node_ == rb_tree_head_->num_of_node_)
        {
            return true;
        }

        return false;
    }

    //�ڲ�������ǰ����,����������
    bool full()
    {
        if (rb_tree_head_->sz_free_node_ == 0 )
        {
            return true;
        }

        return false;
    };

    size_t size() const
    {
        return rb_tree_head_->sz_use_node_;
    }

    size_t capacity() const
    {
        return rb_tree_head_->num_of_node_;
    }

    //���еĽڵ����
    size_t sizefree()
    {
        return rb_tree_head_->sz_free_node_;
    }

protected:

    //�����������δ���ȫ���궨��ģ���������һ�£����û���inline���㹻�ˡ�
    //��Ͼ����ô����ó�ª�����ˡ�������Щ�����ĳ���Ӧ���ǿ��Ա�inline�ġ�

    inline size_t  &header() const
    {
        return rb_tree_head_->num_of_node_;
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

    inline color_type  &color(size_t x)
    {
        return (index_base_ + x)->color_;
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
        while ( left(x) != _INVALID_POINT )
        {
            x = left(x);
        }

        return x;
    }

    //ȡ��Сֵ
    size_t maximum(size_t x)
    {
        while ( right(x) != _INVALID_POINT )
        {
            x = right(x);
        }

        return x;
    }

protected:
    //�����Ĳ����������������ɵ�
    std::pair<iterator, bool>  _insert(size_t x, size_t y, const _value_type &v)
    {
        size_t z = create_node(v);
        //����ռ䲻�㣬�޷����룬����end,false��pair
        if (_INVALID_POINT == z)
        {
            return std::pair<iterator, bool>(end(), false);
        }

        if ( y == header() || x != _INVALID_POINT || _compare_key()(_extract_key()(v), key(y)) )
        {
            left(y) = z;

            if ( y == header() )
            {
                root() = z;
                rightmost() = z;
            }
            else if ( y == leftmost() )
            {
                leftmost() = z;
            }
        }
        else
        {
            right(y) = z;

            if ( y == rightmost())
            {
                rightmost() = z;
            }
        }

        parent(z) = y;
        left(z) = _INVALID_POINT;
        right(z) = _INVALID_POINT;
        *(data_base_ + z) = v;

        _rb_tree_rebalance(z, parent(header()));
        return  std::pair<iterator, bool>(iterator(z, this), true);
    }

    //ͨ����ת�ͱ�ɫ���������������������RBTreeҪ��
    //����1�������ڵ�
    //����2�����ڵ�
    void _rb_tree_rebalance(size_t x, size_t &root)
    {
        color(x) = RB_TREE_RED;

        while ( x != root && color(parent(x)) == RB_TREE_RED )
        {
            if ( parent(x) == left(parent(parent(x))) )
            {
                size_t y = right(parent(parent(x)));

                if ( y != _INVALID_POINT && color(y) == RB_TREE_RED )
                {
                    color(parent(x)) = RB_TREE_BLACK;
                    color(y) = RB_TREE_BLACK;
                    color(parent(parent(x))) = RB_TREE_RED;
                    x = parent(parent(x));
                }
                else
                {
                    if ( x == right(parent(x)) )
                    {
                        x = parent(x);
                        _rb_tree_rotate_left(x, root);
                    }

                    color(parent(x)) = RB_TREE_BLACK;
                    color(parent(parent(x))) = RB_TREE_RED;
                    _rb_tree_rotate_right(parent(parent(x)), root);
                }
            }
            else
            {
                size_t y = left(parent(parent(x)));

                if ( y != _INVALID_POINT && color(y) == RB_TREE_RED )
                {
                    color(parent(x)) = RB_TREE_BLACK;
                    color(y) = RB_TREE_BLACK;
                    color(parent(parent(x))) = RB_TREE_RED;
                    x = parent(parent(x));
                }
                else
                {
                    if ( x == left(parent(x)) )
                    {
                        x = parent(x);
                        _rb_tree_rotate_right(x, root);
                    }

                    color(parent(x)) = RB_TREE_BLACK;
                    color(parent(parent(x))) = RB_TREE_RED;
                    _rb_tree_rotate_left(parent(parent(x)), root);
                }
            }
        }

        color(root) = RB_TREE_BLACK;
    }

    //��������
    //����1�������ڵ�
    //����2�����ڵ�
    void _rb_tree_rotate_left(size_t x, size_t &root)
    {
        size_t y = right(x);
        right(x) = left(y);

        if ( left(y) != _INVALID_POINT)
        {
            parent(left(y)) = x;
        }

        parent(y) = parent(x);

        if ( x == root )
        {
            root = y;
        }
        else if ( x == left(parent(x)) )
        {
            left(parent(x)) = y;
        }
        else
        {
            right(parent(x)) = y;
        }

        left(y) = x;
        parent(x) = y;
    }

    //��������
    //����1�������ڵ�
    //����2�����ڵ�
    void _rb_tree_rotate_right(size_t x, size_t &root)
    {
        size_t y = left(x);
        left(x) = right(y);

        if ( right(y) != _INVALID_POINT )
        {
            parent(right(y)) = x;
        }

        parent(y) = parent(x);

        if ( x == root )
        {
            root = y;
        }
        else if ( x == right(parent(x)) )
        {
            right(parent(x)) = y;
        }
        else
        {
            left(parent(x)) = y;
        }

        right(y) = x;
        parent(x) = y;
    }

    //ɾ��ʱ�����ε������������RBTreeҪ��
    size_t _erase(size_t z)
    {
        size_t y = z;
        size_t x = _INVALID_POINT;
        size_t x_parent = _INVALID_POINT;

        //���������ΪNULL��ѡ��������
        if (left(y) == _INVALID_POINT)
        {
            x = right(y);
        }
        else
        {
            //�����������ΪNULL����������ΪNULL
            if (right(x) == _INVALID_POINT)
            {
                x = left(y);
            }
            //���������������ΪNULL
            else
            {
                y = minimum(right(y));
                x = right(y);
            }
        }

        //y����z,��ʾy�����Լ���������ļ�Сֵ
        if (y != z)
        {
            parent(left(z)) = y;
            left(y) = left(z);

            if (y != right(z))
            {
                x_parent = parent(y);

                if (x != _INVALID_POINT)
                {
                    parent(x) = parent(y);
                }

                left(parent(y)) = x;
                right(y) = right(z);
                parent(right(z)) = y;
            }
            else
            {
                x_parent = y;
            }

            if (root() == z)
            {
                root() = y;
            }
            else if (left(parent(z)) == z)
            {
                left(parent(z)) = y;
            }
            else
            {
                right(parent(z)) = y;
            }

            parent(y) = parent(z);
            color_type  c = color(y);
            color(y) = color(z);
            color(z) = c;
            y = z;
        }
        else
        {
            x_parent = parent(y);

            if (x != _INVALID_POINT)
            {
                parent(x) = parent(y);
            }

            if (root() == z)
            {
                root() = x;
            }
            else
            {
                if (left(parent(z)) == z)
                {
                    left(parent(z)) = x;
                }
                else
                {
                    right(parent(z)) = x;
                }
            }

            if (leftmost() == z)
            {
                if (right(z) == _INVALID_POINT)
                {
                    leftmost() = parent(z);
                }
                else
                {
                    leftmost() = minimum(x);
                }
            }

            if (rightmost() == z)
            {
                if (left(z) == _INVALID_POINT)
                {
                    rightmost() = parent(z);
                }
                else
                {
                    rightmost() = maximum(x);
                }
            }
        }

        if (color(y) != RB_TREE_RED)
        {
            while (x != root() && (x == _INVALID_POINT || color(x) == RB_TREE_BLACK))
            {
                if (x == left(x_parent))
                {
                    size_t w = right(x_parent);

                    if (color(w) == RB_TREE_RED)
                    {
                        color(w) = RB_TREE_BLACK;
                        color(x_parent) = RB_TREE_RED;
                        _rb_tree_rotate_left(x_parent, root());
                        w = right(x_parent);
                    }

                    if ((left(w) == _INVALID_POINT || color(left(w)) == RB_TREE_BLACK) &&
                        (right(w) == _INVALID_POINT || color(right(w)) == RB_TREE_BLACK))
                    {
                        color(w) = RB_TREE_RED;
                        x = x_parent;
                        x_parent = parent(x_parent);
                    }
                    else
                    {
                        if (right(w) == _INVALID_POINT || color(right(w)) == RB_TREE_BLACK)
                        {
                            if (left(w) != _INVALID_POINT)
                            {
                                color(left(w)) = RB_TREE_BLACK;
                            }

                            color(w) = RB_TREE_RED;
                            _rb_tree_rotate_right(w, root());
                            w = right(x_parent);
                        }

                        color(w) = color(x_parent);
                        color(x_parent) = RB_TREE_BLACK;

                        if (right(w) != _INVALID_POINT)
                        {
                            color(right(w)) = RB_TREE_BLACK;
                        }

                        _rb_tree_rotate_left(x_parent, root());
                        break;
                    }
                }
                else
                {
                    size_t w = left(x_parent);

                    if (color(w) == RB_TREE_RED)
                    {
                        color(w) = RB_TREE_BLACK;
                        color(x_parent) = RB_TREE_RED;
                        _rb_tree_rotate_right(x_parent, root());
                        w = left(x_parent);
                    }

                    if ((right(w) == _INVALID_POINT || color(right(w)) == RB_TREE_BLACK) &&
                        (left(w) == _INVALID_POINT || color(left(w)) == RB_TREE_BLACK))
                    {
                        color(w) = RB_TREE_RED;
                        x = x_parent;
                        x_parent = parent(x_parent);
                    }
                    else
                    {
                        if (left(w) == _INVALID_POINT || color(left(w)) == RB_TREE_BLACK)
                        {
                            if (right(w) != _INVALID_POINT)
                            {
                                color(right(w)) = RB_TREE_BLACK;
                            }

                            color(w) = RB_TREE_RED;
                            _rb_tree_rotate_left(w, root());
                            w = left(x_parent);
                        }

                        color(w) = color(x_parent);
                        color(x_parent) = RB_TREE_BLACK;

                        if (left(w) != _INVALID_POINT)
                        {
                            color(left(w)) = RB_TREE_BLACK;
                        }

                        _rb_tree_rotate_right(x_parent, root());
                        break;
                    }
                }
            }

            if (x != _INVALID_POINT)
            {
                color(x) = RB_TREE_BLACK;
            }

        }

        return y;

    }

public:

    //�����ظ�key����Ĳ��뺯����Multimap��Multimap�����
    std::pair<iterator, bool>  insert_equal(const _value_type &v)
    {
        size_t y = header();
        size_t x = root();

        while (x != _INVALID_POINT)
        {
            y = x;
            x = _compare_key()( _extract_key()(v), key(x) ) ? left(x) : right(x);
        }

        return _insert(x, y, v);
    }

    //�ظ�key������ʧ�ܵĲ��뺯����Map��Sap�����
    std::pair<iterator, bool> insert_unique(const _value_type &v)
    {
        size_t y = header();
        size_t x = root();
        bool comp = true;

        while (x != _INVALID_POINT)
        {
            y = x;
            comp = _compare_key()( _extract_key()(v), key(x) );
            x = comp ? left(x) : right(x);
        }

        iterator j = iterator(y, this);

        if (comp)
        {
            if ( j == begin() )
            {
                return _insert(x, y, v);
            }
            else
            {
                --j;
            }
        }

        if ( _compare_key()(key(j.getserial()), _extract_key()(v)) )
        {
            return _insert(x, y, v);
        }

        return std::pair<iterator, bool>(j, false);
    }

    //ͨ��������ɾ��һ���ڵ�
    iterator erase(const iterator &pos)
    {
        size_t tmp = _erase(pos.getserial());
        destroy_node(pos.getserial());
        return iterator(tmp, this);
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

    //ͨ��keyɾ���ڵ㣬Map��Set��
    size_t erase_unique(const _key_type &k)
    {
        iterator it = find(k);

        if (it != end())
        {
            erase(it);
            return 1;
        }

        return 0;
    }

    //ͨ��valueɾ���ڵ㣬Map��Set��
    size_t erase_unique_value(const _value_type &v)
    {
        _extract_key get_key;
        return erase_unique(get_key(v));
    }

    //ͨ��keyɾ���ڵ㣬Multimap��Multiset��
    size_t erase_equal(const _key_type &k)
    {
        iterator it_l = lower_bound(k);
        iterator it_u = upper_bound(k);
        return erase(it_l, it_u);
    }

    //ͨ��ֵɾ���ڵ㣬Multimap��Multiset��
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
            if (!_compare_key()(key(x), k) )
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

        while ( x != _INVALID_POINT )
        {
            if ( !_compare_key()(key(x), k) )
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


protected:
    //index��Ҫ������������,һ����ͷָ�룬һ���ǿսڵ��ͷָ��
    static const size_t ADDED_NUM_OF_INDEX = 2;

protected:
    //RBTreeͷ��
    _shm_rb_tree_head                  *rb_tree_head_;

    //���е�ָ�붼�Ǹ��ݻ���ַ����õ���,���ڷ������,ÿ�γ�ʼ�������¼���
    //����������,
    _shm_rb_tree_index                 *index_base_;

    //��������ʼָ��,
    _value_type                         *data_base_;

    //ͷ�ڵ��ͷָ��,N+1������λ��ʾ
    _shm_rb_tree_index                 *head_index_;

    //�սڵ��ͷָ��,N+2������λ��ʾ����������right�ڵ������ӣ��ѿսڵ㴮������
    _shm_rb_tree_index                 *free_index_;

};

//��RBTreeʵ��SET��������multiset��set��ͨ����ͨ��insert�Լ�����
template<class _value_type, class _compare_key = std::less<_value_type> >
class mmap_set :
    public shm_rb_tree< _value_type, _value_type, smem_identity<_value_type>, _compare_key >
{
protected:
    //����ڹ����ڴ�ʹ��,û��new,����ͳһ��initialize ��ʼ��
    //�������,��������,���ǲ�������
    mmap_set<_value_type, _compare_key >(size_t numnode, void *pmmap, bool if_restore):
        shm_rb_tree<_value_type, _value_type, smem_identity<_value_type>, _compare_key>(numnode, pmmap, if_restore)
    {
        initialize(numnode, pmmap, if_restore);
    }

    ~mmap_set<_value_type, _compare_key >()
    {
    }

public:
    static mmap_set< _value_type, _compare_key  > *
    initialize(size_t &numnode, char *pmmap, bool if_restore = false)
    {
        return reinterpret_cast<mmap_set< _value_type, _compare_key  >*>(
                   shm_rb_tree<_value_type, _value_type, smem_identity<_value_type>, _compare_key>::initialize(numnode, pmmap, if_restore));
    }
};

//��RBTreeʵ��MAP��������multiset��set��ͨ����ͨ��insert�Լ�����
template<class _key_type, class _value_type, class _extract_key = mmap_select1st <std::pair <_key_type, _value_type> >, class _compare_key = std::less<_value_type>  >
class mmap_map :
    public shm_rb_tree< std::pair <_key_type, _value_type>, _key_type, _extract_key, _compare_key  >
{
protected:
    //����ڹ����ڴ�ʹ��,û��new,����ͳһ��initialize ��ʼ��
    //�������,��������,���ǲ�������
    mmap_map<_key_type, _value_type, _extract_key, _compare_key >(size_t numnode, void *pmmap, bool if_restore):
        shm_rb_tree< std::pair <_key_type, _value_type>, _key_type, _extract_key, _compare_key  >(numnode, pmmap, if_restore)
    {
        initialize(numnode, pmmap, if_restore);
    }

    ~mmap_map<_key_type, _value_type, _extract_key, _compare_key >()
    {
    }
public:
    static mmap_map< _key_type, _value_type, _extract_key, _compare_key  > *
    initialize(size_t &numnode, char *pmmap, bool if_restore = false)
    {
        return reinterpret_cast<mmap_map< _key_type, _value_type, _extract_key, _compare_key  >*>(
                   shm_rb_tree< std::pair <_key_type, _value_type>, _key_type, _extract_key, _compare_key>::initialize(numnode, pmmap, if_restore));
    }
    //[]�����������ŵ��ȱ�㣬����ʹ��
    _value_type &operator[](const _key_type &key)
    {
        return (find_or_insert(std::pair<_key_type, _value_type >(key, _value_type()))).second;
    }
};

};

#endif //ZCE_LIB_SHM_RB_TREE_H_
