/*!
* @copyright  2004-2021  Apache License, Version 2.0 FULLSAIL
* @filename   zce/pool/object_pool.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date
* @brief      对象池子，用于对象分配，通过alloc得到对象的指针
*             free归还对象给池子，
* @details    对象池内部没有日志。因为对象池要给日志类使用，
*             如果有日志，会形成交叉引用。
*
*
* @note
*
*/
#pragma once
namespace zce
{
/*!
* @brief      （单）对象池子，可以用于分配对象，避免反复使用new or delete
*
* @tparam     LOCK 锁，可以是zce::null_lock,也可以是std::mutex
* @tparam     T 对象类型，
*/
template<typename LOCK, typename T>
class shareptr_pool
{
public:
    //!对象池子对象
    typedef T object;

    //!构造函数，析构函数，赋值函数
    shareptr_pool() = default;
    ~shareptr_pool() = default;

    /*!
    * @brief
    * @return     bool 是否初始化成果
    * @param      init_pool_size
    * @param      extend_size  每次扩展的尺寸
    * @param      new_fun      T*的new函数，如果为空直接使用new，
    * @note
    */
    void initialize(size_t init_pool_size,
                    size_t extend_size,
                    std::function <T* () >* new_fun = nullptr)
    {
        std::lock_guard<LOCK> lock(lock_);
        init_pool_size_ = init_pool_size;
        extend_size_ = extend_size;
        if (new_fun)
        {
            new_fun_ = (*new_fun);
        }
        extend(init_pool_size_);
        return;
    }

    //!最后的销毁处理
    void terminate(size_t &leak_mem)
    {
        std::lock_guard<LOCK> lock(lock_);
        size_t sz = obj_pool_.size();
        leak_mem = 0;
        for (size_t i = 0; i < sz; i++)
        {
            if (obj_pool_[i])
            {
                //记录泄露
                if (obj_pool_[i].use_count() > 1)
                {
                    ++leak_mem;
                }
                //反正我不管了，
                obj_pool_[i].reset();
            }
        }
    }

    //!分配一个对象
    std::shared_ptr<T> alloc_object(bool &extend_pool)
    {
        std::lock_guard<LOCK> lock(lock_);
        extend_pool = false;
        size_t sz = obj_pool_.size(), i = 0;
        for (; i < sz; i++)
        {
            find_pos_ = (find_pos_ + 1) % sz;
            
            assert(obj_pool_[find_pos_]);
            //第一次的时候new分配一个数据
            if (obj_pool_[find_pos_].use_count() == 1)
            {
                return obj_pool_[find_pos_];
            }
        }
        //地主没有余量了
        if (i == sz)
        {
            extend_pool = true;
            extend(extend_size_);
        }

        return  obj_pool_[sz];
    }

    //!扩展池子的容量
    void extend(size_t extend_size)
    {
        size_t pool_size = obj_pool_.size();
        obj_pool_.resize(pool_size + extend_size);
        for (i = 0; i < extend_size; ++i)
        {
            if (new_fun_)
            {
                obj_pool_[pool_size+i] = std::shared_ptr<T>(new_fun_());
            }
            else
            {
                obj_pool_[pool_size + i] = std::shared_ptr<T>(new T());
            }
        }
        return;
    }

    //有点耗时的操作，会检测所有的
    size_t size()
    {
        size_t use = 0;
        std::lock_guard<LOCK> lock(lock_);
        size_t sz = obj_pool_.size(), i = 0;
        for (; i < sz; i++)
        {
            if (obj_pool_[i])
            {
                if (obj_pool_[i].use_count() > 1)
                {
                    ++use;
                }
            }
        }
        return use;
    }

    inline size_t capacity()
    {
        std::lock_guard<LOCK> lock(lock_);
        return obj_pool_.size();
    }
    inline bool empty()
    {
        std::lock_guard<LOCK> lock;
        return size() == 0;
    }
    inline bool full()
    {
        std::lock_guard<LOCK> lock(lock_);
        return size() == capacity();
    }

    //也许未来可以加个收缩

protected:

    //! 池子初始化大小
    size_t init_pool_size_ = 0;
    //! 扩展的尺寸
    size_t extend_size_ = 0;
    //! 每次寻找的时候，从find_pos_开始寻找，如果每次从0开始慢
    size_t find_pos_ = 0;

    //! 对象池子
    std::vector<std::shared_ptr<T> >  obj_pool_;

    //! T的初始化函数，
    std::function <T* ()> new_fun_;
    //! 锁记录
    LOCK lock_;
};
}
