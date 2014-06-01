/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_boost_any.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2014年5月30日
* @brief      和Boost 的any的一样实现，用于一些对类型要求不太严格，但又需要数据
*             灵活性的地方。
* 
* @details
* 
* @note       蔡英文：现在的台湾，没有一个人会认为政府有权力用武力镇压人民，
*             也没有一个人会认为批评政府可以被暴力消音。这一条路，我们走得很
*             漫长，走得很辛苦。今天我们在台湾追思“二.二八”，我相信有一天民
*             主的中国也会用同样慎重的心情来追思那个日子。台湾能，大陆没有理
*             由不能！
*/

#ifndef ZCE_LIB_BOOST_ANY_H_
#define ZCE_LIB_BOOST_ANY_H_

namespace ZCE_LIB
{

    class any
    {
    public:
        // construct/copy/destruct
        any();
        any(const any &);
        any(any &&);
        template<typename ValueType> any(const ValueType &);
        template<typename ValueType> any(ValueType &&);
        any & operator=(const any &);
        any & operator=(any &&);
        template<typename value_type> any & operator=(const value_type &);
        template<typename value_type> any & operator=(value_type &&);
        ~any();

        // modifiers
        any & swap(any &);

        // queries
        bool empty() const;
        const std::type_info & type() const;
    };


};

#endif //ZCE_LIB_BOOST_ANY_H_
