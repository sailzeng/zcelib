/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_boost_non_copyable.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011年7月1日
* @brief      参考boost写的一个类，用于不可复制的类型，只要从这个类继承就OK了
*             如果希望一个类不能拷贝，继承ZCE_NON_Copyable就可以了。
*
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_BOOST_NON_COPYABLE_H_
#define ZCE_LIB_BOOST_NON_COPYABLE_H_

/*!
* @brief      不能复制的通过声明但不实现，拷贝构造函数和赋值函数，达到无法复制的目的
*/
class ZCELIB_EXPORT ZCE_NON_Copyable
{
protected:
    ///构造函数和析构函数，保护，保证可以继承
    ZCE_NON_Copyable()
    {

    }
    ~ZCE_NON_Copyable()
    {
    }
    // emphasize the following members are private
private:

    ///拷贝构造函数，声明但不实现
    ZCE_NON_Copyable(const ZCE_NON_Copyable & );
    ///赋值函数，声明但不实现
    const ZCE_NON_Copyable &operator=( const ZCE_NON_Copyable & );
};

#endif //ZCE_LIB_BOOST_NON_COPYABLE_H_

