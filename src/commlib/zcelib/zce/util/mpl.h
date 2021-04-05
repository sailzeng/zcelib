#ifndef ZCE_LIB_BOOST_MPL_H_
#define ZCE_LIB_BOOST_MPL_H_

//���쿴��һ�Σ��׷���ǧ�ɣ�����д���У���֪������δ���������

namespace zce
{

//----------------------------------------------------------------------------

//�����������boost ��
//JiangweiSun  �������blog �� http://www.suninf.net/SFINAE-and-enable_if/ �ܺõ�����

template<typename T>
struct has_def_typehelper
{
    typedef void type;
};

// �������T�Ƿ�����ڲ�����Ϊtype��typedef

// ��Ĭ��ģ�����ָʾĬ�������Ĭ�ϻ��Ƶ������false
template<typename T, typename U = void>
struct has_def_type_impl
{
    static const bool value = false;
};

//ƫ�ػ�������ȥ�����T��ȡ���ͣ� T�����õ�typeʱѡ��
template<typename T>
struct has_def_type_impl < T, typename has_def_typehelper<typename T::type>::type >
{
    static const bool value = true;
};

//�������T�Ƿ�����ڲ�����Ϊtype��typedef
template<typename T>
struct has_def_type : has_def_type_impl < T >
{
};

//----------------------------------------------------------------------------
//��ͼ��ȡ�����ĵ���������
template <class container_type>
struct container_traits
{
    typedef typename container_type::iterator   iterator;
    typedef typename container_type::value_type   value_type;
};

//���������������ػ�
template <typename T>
struct container_traits<T *>
{
    typedef T  *iterator;
    typedef T   value_type;
};

//----------------------------------------------------------------------------

//std::enable_if�Ĳ��䣬TRUE������type_a��FALSE��type_b
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

//----------------------------------------------------------------------------


template<typename input_iter>
struct is_random_access_iterator
{
    static const bool value = typeid(std::iterator_traits<input_iter>::iterator_category) == typeid(std::random_access_iterator_tag);
};
//----------------------------------------------------------------------------

template<typename fun_t, typename para_tuple, std::size_t... I>
auto g_func_invoke_impl(fun_t &f, const para_tuple &a, std::index_sequence<I...>)
{
    return f(std::get<I>(a)...);
}

//ȫ�ֺ������õ���tuple����
template<typename fun_t, typename para_tuple>
auto g_func_tuplearg_invoke(fun_t &&f, const para_tuple &a)
{
    static constexpr auto t_count = std::tuple_size<para_tuple>::value;
    //make_index_sequence �ǵõ�size_t�����У�����g_fun_invoke_impl��չ������
    //�����չTMD̫�����ˣ��Ҷ����չ���﷨Ҳ��ʾ���
    return g_func_invoke_impl(std::forward <fun_t>(f),
                              a,
                              std::make_index_sequence<t_count>());
}

template<typename fun_t, typename ...args_type>
auto g_func_invoke(fun_t &&f, args_type &&...datalist)
{
    auto a = std::make_tuple(std::forward<args_type>(datalist)...);
    g_func_tuplearg_invoke(f, a);
}

template<typename class_type, typename fun_t, typename para_tuple, std::size_t... I>
auto mem_func_invoke_impl(class_type *obj, fun_t &f, const para_tuple &a, std::index_sequence<I...>)
{
    //ע�����һ��Ҫ��(),�����������ץ��
    return (obj->*f)(std::get<I>(a)...);
}

//��Ա�������÷�װ
template<typename class_type, typename fun_t, typename para_tuple>
auto mem_func_tuplearg_invoke(class_type *obj, fun_t &f, const para_tuple &a)
{
    auto t_count = std::tuple_size<para_tuple>::value;
    return mem_func_invoke_impl(obj, 
                                f, 
                                a,
                                std::make_index_sequence<t_count>());
}

template<typename class_type,typename fun_t,typename ...args_type>
auto mem_func_invoke(class_type* obj,fun_t& f,args_type&& ...datalist)
{
    auto a=std::make_tuple(std::forward<args_type>(datalist)...);
    return mem_func_invoke_impl(obj,f,a);
}

};






#endif //ZCE_LIB_BOOST_MPL_H_
