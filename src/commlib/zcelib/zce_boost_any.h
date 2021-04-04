/*!
* @copyright  2004-2019  Apache License, Version 2.0 FULLSAIL
* @filename   zce_boost_any.h
* @author     ����chrislin����Ʒ��
* @version
* @date       2014��5��30��
* @brief      ��Boost ��any��һ��ʵ�֣�����һЩ������Ҫ��̫�ϸ񣬵�����Ҫ����
*             ����Եĵط���
*
* @details    ͨ��any_cast����ת����
*
* @note       ��Ӣ�ģ����ڵ�̨�壬û��һ���˻���Ϊ������Ȩ����������ѹ����
*             Ҳû��һ���˻���Ϊ�����������Ա�������������һ��·�������ߵú�
*             �������ߵú����ࡣ����������̨��׷˼����.���ˡ�����������һ����
*             �����й�Ҳ����ͬ�����ص�������׷˼�Ǹ����ӡ�̨���ܣ���½û����
*             �ɲ��ܣ�
*             BTW���Ҳ�ϲ��СӢ��
*/

#ifndef ZCE_LIB_BOOST_ANY_H_
#define ZCE_LIB_BOOST_ANY_H_

//��������ʹ��using namespace;
namespace zce
{


/*!
* @brief      any ��һ�����ܵ���������Windows�ڲ���variant�е����ƣ�
*             ��ע��BOOSTҲ��һ��variant����BOOST�Ǹ���һ��ģ���࣬��
*             Anyͨ���ڲ�����ָ������ݣ��ﵽ����������ݵ�Ŀ�ģ��������
*             �Ĳ�����void *�����ҿ��������ͼ�顣
*             ����ͨ��һ��holder_base ����ָ�룬�Լ��������͵�holder��
*             ������ͼ��
* @note       any ������ò�Ҫֱ�ӱ���ָ�룬����ʹ��share_ptr����ָ��
*             ����
*
*/
class any
{
    //���캯������������
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

    //��¡��һ��
    any(const any &other)
        : content_(other.content_ ? other.content_->clone() : 0)
    {
    }

    ~any()
    {
        delete content_;
    }

public: // modifiers

    ///����
    any &swap(any &rhs)
    {
        std::swap(content_, rhs.content_);
        return *this;
    }

    ///boost any �Ĳ���=���ŵ������ǽ��������Ҫע��
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

    ///���ָ��ΪNULL��ʾû��Ϊ��
    bool empty() const
    {
        return !content_;
    }

    const std::type_info &type() const
    {
        return content_ ? content_->type() : typeid(void);
    }

    //�������ָ�������
    class holder_base
    {
    public: // structors

        //����������
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

        //��¡��newһ���µ�
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
