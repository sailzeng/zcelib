#ifndef ZCE_LIB_BOOST_MPL_H_
#define ZCE_LIB_BOOST_MPL_H_

//今天看到一段，白发三千丈，代码写万行，不知程序里，何处话凄凉，

namespace ZCE_LIB
{

//==================================================================

//这个方法来自boost ，
//JiangweiSun  在其个人blog 上 http://www.suninf.net/SFINAE-and-enable_if/ 很好的文章
template<typename T>
struct helper
{
    typedef void type;
};


// 检测类型T是否具有内部的名为type的typedef

// 用默认模板参数指示默认情况
template<typename T, typename U = void>
struct has_def_type_impl
{
    static const bool value = false;
};

//偏特化，优先去适配从T萃取类型， T有内置的type时选择
template<typename T>
struct has_def_type_impl < T, typename helper<typename T::type>::type >
{
    static const bool value = true;
};

// 检测类型T是否具有内部的名为type的typedef
template<typename T>
struct has_def_type : has_def_type_impl < T >
{
};


//==================================================================
//试图萃取容器的迭代器类型
template <class container_type>
struct container_traits
{
    typedef typename container_type::iterator   iterator;
    typedef typename container_type::value_type   value_type;
};

//对数组容器进行特化
template <typename T>
struct container_traits< T *>
{
    typedef typename T  *iterator;
    typedef typename T   value_type;
};

//==================================================================

// type trait
template<bool bool_vale, typename type_a, typename type_b>
struct if_
{
};
template<typename type_a, typename type_b>
struct if_ < true, type_a, type_b >
{
    typedef type_a type;
};
template<typename type_a, typename type_b>
struct if_ < false, type_a, type_b >
{
    typedef type_b type;
};

//==================================================================


template<typename input_iter>
struct is_random_access_iterator
{
    static const bool value = typeid(std::iterator_traits<input_iter>::iterator_category) == typeid(std::random_access_iterator_tag);
};


};

//==================================================================



#endif //ZCE_LIB_BOOST_MPL_H_
