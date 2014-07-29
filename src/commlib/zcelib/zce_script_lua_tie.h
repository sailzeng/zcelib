/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   zce_script_lua_tie.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       Sunday, June 22, 2014
* @brief      这个代码是参考Tinker实现的，仍然感谢原作者
*
*             这个代码的目的一方面是我对于如何捏合一个脚本语言总是好奇，今天
*             终于有了机会折腾一下
*
* @details
*
*
*
* @note       感谢fergzhang帮忙解释了很多Tinker的实现，
*             我等了四年，就是要等一个机会，我要争一口气，不是想证明我了不起，我是要告诉人家，我失去的东西一定要亲手拿回来！
*             -- 《英雄本色》 小马哥
*             2014年6月13日早上，荷兰干净利落的爆了西班牙5：1，
*
*             2014年6月25日 别了，布冯，别了，皮尔洛，别了，意大利。
*             超级颜论的颜强提醒我们应该用这首歌给皮尔洛送别。
*             长亭外，古道边
*             芳草碧连天
*             晚风拂柳笛声残
*             夕阳山外山
*             天之涯，地之角
*             知交半零落
*             一瓢(觚)[a]浊酒尽余欢
*             今宵别梦寒
*/



#ifndef ZCE_LIB_SCRIPT_LUA_H_
#define ZCE_LIB_SCRIPT_LUA_H_

#include "zce_predefine.h"
#include "zce_boost_mpl.h"


//LUA目前的包装代码使用C11的新特效，必须用新的编译器
#if ZCE_USE_LUA == 1 && defined ZCE_SUPPORT_CPP11

//LUA 5.02版本目前而言，大部分组件都还不支持，所以……
#if LUA_VERSION_NUM != 501
#error "[Error] please check your lua libary version,only support 5.1,Lua 5.2 is not mature. LUA_VERSION_NUM != 501."
#endif


//在Lua的使用过程中，如果一个类不需要再Lua中使用，其实可以不需要先注册进去，
//这个类就没有meta table而已，但有时候会造成调试麻烦，所以增加这宏，平常可
//以关闭,仅仅在需要调试的阶段打开。
#ifndef ZCE_CHECK_CLASS_NOMETA
#define ZCE_CHECK_CLASS_NOMETA 0
#endif

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( push )
#pragma warning ( disable : 4127)
#endif

///@namespace ZCE_LUA 最后还是使用用了ZCE_LUA的名字空间，因为用ZCE_LIB
///感觉不能完全避免冲突性
namespace ZCE_LUA
{

///只读的table的newdindex
int newindex_onlyread(lua_State *state);

///dump lua运行的的堆栈，用于检查lua运行时的问题，错误处理等
int dump_luacall_stack(lua_State *state);

///dump C调用lua的堆栈，
int enum_clua_stack(lua_State *state);

///在函数处理错误后的处理函数
int on_error(lua_State *state);



/*!
* @brief      用模板函数辅助帮忙实现一个方法，
*             可以通过class 找到对应的类名称（注册到LUA的名称），
* @tparam     class_type 类型
*/
template<typename class_type>
class class_name
{
public:
    /*!
    * @brief      记录(返回)类型的全局的名称
    * @return     const char* 返回的名称
    * @param      name        记录的名称，非空的时候标记名称，为空取得原来的一个
    * @note
    */
    static const char *name(const char *name = NULL)
    {
        const size_t MAX_LEN = 255;
        static char cname[MAX_LEN + 1] = "";
        if (name)
        {
            strncpy(cname, name, MAX_LEN);
            cname[MAX_LEN] = '\0';
        }
        return cname;
    }
};

/*!
* @brief      USER DATA的基类，保存一个void *的指针
*             用于记录数据的指针，通过继承实现对于val，ptr，ref的对象管理
*/
class lua_udat_base
{
public:
    lua_udat_base(void *p)
        : obj_ptr_(p)
    {
    }
    ///必须是virtual
    virtual ~lua_udat_base()
    {
    }
    void *obj_ptr_;
};

/*!
* @brief      把一个val值放入LUA USER DATA的封装
* @tparam     val_type 值得类型
*/
template<typename val_type>
class val_2_udat : public lua_udat_base
{
public:
    val_2_udat()
        : lua_udat_base(new val_type)
    {
    }

    //args_type是构造函数的参数,args_type1 存在的目的是和默认构造函数分开，
    template<typename args_type1, typename ...args_tlist>
    val_2_udat(args_type1 arg1, args_tlist ...arg) : lua_udat_base(new val_type(arg1, arg ...))
    {
    }

    template<typename args_type>
    val_2_udat(args_type arg) : lua_udat_base(new val_type(arg))
    {
    }


    //辅助完成析构,会,释放分配的空间,原作好像少了virtual
    virtual ~val_2_udat()
    {
        delete ((val_type *)obj_ptr_);
    }
};

/*!
* @brief      把一个ptr指针放入LUA USER DATA的封装
* @tparam     val_type 指针的类型
*/
template<typename val_type>
class ptr_2_udat : public lua_udat_base
{
public:
    ptr_2_udat(val_type t)
        : lua_udat_base((void *)t)
    {
    }
};

/*!
* @brief      把一个ref引用放入LUA USER DATA的封装
* @tparam     val_type 引用的类型
*/
template<typename val_type>
class ref_2_udat : public lua_udat_base
{
public:
    //注意第一个&t表示是引用参数，第二个是标示传递指针给lua_udat_base基类
    ref_2_udat(val_type t) :
        lua_udat_base(&t)
    {
    }
};

/*!
* @brief      把一个数组的引用入LUA USER DATA的封装
* @tparam     val_type 引用的类型
*/
template<typename ary_type>
class arrayref_2_udat : public lua_udat_base
{
public:
    ///构造函数
    arrayref_2_udat(ary_type *ary_ptr, size_t ary_size, bool read_only) :
        lua_udat_base(ary_ptr),
        ary_size_(ary_size),
        read_only_(read_only)
    {
    }

