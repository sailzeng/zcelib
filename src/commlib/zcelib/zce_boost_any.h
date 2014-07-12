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

//不建议你使用using namespace;
namespace ZCE_LIB
{

class any
{
public: 

    any()
        : content_(0)
    {
    }

    template<typename ValueType>
    any(const ValueType & value)
        : content_(new holder<ValueType>(value))
    {
    }

    any(const any & other)
        : content_(other.content_ ? other.content_->clone() : 0)
    {
    }

    ~any()
    {
        delete content_;
    }

public: // modifiers

    any & swap(any & rhs)
    {
        std::swap(content_, rhs.content_);
        return *this;
    }

    template<typename ValueType>
    any & operator=(const ValueType & rhs)
    {
        any(rhs).swap(*this);
        return *this;
    }

    any & operator=(any rhs)
    {
        rhs.swap(*this);
        return *this;
    }

public: // queries

    bool empty() const
    {
        return !content_;
    }

    const std::type_info & type() const
    {
        return content_ ? content_->type() : typeid(void);
    }

    class placeholder
    {
    public: // structors

        virtual ~placeholder()
        {
        }

    public: // queries

        virtual const std::type_info & type() const = 0;

        virtual placeholder * clone() const = 0;

    };

    template<typename ValueType>
    class holder : public placeholder
    {
    public: // structors

        holder(const ValueType & value)
            : held(value)
        {
        }

    public: // queries

        virtual const std::type_info & type() const
        {
            return typeid(ValueType);
        }

        virtual placeholder * clone() const
        {
            return new holder(held);
        }

    public: // representation

        ValueType held;

    private: // intentionally left unimplemented
        holder & operator=(const holder &);
    };


private: // representation

    template<typename ValueType>
    friend ValueType * any_cast(any *);

    template<typename ValueType>
    friend ValueType * unsafe_any_cast(any *);

    placeholder * content_;

};

class bad_any_cast : public std::bad_cast
{
public:
    virtual const char * what() const throw()
    {
        return "boost::bad_any_cast: "
            "failed conversion using boost::any_cast";
    }
};

template<typename ValueType>
ValueType * any_cast(any * operand)
{
    return operand &&
#ifdef BOOST_AUX_ANY_TYPE_ID_NAME
        std::strcmp(operand->type().name(), typeid(ValueType).name()) == 0
#else
        operand->type() == typeid(ValueType)
#endif
        ? &static_cast<any::holder<ValueType> *>(operand->content_)->held
        : 0;
}

template<typename ValueType>
inline const ValueType * any_cast(const any * operand)
{
    return any_cast<ValueType>(const_cast<any *>(operand));
}

template<typename ValueType>
ValueType any_cast(any & operand)
{
    typedef BOOST_DEDUCED_TYPENAME remove_reference<ValueType>::type nonref;

#ifdef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
    // If 'nonref' is still reference type, it means the user has not
    // specialized 'remove_reference'.

    // Please use BOOST_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION macro
    // to generate specialization of remove_reference for your class
    // See type traits library documentation for details
    BOOST_STATIC_ASSERT(!is_reference<nonref>::value);
#endif

    nonref * result = any_cast<nonref>(&operand);
    if (!result)
        boost::throw_exception(bad_any_cast());
    return *result;
}

template<typename ValueType>
inline ValueType any_cast(const any & operand)
{
    typedef BOOST_DEDUCED_TYPENAME remove_reference<ValueType>::type nonref;

#ifdef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
    // The comment in the above version of 'any_cast' explains when this
    // assert is fired and what to do.
    BOOST_STATIC_ASSERT(!is_reference<nonref>::value);
#endif

    return any_cast<const nonref &>(const_cast<any &>(operand));
}

// Note: The "unsafe" versions of any_cast are not part of the
// public interface and may be removed at any time. They are
// required where we know what type is stored in the any and can't
// use typeid() comparison, e.g., when our types may travel across
// different shared libraries.
template<typename ValueType>
inline ValueType * unsafe_any_cast(any * operand)
{
    return &static_cast<any::holder<ValueType> *>(operand->content_)->held;
}

template<typename ValueType>
inline const ValueType * unsafe_any_cast(const any * operand)
{
    return unsafe_any_cast<ValueType>(const_cast<any *>(operand));
}
};

#endif //ZCE_LIB_BOOST_ANY_H_
