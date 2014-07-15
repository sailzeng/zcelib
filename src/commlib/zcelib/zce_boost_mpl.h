#ifndef ZCE_LIB_BOOST_MPL_H_
#define ZCE_LIB_BOOST_MPL_H_

//今天看到一段，白发三千丈，代码写万行，不知程序里，何处话凄凉，

namespace ZCE_LIB
{

//这个方法来自boost ，
//JiangweiSun  在其个人blog 上 http://www.suninf.net/SFINAE-and-enable_if/ 很好的文章
template<typename T>
struct helper
{
    typedef void type;
};

//==================================================================
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
//检测类型T是否具有内部的名为first_type的typedef

template<typename T, typename U = void>
struct has_def_first_type_impl
{
    static const bool value = false;
};

template<typename T>
struct has_def_first_type_impl < T, typename helper<typename T::first_type>::type >
{
    static const bool value = true;
};


template<typename T>
struct has_def_first_type : has_def_first_type_impl < T >
{
};


//==================================================================
// 检测类型T是否具有内部的名为key_type的typedef

template<typename T, typename U = void>
struct has_def_key_type_impl
{
    static const bool value = false;
};


template<typename T>
struct has_def_key_type_impl < T, typename helper<typename T::key_type>::type >
{
    static const bool value = true;
};


template<typename T>
struct has_def_key_type : has_def_key_type_impl < T >
{
};


};





#endif //ZCE_LIB_BOOST_MPL_H_