    ///空间长度
    size_t ary_size_;
    ///是否只读
    bool   read_only_;

};

//=======================================================================================================
//Push 一个数据到Lua的堆栈中

///避免某些编译错误的函数
void push_stack(lua_State * /*state*/);

///为变参模板函数递归准备的函数
template<typename val_type, typename... val_tlist>
void push_stack(lua_State *state, val_type val, val_tlist ... val_s)
{
    push_stack<val_type>(state, val);
    push_stack(state, val_s...);
    return;
}

/*!
* @brief      放入一个引用
* @tparam     val_type  引用的类型
* @param      state     lua state
* @param      ref       引用，
* @note
*/
template<typename val_type  >
void push_stack(lua_State *state,
                typename std::enable_if<std::is_reference<val_type>::value, val_type>::type ref)
{
    //
    new(lua_newuserdata(state,
                        sizeof(ref_2_udat<val_type>))) ref_2_udat<val_type>(ref);

    //如果原类型（去掉引用）是一个object，
    if (std::is_class<std::remove_reference<val_type>::type >::value)
    {
        //根据类的名称，设置metatable，注意这儿去掉了引用，进行的查询，
        lua_pushstring(state, class_name<
            std::remove_reference<
            std::remove_cv<val_type>::type>::type >::name());
        lua_gettable(state, LUA_GLOBALSINDEX);
        if (!lua_istable(state, -1))
        {
#if ZCE_CHECK_CLASS_NOMETA == 1
            ZCE_LOGMSG(RS_ERROR, "[LUATIE][%s][%s] is not tie to lua,name[%s]?"
                       " May be you don't register or name conflict? ",
                       __ZCE_FUNCTION__,
                       typeid(ref).name(),
                       class_name<val_type >::name());
#endif
            lua_remove(state, -1);
            return;
        }
        lua_setmetatable(state, -2);
    }

    return;
}


/*!
* @brief      放入一个指针
* @tparam     val_type 指针的类型
* @param      state    Lua state
* @param      ptr      放入的指针
*/
template<typename ptr_type  >
void push_stack(lua_State *state,
    typename std::enable_if<std::is_pointer<ptr_type>::value, ptr_type>::type ptr)
{
    return push_stack_ptr<std::remove_cv<ptr_type>::type >(state, ptr);
}

template<typename ptr_type >
void push_stack_ptr(lua_State *state, typename ptr_type ptr)
{
    if (ptr)
    {
        new(lua_newuserdata(state, sizeof(ptr_2_udat<ptr_type>))) ptr_2_udat<ptr_type>(ptr);

        //如果原类型（去掉指针）是一个object，
        if (std::is_class<std::remove_pointer<ptr_type>::type >::value)
        {
            //根据类的名称，设置metatable，注意这儿去掉了指针，进行的查询，
            lua_pushstring(state, class_name <
                std::remove_pointer <ptr_type> ::type > ::name());
            lua_gettable(state, LUA_GLOBALSINDEX);
            if (!lua_istable(state, -1))
            {
#if ZCE_CHECK_CLASS_NOMETA == 1
                ZCE_LOGMSG(RS_ERROR,
                    "[LUATIE][%s][%s] is not tie to lua,name[%s]? "
                    "May be you don't register or name conflict? ",
                    __ZCE_FUNCTION__,
                    typeid(ptr).name(),
                    class_name<std::remove_pointer <ptr_type> ::type >::name());
#endif
                lua_remove(state, -1);
                return;
            }
            lua_setmetatable(state, -2);
        }
    }
    else
    {
        lua_pushnil(state);
    }
    return;
}

///指针的两种特化
template<> void push_stack_ptr(lua_State *state, char * const val);
template<> void push_stack_ptr(lua_State *state, const char * const val);

//枚举
template<typename val_type  >
void push_stack(lua_State *state,
                typename  std::enable_if<std::is_enum<val_type>::value, val_type>::type val)
{
    lua_pushnumber(state, val);
}

//放入一个val，
template<typename val_type  >
void push_stack(lua_State *state,
                typename  std::enable_if 
                <!(std::is_pointer<val_type>::value ||
                  std::is_reference<val_type>::value || 
                  std::is_enum<val_type>::value), val_type 
                >::type val)
{
    return push_stack_val<std::remove_cv<val_type>::type >(state, val);
}

//对于非object类型的数据，如果要放入引用和指针，如果希望Lua能使用，要先声明这些对象
//这个请注意！！！

template<typename val_type >
void push_stack_val(lua_State *state, typename val_type val)
{
    //这儿只针对非object对象
    if (!std::is_class<val_type>::value)
    {
        ZCE_LOGMSG(RS_ERROR, "[LUATIE]Type[%s] not support in this code?", typeid(val_type).name());
        return;
    }

    new(lua_newuserdata(state,
                        sizeof(val_2_udat<val_type>)))
    val_2_udat<std::remove_cv<val_type>::type >(val);

    //根据类的名称，设置metatable
    lua_pushstring(state, class_name<std::remove_cv<val_type>::type >::name());
    lua_gettable(state, LUA_GLOBALSINDEX);
    if (!lua_istable(state, -1))
    {
#if ZCE_CHECK_CLASS_NOMETA == 1
        ZCE_LOGMSG(RS_ERROR, 
                   "[LUATIE][%s][%s] is not tie to lua,name[%s]? "
                   "May be you don't register or name conflict? ",
                   __ZCE_FUNCTION__,
                   typeid(val).name(),
                   class_name<std::remove_cv<val_type>::type >::name());
#endif
        lua_remove(state, -1);
        return;
    }
    lua_setmetatable(state, -2);
    return;
}


///放入一个数组的引用
template<typename array_type  >
void push_stack(lua_State *state, typename arrayref_2_udat<array_type> & ary_dat)
{
    new (lua_newuserdata(state, sizeof(arrayref_2_udat<array_type>)))
    arrayref_2_udat<array_type>(ary_dat);
    lua_newtable(state);

    lua_pushstring(state, "__array_size");
    lua_pushnumber(state, static_cast<int>(ary_dat.ary_size_));
    lua_rawset(state, -3);

    lua_pushstring(state, "__index");
    lua_pushcclosure(state, ZCE_LUA::array_meta_get<array_type>, 0);
    lua_rawset(state, -3);

    //非只读
    if (!ary_dat.read_only_)
    {
        lua_pushstring(state, "__newindex");
        lua_pushcclosure(state, ZCE_LUA::array_meta_set<array_type>, 0);
        lua_rawset(state, -3);
    }
    //如果只读，__newindex
    else
    {
        lua_pushstring(state, "__newindex");
        lua_pushcclosure(state, ZCE_LUA::newindex_onlyread, 0);
        lua_rawset(state, -3);
    }
    lua_setmetatable(state, -2);
    return;
}

//各种重载的push_back函数
template<> void push_stack_val(lua_State *state, char val);
template<> void push_stack_val(lua_State *state, unsigned char val);
template<> void push_stack_val(lua_State *state, short val);
template<> void push_stack_val(lua_State *state, unsigned short val);
template<> void push_stack_val(lua_State *state, int val);
template<> void push_stack_val(lua_State *state, unsigned int val);
template<> void push_stack_val(lua_State *state, float val);
template<> void push_stack_val(lua_State *state, double val);
template<> void push_stack_val(lua_State *state, bool val);
template<> void push_stack_val(lua_State *state, int64_t val);
template<> void push_stack_val(lua_State *state, uint64_t val);

//不允许出现long的变量，因为long无法移植，所以只定义，不实现
template<> void push_stack_val(lua_State *state, long val);
template<> void push_stack_val(lua_State *state, unsigned long val);


//=======================================================================================================

///读取一个指针
template<typename ret_type>
typename std::enable_if<std::is_pointer<ret_type>::value, typename std::remove_cv<ret_type>::type >::type
read_stack(lua_State *state, int index)
{
    return ((std::remove_cv<ret_type>::type)(((lua_udat_base *)lua_touserdata(state, index))->obj_ptr_));
}

template < typename ptr_type >
ptr_type read_stack_ptr(lua_State *state, int index)
{
    return (ret_type)(((lua_udat_base *)lua_touserdata(state, index))->obj_ptr_);
}

template<> char               *read_stack_ptr(lua_State *state, int index);
template<> const char         *read_stack_ptr(lua_State *state, int index);

///读取一个引用
template<typename ret_type>
typename std::enable_if<std::is_reference<ret_type>::value, typename std::remove_cv<ret_type>::type>::type
read_stack(lua_State *state, int index)
{
    return *((std::remove_cv< std::remove_reference<ret_type>::type >::type *)
        (((lua_udat_base *)lua_touserdata(state, index))->obj_ptr_));
}

///读取枚举值
template<typename ret_type>
typename std::enable_if<std::is_enum<ret_type>::value, ret_type>::type
read_stack(lua_State *state, int index)
{
    return (val_type)lua_tonumber(state, index);
}

///读取一个val
template<typename ret_type>
typename std::enable_if < !(std::is_pointer<ret_type>::value ||
                            std::is_reference<ret_type>::value || 
                            std::is_enum<ret_type>::value
                            ), typename std::remove_cv<ret_type>::type >::type
read_stack(lua_State *state, int index)
{
    return read_stack_val<std::remove_cv<ret_type>::type>(state, index);
}


///从堆栈中读取某个类型
template < typename ret_type >
ret_type read_stack_val(lua_State *state, int index)
{
    return *(ret_type *)(((lua_udat_base *)lua_touserdata(state, index))->obj_ptr_);
}

template<> char                read_stack_val(lua_State *state, int index);
template<> unsigned char       read_stack_val(lua_State *state, int index);
template<> short               read_stack_val(lua_State *state, int index);
template<> unsigned short      read_stack_val(lua_State *state, int index);
template<> int                 read_stack_val(lua_State *state, int index);
template<> unsigned int        read_stack_val(lua_State *state, int index);
template<> float               read_stack_val(lua_State *state, int index);
template<> double              read_stack_val(lua_State *state, int index);
template<> bool                read_stack_val(lua_State *state, int index);
template<> int64_t             read_stack_val(lua_State *state, int index);
template<> uint64_t            read_stack_val(lua_State *state, int index);
template<> std::string         read_stack_val(lua_State *state, int index);

///
template<typename val_type>
val_type pop_stack(lua_State *state)
{
    val_type t = read_stack<val_type>(state, -1);
    lua_pop(state, 1);
    return t;
}


//=======================================================================================================


///让LUA从一个绑定的数组（指针）那里得到数组的的值
template<typename array_type>
int array_meta_get(lua_State *state)
{
    //如果不是
    int index = static_cast<int>( lua_tonumber(state, -1));
    arrayref_2_udat<array_type> *ptr = ((arrayref_2_udat<array_type> *)lua_touserdata(state, -2));
    if (index < 1 && index > static_cast<int>( ptr->ary_size_ ))
    {
        ZCE_LOGMSG(RS_ERROR, "Lua script use error index [%d] to visit array %s[] size[%u].",
                   index,
                   typeid(array_type).name(),
                   static_cast<uint32_t>(ptr->ary_size_));
        ZCE_ASSERT(false);
        lua_pushnil(state);

    }
    else
    {
        array_type *ary_ptr = (array_type *)(ptr->obj_ptr_);
        //注意这儿为了符合Lua的习惯，LUA的索引是从1开始
        push_stack<array_type>(state, ary_ptr[index - 1]);
    }

    //index 应该做个检查

    return 1;
}

///让LUA可以设置一个绑定的数组（指针）的某个值
template<typename array_type>
int array_meta_set(lua_State *state)
{
    arrayref_2_udat<array_type> *ptr = ((arrayref_2_udat<array_type> *)lua_touserdata(state, -3));
    int index = static_cast<int>(lua_tonumber(state, -2));

    //对index做边界检查
    if (index < 1 && index > static_cast<int>(ptr->ary_size_) )
    {
        ZCE_LOGMSG(RS_ERROR, "Lua script use error index [%d] to visit array %s[] size[%u].",
                   index,
                   typeid(array_type).name(),
                   static_cast<uint32_t>(ptr->ary_size_));
        ZCE_ASSERT(false);

    }
    else
    {
        array_type *ary_ptr = (array_type *)(ptr->obj_ptr_);
        //注意这儿为了符合Lua的习惯，LUA的索引是从1开始
        ary_ptr[index - 1] = read_stack<array_type>(state, -1);
    }

    return 0;
}

//=======================================================================================================


/*!
* @brief      全局函数的封装类，辅助调用实际注册的全局函数
*             根据ret_type 是否是void，分了2个版本。另外一个见g_functor_void
* @tparam     ret_type  返回值类型
* @tparam     args_type 参数类型列表，0-N个参数
* note        这儿要注意，有一个严重的问题。而我确实找不到方法规避，（不使用变参是可以避免），
*             这样的，C++的模板变参的函数包扩展，在VS2013的编译器，和GCC 4.8的编译器上都有
*             不足，会出现参数顺序颠倒的情况，问题估计是编译器在扩展处理是，是采用的标准参数处
*             理顺序，从左到右，但是其处理一个参数就将其后就入栈了，而C++的编译器，标准的栈处理
*             顺序是从右到左，所函数得到的参数顺序就是反的。所以我只有反过来取参数，
*             但是这应该是一个bug，我不知道哪天编译器会修复这个问题，咩咩，那时候又只有……
*/
template<bool last_yield, 
    typename ret_type, 
    typename ...args_type>
class g_functor_ret
{
public:
    static int invoke(lua_State *state)
    {
        //push是将结果放入堆栈
        void *upvalue_1 = lua_touserdata(state, lua_upvalueindex(1));
        ret_type(*fun_ptr)(args_type...) = (ret_type( *)(args_type...)) (upvalue_1);

        size_t sz_par = sizeof...(args_type);
        int para_idx = static_cast<int>(sz_par);
        //没有参数时para_idx可能会被编译器认为没有使用过。
        ZCE_UNUSED_ARG(para_idx);

        //如果参数传递顺序错误，请参考note的说明，
        
        //根据是否有返回值，决定如何处理，是否push_stack
        push_stack<ret_type>(state, fun_ptr(read_stack<args_type>(state, para_idx--)...));
        if (last_yield)
        {
            return lua_yield(state, 1);
        }
        else
        {
            return 1;
        }
        
    }
};
///全局没有返回值的函数封装，详细信息见g_functor_ret
template<bool last_yield,
    typename... args_type>
class g_functor_void
{
public:
    static int invoke(lua_State *state)
    {

        //push是将结果放入堆栈
        void *upvalue_1 = lua_touserdata(state, lua_upvalueindex(1));
        void (*fun_ptr)(args_type...) = (void(*)(args_type...)) (upvalue_1);

        size_t sz_par = sizeof...(args_type);
        int para_idx = static_cast<int>(sz_par);
        //没有参数时para_idx可能会被编译器认为没有使用过。
        ZCE_UNUSED_ARG(para_idx);

        fun_ptr(read_stack<args_type>(state, para_idx--)...);
        if (last_yield)
        {
            return lua_yield(state, 0);
        }
        else
        {
            return 0;
        }
    }
};


///lua读取类里面的数据的meta table的__index对应函数
int class_meta_get(lua_State *state);

///lua写入类里面的数据的meta table的__newindex对应函数
int class_meta_set(lua_State *state);


///调用父母的函数
int class_parent(lua_State *state);


/*!
* @brief      封装类的构造函数给LUA使用
* @tparam     class_type
* @tparam     args_type
* @return     int
* @param      state
*/
template<typename class_type, typename... args_type>
int constructor(lua_State *state)
{
    //new 一个user data，用<T>的大小,同时，同时用placement new 的方式，
    //（指针式lua_newuserdata分配的）完成构造函数
    int para_idx = 1;
    //如果参数个数为0，会引发一个告警
    ZCE_UNUSED_ARG(para_idx);

    new(lua_newuserdata(state,
                        sizeof(val_2_udat<class_type>)))
    val_2_udat<class_type>(read_stack<args_type>(state, ++para_idx)...);

    lua_pushstring(state, class_name<class_type>::name());
    lua_gettable(state, LUA_GLOBALSINDEX);

    lua_setmetatable(state, -2);

    return 1;
}

//调用USER_DATA的基类的析构,由于userdata_base其实是一个LUA使用的userdata对象的基类，
//其子类包括3种，val,ptr,ref,其中val的析构会释放对象，ptr，ref的对象什么都不会做，
//这样就保证无论你传递给LUA什么，他们的生命周期都是正确的，
int destroyer(lua_State *state);


/*!
* @brief      用C++11的新特效，变参实现的类函数桥接
* @tparam     last_yield 函数最后是否使用lua_yield返回
* @tparam     class_type 类的类型
* @tparam     ret_type   返回值的类型
* @tparam     ...args_type 变参的参数类型列表
* note        有一个值得注意的问题请参考说明g_functor_ret
*/
template<bool last_yield,typename class_type, typename ret_type, typename ...args_type>
class member_functor_ret
{
public:
    static int invoke(lua_State *state)
    {
        //push是将结果放入堆栈
        void *upvalue_1 = lua_touserdata(state, lua_upvalueindex(1));

        typedef ret_type (class_type::*mem_fun)(args_type...);
        mem_fun fun_ptr = *(mem_fun *)(upvalue_1);

        //第一个参数是对象指针
        class_type *obj_ptr = read_stack<class_type *>(state, 1);

        //得到参数个数，+1是因为，第一个参数是函数指针
        size_t sz_par = sizeof...(args_type);
        int para_idx = static_cast<int>(sz_par + 1);
        //避免告警
        ZCE_UNUSED_ARG(para_idx);

        //根据是否有返回值，决定如何处理，是否push_stack
        push_stack<ret_type>(state,
            (obj_ptr->*fun_ptr)(read_stack<args_type>(state, \
            para_idx--)...));
        if (last_yield)
        {
            return lua_yield(state, 1);
        }
        {
            return 1;
        }
    }
};
///
template<bool last_yield, typename class_type, typename ...args_type>
class member_functor_void
{
public:
    static int invoke(lua_State *state)
    {
        //push是将结果放入堆栈
        void *upvalue_1 = lua_touserdata(state, lua_upvalueindex(1));

        typedef void (class_type::*mem_fun)(args_type...);
        mem_fun fun_ptr = *(mem_fun *)(upvalue_1);

        //第一个参数是对象指针
        class_type *obj_ptr = read_stack<class_type *>(state, 1);

        //得到参数个数，+1是因为，第一个参数是函数指针
        size_t sz_par = sizeof...(args_type);
        int para_idx = static_cast<int>(sz_par + 1);
        //避免告警
        ZCE_UNUSED_ARG(para_idx);

        //void 函数

        //我恨函数指针，我更恨类成员的指针,注意下面的那个括号。一定要，否则，我看了1个小时
        //为什么采用--，请参考前面的解释 g_functor_ret
        (obj_ptr->*fun_ptr)(read_stack<args_type>(state, para_idx--)...);
        if (last_yield)
        {
            return lua_yield(state, 0);
        }
        {
            return 0;
        }
    }
};

/*!
* @brief      成员变量的处理的基类，用于class_meta_get,class_meta_set内部处理
* @note       memvar_base 注意其不是一个有模板的函数，是一个基类
*             这样就可以保证通过LUA user data里面的void * 转换为 memvar_base *
*             而通过memvar_base *的调用 get, set 帮忙恰恰能直接调用到真正的
*             member_var <class_type,var_type>
*/
class memvar_base
{
public:
    virtual void get(lua_State *state) = 0;
    virtual void set(lua_State *state) = 0;
};


/*!
* @brief      成员变量绑定的辅助类，也就是user data保存的类
*
* @tparam     class_type class_type 为class 类型，
* @tparam     var_type   为变量类型
*/
template<typename class_type, typename var_type>
class member_var : memvar_base
{
public:
    member_var(var_type class_type::*val) :
        var_ptr_(val)
    {
    }

