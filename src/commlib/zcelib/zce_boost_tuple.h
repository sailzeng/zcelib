/*!
@copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
@filename   zce_boost_tuple.h
@author     Chrislin <Chrislin@tencent.com>
@version
@date       2009年1月1日
@brief      tuple的功能类似pair，但可以放置多个变量，参考boost实现的一个类，

@details

@note

*/

#ifndef ZCE_LIB_BOOST_TUPLE_H_
#define ZCE_LIB_BOOST_TUPLE_H_

#include "zce_shm_predefine.h"

namespace ZCE_LIB
{
struct null_type {};

inline const null_type cnull()
{
    return null_type();
}

template <class T> struct access_traits
{
    typedef const T &const_type;
    typedef T &non_const_type;
};

template <class T> struct access_traits<T &>
{
    typedef T &const_type;
    typedef T &non_const_type;
};

//前向声明 cons
template <class HT, class TT> struct cons;
//前向声明 tuple
template < class T0 = null_type, class T1 = null_type, class T2 = null_type, class T3 = null_type,
         class T4 = null_type, class T5 = null_type, class T6 = null_type, class T7 = null_type,
         class T8 = null_type, class T9 = null_type >
class tuple;

//计算tuple类型参数长度
template<class T> struct length
{
    static const int value = 1 + length<typename T::tail_type>::value;
};
template<> struct length<tuple<> >
{
    static const int value = 0;
};
template<> struct length<null_type>
{
    static const int value = 0;
};

//取得tuple第N个参数类型
template<int N, class T>
struct element
{
    typedef typename element < N - 1, typename T::tail_type >::type type;
};
template<class T> struct element<0, T>
{
    typedef typename T::head_type type;
};

//取得tuple第N成员数据
template <int N>
struct get_class
{
    template < class RET, class HT, class TT >
    inline static RET get(const cons<HT, TT>& t)
    {
        return get_class < N - 1 >::NESTED_TEMPLATE get<RET>(t.tail);
    }

    template < class RET, class HT, class TT >
    inline static RET get(cons<HT, TT>& t)
    {
        return get_class < N - 1 >::NESTED_TEMPLATE get<RET>(t.tail);
    }
};

template<>
struct get_class<0>
{
    template<class RET, class HT, class TT>
    inline static RET get(const cons<HT, TT>& t)
    {
        return t.head;
    }

    template<class RET, class HT, class TT>
    inline static RET get(cons<HT, TT>& t)
    {
        return t.head;
    }
};

template<int N, class HT, class TT>
inline typename access_traits< typename element<N, cons<HT, TT> >::type >::non_const_type
get(cons<HT, TT>& c)
{
    return get_class<N>::NESTED_TEMPLATE get <
           typename access_traits<typename element<N, cons<HT, TT> >::type>::non_const_type,
           HT, TT > (c);
}

template<int N, class HT, class TT>
inline typename access_traits<typename element<N, cons<HT, TT> >::type>::const_type
get(const cons<HT, TT>& c)
{
    return get_class<N>::NESTED_TEMPLATE get <
           typename access_traits<typename element<N, cons<HT, TT> >::type>::const_type,
           HT, TT
           > (c);
}

//实际的数据在这里面
template <typename HT, typename TT>
struct cons
{
    typedef HT head_type;
    typedef TT tail_type;

    head_type head;
    tail_type tail;

    typename access_traits<head_type>::non_const_type
    get_head()
    {
        return head;
    }

    typename access_traits<tail_type>::non_const_type
    get_tail()
    {
        return tail;
    }

    typename access_traits<head_type>::const_type
    get_head() const
    {
        return head;
    }

    typename access_traits<tail_type>::const_type
    get_tail() const
    {
        return tail;
    }

    cons() : head(), tail() {}

    cons(const head_type &h, const tail_type &t): head (h), tail(t) {}

    template < class T1, class T2, class T3, class T4, class T5,
             class T6, class T7, class T8, class T9, class T10 >
    cons( T1 &t1, T2 &t2, T3 &t3, T4 &t4, T5 &t5, T6 &t6, T7 &t7, T8 &t8, T9 &t9, T10 &t10 )
        : head (t1), tail (t2, t3, t4, t5, t6, t7, t8, t9, t10, cnull()) {}

    template < class T2, class T3, class T4, class T5,
             class T6, class T7, class T8, class T9, class T10 >
    cons( const null_type & /*t1*/, T2 &t2, T3 &t3, T4 &t4, T5 &t5,
          T6 &t6, T7 &t7, T8 &t8, T9 &t9, T10 &t10 )
        : head (), tail (t2, t3, t4, t5, t6, t7, t8, t9, t10, cnull()) {}

