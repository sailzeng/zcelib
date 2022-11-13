/*!
* @copyright  2004-2019  Apache License, Version 2.0 FULLSAIL
* @filename   zce/util/non_copyable.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年7月1日
* @brief      参考boost写的一个类，用于不可复制的类型，只要从这个类继承就OK了
*             如果希望一个类不能拷贝，继承zce::NON_Copyable就可以了。
*
* @details
*
* @note
*
*/

#pragma once

namespace zce
{
/*!
* @brief      不能复制的通过声明但不实现，拷贝构造函数和赋值函数，达到无法复制的目的
*/
class non_copyable
{
protected:
    ///构造函数和析构函数，保护，保证可以继承
    non_copyable() = default;

    ~non_copyable() = default;

    // emphasize the following members are private
private:

    ///拷贝构造函数，声明但不实现
    non_copyable(const zce::non_copyable&) = delete;
    ///赋值函数，声明但不实现
    const non_copyable& operator=(const zce::non_copyable&) = delete;
};
}