    ///完成设置某个
    void set(lua_State *state)
    {
        //这段看起来是不是有点点晕，其实他干的事情就是下面这段，他是利用类的成员变量指针完成成员的写，
        //float A::*pfl = &A::float_var;
        //A a1;  a1.*pfl = 0.5f;
        //这个真要感谢airfu GG的精心讲解
        read_stack<class_type *>(state, 1)->*(var_ptr_) = read_stack<var_type>(state, 3);
    }

    //get是LUA读取的操作，也就是把C++的数据读取到LUA里面，所以是PUSH
    void get(lua_State *state)
    {
        //read_stack其实就是把类的对象的指针读取出来。
        //然后通过类成员指针，把成员获取出来
        push_stack<var_type>(state,
                             read_stack<class_type *>(state, 1)->*(var_ptr_) );
    }

    //存放类的成员指针
    var_type class_type::*var_ptr_;
};



/*!
* @brief      成员数组类型的类，用于userdata的处理
*
* @tparam     class_type 类类型
* @tparam     ary_type   数组类型
* @tparam     ary_size   数组长度
* @note
*/
template<typename class_type, typename ary_type, size_t ary_size>
class member_array : memvar_base
{
public:

    //构造函数
    member_array(typename ary_type (class_type::*mem_ary_ptr)[ary_size],
                 bool read_only) :
        mem_ary_ptr_(mem_ary_ptr),
        read_only_(read_only)
    {
    }