    template <class HT2, class TT2>
    cons( const cons<HT2, TT2>& u ) : head(u.head), tail(u.tail) {}

    template <class HT2, class TT2>
    cons &operator=( const cons<HT2, TT2>& u )
    {
        head = u.head;
        tail = u.tail;
        return *this;
    }

    cons &operator=(const cons &u)
    {
        head = u.head;
        tail = u.tail;
        return *this;
    }

    template <class T1, class T2>
    cons &operator=( const std::pair<T1, T2>& u )
    {
        STATIC_ASSERT(length<cons>::value == 2); // check length = 2
        head = u.first;
        tail.head = u.second;
        return *this;
    }

    template <int N>
    typename access_traits< typename element<N, cons<HT, TT> >::type >::non_const_type
    get()
    {
        return ZCE_LIB::get<N>(*this);
    }

    template <int N>
    typename access_traits< typename element<N, cons<HT, TT> >::type >::const_type
    get() const
    {
        return ZCE_LIB::get<N>(*this);
    }
};

template <class HT>
struct cons<HT, null_type>
{
    typedef HT head_type;
    typedef null_type tail_type;
    typedef cons<HT, null_type> self_type;
    head_type head;

    typename access_traits<head_type>::non_const_type
    get_head()
    {
        return head;
    }

    null_type get_tail()
    {
        return null_type();
    }

    typename access_traits<head_type>::const_type
    get_head() const
    {
        return head;
    }

    const null_type get_tail() const
    {
        return null_type();
    }

    cons() : head() {}

    cons(const head_type &h, const null_type& = null_type()): head (h) {}

    template<class T1>
    cons(T1 &t1, const null_type &, const null_type &, const null_type &,
         const null_type &, const null_type &, const null_type &,
         const null_type &, const null_type &, const null_type &): head (t1) {}

    cons(const null_type &,
         const null_type &, const null_type &, const null_type &,
         const null_type &, const null_type &, const null_type &,
         const null_type &, const null_type &, const null_type &): head () {}

    template <class HT2>
    cons( const cons<HT2, null_type>& u ) : head(u.head) {}

    template <class HT2>
    cons &operator=(const cons<HT2, null_type>& u )
    {
        head = u.head;
        return *this;
    }

    cons &operator=(const cons &u)
    {
        head = u.head;
        return *this;
    }

    template <int N>
    typename access_traits<typename element<N, self_type>::type>::non_const_type
    get()
    {
        return ZCE_LIB::get<N>(*this);
    }

    template <int N>
    typename access_traits<typename element<N, self_type>::type>::const_type
    get() const
    {
        return ZCE_LIB::get<N>(*this);
    }
};

template < class T0, class T1, class T2, class T3, class T4,
         class T5, class T6, class T7, class T8, class T9 >
struct map_tuple_to_cons
{
    typedef cons < T0, typename map_tuple_to_cons < T1, T2, T3, T4, T5,
            T6, T7, T8, T9, null_type >::type > type;
};

template <>
struct map_tuple_to_cons < null_type, null_type, null_type, null_type, null_type,
        null_type, null_type, null_type, null_type, null_type >
{
    typedef null_type type;
};

//元组，最多支持长度为10的元数据
template < class T0, class T1, class T2, class T3, class T4,
         class T5, class T6, class T7, class T8, class T9 >
class tuple : public map_tuple_to_cons<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>::type
{
public:
    typedef typename map_tuple_to_cons<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>::type inherited;
    typedef typename inherited::head_type head_type;
    typedef typename inherited::tail_type tail_type;

    tuple() {}

    tuple(T0 t0): inherited(t0, cnull(), cnull(), cnull(), cnull(),
                                cnull(), cnull(), cnull(), cnull(), cnull()) {}

    tuple(T0 t0, T1 t1): inherited(t0, t1, cnull(), cnull(), cnull(),
                                       cnull(), cnull(), cnull(), cnull(), cnull()) {}

    tuple(T0 t0, T1 t1, T2 t2): inherited(t0, t1, t2, cnull(), cnull(),
                                              cnull(), cnull(), cnull(), cnull(), cnull()) {}

    tuple(T0 t0, T1 t1, T2 t2, T3 t3): inherited(t0, t1, t2, t3, cnull(),
                                                     cnull(), cnull(), cnull(), cnull(), cnull()) {}

    tuple(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4): inherited(t0, t1, t2, t3, t4,
                                                            cnull(), cnull(), cnull(), cnull(), cnull()) {}

