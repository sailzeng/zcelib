#ifndef ZCE_LIB_BOOST_MPL_H_
#define ZCE_LIB_BOOST_MPL_H_

//今天看到一段，白发三千丈，代码写万行，不知程序里，何处话凄凉，

namespace zce
{
//----------------------------------------------------------------------------

//这个方法来自boost ，
//JiangweiSun  在其个人blog 上 http://www.suninf.net/SFINAE-and-enable_if/ 很好的文章

template<typename T>
struct has_def_typehelper
{
    using type = void;
};

// 检测类型T是否具有内部的名为type的typedef

// 用默认模板参数指示默认情况，默认会推导到这儿false
template<typename T, typename U = void>
struct has_def_type_impl
{
    static const bool value = false;
};

//偏特化，优先去适配从T萃取类型， T有内置的type时选择
template<typename T>
struct has_def_type_impl < T, typename has_def_typehelper<typename T::type>::type >
{
    static const bool value = true;
};

//检测类型T是否具有内部的名为type的typedef
template<typename T>
struct has_def_type : has_def_type_impl < T >
{
};

//----------------------------------------------------------------------------
//试图萃取容器的迭代器类型
template <class container_type>
struct container_traits
{
    using iterator = typename container_type::iterator;
    using  value_type = typename container_type::value_type;
};

//对数组容器进行特化
template <typename T>
struct container_traits<T*>
{
    using iterator = T*;
    using value_type = T;
};

//----------------------------------------------------------------------------

//std::enable_if的补充，TRUE用类型type_a，FALSE用type_b
template<bool bool_vale, typename type_a, typename type_b>
struct if_
{
};
template<typename type_a, typename type_b>
struct if_ < true, type_a, type_b >
{
    using type = type_a;
};
template<typename type_a, typename type_b>
struct if_ < false, type_a, type_b >
{
    using type = type_b;
};

//----------------------------------------------------------------------------

template<typename input_iter>
struct is_random_access_iterator
{
    static const bool value = typeid(std::iterator_traits<input_iter>::iterator_category) == \
        typeid(std::random_access_iterator_tag);
};
//----------------------------------------------------------------------------

//关注一下 std::invoke 函数std::result_of, std::invoke_result，std::mem_fun
//如果使用tuple作为参数，可以考虑std::apply std::make_from_tuple

template<typename fun_t, typename para_tuple, std::size_t... I>
auto g_func_invoke_impl(fun_t& f,
                        const para_tuple& a,
                        std::index_sequence<I...>)
{
    return f(std::get<I>(a)...);
}

//全局函数调用调用tuple参数
template<typename fun_t, typename para_tuple>
auto g_func_tuplearg_invoke(fun_t&& f, const para_tuple& a)
{
    static constexpr auto t_count = std::tuple_size<para_tuple>::value;
    //make_index_sequence 是得到size_t的序列，方便g_fun_invoke_impl的展开调用
    //这个扩展TMD太神奇了，我对这个展开语法也表示惊诧。
    return g_func_invoke_impl(std::forward <fun_t>(f),
                              a,
                              std::make_index_sequence<t_count>());
}

template<typename fun_t, typename ...args_type>
auto g_func_invoke(fun_t&& f, args_type &&...datalist)
{
    auto a = std::make_tuple(std::forward<args_type>(datalist)...);
    g_func_tuplearg_invoke(f, a);
}

template<typename class_type,
    typename fun_t,
    typename para_tuple,
    std::size_t... I>
auto mem_func_invoke_impl(class_type* obj,
                          fun_t& f,
                          const para_tuple& a,
                          std::index_sequence<I...>)
{
    //注意这儿一定要加(),否则编译器会抓狂，让我找错误找了一天
    return (obj->*f)(std::get<I>(a)...);
}

//成员函数调用封装
template<typename class_type, typename fun_t, typename para_tuple>
auto mem_func_tuplearg_invoke(class_type* obj, fun_t& f, const para_tuple& a)
{
    auto t_count = std::tuple_size<para_tuple>::value;
    return mem_func_invoke_impl(obj,
                                f,
                                a,
                                std::make_index_sequence<t_count>());
}

template<typename class_type, typename fun_t, typename ...args_type>
auto mem_func_invoke(class_type* obj, fun_t& f, args_type && ...datalist)
{
    auto a = std::make_tuple(std::forward<args_type>(datalist)...);
    return mem_func_invoke_impl(obj, f, a);
}

//----------------------------------------------------------------------------
template<typename T, typename ...Args>
struct new_helper
{
    //Args 没有使用Args&&，因为我这儿要明确指明参数类型调用
    static T* invoke(Args... args)
    {
        return new T(args...);
    }
};

//----------------------------------------------------------------------------
template <typename T, typename = void>
struct is_container : std::false_type {};

template <typename T>
struct is_container<
    T,
    std::void_t<
    decltype(std::declval<T>().begin()),
    decltype(std::declval<T>().end())
    >
> : std::true_type
{
};

// -----------------------------
// 是否是支持 push_back 的容器
// -----------------------------
template <typename T, typename = void>
struct has_push_back : std::false_type {};

template <typename T>
struct has_push_back<
    T,
    std::void_t<
    decltype(std::declval<T>().push_back(std::declval<typename T::value_type>()))
    >
> : std::true_type
{
};

//! 是否拥有 key_type（map/set）
template <typename T, typename = void>
struct has_key_type : std::false_type {};

template <typename T>
struct has_key_type<
    T,
    std::void_t<typename T::key_type>
> : std::true_type
{
};

//! 是否是单类型容器（vector、list 等）
template <typename T>
struct is_single_type_container : std::bool_constant<
    is_container<T>::value &&
    !has_key_type<T>::value
>
{
};

template <typename T>
struct is_associative_container : std::bool_constant<
    is_container<T>::value&&
    has_key_type<T>::value
>
{
};
}

#endif //ZCE_LIB_BOOST_MPL_H_