    ///对于成员数组，设置没有任何意义，
    void set(lua_State *state)
    {
        luaL_error(state, "Member array ptr is read only ,can't modify,please check your code.");
    }

    //get是LUA读取的操作，也就是把C++的数据读取到LUA里面，所以是PUSH
    void get(lua_State *state)
    {
        //read_stack其实就是把类的对象的指针读取出来。
        //然后通过类成员指针，把成员获取出来
        ary_type *ary_ptr = read_stack<class_type *>(state, 1)->*(mem_ary_ptr_);
        arrayref_2_udat<ary_type> ary_dat(ary_ptr, ary_size, read_only_);
        push_stack(state, ary_dat);
    }

    ///成员数组的指针
    ary_type(class_type:: *mem_ary_ptr_)[ary_size];

    ///是否只读
    bool                   read_only_;
};



};  //namespace ZCE_LUA

class ZCE_Lua_Base;


//=======================================================================================================

/*!
* @brief      给lua绑定类的语法糖，每个函数会返回*this的引用，主要是为了实现连续.操作语法
*             这样的语法，让代码书写更加简单一点。
*             tie.reg_class<TA>("TA",false).mem_var(...).mem_var(...)
*             当然缺点也会有，因为这样的操作没有返回值，所以即使有错误也无法反馈
* @tparam     class_type 绑定的类的名称
* @note       具体函数的解释请参考ZCE_Lua_Tie
*/
template <typename class_type>
class Candy_Tie_Class
{
public:
    ///构造函数
    Candy_Tie_Class(ZCE_Lua_Base *lua_tie,
                    bool read_only):
        lua_tie_(lua_tie),
        read_only_(read_only)
    {
    }

    ////在类的meta table注册构造函数
    template <typename construct_fun >
    Candy_Tie_Class &construct(construct_fun func)
    {
        lua_tie_->class_constructor<class_type, construct_fun >(func);
        return *this;
    }

    ///在类的meta table注册成员变量
    template <typename var_type >
    Candy_Tie_Class &mem_var(const char *name, var_type class_type::*val)
    {
        lua_tie_->class_mem_var<class_type, var_type >(name, val);
        return *this;
    }

    ///在类的meta table注册成员数组
    template <typename array_type , size_t array_size>
    Candy_Tie_Class &mem_ary(const char *name, array_type (class_type::*ary)[array_size])
    {
        lua_tie_->class_mem_ary<class_type, array_type, array_size >(name, ary, read_only_);
        return *this;
    }

    ///在类的meta table绑定函数
    template<typename ret_type, typename... args_type>
    Candy_Tie_Class &mem_fun(const char *name, typename ret_type(class_type::*func)(args_type...))
    {
        lua_tie_->class_mem_fun<class_type, ret_type, args_type...>(name, func);
        return *this;
    }

    template<typename ret_type, typename... args_type>
    Candy_Tie_Class &mem_yield_fun(const char *name, typename ret_type(class_type::*func)(args_type...))
    {
        lua_tie_->class_mem_yield_fun<class_type, ret_type, args_type...>(name, func);
        return *this;
    }

    //从某个类继承
    template<typename parent_type>
    Candy_Tie_Class &inherit()
    {
        lua_tie_->class_inherit<class_type, parent_type>();
        return *this;
    }

protected:
    ///Lua的解释器的状态
    ZCE_Lua_Base   *lua_tie_ = nullptr;

    ///这个类是否是只读的方式
    bool           read_only_ = false;
};




//=======================================================================================================


/*!
* @brief      Lua 各种封装的基类，大部分功能都封装在这儿，
*             包括堆栈，各种封装，class，函数的注册，
* @note       因为协程和Tie都应该会使用到这部分。所以独立
*             作为基类
*/
class ZCE_Lua_Base
{

protected:

    ///构造函数
    explicit ZCE_Lua_Base(lua_State *lua_state);
    ///析构函数
    virtual ~ZCE_Lua_Base();

public:

    ///执行一个lua文件
    int do_file(const char *filename);
    ///执行一个LUA的buffer
    int do_buffer(const char *buff, size_t sz);

    ///dump C调用lua的堆栈，
    void enum_stack();
    ///dump lua运行的的堆栈，用于检查lua运行时的问题，错误处理等
    void dump_stack();


    ///向LUA注册int64_t的类型，因为LUA内部的number默认是double，所以其实无法表示。所以要注册这个
    void reg_int64();
    ///向LUA注册uint64_t的类型
    void reg_uint64();
    ///向LUA注册std::string的类型
    void reg_stdstring();


    //封装lua堆栈的函数

    /// 删除指定索引上的元素，并将该位置之上的所有元素下移。
    inline void stack_remove(int index)
    {
        lua_remove(lua_state_, index);
    }

    ///会上移指定位置之上的所有元素以开辟一个槽的空间，然后将栈顶元素移到该位置
    inline void stack_insert(int index)
    {
        return lua_insert(lua_state_, index);
    }

    ///返回栈的元素个数
    inline int stack_gettop()
    {
        return lua_gettop(lua_state_);
    }

    ///设置栈的元素个数，如果原来的栈空间小于index，填充nil，如果大于index，删除多余元素
    inline void stack_settop(int index)
    {
        return lua_settop(lua_state_, index);
    }

    ///确保堆栈空间有extra那么大
    inline int stack_check(int extra)
    {
        return lua_checkstack(lua_state_, extra);
    }

    ///把index位置上的值在堆栈顶复制push一个
    inline void stack_pushvalue(int index)
    {
        return lua_pushvalue(lua_state_, index);
    }

    ///取得index位置的类型，返回值LUA_TNIL等枚举值
    ///lua_type is LUA_TNIL, LUA_TNUMBER, LUA_TBOOLEAN, LUA_TSTRING, LUA_TTABLE, 
    ///LUA_TFUNCTION, LUA_TUSERDATA, LUA_TTHREAD, and LUA_TLIGHTUSERDATA
    inline int stack_type(int index)
    {
        return lua_type(lua_state_, index);
    }

    ///检查索引index的位置的数据的类型是否是lua_t
    inline bool stack_istype(int index, int lua_t)
    {
        return lua_type(lua_state_, index) == lua_t;
    }