    tuple(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
        : inherited(t0, t1, t2, t3, t4, t5, cnull(), cnull(), cnull(), cnull()) {}

    tuple(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
        : inherited(t0, t1, t2, t3, t4, t5, t6, cnull(), cnull(), cnull()) {}

    tuple(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)
        : inherited(t0, t1, t2, t3, t4, t5, t6, t7, cnull(), cnull()) {}

    tuple(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)
        : inherited(t0, t1, t2, t3, t4, t5, t6, t7, t8, cnull()) {}

    tuple(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9)
        : inherited(t0, t1, t2, t3, t4, t5, t6, t7, t8, t9) {}

    template<class U1, class U2>
    tuple(const cons<U1, U2>& p) : inherited(p) {}

    template <class U1, class U2>
    tuple &operator=(const cons<U1, U2>& k)
    {
        inherited::operator=(k);
        return *this;
    }

    template <class U1, class U2>
    tuple &operator=(const std::pair<U1, U2>& k)
    {
        STATIC_ASSERT(length<tuple>::value == 2);// check_length = 2
        this->head = k.first;
        this->tail.head = k.second;
        return *this;
    }
};

template <>
class tuple < null_type, null_type, null_type, null_type, null_type,
    null_type, null_type, null_type, null_type, null_type > : public null_type
{
public:
    typedef null_type inherited;
};

template < class T0 = null_type, class T1 = null_type, class T2 = null_type, class T3 = null_type,
         class T4 = null_type, class T5 = null_type, class T6 = null_type, class T7 = null_type,
         class T8 = null_type, class T9 = null_type >
struct make_tuple_mapper
{
    typedef tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> type;
};

//make_tuple function templates
inline tuple<> make_tuple()
{
    return tuple<>();
}

template<class T0>
inline typename make_tuple_mapper<T0>::type
make_tuple(const T0 &t0)
{
    typedef typename make_tuple_mapper<T0>::type t;
    return t(t0);
}

template<class T0, class T1>
inline typename make_tuple_mapper<T0, T1>::type
make_tuple(const T0 &t0, const T1 &t1)
{
    typedef typename make_tuple_mapper<T0, T1>::type t;
    return t(t0, t1);
}

template<class T0, class T1, class T2>
inline typename make_tuple_mapper<T0, T1, T2>::type
make_tuple(const T0 &t0, const T1 &t1, const T2 &t2)
{
    typedef typename make_tuple_mapper<T0, T1, T2>::type t;
    return t(t0, t1, t2);
}

template<class T0, class T1, class T2, class T3>
inline typename make_tuple_mapper<T0, T1, T2, T3>::type
make_tuple(const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3)
{
    typedef typename make_tuple_mapper<T0, T1, T2, T3>::type t;
    return t(t0, t1, t2, t3);
}

template<class T0, class T1, class T2, class T3, class T4>
inline typename make_tuple_mapper<T0, T1, T2, T3, T4>::type
make_tuple(const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4)
{
    typedef typename make_tuple_mapper<T0, T1, T2, T3, T4>::type t;
    return t(t0, t1, t2, t3, t4);
}

template<class T0, class T1, class T2, class T3, class T4, class T5>
inline typename make_tuple_mapper<T0, T1, T2, T3, T4, T5>::type
make_tuple(const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5)
{
    typedef typename make_tuple_mapper<T0, T1, T2, T3, T4, T5>::type t;
    return t(t0, t1, t2, t3, t4, t5);
}

template<class T0, class T1, class T2, class T3, class T4, class T5, class T6>
inline typename make_tuple_mapper<T0, T1, T2, T3, T4, T5, T6>::type
make_tuple(const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3,
           const T4 &t4, const T5 &t5, const T6 &t6)
{
    typedef typename make_tuple_mapper<T0, T1, T2, T3, T4, T5, T6>::type t;
    return t(t0, t1, t2, t3, t4, t5, t6);
}

template < class T0, class T1, class T2, class T3, class T4, class T5, class T6,
         class T7 >
inline typename make_tuple_mapper<T0, T1, T2, T3, T4, T5, T6, T7>::type
make_tuple(const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3,
           const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7)
{
    typedef typename make_tuple_mapper<T0, T1, T2, T3, T4, T5, T6, T7>::type t;
    return t(t0, t1, t2, t3, t4, t5, t6, t7);
}

template<class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
inline typename make_tuple_mapper <T0, T1, T2, T3, T4, T5, T6, T7, T8>::type
make_tuple(const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4,
           const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8)
{
    typedef typename make_tuple_mapper<T0, T1, T2, T3, T4, T5, T6, T7, T8>::type t;
    return t(t0, t1, t2, t3, t4, t5, t6, t7, t8);
}

template<class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
inline typename make_tuple_mapper <T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>::type
make_tuple(const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4,
           const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const T9 &t9)
{
    typedef typename make_tuple_mapper<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>::type t;
    return t(t0, t1, t2, t3, t4, t5, t6, t7, t8, t9);
}
}

#endif //ZCE_LIB_BOOST_TUPLE_H_

