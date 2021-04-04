/*!
* @copyright  2004-2018  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_trie_tree.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2018��9��9��
* @brief      һ���ֵ���(ORǰ׺��)��ʵ�֡��ֵ�������HW�ļƷѵ�ʱ��ʵ�ֹ�����ʱ��ҪΪ�����Ʒѡ�
*             ʱ��10����ڻ���Ӱ���з�����һ�����ó����������Ұ�Ŀ������������Ҳ��Ҫһ������
*             ���ơ��ź��á�
*
* @details
*
*
*
* @note
*
*   ��������� ��ԥ
*   ��Ҫ���Ҵ�������
*   �ҵĹ�����Զ��
*   Ϊʲô����
*   ����Զ�� ����
*   Ϊ����շ����С��
*   Ϊ��ɽ��������СϪ
*   Ϊ�˿����Ĳ�ԭ
*   ����Զ�� ����
*   ���л���
*   Ϊ�����е������ �����
*   ��Ҫ���Ҵ�������
*   �ҵĹ�����Զ��
*   Ϊʲô����
*   Ϊʲô����Զ��
*   Ϊ�������е������
*   ��Ҫ���Ҵ�������
*   �ҵĹ�����Զ��
*   Ϊʲô����
*   ����Զ�� ���� [4]
*
*/

#pragma once



namespace zce
{

//============================================================================================

/*!
@brief      TRIE TREE��������ͷ�������ڴ�Ŵ˴Σ�����������������
            ʹ�õ�NODE������FREE��NODE������
*/
struct _shm_trie_tree_head
{
    //�Ҳ��ܶ�ģ������Ԫ�����ˣ���ʼ���ų�����
public:

    ///�ڴ����ĳ���
    uint32_t         size_of_mmap_ = 0;
    ///NODE������
    uint32_t         num_of_node_ = 0;

    ///FREE��NODE����
    uint32_t         size_free_node_ = 0;
    ///USE��NODE����
    uint32_t         size_use_node_ = 0;
};

//============================================================================================

///TRIE TREE������NODE������
template <class _meta_type> class _shm_trie_tree_node
{
public:

    ///
    _meta_type  meta_;
    ///�ֽڵ�
    uint32_t    idx_brother_ = static_cast<uint32_t>(-1);
    ///�ӽڵ�
    uint32_t    idx_children_ = static_cast<uint32_t>(-1);

};


//============================================================================================
/*!
@brief      �ֵ���(ORǰ׺��)�������ڹ����ڴ棨��ͨ�ڴ�Ҳ�У���һ���ڴ��У�ʹ�ã�
            Ҳ���Զ���̹�������Ȼͬ����������飬���Լ����ǣ�

            ������Ҫ�ĵ�ַ�ռ��С˵����ÿ��node������Ҫ2��shm_index_t��С�Ŀռ䣬

@tparam     _meta_type Ԫ���ͣ�ÿ��NODE����������
*/
template <class _meta_type> class smem_trie_tree
{
protected:

    //index��Ҫ������������,һ����ʾ
    static const size_t ADDED_NUM_OF_ROOT = 1;

protected:

    //���е�ָ�붼�Ǹ��ӻ���ַ����õ���,���ڷ������,ÿ�γ�ʼ�������¼���
    //�ڴ������ַ
    char                            *smem_base_;

    //��������ʼָ��,
    _shm_trie_tree_node<_meta_type> *data_base_;

    //����������ָ��,
    _shm_trie_tree_node             *index_base_;

    //LIST��ͷ����ָ��
    _shm_trie_tree_head             *list_head_;

    //FREE NODE��ͷָ��,N+1������λ��ʾ
    _shm_trie_tree_node             *free_node_;


public:

    ///����ڹ����ڴ�ʹ��,û��new,����ͳһ��initialize ��ʼ��
    ///�������,��������,���ǲ�������
    smem_trie_tree<_meta_type>(size_t numnode, void *pmmap, bool if_restore):
        _shm_memory_base(pmmap),
        list_head_(NULL),
        index_base_(NULL),
        data_base_(NULL),
        freenode_(NULL),
        usenode_(NULL)
    {
    }

    smem_trie_tree<_meta_type>():
        _shm_memory_base(NULL),
        list_head_(NULL),
        index_base_(NULL),
        data_base_(NULL),
        freenode_(NULL),
        usenode_(NULL)
    {
    }

    ~smem_list<_meta_type>()
    {
    }

    //ֻ����,��ʵ��,
    const smem_list<_meta_type> &operator=(const smem_list<_meta_type> &others);

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
    size_t create_node(const _meta_type &val)
    {
        //���û�пռ���Է���
        if (list_head_->size_free_node_ == 0)
        {
            return _INVALID_POINT;
        }

        //������ȡ1������
        size_t node = freenode_->idx_next_;

        freenode_->idx_next_ = (index_base_ + node)->idx_next_;
        //
        (index_base_ + freenode_->idx_next_)->idx_prev_ = (index_base_ + node)->idx_prev_;

        //��placement new��������
        new (data_base_ + node) _meta_type(val) ;

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
    }




};

};

#endif //ZCE_LIB_SHARE_MEM_TRIE_TREE_H_