    ///得到堆栈上index位置的类型名称，
    inline const char *stack_typename(int index)
    {
        return lua_typename(lua_state_, lua_type(lua_state_,index));
    }

    ////检查index位置的类型，
    inline void stack_checktype(int index,int lua_t)
    {
        return luaL_checktype(lua_state_, index, lua_t);
    }

    ///得到对象的长度
    ///for tables, this is the result of the length operator ('#'); 
    ///for userdata, this is the size of the block of memory allocated for the userdata;
    ///for other values, it is 0. 
    inline size_t get_objlen(int index)
    {
        return lua_objlen(lua_state_, index);
    }

    ///取得table的所有元素个数,注意其和stack_objlen的其别,此函数绝对不高效，呵呵
    inline size_t get_tablecount(int index)
    {
        size_t table_count = 0;
        //放入迭代器
        lua_pushnil(lua_state_);
        while (lua_next(lua_state_, index) != 0)
        {
            ++table_count;
        }
        return table_count;
    }

    ///通过名称取得lua对象，并且检查
    inline int get_luaobj(const char *obj_name,int luatype)
    {
        lua_pushstring(lua_state_, obj_name);
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);
        if (lua_type(lua_state_, -1) != luatype)
        {
            return -1;
        }
        return 0;
    }

    /*!
    * @brief      向LUA设置一个（对LUA而言）全局变量（名称和变量对应值的拷贝）
    * @tparam     val_type 放入的数据类型，如果是val，会在LUA里面保留拷贝，如果是ptr，ref，lua内部可以直接修改这个变量
    * @param      name 名称
    * @param      val  放入的变量，注意如果要放入引用，需要set_gvar<var_type &>(ref)，这样写
    */
    template<typename var_type>
    void set_gvar(const char *name, typename var_type var)
    {
        //名称对象，
        lua_pushstring(lua_state_, name);
        //模板函数，根据val_type绝对如何push
        ZCE_LUA::push_stack<var_type>(lua_state_, var);
        lua_settable(lua_state_, LUA_GLOBALSINDEX);
    }

    ///根据名称，从LUA读取一个变量
    template<typename var_type>
    typename var_type get_gvar(const char *name)
    {
        lua_pushstring(lua_state_, name);
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);
        return ZCE_LUA::pop_stack<var_type>(lua_state_);
    }

    /*!
    * @brief      向LUA设置一个数组的引用,在LUA内部保存一个相关的userdata，
    * @tparam     array_type
    * @param      name
    * @param      ary_data
    * @param      ary_size
    * @param      read_only
    * @note       注意是引用喔，一定要注意喔。
    */
    template<typename array_type>
    void set_garray(const char *name,
                    typename array_type ary_data[],
                    size_t ary_size,
                    bool read_only = false)
    {
        ZCE_LUA::arrayref_2_udat<array_type> aux_ary(ary_data, ary_size, read_only);
        //名称对象，
        lua_pushstring(lua_state_, name);
        ZCE_LUA::push_stack(lua_state_, aux_ary);
        lua_settable(lua_state_, LUA_GLOBALSINDEX);
    }

    ///从LUA中获取一个全局的数组
    template<typename ary_type>
    int get_garray(const char *name,
                   typename ary_type ary_data[],
                   size_t &ary_size)
    {
        //名称对象，
        lua_pushstring(lua_state_, name);
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);

        //如果不是一个table，错误哦
        if (!lua_isuserdata(lua_state_, -1))
        {
            lua_remove(lua_state_, -1);
            return -1;
        }
        ZCE_LUA::arrayref_2_udat<array_type> aux_ary =
            *(aux_ary *)lua_touserdata(lua_state_, -1);
        ary_size = aux_ary.ary_size_;
        for (size_t i = 0; i < ary_size; ++i)
        {
            ary_data[i] = *((ary_type *)aux_ary.obj_ptr_ + i);
        }
        return 0;
    }


    ///向LUA注册一个全局函数，或者类的静态函数给lua调用
    ///参数详细说明请参考reg_gfun_all
    template<typename ret_type, typename... args_type>
    void reg_gfun(const char *name, ret_type(*func)(args_type...))
    {
        reg_gfun_all<false, ret_type, args_type...>(name, func);
    }


    ///向LUA注册一个全局函数，或者类的静态函数给lua调用.和reg_gfun的区别是，最后会使用lua_yield返回，
    ///参数详细说明请参考reg_gfun_all
    template<typename ret_type, typename... args_type>
    void reg_yeild_gfun(const char *name, ret_type(*func)(args_type...))
    {
        reg_gfun_all<true, ret_type, args_type...>(name, func);
    }

    /*!
    * @brief      将一个容器里面的数据放入Lua的table中，根据迭代器的类型会选择不同的
    *             重载函数
    * @tparam     iter_type   迭代器的类型，（当然也可以是指针）
    * @param      table_name  table的名称
    * @param      first       第一个first迭代器
    * @param      last        最后一个last的迭代器
    */
    template<class iter_type >
    void to_luatable(const char *table_name,
                     const iter_type first,
                     const iter_type last)
    {
        to_luatable(table_name,
                    first,
                    last,
                    std::iterator_traits<iter_type>::iterator_category());
        return;
    }

    /*!
    * @brief
    * @tparam     container_type
    * @return     int
    * @param      table_name
    * @param      container_dat
    * @note
    */
    template<class container_type>
    int from_luatable(const char *table_name,
                      container_type &container_dat)
    {
        return from_luatable(table_name,
                             container_dat,
                             std::iterator_traits<container_type::iterator>::iterator_category());
    }

    //
    template<class array_type>
    int from_luatable(const char *table_name,
                      array_type *array_dat)
    {
        //根据类的名称，取得类的metatable的表，或者说原型。
        lua_pushstring(lua_state_, table_name);
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);

        if (!lua_istable(lua_state_, -1))
        {
            ZCE_LOGMSG(RS_ERROR, "[LUATIE] table name[%s] is not tie to lua.",
                       table_name);
            lua_remove(lua_state_, -1);
            return -1;
        }

        //first key ,ferg让我把这东西就理解成迭代器
        lua_pushnil(lua_state_);
        while (lua_next(lua_state_, -2) != 0)
        {
            // uses 'key' (at index -2) and 'value' (at index -1)
            int index = ZCE_LUA::read_stack<int>(lua_state_, -2) - 1;
            array_dat[index] = ZCE_LUA::read_stack <array_type>(lua_state_, -1);
            // removes 'value'; keeps 'key' for next iteration
            lua_remove(lua_state_, -1);
        }
        return 0;
    }

    ///调用LUA的函数，只有一个返回值
    template<typename... args_type>
    int call_luafun_0(const char *fun_name, args_type... args)
    {
        int ret = 0;
        ret = call_luafun(fun_name, 0, args...);
        if (ret != 0)
        {
            return ret;
        }
        return 0;
    }

    ///调用LUA的函数，只有一个返回值
    template<typename ret_type1, typename... args_type>
    int call_luafun_1(const char *fun_name, ret_type1 &ret_val1, args_type... args)
    {
        int ret = 0;
        ret = call_luafun(fun_name, 1, args...);
        if (ret != 0)
        {
            return ret;
        }
        //在堆栈弹出返回值
        ret_val1 = ZCE_LUA::read_stack<ret_type1>(lua_state_, -1);
        lua_pop(lua_state_, 1);
        return 0;
    }

    ///调用LUA的函数，有两个返回值
    template<typename ret_type1, typename ret_type2, typename... args_type>
    int call_luafun_2(const char *fun_name,
                      ret_type1 &ret_val1,
                      ret_type2 &ret_val2,
                      args_type... args)
    {
        int ret = 0;
        ret = call_luafun(fun_name, 2, args...);
        if (ret != 0)
        {
            return ret;
        }
        //在堆栈弹出返回值
        ret_val1 = ZCE_LUA::read_stack<ret_type1>(lua_state_, -2);
        ret_val2 = ZCE_LUA::read_stack<ret_type2>(lua_state_, -1);
        lua_pop(lua_state_, 2);
        return 0;
    }

    ///调用LUA的函数，有三个返回值,好吧就支持到3个返回值把，实在没兴趣了
    template<typename ret_type1, typename ret_type2, typename ret_type3, typename... args_type>
    int call_luafun_3(const char *fun_name,
                      ret_type1 &ret_val1,
                      ret_type2 &ret_val2,
                      ret_type3 &ret_val3,
                      args_type... args)
    {
        int ret = 0;
        ret = call_luafun(fun_name, 3, args...);
        if (ret != 0)
        {
            return ret;
        }
        //在堆栈弹出返回值,注意参数顺序
        ret_val1 = ZCE_LUA::read_stack<ret_type1>(lua_state_, -3);
        ret_val2 = ZCE_LUA::read_stack<ret_type2>(lua_state_, -2);
        ret_val3 = ZCE_LUA::read_stack<ret_type3>(lua_state_, -1);
        lua_pop(lua_state_, 3);
        return 0;
    }


    /*!
    * @brief      在Lua里面new一个table，同时把pair_list参数里面的数据放入到table中
    * @tparam     pair_tlist pair list的类型列表，必须是std::pair
    * @param      table_name 表的名称，
    * @param      pair_list  piar的list同时把pair的first，作为key，pair的second作为value，
    * @note       通过这个函数，可以轻松的把枚举呀，注册给lua使用，比如下面这种方式
    *             lua_tie.new_table("tolua_enum",std::make_pair("ENUM_0001", ENUM_0001),
    *                 std::make_pair("ENUM_0002", ENUM_0002);
    */
    template<typename ...pair_tlist>
    void new_table(const char *table_name, pair_tlist ... pair_list)
    {
        lua_pushstring(lua_state_, table_name);
        lua_newtable(lua_state_);
        //向table里面添加pair
        newtable_addkv(pair_list...);
        lua_settable(lua_state_, LUA_GLOBALSINDEX);
    }

        
    /*!
    * @brief      绑定类的给Lua使用，定义类的metatable的表，或者说原型的表。
    * @tparam     class_type      绑定类的类型
    * @return     Candy_Tie_Class 用于方便绑定类的成员，可以让你写出连续.的操作
    * @param      class_name      类的名称，在Lua中使用
    * @param      read_only       这个类的数据是否只读，而不能写
    */
    template<typename class_type>
    Candy_Tie_Class<class_type> reg_class(const char *class_name,
                                          bool read_only = false)
    {
        //绑定T和名称,类的名称
        lua_pushstring(lua_state_, ZCE_LUA::class_name<class_type>::name(class_name));
        //new 一个table，这个table是作为其他的类的metatable的（某种程度上也可以说是原型），
        lua_newtable(lua_state_);

        //__name不是标准的元方法，但在例子中有使用
        lua_pushstring(lua_state_, "__name");
        lua_pushstring(lua_state_, ZCE_LUA::class_name<class_type>::name());
        lua_rawset(lua_state_, -3);

        //将meta_get函数作为__index函数
        lua_pushstring(lua_state_, "__index");
        lua_pushcclosure(lua_state_, ZCE_LUA::class_meta_get, 0);
        lua_rawset(lua_state_, -3);


        if (!read_only)
        {
            //非只读情况将meta_set函数作为__newindex函数
            lua_pushstring(lua_state_, "__newindex");
            lua_pushcclosure(lua_state_, ZCE_LUA::class_meta_set, 0);
            lua_rawset(lua_state_, -3);
        }
        else
        {
            lua_pushstring(lua_state_, "__newindex");
            lua_pushcclosure(lua_state_, ZCE_LUA::newindex_onlyread, 0);
            lua_rawset(lua_state_, -3);
        }

        //垃圾回收函数
        lua_pushstring(lua_state_, "__gc");
        lua_pushcclosure(lua_state_, ZCE_LUA::destroyer, 0);
        lua_rawset(lua_state_, -3);

        lua_settable(lua_state_, LUA_GLOBALSINDEX);

        return Candy_Tie_Class<class_type> (this, read_only);
    }



    /*!
    * @brief      注册类的构造函数，
    * @tparam     class_type class_type 是类
    * @tparam     construct_fun 是构造函数的封装，ZCE_LUA::constructor
    * @return     int
    * @param      func
    */
    template<typename class_type, typename construct_fun>
    int class_constructor(construct_fun func)
    {
        //根据类的名称，取得类的metatable的表，或者说原型。
        lua_pushstring(lua_state_, ZCE_LUA::class_name<class_type>::name());
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);

        //如果栈顶是不是一个表，进行错误处理
        if (!lua_istable(lua_state_, -1))
        {
            ZCE_LOGMSG(RS_ERROR, "[LUATIE] class name[%s] is not tie to lua.",
                       ZCE_LUA::class_name<class_type>::name());
            ZCE_ASSERT(false);
            lua_pop(lua_state_, 1);
            return -1;
        }

        //对这个类的metatable的表，设置一个metatable，在其中增加一个__call的对应函数
        //这样的目的是这样的，__call是对应一个()调用，但实体不是函数式，的调用函数
        //LUA中出现这样的调用，var1,var2是构造参数，和构造函数一样，这个地方要感谢fergzhang GG的帮助
        //object =class_name(var1,var2)
        lua_newtable(lua_state_);

        lua_pushstring(lua_state_, "__call");
        lua_pushcclosure(lua_state_, func, 0);
        lua_rawset(lua_state_, -3);
        //设置这个table作为class 原型的metatable.
        //或者说设置这个table作为class metatable的metatable.
        lua_setmetatable(lua_state_, -2);

        lua_pop(lua_state_, 1);
        return 0;
    }


    /*!
    * @brief      本来想把多重继承实现了，但后面和ferg讨论，觉得结果发现意义不大，
    *             而且容易
    * @tparam     class_type  类型
    * @tparam     parent_type 父类型
    * @return     int == 0 返回值标识成功
    */
    template<typename class_type, typename parent_type>
    int class_inherit()
    {
        //根据类的名称，取得类的metatable的表，或者说原型。
        lua_pushstring(lua_state_, ZCE_LUA::class_name<class_type>::name());
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);

        //如果栈顶是一个表
        if (!lua_istable(lua_state_, -1))
        {
            ZCE_LOGMSG(RS_ERROR, "[LUATIE] class name[%s] is not tie to lua.",
                       ZCE_LUA::class_name<class_type>::name());
            ZCE_ASSERT(false);
            lua_pop(lua_state_, 1);
            return -1;
        }

        //设置__parent 为 父类名称，目前不能多重继承,我和freg讨论过，其实实现多重集成也可以，
        //就是在处理查询的时候各种麻烦

