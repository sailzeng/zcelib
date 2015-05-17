/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_boost_any.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2014年5月30日
* @brief      和Boost 的any的一样实现，用于一些对类型要求不太严格，但又需要数据
*             灵活性的地方。
*
* @details    通过any_cast进行转换。
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


/*!
* @brief      any 是一个万能的容器，和Windows内部的variant有点类似，
*             （注意BOOST也有一个variant，但BOOST那个是一个模板类，）
*             Any通过内部保存指针的数据，达到存放所有数据的目的，但他存放
*             的并不是void *，而且可以做类型检查。
*             里面通过一个holder_base 基类指针，以及带有类型的holder，
*             完成类型检查
* @note       any 里面最好不要直接保存指针，而是使用share_ptr这类指针
*             容器
*
*/
class any
{
    //构造函数和析构函数
public:

    any()
        : content_(nullptr)
    {
    }

    template<typename value_type>
    any(const value_type &value)
        : content_(new holder<value_type>(value))
    {
    }

    //克隆了一个
    any(const any &other)
        : content_(other.content_ ? other.content_->clone() : 0)
    {
    }

    ~any()
    {
        delete content_;
    }

public: // modifiers

    ///交换
    any &swap(any &rhs)
    {
        std::swap(content_, rhs.content_);
        return *this;
    }

    ///boost any 的操作=符号的作用是交换。这点要注意
    template<typename value_type>
    any &operator=(const value_type &rhs)
    {
        any(rhs).swap(*this);
        return *this;
    }

    any &operator=(any rhs)
    {
        rhs.swap(*this);
        return *this;
    }

public: // queries

    ///如果指针为NULL表示没有为空
    bool empty() const
    {
        return !content_;
    }

    const std::type_info &type() const
    {
        return content_ ? content_->type() : typeid(void);
    }

    //保存基类指针的类型
    class holder_base
    {
    public: // structors

        //虚析构函数
        virtual ~holder_base()
        {
        }

    public: // queries

        virtual const std::type_info &type() const = 0;

        virtual holder_base *clone() const = 0;

    };

    //
    template<typename value_type>
    class holder : public holder_base
    {
    public: // structors

        holder(const value_type &value)
            : held(value)
        {
        }

    public: // queries

        virtual const std::type_info &type() const
        {
            return typeid(value_type);
        }

        //克隆是new一个新的
        virtual holder_base *clone() const
        {
            return new holder(held);
        }

    public: // representation

        value_type held;

        // intentionally left unimplemented
    private:
        holder &operator=(const holder &);
    };


private: // representation

    template<typename value_type>
    friend value_type *any_cast(any *);

    template<typename value_type>
    friend value_type *unsafe_any_cast(any *);

    holder_base *content_;

};

class bad_any_cast : public std::bad_cast
{
public:
    virtual const char *what() const throw()
    {
        return "boost::bad_any_cast: "
               "failed conversion using boost::any_cast";
    }
};

template<typename value_type>
value_type *any_cast(any *operand)
{
    return operand &&
#ifdef BOOST_AUX_ANY_TYPE_ID_NAME
           std::strcmp(operand->type().name(), typeid(value_type).name()) == 0
#else
           operand->type() == typeid(value_type)
#endif
           ? &static_cast<any::holder<value_type> *>(operand->content_)->held
           : 0;
}

template<typename value_type>
inline const value_type *any_cast(const any *operand)
{
    return any_cast<value_type>(const_cast<any *>(operand));
}

template<typename value_type>
value_type any_cast(any &operand)
{
    typedef BOOST_DEDUCED_TYPENAME remove_reference<value_type>::type nonref;

#ifdef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
    // If 'nonref' is still reference type, it means the user has not
    // specialized 'remove_reference'.

    // Please use BOOST_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION macro
    // to generate specialization of remove_reference for your class
    // See type traits library documentation for details
    BOOST_STATIC_ASSERT(!is_reference<nonref>::value);
#endif

    nonref *result = any_cast<nonref>(&operand);
    if (!result)
    {
        boost::throw_exception(bad_any_cast());
    }
    return *result;
}

template<typename value_type>
inline value_type any_cast(const any &operand)
{
    typedef BOOST_DEDUCED_TYPENAME remove_reference<value_type>::type nonref;

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
template<typename value_type>
inline value_type *unsafe_any_cast(any *operand)
{
    return &static_cast<any::holder<value_type> *>(operand->content_)->held;
}

template<typename value_type>
inline const value_type *unsafe_any_cast(const any *operand)
{
    return unsafe_any_cast<value_type>(const_cast<any *>(operand));
}
};

#endif //ZCE_LIB_BOOST_ANY_H_
