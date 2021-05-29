#ifndef SOARING_LIB_MMAPALLOC_H_
#define SOARING_LIB_MMAPALLOC_H_

template <typename  POOL_OBJ>
class mmap_allocator //: public allocator<POOL_OBJ>
{
public:
    typedef POOL_OBJ                 value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t       size_type;
    typedef std::ptrdiff_t    difference_type;

    mmap_allocator() {}
    mmap_allocator(const mmap_allocator&) {}
    ~mmap_allocator() {}

    template <typename U>
    struct rebind
    {
        typedef my_allocator <U> other;
    };

    struct listnode
    {
        listnode* next;
        unsigned char   state;
        POOL_OBJ               data;
    };

    pointer address(reference x) const
    {
        return &x;
    }
    const_pointer address(const_reference x) const
    {
        return &x;
    }

    pointer allocate(size_type n, const_pointer = 0)
    {
        void* p = std::malloc(n * sizeof(POOL_OBJ));

        if (!p)
        {
            throw std::bad_alloc();
        }

        return static_cast<pointer>(p);
    }

    void deallocate(pointer p, size_type)
    {
        std::free(p);
    }

    size_type max_size() const
    {
        return freenum;
    }

    void construct(pointer p, const value_type& x)
    {
        new (p) value_type(x);
    }

    void destroy(pointer p)
    {
        p->~value_type();
    }

private:
    void operator = (const mmap_allocator&);

    static listnode* freelist;

    static size_t freenum;
};

template<typename _T1, typename _T2>
inline bool operator == (const mmap_allocator<_T1>&, const mmap_allocator<_T2>&)
{
    return false;
}

template<typename _T1, typename _T2>
inline bool operator != (const mmap_allocator<_T1>&, const mmap_allocator<_T2>&)
{
    return true;
}

template<typename POOL_OBJ>
static

#endif