#if defined DEBUG || defined _DEBUG
        lua_pushstring(lua_state_, "__parent");
        lua_gettable(lua_state_, -2);
        if (lua_isnil(lua_state_, -1))
        {
            lua_remove(lua_state_, -1);
        }
        else
        {
            ZCE_ASSERT(false);
        }
#endif

        lua_pushstring(lua_state_, "__parent");
        lua_pushstring(lua_state_, ZCE_LUA::class_name<parent_type>::name());
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);
        if (!lua_istable(lua_state_, -1))
        {
            ZCE_LOGMSG(RS_ERROR, "[LUATIE] class name[%s] is not tie to lua.",
                       ZCE_LUA::class_name<parent_type>::name());
            ZCE_ASSERT(false);
            lua_pop(lua_state_, 1);
            return -1;
        }
        lua_rawset(lua_state_, -3);

        //从堆栈弹出push_meta取得的vlue
        lua_pop(lua_state_, 1);
        return 0;
    }


    /*!
    * @brief      给一个类（的meta table）注册绑定成员变量
    * @tparam     class_type 绑定的类的类型
    * @tparam     var_type   成员类型
    * @return     int        是否绑定成功
    * @param      name       绑定的名称
    * @param      class_type::*val 绑定的成员变量的指针
    * @note       Luatinker中还有一个模板参数base_type，成员所属的类，一般我认为class_type
    *             和base_type是一样的,但你也可以把一个基类的成员绑定在子类的Lua的meta table里面，
    *             我自己感觉这东东的存在必要性不大，去掉了
    */
    template<typename class_type, typename var_type>
    int class_mem_var(const char *name, var_type class_type::*val)
    {
        //根据类的名称，取得类的metatable的表，或者说原型。
        lua_pushstring(lua_state_, ZCE_LUA::class_name<class_type>::name());
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);

        //
        if (!lua_istable(lua_state_, -1))
        {
            ZCE_LOGMSG(RS_ERROR, "[LUATIE] class name[%s] is not tie to lua.",
                       ZCE_LUA::class_name<class_type>::name());
            ZCE_ASSERT(false);
            lua_pop(lua_state_, 1);
            return -1;
        }

        lua_pushstring(lua_state_, name);
        //mem_var 继承于var_base,实际调用的时候利用var_base的虚函数完成回调。
        new(lua_newuserdata(lua_state_, sizeof(ZCE_LUA::member_var<class_type, var_type>)))  \
        ZCE_LUA::member_var<class_type, var_type>(val);
        lua_rawset(lua_state_, -3);

        lua_pop(lua_state_, 1);
        return 0;
    }


    /*!
    * @brief      给一个类的meta table 绑定成员数组
    * @tparam     class_type 类的类型
    * @tparam     ary_type   数组的类型
    * @tparam     ary_size   数组的长度
    * @return     int        == 0表示成功
    * @param      name       注册的名字
    * @param      mem_ary    数组的指针
    * @param      read_only  数组只读
    */
    template<typename class_type, typename ary_type, size_t ary_size>
    int class_mem_ary(const char *name,
                      ary_type(class_type:: *mem_ary)[ary_size],
                      bool read_only = false)
    {
        //根据类的名称，取得类的metatable的表，或者说原型。
        lua_pushstring(lua_state_, ZCE_LUA::class_name<class_type>::name());
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);

        //
        if (!lua_istable(lua_state_, -1))
        {
            ZCE_LOGMSG(RS_ERROR, "[LUATIE] class name[%s] is not tie to lua.",
                       ZCE_LUA::class_name<class_type>::name());
            ZCE_ASSERT(false);
            lua_pop(lua_state_, 1);
            return -1;
        }

        lua_pushstring(lua_state_, name);
        //mem_var 继承于var_base,实际调用的时候利用var_base的虚函数完成回调。
        new(lua_newuserdata(lua_state_, sizeof(ZCE_LUA::member_array<class_type, ary_type, ary_size>)))  \
        ZCE_LUA::member_array<class_type, ary_type, ary_size>(mem_ary, read_only);
        lua_rawset(lua_state_, -3);

        lua_pop(lua_state_, 1);
        return 0;
    }

    ///注册一个类的成员函数，名称是name，详细的函数说明，请参考class_mem_fun_all
    template<typename class_type, typename ret_type, typename... args_type>
    int class_mem_fun(const char *name, typename ret_type(class_type::*func)(args_type...))
    {
        return class_mem_fun_all<false, class_type, ret_type, args_type...>(name, func);
    }

    ///注册一个类的成员函数，和class_mem_fun的区别是函数最后的返回会调用lua_yield,函数名称是name，
    ///详细的函数说明，请参考class_mem_fun_all
    template<typename class_type, typename ret_type, typename... args_type>
    int class_mem_yield_fun(const char *name, typename ret_type(class_type::*func)(args_type...))
    {
        return class_mem_fun_all<true, class_type, ret_type, args_type...>(name, func);
    }

    

    ///放入某个东东到堆栈
    template<typename val_type >
    inline void push(val_type val)
    {
        ZCE_LUA::push_stack<val_type>(lua_state_, val);
    }

    ///读取堆栈上的某个数据
    template<typename val_type >
    inline val_type read(int index)
    {
        return ZCE_LUA::read_stack<val_type>(lua_state_, index);
    }

    template<typename val_type >
    inline val_type pop()
    {
        return ZCE_LUA::pop_stack<val_type>(lua_state_);
    }

    //
    inline lua_State  *get_state()
    {
        return lua_state_;
    }

