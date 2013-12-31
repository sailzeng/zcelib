/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_boost_singleton.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013年1月1日
* @brief      signleton的模版实现。
* 
* @details
* 
* @note       我其实并不特别赞同使用signleton的的模版，特别是所谓的小技巧的signleton的的模
*             版。你可以看到我的代码里面大部分都是自己实现的signleton instance函数。
*             关于signleton的的模版实现，大致有2个好处，一是避免重复初始化，一是少写代码。
*             首先我个人认为signleton的多次初始化问题是一个被放大夸大来用于卖弄技巧的的问题，
*             其实好的代码都应该保证自己的初始化代码是单线程状态，而且由于如果你代码逻辑复杂，
*             很多代码都是有先后初始化顺序的，必须先实现A，才能初始化B，而且销毁优势有顺序，
*             而且两者顺序可能不一定正好相反（初始化A，B，C，销毁C，B，A），
* 
*             这个问题让我想起来了一句话。时间对于别出心裁的小花样是最无情的,某种程度上我认为
*             这玩意就是这样一个东西，
* 
*             网上有一偏文章是，里面有若干signleton的的模版
*             http://leoxiang.com/dev/different-cpp-singleton-implementaion
*             我自己对这个问题也做过分析，
*             http://www.cnblogs.com/fullsail/archive/2013/01/03/2842618.html
*/

#ifndef ZCE_LIB_BOOST_SINGLETON_H_
#define ZCE_LIB_BOOST_SINGLETON_H_

//========================================================================================================
//

/*!
* @brief      所谓的普通青年的单件模版，用于单件模式的快速应用，
*             优点：含义清晰，方法明确，构造顺序可控，销毁顺序正好相反，
*             和C++大部分语义一致，
*             缺点：多线程以及特殊的构造依赖和销毁依赖仍然不支持，
*             个人感觉在依赖关系不大，不需要考虑所谓多线程的初始化过程
*             可以使用
* @tparam     T   希望使用singleton模式的数据结构
*/
template <typename T>
class ZCE_Singleton
{

public:
    ///实例函数
    inline static T *instance()
    {
        static T obj;
        return &obj;
    }

private:
    /// ctor is hidden
    ZCE_Singleton();
    /// copy ctor is hidden
    ZCE_Singleton(ZCE_Singleton const &);

};

//========================================================================================================

/*!
* @brief      这个准确说并不是BOOST的singleton实现，而是BOOST的POOL库的
*             singleton实现。用于快速创建SingleTon模式的代码。
*             优点：多线程安全，存在（构造）依赖关系下，可以保证构造安全
*             缺点：代码理解困难，对于如果存在复杂的先后生成关系调用顺序仍然
*             有问题（类全局静态变量的初始化顺序仍然是不可控的），
*             个人并不是特别推荐使用这个模版，仍然使用类静态变量这种不可控制
*             周期的方式，而且中间的技巧，呵呵。（你第一次就看明白了？）
* @tparam     T    希望使用singleton模式的数据结构
*/
template <typename T>
class ZCE_Boost_Singleton
{
private:

    struct object_creator
    {
        object_creator()
        {
            ZCE_Boost_Singleton<T>::instance();
        }
        inline void do_nothing() const {}
    };

    //利用类的静态对象object_creator的构造初始化,在进入main之前已经调用了instance
    //从而避免了多次初始化的问题
    static object_creator create_object_;
public:

    //
    static T *instance()
    {
        static T obj;
        //do_nothing 是必要的，do_nothing的作用有点意思，
        //如果不加create_object_.do_nothing();这句话，在main函数前面
        //create_object_的构造函数都不会被调用，instance当然也不会被调用，
        //我的估计是模版的延迟实现的特效导致，如果没有这句话，编译器也不会实现
        // Singleton_WY<T>::object_creator,所以就会导致这个问题
        create_object_.do_nothing();
        return &obj;
    }
};
//因为create_object_是类的静态变量，必须有一个通用的声明
template <typename T>  typename ZCE_Boost_Singleton<T>::object_creator ZCE_Boost_Singleton<T>::create_object_;

//========================================================================================================

#endif