protected:

    ///辅助函数，展开kv pari list
    template<typename... pair_tlist>
    void newtable_addkv(pair_tlist ... pair_list)
    {
        return;
    }
    ///辅助函数，展开kv pari list,同时把第一个参数放入table中
    template<typename pair_type, typename... pair_tlist>
    void newtable_addkv(pair_type pair_dat, pair_tlist ... pair_list)
    {
        ZCE_LUA::push_stack<typename pair_type::first_type>(lua_state_, pair_dat.first);
        ZCE_LUA::push_stack<typename pair_type::second_type>(lua_state_, pair_dat.second);
        lua_settable(lua_state_, -3);
        newtable_addkv(pair_list...);
        return;
    }

    /*!
    * @brief      调用LUA的函数，没有返回值，（或者暂不取回一个返回值）
    * @tparam     args_type 参数类型列表
    * @return     int == 0 表示成功
    * @param      fun_name 函数名称
    * @param      ret_num  返回值得数量
    * @param      args     参数列表
    */
    template<typename... args_type>
    int call_luafun(const char *fun_name, size_t ret_num, args_type... args)
    {
        int ret = 0;
        //放入错误处理的函数，并且记录堆栈的地址
        lua_pushcclosure(lua_state_, ZCE_LUA::on_error, 0);
        int errfunc = lua_gettop(lua_state_);

        lua_pushstring(lua_state_, fun_name);
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);
        //检查其是否是函数
        if (!lua_isfunction(lua_state_, -1))
        {
            ZCE_LOGMSG(RS_ERROR, "[LUATIE] call_luafun() attempt to call global `%s' (not a function)",
                       fun_name);
            lua_pop(lua_state_, 1);
            return -1;
        }

        size_t arg_num = sizeof...(args);
        if (arg_num)
        {
            //放入堆栈参数，args
            ZCE_LUA::push_stack(lua_state_, args...);
        }



        //调用lua的函数，
        ret = lua_pcall(lua_state_,
                        static_cast<int>(arg_num),
                        static_cast<int>(ret_num),
                        errfunc);
        if (ret != 0)
        {
            ZCE_LOGMSG(RS_ERROR, "[LUATIE] lua_pcall ret = %d", ret);
        }

        //在堆栈删除掉错误处理的函数
        lua_remove(lua_state_, errfunc);

        //注意这儿，还没有取出返回值，或者这个函数就没有返回值

        return 0;
    }

    /*!
    * @brief      从Lua中拷贝数据到C++的容器中，包括数组，vector，vector类要先resize
    * @tparam     container_type  容器类型，
    * @return     int             成功返回0
    * @param      table_name      表的名称
    * @param      container_dat   容器数据
    * @param      nouse           没有使用的参数，仅仅用于类型重载识别
    */
    template<class container_type>
    int from_luatable(const char *table_name,
        container_type &container_dat,
        std::random_access_iterator_tag /*nouse*/)
    {
        //根据类的名称，取得类的metatable的表，或者说原型。
        lua_pushstring(lua_state_, table_name);
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);

        if (!lua_istable(lua_state_, -1))
        {
            ZCE_LOGMSG(RS_ERROR, "[LUATIE] table name[%s] is not tie to lua.",
                table_name);
            lua_remove(lua_state_, -1);
            return -1;
        }

        //first key ,ferg让我把这东西就理解成迭代器
        lua_pushnil(lua_state_);
        while (lua_next(lua_state_, -2) != 0)
        {
            // uses 'key' (at index -2) and 'value' (at index -1)
            int index = ZCE_LUA::read_stack<int>(lua_state_, -2) - 1;
            container_dat[index] =
                ZCE_LUA::read_stack
                <std::iterator_traits<iter_type>::value_type>
                (lua_state_, -1);
            // removes 'value'; keeps 'key' for next iteration
            lua_remove(lua_state_, -1);
        }
        return 0;
    }

    //从Lua中拷贝数据到C++的容器中，包括数组，vector，vector类要先resize
    template<class container_type>
    int from_luatable(const char *table_name,
        container_type &container_dat,
        std::bidirectional_iterator_tag /*nouse*/)
    {
        //根据类的名称，取得类的metatable的表，或者说原型。
        lua_pushstring(lua_state_, table_name);
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);

        if (!lua_istable(lua_state_, -1))
        {
            ZCE_LOGMSG(RS_ERROR, "[LUATIE] table name[%s] is not tie to lua.",
                table_name);
            ZCE_ASSERT(false);
            lua_remove(lua_state_, -1);
            return -1;
        }

        //first key ,ferg让我把这东西就理解成迭代器
        lua_pushnil(lua_state_);
        while (lua_next(lua_state_, -2) != 0)
        {
            // uses 'key' (at index -2) and 'value' (at index -1)
            container_dat[ZCE_LUA::read_stack<container_type::value_type::first_type>(lua_state_, -2)] =
                ZCE_LUA::read_stack<container_type::value_type::second_type>(lua_state_, -1);
            // removes 'value'; keeps 'key' for next iteration
            lua_remove(lua_state_, -1);
        }
        return 0;
    }

    /*!
    * @brief      使用迭代器拷贝数组，vector的数据,到LUA中的一个table中，
    * @tparam     raiter_type 输入的迭代器，random_access_iterator_tag 类型的迭代器
    * @param      table_name table的名称
    * @param      first      第一个first迭代器
    * @param      last       最后一个last的迭代器
    * @param      nouse      没有使用的参数，仅仅用于类型重载识别
    * @note
    */
    template<class raiter_type >
    void to_luatable(const char *table_name,
        const raiter_type first,
        const raiter_type last,
        std::random_access_iterator_tag /*nouse*/)
    {
        lua_pushstring(lua_state_, table_name);
        lua_createtable(lua_state_,
            static_cast<int>(std::distance(first, last)), 0);
        typename raiter_type iter_temp = first;
        for (int i = 0; iter_temp != last; iter_temp++, i++)
        {
            //Lua的使用习惯索引是从1开始
            lua_pushnumber(lua_state_, i + 1);
            //通过迭代器萃取得到类型，
            ZCE_LUA::push_stack<std::iterator_traits<raiter_type>::value_type >(
                lua_state_,
                *iter_temp);
            lua_settable(lua_state_, -3);
        }
        lua_settable(lua_state_, LUA_GLOBALSINDEX);
    }

    /*!
    * @brief      通过迭代器将一个map,或者unorder_map放入lua table，
    * @tparam     biiter_type 迭代器类型,bidirectional_iterator_tag,双向迭代器
    * @param      table_name  表的名称
    * @param      first       第一个迭代器，
    * @param      last        最后一个迭代器
    * @param      nouse       没有使用的参数，仅仅用于类型重载识别
    * @note       这个其实也支持和multi的几个map，但，但……，另外list也是bidirectional_iterator_tag
    */
    template<class biiter_type >
    void to_luatable(const char *table_name,
        const biiter_type  first,
        const biiter_type last,
        std::bidirectional_iterator_tag /*nouse*/)
    {
        lua_pushstring(lua_state_, table_name);
        lua_createtable(lua_state_,
            0,
            static_cast<int>(std::distance(first, last)));

        typename biiter_type iter_temp = first;
        for (; iter_temp != last; iter_temp++)
        {
            //将map的key作为table的key
            ZCE_LUA::push_stack<std::remove_cv<
                std::iterator_traits<biiter_type>::value_type::first_type
            >::type >(lua_state_, iter_temp->first);
            ZCE_LUA::push_stack<std::remove_cv<
                std::iterator_traits<biiter_type>::value_type::second_type
            >::type >(lua_state_, iter_temp->second);
            lua_settable(lua_state_, -3);
        }

        lua_settable(lua_state_, LUA_GLOBALSINDEX);
        return;
    }

    /*!
    * @brief      向LUA注册一个全局函数，或者类的静态函数给lua调用
    *             根据last_yield确定返回的方式，是否调用lua_yield
    * @tparam     last_yield 函数的最后，是否使用yield返回，
    * @tparam     ret_type   返回参数类型
    * @tparam     args_type  函数的参数类型，变参
    * @param      name       向LUA注册的函数名称
    * @param      func       注册的C函数
    */
    template<bool last_yield, typename ret_type, typename... args_type>
    void reg_gfun_all(const char *name, ret_type(*func)(args_type...))
    {
        //函数名称
        lua_pushstring(lua_state_, name);
        //将函数指针转换为void * ，作为lightuserdata 放入堆栈，作为closure的upvalue放入
        lua_pushlightuserdata(lua_state_, (void *)func);
        //functor模板函数，放入closure,
        lua_pushcclosure(lua_state_, 
            ZCE_LIB::if_< std::is_void<ret_type>::value,
            ZCE_LUA::g_functor_void<last_yield, args_type...>,
            ZCE_LUA::g_functor_ret<last_yield, ret_type ,args_type...> 
            >::type::invoke, 
            1);
        
        //将其放入全局环境表中
        lua_settable(lua_state_, LUA_GLOBALSINDEX);
    }
    
    /*!
    * @brief      注册类的成员函数
    * @tparam     last_yield 最后，是否使用lua_yield函数返回，主要用于协程中
    * @tparam     class_type 成员函数所属的类
    * @tparam     ret_type   返回值
    * @tparam     args_type  参数列表，是多个参数类型
    * @return     int   == 0 表示注册成功
    * @param      name  函数的名字
    * @param      func  成员函数指针
    */
    template<bool last_yield, typename class_type, typename ret_type, typename... args_type>
    int class_mem_fun_all(const char *name, typename ret_type(class_type::*func)(args_type...))
    {
        //根据类的名称，取得类的metatable的表，或者说原型。
        lua_pushstring(lua_state_, ZCE_LUA::class_name<class_type>::name());
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);

        //
        if (!lua_istable(lua_state_, -1))
        {
            ZCE_LOGMSG(RS_ERROR, "[LUATIE] class name[%s] is not tie to lua.",
                ZCE_LUA::class_name<class_type>::name());
            ZCE_ASSERT(false);
            lua_pop(lua_state_, 1);
            return -1;
        }

        lua_pushstring(lua_state_, name);
        //这个类的函数指针作为upvalue_的。
        //注意这儿是类的成员指针（更加接近size_t），而不是实际的指针，所以这儿不能用light userdata
        //下面这个写法真是要了人民，非要用typedef中转一下
        typedef ret_type(class_type:: *mem_fun)(args_type...);
        new(lua_newuserdata(lua_state_, sizeof(mem_fun))) mem_fun(func);
        //
        lua_pushcclosure(lua_state_,
            ZCE_LIB::if_< std::is_void<ret_type>::value,
            ZCE_LUA::member_functor_void<last_yield,class_type, args_type...>,
            ZCE_LUA::member_functor_ret<last_yield, class_type, ret_type, args_type...>
            >::type::invoke,
            1);
        lua_rawset(lua_state_, -3);

        lua_remove(lua_state_, -1);
        return 0;
    }

protected:

    //Lua的解释器的状态
    lua_State   *lua_state_;
};



//=======================================================================================================

/*!
* @brief      Lua的协程的封装，保存协程的state以及其在堆栈的id
*             其Lua的功能都从ZCE_Lua_Base 得到
* @note       Lua的代码里面自称是Thread，但其实是协程，而为了统一我也没有使用coroutine命名， 
*             我的代码里面也有真正的thread相关的东东，避免误解，统一使用Lua Thread这样的命名
*/
class ZCE_Lua_Thread :public ZCE_Lua_Base
{
public:

    ZCE_Lua_Thread();

    ///析构代码，Lua Thread的代码不会自己释放自己，Lua Thread在
    ///堆栈被清空的时候，会被GC回收掉。所以，析构函数什么也不做。
    ~ZCE_Lua_Thread();

    ///设置线程相关的数据
    void set_thread(lua_State * lua_thread, int thread_stackidx);

    ///取得线程在创建者堆栈的位置索引
    int get_thread_stackidx();


    ///恢复线程运行
    int resume(int narg);

    ///挂起线程运行
    int yield(int nresults);

protected:

    ///线程在创建者堆栈的位置索引
    int luathread_stackidx_ = 0;
};

//=======================================================================================================


/*!
* @brief      LUA 鞋带，用于帮助绑定何种C，或者C++的代码到Lua，或者使用Lua的各种功能。
*             同时还可以使用线程等功能
* @note       
*/
class ZCE_Lua_Tie :public ZCE_Lua_Base
{
public:

    ZCE_Lua_Tie();
    ~ZCE_Lua_Tie();
    
    /*!
    * @brief      打开lua state
    * @return     int
    * @param      open_libs  是否打开常用的一些LUA库
    * @param      reg_common 是否注册一些常用
    */
    int open(bool open_libs,
        bool reg_common);
    ///关闭lua state
    void close();
    
    ///开启一个新的lua thread
    int new_thread(ZCE_Lua_Thread *lua_thread);

    ///关闭，回收一个lua thread
    void del_thread(ZCE_Lua_Thread *lua_thread);

    ///恢复一个线程的运行
    int resume_thread(ZCE_Lua_Thread *lua_thread, int narg);
};


//=======================================================================================================

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( pop )
#endif

#endif //#if defined  ZCE_USE_LUA && defined ZCE_SUPPORT_CPP11

#endif // ZCE_LIB_SCRIPT_LUA_H_

