/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   zce_script_lua_tie.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       Sunday, June 22, 2014
* @brief      ��������ǲο�Tinkerʵ�ֵģ���Ȼ��лԭ����
*
*             ��������Ŀ��һ�������Ҷ���������һ���ű��������Ǻ��棬����
*             �������˻�������һ��.
*             Ŀǰ֧��LUA 5.1
* @details
*
*
*
* @note       ��лfergzhang��æ�����˺ܶ�Tinker��ʵ�֣�
*             �ҵ������꣬����Ҫ��һ�����ᣬ��Ҫ��һ������������֤�����˲�������Ҫ�����˼ң���ʧȥ�Ķ���һ��Ҫ�����û�����
*             -- ��Ӣ�۱�ɫ�� С���
*             2014��6��13�����ϣ������ɾ�����ı���������5��1��
*
*             2014��6��25�� ���ˣ����룬���ˣ�Ƥ���壬���ˣ��������
*             �������۵���ǿ��������Ӧ�������׸��Ƥ�����ͱ�
*             ��ͤ�⣬�ŵ���
*             ���ݱ�����
*             ������������
*             Ϧ��ɽ��ɽ
*             ��֮�ģ���֮��
*             ֪��������
*             һư(��)[a]�Ǿƾ��໶
*             �������κ�
*/



#ifndef ZCE_LIB_SCRIPT_LUA_H_
#define ZCE_LIB_SCRIPT_LUA_H_

#include "zce_predefine.h"
#include "zce_boost_mpl.h"


//LUAĿǰ�İ�װ����ʹ��C11������Ч���������µı�����
#if ZCE_USE_LUA == 1 && ZCE_SUPPORT_CPP11 == 1

//LUA 5.2�汾Ŀǰ���ԣ��󲿷����������֧�֣����ԡ���
#if LUA_VERSION_NUM != 501
#error "[Error] please check your lua libary version,only support 5.1,"\
    "Lua 5.2 is not mature. LUA_VERSION_NUM != 501."
#endif


//��Lua��ʹ�ù����У����һ���಻��Ҫ��Lua��ʹ�ã���ʵ���Բ���Ҫ��ע���ȥ��
//������û��meta table���ѣ�����ʱ�����ɵ����鷳������������꣬ƽ����
//�Թر�,��������Ҫ���ԵĽ׶δ򿪡�
#ifndef ZCE_CHECK_CLASS_NOMETA
#define ZCE_CHECK_CLASS_NOMETA 0
#endif

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( push )
#pragma warning ( disable : 4127)
#pragma warning ( disable : 4189)
#elif defined (ZCE_OS_LINUX)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsequence-point"
#endif


namespace zce
{

namespace luatie
{
///ֻ����table��newdindex
int newindex_onlyread(lua_State *state);

///dump lua���еĵĶ�ջ�����ڼ��lua����ʱ�����⣬�������
int dump_luacall_stack(lua_State *state);

///dump C����lua�Ķ�ջ��
int enum_clua_stack(lua_State *state);

///�ں�����������Ĵ�����
int on_error(lua_State *state);



/*!
* @brief      ��ģ�庯��������æʵ��һ��������
*             ����ͨ��class �ҵ���Ӧ�������ƣ�ע�ᵽLUA�����ƣ���
* @tparam     class_type ����
*/
template<typename class_type>
class class_name
{
public:
    /*!
    * @brief      ��¼(����)���͵�ȫ�ֵ�����
    * @return     const char* ���ص�����
    * @param      name        ��¼�����ƣ��ǿյ�ʱ�������ƣ�Ϊ��ȡ��ԭ����һ��
    * @note
    */
    static const char *name(const char *name = NULL)
    {
        const size_t MAX_LEN = 255;
        static char cname[MAX_LEN + 1] = "";
        if (name)
        {
            ::strncpy(cname, name, MAX_LEN);
            cname[MAX_LEN] = '\0';
        }
        return cname;
    }
};

/*!
* @brief      USER DATA�Ļ��࣬����һ��void *��ָ��
*             ���ڼ�¼���ݵ�ָ�룬ͨ���̳�ʵ�ֶ���val��ptr��ref�Ķ������
*/
class lua_udat_base
{
public:
    lua_udat_base(void *p)
        : obj_ptr_(p)
    {
    }
    ///������virtual
    virtual ~lua_udat_base()
    {
    }
    //��Ϊ��¼�˶������ͣ�����ָ�룬ptr��ref�ȣ�������void *
    void *obj_ptr_;
};

/*!
* @brief      ��һ��valֵ����LUA USER DATA�ķ�װ
* @tparam     val_type ֵ������
*/
template<typename val_type>
class val_2_udat: public lua_udat_base
{
public:
    val_2_udat()
        : lua_udat_base(new val_type)
    {
    }
    //args_type�ǹ��캯���Ĳ���,args_type1 ���ڵ�Ŀ���Ǻ�Ĭ�Ϲ��캯���ֿ���
    template<typename ...args_tlist>
    val_2_udat(args_tlist ...arg)
        :lua_udat_base(new val_type(arg ...))
    {
    }

    template<typename args_type>
    val_2_udat(args_type arg)
        : lua_udat_base(new val_type(arg))
    {
    }


    //�����������,��,�ͷŷ���Ŀռ�,ԭ����������virtual
    virtual ~val_2_udat()
    {
        delete ((val_type *)obj_ptr_);
    }
};

/*!
* @brief      ��һ��ptrָ�����LUA USER DATA�ķ�װ
* @tparam     val_type ָ�������
*/
template<typename val_type>
class ptr_2_udat: public lua_udat_base
{
public:
    ptr_2_udat(val_type t)
        : lua_udat_base((void *)t)
    {
    }
};

/*!
* @brief      ��һ��ref���÷���LUA USER DATA�ķ�װ
* @tparam     val_type ���õ�����
*/
template<typename val_type>
class ref_2_udat: public lua_udat_base
{
public:
    //ע���һ��&t��ʾ�����ò������ڶ����Ǳ�ʾ����ָ���lua_udat_base����
    ref_2_udat(val_type t):
        lua_udat_base(&t)
    {
    }
};

/*!
* @brief      ��һ�������������LUA USER DATA�ķ�װ
* @tparam     val_type ���õ�����
*/
template<typename ary_type>
class arrayref_2_udat: public lua_udat_base
{
public:
    ///���캯��
    arrayref_2_udat(ary_type *ary_ptr, size_t ary_size, bool read_only):
        lua_udat_base(ary_ptr),
        ary_size_(ary_size),
        read_only_(read_only)
    {
    }

    ///�ռ䳤��
    size_t ary_size_;
    ///�Ƿ�ֻ��
    bool   read_only_;

};

//=======================================================================================================
//Push һ�����ݵ�Lua�Ķ�ջ��

///����ĳЩ�������ĺ���
void push_stack(lua_State * /*state*/);

///Ϊ���ģ�庯���ݹ�׼���ĺ���
template<typename val_type, typename... val_tlist>
void push_stack(lua_State *state, val_type val, val_tlist ... val_s)
{
    push_stack<val_type>(state, val);
    push_stack(state, val_s...);
    return;
}

/*!
* @brief      ����һ������
* @tparam     val_type  ���õ�����
* @param      state     lua state
* @param      ref       ���ã�
* @note
*/
template<typename val_type  >
void push_stack(lua_State *state,
                typename std::enable_if< std::is_reference<val_type>::value, val_type>::type ref)
{
    //
    new (::lua_newuserdata(state,
                           sizeof(ref_2_udat<val_type>))) ref_2_udat<val_type>(ref);

    //���ԭ���ͣ�ȥ�����ã���һ��object��
    if (std::is_class<typename std::remove_reference<val_type>::type >::value)
    {
        //����������ƣ�����metatable��ע�����ȥ�������ã����еĲ�ѯ��
        ::lua_pushstring(state, class_name < typename
                         std::remove_reference < typename
                         std::remove_cv<val_type>::type >::type >::name());
        ::lua_gettable(state, LUA_GLOBALSINDEX);
        if (!lua_istable(state, -1))
        {
#if ZCE_CHECK_CLASS_NOMETA == 1
            ZCE_LOG(RS_ERROR, "[LUATIE][%s][%s] is not tie to lua,name[%s]?"
                    " May be you don't register or name conflict? ",
                    __ZCE_FUNC__,
                    typeid(ref).name(),
                    class_name<val_type >::name());
#endif
            ::lua_remove(state, -1);
            return;
        }
        ::lua_setmetatable(state, -2);
    }

    return;
}

template<typename ptr_type >
void push_stack_ptr(lua_State *state, ptr_type ptr)
{
    if (ptr)
    {
        new (::lua_newuserdata(state, sizeof(ptr_2_udat<ptr_type>))) ptr_2_udat<ptr_type>(ptr);

        //���ԭ���ͣ�ȥ��ָ�룩��һ��object��
        if (std::is_class<typename std::remove_pointer<ptr_type>::type >::value)
        {
            //����������ƣ�����metatable��ע�����ȥ����ָ�룬���еĲ�ѯ��
            ::lua_pushstring(state, class_name <
                             typename std::remove_pointer <ptr_type> ::type > ::name());
            ::lua_gettable(state, LUA_GLOBALSINDEX);
            if (!lua_istable(state, -1))
            {
#if ZCE_CHECK_CLASS_NOMETA == 1
                ZCE_LOG(RS_ERROR,
                        "[LUATIE][%s][%s] is not tie to lua,name[%s]? "
                        "May be you don't register or name conflict? ",
                        __ZCE_FUNC__,
                        typeid(ptr).name(),
                        class_name<std::remove_pointer <ptr_type> ::type >::name());
#endif
                ::lua_remove(state, -1);
                return;
            }
            ::lua_setmetatable(state, -2);
        }
    }
    else
    {
        ::lua_pushnil(state);
    }
    return;
}


/*!
* @brief      ����һ��ָ��
* @tparam     val_type ָ�������
* @param      state    Lua state
* @param      ptr      �����ָ��
*/
template<typename ptr_type  >
void push_stack(lua_State *state,
                typename std::enable_if<std::is_pointer<ptr_type>::value, ptr_type>::type ptr)
{
    return push_stack_ptr<typename std::remove_cv<ptr_type>::type >(state, ptr);
}

//���ڷ�object���͵����ݣ����Ҫ�������ú�ָ�룬���ϣ��Lua��ʹ�ã�Ҫ��������Щ����
//�����ע�⣡����
template<typename val_type >
void push_stack_val(lua_State *state, val_type val)
{
    //���ֻ��Է�object����
    if (!std::is_class<val_type>::value)
    {
        ZCE_LOG(RS_ERROR, "[LUATIE]Type[%s] not support in this code?", typeid(val_type).name());
        return;
    }

    new (::lua_newuserdata(state,
                           sizeof(val_2_udat<val_type>)))
    val_2_udat<typename std::remove_cv<val_type>::type >(val);

    //����������ƣ�����metatable
    ::lua_pushstring(state, class_name<typename std::remove_cv<val_type>::type >::name());
    ::lua_gettable(state, LUA_GLOBALSINDEX);
    if (!lua_istable(state, -1))
    {
#if ZCE_CHECK_CLASS_NOMETA == 1
        ZCE_LOG(RS_ERROR,
                "[LUATIE][%s][%s] is not tie to lua,name[%s]? "
                "May be you don't register or name conflict? ",
                __ZCE_FUNC__,
                typeid(val).name(),
                class_name<std::remove_cv<val_type>::type >::name());
#endif
        ::lua_remove(state, -1);
        return;
    }
    ::lua_setmetatable(state, -2);
    return;
}

//ö��
template<typename val_type  >
void push_stack(lua_State *state,
                typename  std::enable_if<std::is_enum<val_type>::value, val_type>::type val)
{
    lua_pushnumber(state, val);
}

//����һ��val��
template<typename val_type  >
void push_stack(lua_State *state,
                typename  std::enable_if
                < !(std::is_pointer<val_type>::value ||
                    std::is_reference<val_type>::value ||
                    std::is_enum<val_type>::value), val_type
                >::type val)
{
    return push_stack_val<typename std::remove_cv<val_type>::type >(state, val);
}


///ָ��������ػ�
void push_stack(lua_State *state, char *const val);
void push_stack(lua_State *state, const char *const val);


template<typename array_type> int array_meta_get(lua_State *state);
template<typename array_type> int array_meta_set(lua_State *state);


///����һ�����������
template<typename array_type  >
void push_stack(lua_State *state, arrayref_2_udat<array_type> &ary_dat)
{
    new (::lua_newuserdata(state, sizeof(arrayref_2_udat<array_type>)))
    arrayref_2_udat<array_type>(ary_dat);
    lua_newtable(state);

    ::lua_pushstring(state, "__array_size");
    ::lua_pushnumber(state, static_cast<int>(ary_dat.ary_size_));
    ::lua_rawset(state, -3);

    ::lua_pushstring(state, "__index");
    ::lua_pushcclosure(state, array_meta_get<array_type>, 0);
    ::lua_rawset(state, -3);

    //��ֻ��
    if (!ary_dat.read_only_)
    {
        ::lua_pushstring(state, "__newindex");
        ::lua_pushcclosure(state, array_meta_set<array_type>, 0);
        ::lua_rawset(state, -3);
    }
    //���ֻ����__newindex
    else
    {
        ::lua_pushstring(state, "__newindex");
        ::lua_pushcclosure(state, newindex_onlyread, 0);
        ::lua_rawset(state, -3);
    }
    ::lua_setmetatable(state, -2);
    return;
}


//�������ص�push_back����
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

//���������long�ı�������Ϊlong�޷���ֲ������ֻ���壬��ʵ��
template<> void push_stack_val(lua_State *state,long val);
template<> void push_stack_val(lua_State *state,unsigned long val);
template<> void push_stack_val(lua_State* state,long long val);
template<> void push_stack_val(lua_State* state,unsigned long long val);

//=======================================================================================================

///��ȡһ��ָ��
template<typename ret_type>
typename std::enable_if<std::is_pointer<ret_type>::value, typename std::remove_cv<ret_type>::type >::type
read_stack(lua_State *state, int index)
{
    return ((typename std::remove_cv<ret_type>::type)(((lua_udat_base *)::lua_touserdata(state, index))->obj_ptr_));
}

template < typename ptr_type >
ptr_type read_stack_ptr(lua_State *state, int index)
{
    return (ptr_type)(((lua_udat_base *)::lua_touserdata(state, index))->obj_ptr_);
}

///�Ӷ�ջ�ж�ȡĳ������
template < typename ret_type >
ret_type read_stack_val(lua_State *state, int index)
{
    return *(ret_type *)(((lua_udat_base *)lua_touserdata(state, index))->obj_ptr_);
}

template<> char read_stack_val(lua_State *state, int index);
template<> unsigned char read_stack_val(lua_State *state, int index);
template<> short read_stack_val(lua_State *state, int index);
template<> unsigned short read_stack_val(lua_State *state, int index);
template<> int read_stack_val(lua_State *state, int index);
template<> unsigned int read_stack_val(lua_State *state, int index);
template<> float read_stack_val(lua_State *state, int index);
template<> double read_stack_val(lua_State *state, int index);
template<> bool read_stack_val(lua_State *state, int index);
template<> int64_t read_stack_val(lua_State *state, int index);
template<> uint64_t read_stack_val(lua_State *state, int index);
template<> std::string read_stack_val(lua_State *state, int index);


///��ȡһ������
template<typename ret_type>
typename std::enable_if<std::is_reference<ret_type>::value, typename std::remove_cv<ret_type>::type>::type
read_stack(lua_State *state, int index)
{
    return *((typename std::remove_cv<typename std::remove_reference<ret_type>::type >::type *)
             (((lua_udat_base *)lua_touserdata(state, index))->obj_ptr_));
}

///��ȡö��ֵ
template<typename ret_type>
typename std::enable_if<std::is_enum<ret_type>::value, ret_type>::type
read_stack(lua_State *state, int index)
{
    return (ret_type)lua_tonumber(state, index);
}

///��ȡһ��val
template<typename ret_type>
typename std::enable_if < !(std::is_pointer<ret_type>::value ||
                            std::is_reference<ret_type>::value ||
                            std::is_enum<ret_type>::value
                           ), typename std::remove_cv<ret_type>::type >::type
read_stack(lua_State *state, int index)
{
    return read_stack_val<typename std::remove_cv<ret_type>::type>(state, index);
}

///
template<typename val_type>
val_type pop_stack(lua_State *state)
{
    val_type t = read_stack<val_type>(state, -1);
    lua_pop(state, 1);
    return t;
}

//=======================================================================================================


///��LUA��һ���󶨵����飨ָ�룩����õ�����ĵ�ֵ
template<typename array_type>
int array_meta_get(lua_State *state)
{
    //�������
    int index = static_cast<int>(lua_tonumber(state, -1));
    arrayref_2_udat<array_type> *ptr = ((arrayref_2_udat<array_type> *)lua_touserdata(state, -2));
    if (index < 1 && index > static_cast<int>(ptr->ary_size_))
    {
        ZCE_LOG(RS_ERROR, "Lua script use error index [%d] to visit array %s[] size[%u].",
                index,
                typeid(array_type).name(),
                static_cast<uint32_t>(ptr->ary_size_));
        ZCE_ASSERT(false);
        ::lua_pushnil(state);

    }
    else
    {
        array_type *ary_ptr = (array_type *)(ptr->obj_ptr_);
        //ע�����Ϊ�˷���Lua��ϰ�ߣ�LUA�������Ǵ�1��ʼ
        push_stack<array_type>(state, ary_ptr[index - 1]);
    }

    //index Ӧ���������

    return 1;
}

///��LUA��������һ���󶨵����飨ָ�룩��ĳ��ֵ
template<typename array_type>
int array_meta_set(lua_State *state)
{
    arrayref_2_udat<array_type> *ptr = ((arrayref_2_udat<array_type> *)::lua_touserdata(state, -3));
    int index = static_cast<int>(::lua_tonumber(state, -2));

    //��index���߽���
    if (index < 1 && index > static_cast<int>(ptr->ary_size_))
    {
        ZCE_LOG(RS_ERROR, "Lua script use error index [%d] to visit array %s[] size[%u].",
                index,
                typeid(array_type).name(),
                static_cast<uint32_t>(ptr->ary_size_));
        ZCE_ASSERT(false);

    }
    else
    {
        array_type *ary_ptr = (array_type *)(ptr->obj_ptr_);
        //ע�����Ϊ�˷���Lua��ϰ�ߣ�LUA�������Ǵ�1��ʼ
        ary_ptr[index - 1] = read_stack<array_type>(state, -1);
    }

    return 0;
}


#define __TEST_VARIADIC_FUN 3

//=======================================================================================================

/*!
* @brief      ȫ�ֺ����ķ�װ�࣬��������ʵ��ע���ȫ�ֺ���
*             ����ret_type �Ƿ���void������2���汾������һ����g_func_void
* @tparam     ret_type  ����ֵ����
* @tparam     args_type ���������б�0-N������
* @note       ���Ҫע�⣬����Ĵ�����3���汾����3������ʵ������д�˵���5-6���汾��ʣ��3����
*             ������VARIADICչ���ķ�ʽ�����ֱ��Ѫ��ʷ��
*
*             ��һ����һ��Ǳ�ڵķ��գ����⣩���Ҽٶ��˲�������(չ��)��ʽ��
*             ��VS2013�ı���������GCC 4.8�ı������ϴ󲿷ֵı��չ��˳���Ǵ��ҵ��󣬵�����
*             ��չ��...�õ��Ĳ���˳��Ҳ�Ƿ��ġ�
*             ��ȷʵC++Ҳû����ȷ�涨�Ǵ��ҵ��󴫵ݲ������������ַ�����ʵ����
*             ע�⣬��Ȼ���ǿ����б��ʼ������(airfu�ṩ��һ���÷������������ò���������
*             չ���������ǻ���һ������������Ҫ����++���ں�������չ�����Ƿ��ģ�
*             ��������汾���棬���Ƿ���ȡ�����ġ�
*             ������汾��ʵ����һ�����⣬����GCC�ĸ澯��-Wsequence-point�ĸ澯,GCC׼ȷ
*             ��鵽��������⣬GCC����NBѽ����ʵҲ��û�������ģ��Լ������󱨵ģ���
*             BTW:���Ϊ�˱���澯������һ��û��д�����İ汾������pop_stack��������ȡ����,
*             ͬʱ������++�����������������⣨��ƭ����������
*             
*             �ڶ��֣�std::bind������չ���������ģ�bind�ڲ�����tuple����ôtupleӦ�ÿ��Խ�
*             �á�ע����tuple�����ǲ�����make_tuple��make_tupleҲ�Ǻ���ͬ�ϡ�
*             ��������˵ڶ��ֽⷨ������tuple�ĳ�ʼ������¼չ���Ĳ�����ֵ��Ȼ����ú�����
*             ��GCC��������뻹����-Wsequence-point�ĸ澯�����������³�ʼ������Ӧ���и澯��
*             �ԡ�
*              
*             �����֣�std::bindչ�������Ĺ����У���ʵ��tuple�Ĺ����ù�index_sequence������
*             չ��������ȥ����tuple��¼�����Ĺ��̡�Ҳȥ���˸澯��
*             ��������ȱ����ǣ���Ҫ������֧��C++14.
*             
*             ����variaic����չ����
*             �󲿷�variadic�ĺ�������ͨ���ݹ�չ������������⣬��������޷��ݹ飬
*             ���⣬����std::bind��ʵ�֣����ǿ����ƿ�������⣬������ɱ��е�ߡ�
*             ����һ��չ�����������ó�ʼ���ķ���������
*             std::bindʹ�õ���tuple �� index_sequence  ����������洢�Լ�˳������
*             ���Ǽ�ʹ����tuple��ȷ�õ�������Ҳ����ʹ��++�ĺ�����������ҲҪ��index_sequence
*             ��read_stack��
*
*/
template < bool last_yield,
           typename ret_type,
           typename ...args_type >
class g_func_ret
{
public:

#if defined __TEST_VARIADIC_FUN && __TEST_VARIADIC_FUN == 1
    //���������ں�����������չ���İ汾 LINUX GCC 7��-Wsequence-point�ĸ澯,
    //����澯����ȷ�ģ������ȷʵ��������ȷ�Ĳ���˳��
    //����������read_stack����pop����Ȼ������ƭ���������޴��ı�����
    static int invoke(lua_State* state)
    {
        //ȡ������ָ�룬����ת��
        void* upvalue_1=::lua_touserdata(state,lua_upvalueindex(1));
        ret_type(*fun_ptr)(args_type...)=(ret_type(*)(args_type...)) (upvalue_1);
        //�����������˳�������ο�note��˵����
        //���ݺ����Ƿ��з���ֵ��������δ����Ƿ�push_stack
        //ע������ʹ�õ���para_idx--,�����ʵ�Ƿ��Ŷ�ȡ��ջ�ģ���Ϊ...�Ǵ��ҵ���
        int para_idx=::lua_gettop(state);
        //ͬʱע��decay��������˻��˲�������Ϊ�Һܶ඼����ʱ����
        push_stack<ret_type>(state,
                             fun_ptr(read_stack<typename std::decay<args_type>::type>
                             (state,para_idx--)...));
        return (last_yield)?::lua_yield(state,1):1;
    }
    
#elif defined __TEST_VARIADIC_FUN && __TEST_VARIADIC_FUN == 2
    //����汾������tuple��ֵչ���İ汾��������Linux GCC����Ȼ�и澯��������
    //����Ϊ����澯�Ǵ���ģ���Ϊ�ҵ�չ����tuple��ʼ����˳������ȷ��
    static int invoke(lua_State* state)
    {
        //ȡ������ָ�룬����ת��
        void* upvalue_1=::lua_touserdata(state,lua_upvalueindex(1));
        ret_type(*fun_ptr)(args_type...)=(ret_type(*)(args_type...)) (upvalue_1);
        //ʹ��tupleչ�������������Ǵ�����չ��
        //ͬʱע��decay��������˻��˲�������Ϊ�Һܶ඼����ʱ����
        int para_idx=1;
        std::tuple<typename std::decay<args_type>::type...> para={
            (read_stack<typename std::decay<args_type>::type>(state,para_idx++))...};
        //ʹ��tupleִ�к�������
        push_stack<ret_type>(state,
                             zce::g_func_tuplearg_invoke(fun_ptr,para));
        return (last_yield)?::lua_yield(state,1):1;
    }
#else
    //����make_index_sequenceչ��VARIADIC�ķ����������������ҪC++ 14��֧��
    static int invoke(lua_State* state)
    {
        const static int para_count=sizeof...(args_type);
        return _invoke_witch_stack(state,std::make_index_sequence<para_count>());
    }

private:

    template<std::size_t... I>
    static int _invoke_witch_stack(lua_State* state,std::index_sequence<I...>)
    {
        //ȡ������ָ�룬����ת��
        void* upvalue_1=::lua_touserdata(state,lua_upvalueindex(1));
        ret_type(*fun_ptr)(args_type...)=(ret_type(*)(args_type...)) (upvalue_1);

        push_stack<ret_type>(state,fun_ptr(read_stack<typename std::decay<args_type>::type>
            (state,I+1)...));
        return (last_yield)?::lua_yield(state,1):1;
    }
#endif
};

///ȫ��û�з���ֵ�ĺ�����װ����ϸ��Ϣ��g_func_ret
template < bool last_yield,
           typename... args_type >
class g_func_void
{
public:
    static int invoke(lua_State *state)
    {
        const static int para_count=sizeof...(args_type);
        return _invoke_witch_stack(state,std::make_index_sequence<para_count>());
    }
private:

    template<std::size_t... I>
    static int _invoke_witch_stack(lua_State* state,std::index_sequence<I...>)
    {
        //ȡ������ָ�룬����ת��
        void* upvalue_1=::lua_touserdata(state,lua_upvalueindex(1));
        void (*fun_ptr)(args_type...)=(void(*)(args_type...)) (upvalue_1);

        fun_ptr(read_stack<typename std::decay<args_type>::type>
            (state,I+1)...);
        return (last_yield)?::lua_yield(state,0):0;
    }
};

///lua��ȡ����������ݵ�meta table��__index��Ӧ����
int class_meta_get(lua_State *state);

///luaд������������ݵ�meta table��__newindex��Ӧ����
int class_meta_set(lua_State *state);


///���ø�ĸ�ĺ���
int class_parent(lua_State *state);


/*!
* @brief      ��װ��Ĺ��캯����LUAʹ��
* @tparam     class_type ���������
* @tparam     args_type  ����Ĳ�����
*/
template<typename class_type, typename... args_type>
class constructor
{
public:
    static int invoke(lua_State* state)
    {
        const static int para_count=sizeof...(args_type);
        return _invoke_witch_stack(state,std::make_index_sequence<para_count>());
    }
private:
    template<std::size_t... I>
    static int _invoke_witch_stack(lua_State* state,std::index_sequence<I...>)
    {
        //new һ��user data����<T>�Ĵ�С,ͬʱ��ͬʱ��placement new �ķ�ʽ��
        //��ָ��ʽlua_newuserdata����ģ���ɹ��캯��
        new (::lua_newuserdata(state,
                               sizeof(val_2_udat<class_type>))) \
            val_2_udat<class_type>(read_stack<args_type>(state,I+2)...);

        ::lua_pushstring(state,class_name<class_type>::name());
        ::lua_gettable(state,LUA_GLOBALSINDEX);

        ::lua_setmetatable(state,-2);
        return 1;
    }
};


//����USER_DATA�Ļ��������,����userdata_base��ʵ��һ��LUAʹ�õ�userdata����Ļ��࣬
//���������3�֣�val,ptr,ref,����val���������ͷŶ���ptr��ref�Ķ���ʲô����������
//�����ͱ�֤�����㴫�ݸ�LUAʲô�����ǵ��������ڶ�����ȷ�ģ�
int destroyer(lua_State *state);


/*!
* @brief      ��C++11 C++14������Ч�����ʵ�ֵ��ຯ���Ž�
* @tparam     last_yield ��������Ƿ�ʹ��lua_yield����
* @tparam     class_type �������
* @tparam     ret_type   ����ֵ������
* @tparam     ...args_type ��εĲ��������б�
* note        ��һ��ֵ��ע���������ο�˵��g_func_ret
*/
template<bool last_yield, typename class_type, typename ret_type, typename ...args_type>
class member_func_ret
{
public:
    static int invoke(lua_State *state)
    {
        const static int para_count=sizeof...(args_type);
        return _invoke_witch_stack(state,std::make_index_sequence<para_count>());
    }

private:

    template<std::size_t... I>
    static int _invoke_witch_stack(lua_State* state,std::index_sequence<I...>)
    {
        //push�ǽ���������ջ
        void* upvalue_1=::lua_touserdata(state,lua_upvalueindex(1));
        typedef ret_type(class_type::*mem_fun)(args_type...);
        mem_fun fun_ptr=*(mem_fun*)(upvalue_1);
        //��һ�������Ƕ���ָ��
        class_type* obj_ptr=read_stack<class_type*>(state,1);

        //Ϊʲô��2��1��������ָ��ռ����
        push_stack<ret_type>(state,
            (obj_ptr->*fun_ptr)\
                             (read_stack<typename std::decay<args_type>::type>(state,I+2)...));
        return (last_yield)?::lua_yield(state,1):1;
    }
};

//����void�ĳ�Ա����
template<bool last_yield, typename class_type, typename ...args_type>
class member_func_void
{
public:
    static int invoke(lua_State *state)
    {
        const static int para_count=sizeof...(args_type);
        return _invoke_witch_stack(state,std::make_index_sequence<para_count>());
    }

private:

    template<std::size_t... I>
    static int _invoke_witch_stack(lua_State* state,std::index_sequence<I...>)
    {
        //push�ǽ���������ջ
        void* upvalue_1=::lua_touserdata(state,lua_upvalueindex(1));
        typedef void (class_type::*mem_fun)(args_type...);
        mem_fun fun_ptr=*(mem_fun*)(upvalue_1);
        //��һ�������Ƕ���ָ��
        class_type* obj_ptr=read_stack<class_type*>(state,1);

        //Ϊʲô��2��1��������ָ��ռ����
        (obj_ptr->*fun_ptr)\
            (read_stack<typename std::decay<args_type>::type>(state,I+2)...);
        return (last_yield)?::lua_yield(state,0):0;
    }
};


/*!
* @brief      ��Ա�����Ĵ���Ļ��࣬����class_meta_get,class_meta_set�ڲ�����
* @note       memvar_base ע���䲻��һ����ģ��ĺ�������һ������
*             �����Ϳ��Ա�֤ͨ��LUA user data�����void * ת��Ϊ memvar_base *
*             ��ͨ��memvar_base *�ĵ��� get, set ��æǡǡ��ֱ�ӵ��õ�������
*             member_var <class_type,var_type>
*/
class memvar_base
{
public:
    virtual void get(lua_State *state) = 0;
    virtual void set(lua_State *state) = 0;
};


/*!
* @brief      ��Ա�����󶨵ĸ����࣬Ҳ����user data�������
*
* @tparam     class_type class_type Ϊclass ���ͣ�
* @tparam     var_type   Ϊ��������
*/
template<typename class_type, typename var_type>
class member_var: memvar_base
{
public:
    member_var(var_type class_type::* val):
        var_ptr_(val)
    {
    }

    ///�������ĳ��
    void set(lua_State *state)
    {
        //��ο������ǲ����е���Σ���ʵ���ɵ��������������Σ�����������ĳ�Ա����ָ����ɳ�Ա��д��
        //float A::*pfl = &A::float_var;
        //A a1;  a1.*pfl = 0.5f;
        //�����Ҫ��лairfu GG�ľ��Ľ���
        read_stack<class_type *>(state, 1)->*(var_ptr_) = read_stack<var_type>(state, 3);
    }

    //get��LUA��ȡ�Ĳ�����Ҳ���ǰ�C++�����ݶ�ȡ��LUA���棬������PUSH
    void get(lua_State *state)
    {
        //read_stack��ʵ���ǰ���Ķ����ָ���ȡ������
        //Ȼ��ͨ�����Աָ�룬�ѳ�Ա��ȡ����
        push_stack<var_type>(state,
                             read_stack<class_type *>(state, 1)->*(var_ptr_));
    }

    //�����ĳ�Աָ��
    var_type class_type::*var_ptr_;
};



/*!
* @brief      ��Ա�������͵��࣬����userdata�Ĵ���
*
* @tparam     class_type ������
* @tparam     ary_type   ��������
* @tparam     ary_size   ���鳤��
* @note
*/
template<typename class_type, typename ary_type, size_t ary_size>
class member_array: memvar_base
{
public:

    //���캯��
    member_array(ary_type(class_type::* mem_ary_ptr)[ary_size],
                 bool read_only):
        mem_ary_ptr_(mem_ary_ptr),
        read_only_(read_only)
    {
    }


    ///���ڳ�Ա���飬����û���κ����壬
    void set(lua_State *state)
    {
        luaL_error(state, "Member array ptr is read only ,can't modify,please check your code.");
    }

    //get��LUA��ȡ�Ĳ�����Ҳ���ǰ�C++�����ݶ�ȡ��LUA���棬������PUSH
    void get(lua_State *state)
    {
        //read_stack��ʵ���ǰ���Ķ����ָ���ȡ������
        //Ȼ��ͨ�����Աָ�룬�ѳ�Ա��ȡ����
        ary_type *ary_ptr = read_stack<class_type *>(state, 1)->*(mem_ary_ptr_);
        arrayref_2_udat<ary_type> ary_dat(ary_ptr, ary_size, read_only_);
        push_stack(state, ary_dat);
    }

    ///��Ա�����ָ��
    ary_type(class_type::* mem_ary_ptr_)[ary_size];

    ///�Ƿ�ֻ��
    bool                   read_only_;
};

};
};  //namespace



//=======================================================================================================
template<typename class_type> class Candy_Tie_Class;

/*!
* @brief      Lua ���ַ�װ�Ļ��࣬�󲿷ֹ��ܶ���װ�������
*             ������ջ�����ַ�װ��class��������ע�ᣬ
* @note       ��ΪЭ�̺�Tie��Ӧ�û�ʹ�õ��ⲿ�֡����Զ���
*             ��Ϊ����
*/
class ZCE_Lua_Base
{

protected:

    ///���캯��
    explicit ZCE_Lua_Base(lua_State *lua_state);
    ///��������
    virtual ~ZCE_Lua_Base();

public:

    ///ִ��һ��lua�ļ�
    int do_file(const char *filename);
    ///ִ��һ��LUA��buffer
    int do_buffer(const char *buff, size_t sz);

    ///dump C����lua�Ķ�ջ��
    void enum_stack();
    ///dump lua���еĵĶ�ջ�����ڼ��lua����ʱ�����⣬�������
    void dump_stack();


    ///��LUAע��int64_t�����ͣ���ΪLUA�ڲ���numberĬ����double��������ʵ�޷���ʾ������Ҫע�����
    void reg_int64();
    ///��LUAע��uint64_t������
    void reg_uint64();
    ///��LUAע��std::string������
    void reg_stdstring();


    //��װlua��ջ�ĺ���

    /// ɾ��ָ�������ϵ�Ԫ�أ�������λ��֮�ϵ�����Ԫ�����ơ�
    inline void stack_remove(int index)
    {
        ::lua_remove(lua_state_, index);
    }

    ///������ָ��λ��֮�ϵ�����Ԫ���Կ���һ���۵Ŀռ䣬Ȼ��ջ��Ԫ���Ƶ���λ��
    inline void stack_insert(int index)
    {
        return ::lua_insert(lua_state_, index);
    }

    ///����ջ��Ԫ�ظ���
    inline int stack_gettop()
    {
        return ::lua_gettop(lua_state_);
    }

    ///����ջ��Ԫ�ظ��������ԭ����ջ�ռ�С��index�����nil���������index��ɾ������Ԫ��
    inline void stack_settop(int index)
    {
        return ::lua_settop(lua_state_, index);
    }

    ///ȷ����ջ�ռ���extra��ô��
    inline int stack_check(int extra)
    {
        return ::lua_checkstack(lua_state_, extra);
    }

    ///��indexλ���ϵ�ֵ�ڶ�ջ������pushһ��
    inline void stack_pushvalue(int index)
    {
        return ::lua_pushvalue(lua_state_, index);
    }

    ///ȡ��indexλ�õ����ͣ�����ֵLUA_TNIL��ö��ֵ
    ///lua_type is LUA_TNIL, LUA_TNUMBER, LUA_TBOOLEAN, LUA_TSTRING, LUA_TTABLE,
    ///LUA_TFUNCTION, LUA_TUSERDATA, LUA_TTHREAD, and LUA_TLIGHTUSERDATA
    inline int stack_type(int index)
    {
        return ::lua_type(lua_state_, index);
    }

    ///�������index��λ�õ����ݵ������Ƿ���lua_t
    inline bool stack_istype(int index, int lua_t)
    {
        return ::lua_type(lua_state_, index) == lua_t;
    }

    ///�õ���ջ��indexλ�õ��������ƣ�
    inline const char *stack_typename(int index)
    {
        return ::lua_typename(lua_state_, lua_type(lua_state_, index));
    }

    ////���indexλ�õ����ͣ�
    inline void stack_checktype(int index, int lua_t)
    {
        return ::luaL_checktype(lua_state_, index, lua_t);
    }

    ///�õ�����ĳ���
    ///for tables, this is the result of the length operator ('#');
    ///for userdata, this is the size of the block of memory allocated for the userdata;
    ///for other values, it is 0.
    inline size_t get_objlen(int index)
    {
        return ::lua_objlen(lua_state_, index);
    }

    ///ȡ��table������Ԫ�ظ���,ע�����stack_objlen�����,�˺������Բ���Ч���Ǻ�
    inline size_t get_tablecount(int index)
    {
        size_t table_count = 0;
        //���������
        ::lua_pushnil(lua_state_);
        while (::lua_next(lua_state_, index) != 0)
        {
            ++table_count;
        }
        return table_count;
    }

    ///ͨ������ȡ��lua���󣬲��Ҽ��
    inline int get_luaobj(const char *obj_name, int luatype)
    {
        ::lua_pushstring(lua_state_, obj_name);
        ::lua_gettable(lua_state_, LUA_GLOBALSINDEX);
        if (::lua_type(lua_state_, -1) != luatype)
        {
            return -1;
        }
        return 0;
    }

    /*!
    * @brief      ��LUA����һ������LUA���ԣ�ȫ�ֱ��������ƺͱ�����Ӧֵ�Ŀ�����
    * @tparam     val_type ������������ͣ������val������LUA���汣�������������ptr��ref��lua�ڲ�����ֱ���޸��������
    * @param      name ����
    * @param      val  ����ı�����ע�����Ҫ�������ã���Ҫset_gvar<var_type &>(ref)������д
    */
    template<typename var_type>
    void set_gvar(const char *name, var_type var)
    {
        //���ƶ���
        ::lua_pushstring(lua_state_, name);
        //ģ�庯��������val_type�������push
        zce::luatie::push_stack<var_type>(lua_state_, var);
        ::lua_settable(lua_state_, LUA_GLOBALSINDEX);
    }

    ///�������ƣ���LUA��ȡһ������
    template<typename var_type>
    var_type get_gvar(const char *name)
    {
        ::lua_pushstring(lua_state_, name);
        ::lua_gettable(lua_state_, LUA_GLOBALSINDEX);
        return zce::luatie::pop_stack<var_type>(lua_state_);
    }

    /*!
    * @brief      ��LUA����һ�����������,��LUA�ڲ�����һ����ص�userdata��
    * @tparam     array_type
    * @param      name
    * @param      ary_data
    * @param      ary_size
    * @param      read_only
    * @note       ע��������ร�һ��Ҫע��ม�
    */
    template<typename array_type>
    void set_garray(const char *name,
                    array_type ary_data[],
                    size_t ary_size,
                    bool read_only = false)
    {
        zce::luatie::arrayref_2_udat<array_type> aux_ary(ary_data, ary_size, read_only);
        //���ƶ���
        ::lua_pushstring(lua_state_, name);
        zce::luatie::push_stack(lua_state_, aux_ary);
        ::lua_settable(lua_state_, LUA_GLOBALSINDEX);
    }

    ///��LUA�л�ȡһ��ȫ�ֵ�����
    template<typename ary_type>
    int get_garray(const char *name,
                   ary_type ary_data[],
                   size_t &ary_size)
    {
        //���ƶ���
        ::lua_pushstring(lua_state_, name);
        ::lua_gettable(lua_state_, LUA_GLOBALSINDEX);

        //�������һ��table������Ŷ
        if (!::lua_isuserdata(lua_state_, -1))
        {
            ::lua_remove(lua_state_, -1);
            return -1;
        }
        zce::luatie::arrayref_2_udat<ary_type> aux_ary =
            *(zce::luatie::arrayref_2_udat<ary_type> *)::lua_touserdata(lua_state_, -1);
        ary_size = aux_ary.ary_size_;
        for (size_t i = 0; i < ary_size; ++i)
        {
            ary_data[i] = *((ary_type *)aux_ary.obj_ptr_ + i);
        }
        return 0;
    }

    ///��LUAע��һ��ȫ�ֺ�����������ľ�̬������lua����
    ///������ϸ˵����ο�reg_gfun_all
    template<typename ret_type, typename... args_type>
    void reg_gfunc(const char *name, ret_type(*func)(args_type...))
    {
        reg_g_func<false, ret_type, args_type...>(name, func);
    }


    ///��LUAע��һ��ȫ�ֺ�����������ľ�̬������lua����.��reg_gfun�������ǣ�����ʹ��lua_yield���أ�
    ///������ϸ˵����ο�reg_gfun_all
    template<typename ret_type, typename... args_type>
    void reg_yeild_gfunc(const char *name, ret_type(*func)(args_type...))
    {
        reg_g_func<true, ret_type, args_type...>(name, func);
    }

    /*!
    * @brief      ��һ��������������ݷ���Lua��table�У����ݵ����������ͻ�ѡ��ͬ��
    *             ���غ���
    * @tparam     iter_type   �����������ͣ�����ȻҲ������ָ�룩
    * @param      table_name  table������
    * @param      first       ��һ��first������
    * @param      last        ���һ��last�ĵ�����
    */
    template<class iter_type >
    void to_luatable(const char *table_name,
                     const iter_type first,
                     const iter_type last)
    {
        to_luatable(table_name,
                    first,
                    last,
                    typename std::iterator_traits<iter_type>::iterator_category());
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
                             typename std::iterator_traits<typename container_type::iterator>::iterator_category());
    }

    //
    template<class array_type>
    int from_luatable(const char *table_name,
                      array_type *array_dat)
    {
        //����������ƣ�ȡ�����metatable�ı�����˵ԭ�͡�
        ::lua_pushstring(lua_state_, table_name);
        ::lua_gettable(lua_state_, LUA_GLOBALSINDEX);

        if (!lua_istable(lua_state_, -1))
        {
            ZCE_LOG(RS_ERROR, "[LUATIE] table name[%s] is not tie to lua.",
                    table_name);
            ::lua_remove(lua_state_, -1);
            return -1;
        }

        //first key ,ferg���Ұ��ⶫ�������ɵ�����
        ::lua_pushnil(lua_state_);
        while (::lua_next(lua_state_, -2) != 0)
        {
            // uses 'key' (at index -2) and 'value' (at index -1)
            int index = zce::luatie::read_stack<int>(lua_state_, -2) - 1;
            array_dat[index] = zce::luatie::read_stack <array_type>(lua_state_, -1);
            // removes 'value'; keeps 'key' for next iteration
            ::lua_remove(lua_state_, -1);
        }
        return 0;
    }

    ///����LUA�ĺ�����ֻ��һ������ֵ
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

    ///����LUA�ĺ�����ֻ��һ������ֵ
    template<typename ret_type1, typename... args_type>
    int call_luafun_1(const char *fun_name, ret_type1 &ret_val1, args_type... args)
    {
        int ret = 0;
        ret = call_luafun(fun_name, 1, args...);
        if (ret != 0)
        {
            return ret;
        }
        //�ڶ�ջ��������ֵ
        ret_val1 = zce::luatie::read_stack<ret_type1>(lua_state_, -1);
        lua_pop(lua_state_, 1);
        return 0;
    }

    ///����LUA�ĺ���������������ֵ
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
        //�ڶ�ջ��������ֵ
        ret_val1 = zce::luatie::read_stack<ret_type1>(lua_state_, -2);
        ret_val2 = zce::luatie::read_stack<ret_type2>(lua_state_, -1);
        lua_pop(lua_state_, 2);
        return 0;
    }

    ///����LUA�ĺ���������������ֵ,�ðɾ�֧�ֵ�3������ֵ�ѣ�ʵ��û��Ȥ��
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
        //�ڶ�ջ��������ֵ,ע�����˳��
        ret_val1 = zce::luatie::read_stack<ret_type1>(lua_state_, -3);
        ret_val2 = zce::luatie::read_stack<ret_type2>(lua_state_, -2);
        ret_val3 = zce::luatie::read_stack<ret_type3>(lua_state_, -1);
        lua_pop(lua_state_, 3);
        return 0;
    }


    /*!
    * @brief      ��Lua����newһ��table��ͬʱ��pair_list������������ݷ��뵽table��
    * @tparam     pair_tlist pair list�������б�������std::pair
    * @param      table_name ������ƣ�
    * @param      pair_list  piar��listͬʱ��pair��first����Ϊkey��pair��second��Ϊvalue��
    * @note       ͨ������������������ɵİ�ö��ѽ��ע���luaʹ�ã������������ַ�ʽ
    *             lua_tie.new_table("tolua_enum",std::make_pair("ENUM_0001", ENUM_0001),
    *                 std::make_pair("ENUM_0002", ENUM_0002);
    */
    template<typename ...pair_tlist>
    void new_table(const char *table_name, pair_tlist ... pair_list)
    {
        ::lua_pushstring(lua_state_, table_name);
        lua_newtable(lua_state_);
        //��table�������pair
        newtable_addkv(pair_list...);
        ::lua_settable(lua_state_, LUA_GLOBALSINDEX);
    }



    /*!
    * @brief      ����ĸ�Luaʹ�ã��������metatable�ı�����˵ԭ�͵ı�
    * @tparam     class_type      ���������
    * @return     Candy_Tie_Class ���ڷ������ĳ�Ա����������д������.�Ĳ���
    * @param      class_name      ������ƣ���Lua��ʹ��
    * @param      read_only       �����������Ƿ�ֻ����������д
    */
    template<typename class_type>
    Candy_Tie_Class<class_type> reg_class(const char *class_name,
                                          bool read_only = false)
    {

        //��T������,�������
        ::lua_pushstring(lua_state_, zce::luatie::class_name<class_type>::name(class_name));
        //new һ��table�����table����Ϊ���������metatable�ģ�ĳ�̶ֳ���Ҳ����˵��ԭ�ͣ���
        ::lua_newtable(lua_state_);

        //__name���Ǳ�׼��Ԫ������������������ʹ��
        ::lua_pushstring(lua_state_, "__name");
        ::lua_pushstring(lua_state_, zce::luatie::class_name<class_type>::name());
        ::lua_rawset(lua_state_, -3);

        //��meta_get������Ϊ__index����
        ::lua_pushstring(lua_state_, "__index");
        ::lua_pushcclosure(lua_state_, zce::luatie::class_meta_get, 0);
        ::lua_rawset(lua_state_, -3);

        if (!read_only)
        {
            //��ֻ�������meta_set������Ϊ__newindex����
            ::lua_pushstring(lua_state_, "__newindex");
            ::lua_pushcclosure(lua_state_, zce::luatie::class_meta_set, 0);
            ::lua_rawset(lua_state_, -3);
        }
        else
        {
            ::lua_pushstring(lua_state_, "__newindex");
            ::lua_pushcclosure(lua_state_, zce::luatie::newindex_onlyread, 0);
            ::lua_rawset(lua_state_, -3);
        }

        //�������պ���
        ::lua_pushstring(lua_state_, "__gc");
        ::lua_pushcclosure(lua_state_, zce::luatie::destroyer, 0);
        ::lua_rawset(lua_state_, -3);

        ::lua_settable(lua_state_, LUA_GLOBALSINDEX);

        return Candy_Tie_Class<class_type> (this, read_only);
    }



    /*!
    * @brief      ע����Ĺ��캯����
    * @tparam     class_type class_type ����
    * @tparam     construct_func �ǹ��캯���ķ�װ��zce::luatie::constructor
    * @return     int
    * @param      func
    */
    template<typename class_type, typename construct_func>
    int class_constructor(construct_func func)
    {
        //����������ƣ�ȡ�����metatable�ı�����˵ԭ�͡�
        lua_pushstring(lua_state_, zce::luatie::class_name<class_type>::name());
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);

        //���ջ���ǲ���һ�������д�����
        if (!lua_istable(lua_state_, -1))
        {
            ZCE_LOG(RS_ERROR, "[LUATIE] class name[%s] is not tie to lua.",
                    zce::luatie::class_name<class_type>::name());
            ZCE_ASSERT(false);
            lua_pop(lua_state_, 1);
            return -1;
        }

        //��������metatable�ı�����һ��metatable������������һ��__call�Ķ�Ӧ����
        //������Ŀ���������ģ�__call�Ƕ�Ӧһ��()���ã���ʵ�岻�Ǻ���ʽ���ĵ��ú���
        //LUA�г��������ĵ��ã�var1,var2�ǹ���������͹��캯��һ��������ط�Ҫ��лfergzhang GG�İ���
        //object =class_name(var1,var2)
        lua_newtable(lua_state_);

        lua_pushstring(lua_state_, "__call");
        lua_pushcclosure(lua_state_, func, 0);
        lua_rawset(lua_state_, -3);
        //�������table��Ϊclass ԭ�͵�metatable.
        //����˵�������table��Ϊclass metatable��metatable.
        lua_setmetatable(lua_state_, -2);

        lua_pop(lua_state_, 1);
        return 0;
    }


    /*!
    * @brief      ������Ѷ��ؼ̳�ʵ���ˣ��������ferg���ۣ����ý���������岻��
    *             ��������
    * @tparam     class_type  ����
    * @tparam     parent_type ������
    * @return     int == 0 ����ֵ��ʶ�ɹ�
    */
    template<typename class_type, typename parent_type>
    int class_inherit()
    {
        //����������ƣ�ȡ�����metatable�ı�����˵ԭ�͡�
        lua_pushstring(lua_state_, zce::luatie::class_name<class_type>::name());
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);

        //���ջ����һ����
        if (!lua_istable(lua_state_, -1))
        {
            ZCE_LOG(RS_ERROR, "[LUATIE] class name[%s] is not tie to lua.",
                    zce::luatie::class_name<class_type>::name());
            ZCE_ASSERT(false);
            lua_pop(lua_state_, 1);
            return -1;
        }

        //����__parent Ϊ �������ƣ�Ŀǰ���ܶ��ؼ̳�,�Һ�freg���۹�����ʵʵ�ֶ��ؼ���Ҳ���ԣ�
        //�����ڴ����ѯ��ʱ������鷳

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
        lua_pushstring(lua_state_, zce::luatie::class_name<parent_type>::name());
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);
        if (!lua_istable(lua_state_, -1))
        {
            ZCE_LOG(RS_ERROR, "[LUATIE] class name[%s] is not tie to lua.",
                    zce::luatie::class_name<parent_type>::name());
            ZCE_ASSERT(false);
            lua_pop(lua_state_, 1);
            return -1;
        }
        lua_rawset(lua_state_, -3);

        //�Ӷ�ջ����push_metaȡ�õ�vlue
        lua_pop(lua_state_, 1);
        return 0;
    }


    /*!
    * @brief      ��һ���ࣨ��meta table��ע��󶨳�Ա����
    * @tparam     class_type �󶨵��������
    * @tparam     var_type   ��Ա����
    * @return     int        �Ƿ�󶨳ɹ�
    * @param      name       �󶨵�����
    * @param      class_type::*val �󶨵ĳ�Ա������ָ��
    * @note       Luatinker�л���һ��ģ�����base_type����Ա�������࣬һ������Ϊclass_type
    *             ��base_type��һ����,����Ҳ���԰�һ������ĳ�Ա���������Lua��meta table���棬
    *             ���Լ��о��ⶫ���Ĵ��ڱ�Ҫ�Բ���ȥ����
    */
    template<typename class_type, typename var_type>
    int class_mem_var(const char *name, var_type class_type::*val)
    {
        //����������ƣ�ȡ�����metatable�ı�����˵ԭ�͡�
        ::lua_pushstring(lua_state_, zce::luatie::class_name<class_type>::name());
        ::lua_gettable(lua_state_, LUA_GLOBALSINDEX);

        //
        if (!lua_istable(lua_state_, -1))
        {
            ZCE_LOG(RS_ERROR, "[LUATIE] class name[%s] is not tie to lua.",
                    zce::luatie::class_name<class_type>::name());
            ZCE_ASSERT(false);
            lua_pop(lua_state_, 1);
            return -1;
        }

        ::lua_pushstring(lua_state_, name);
        //mem_var �̳���var_base,ʵ�ʵ��õ�ʱ������var_base���麯����ɻص���
        new (lua_newuserdata(lua_state_,
                             sizeof(zce::luatie::member_var<class_type, var_type>))) \
        zce::luatie::member_var<class_type, var_type>(val);
        ::lua_rawset(lua_state_, -3);

        lua_pop(lua_state_, 1);
        return 0;
    }


    /*!
    * @brief      ��һ�����meta table �󶨳�Ա����
    * @tparam     class_type �������
    * @tparam     ary_type   ���������
    * @tparam     ary_size   ����ĳ���
    * @return     int        == 0��ʾ�ɹ�
    * @param      name       ע�������
    * @param      mem_ary    �����ָ��
    * @param      read_only  ����ֻ��
    */
    template<typename class_type, typename ary_type, size_t ary_size>
    int class_mem_ary(const char *name,
                      ary_type(class_type:: *mem_ary)[ary_size],
                      bool read_only = false)
    {
        //����������ƣ�ȡ�����metatable�ı�����˵ԭ�͡�
        lua_pushstring(lua_state_, zce::luatie::class_name<class_type>::name());
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);

        //
        if (!lua_istable(lua_state_, -1))
        {
            ZCE_LOG(RS_ERROR, "[LUATIE] class name[%s] is not tie to lua.",
                    zce::luatie::class_name<class_type>::name());
            ZCE_ASSERT(false);
            lua_pop(lua_state_, 1);
            return -1;
        }

        lua_pushstring(lua_state_, name);
        //mem_var �̳���var_base,ʵ�ʵ��õ�ʱ������var_base���麯����ɻص���
        new (lua_newuserdata(lua_state_,
                             sizeof(zce::luatie::member_array<class_type, ary_type, ary_size>))) \
        zce::luatie::member_array<class_type, ary_type, ary_size>(mem_ary, read_only);
        lua_rawset(lua_state_, -3);

        lua_pop(lua_state_, 1);
        return 0;
    }

    ///ע��һ����ĳ�Ա������������name����ϸ�ĺ���˵������ο�class_mem_fun_all
    template<typename class_type, typename ret_type, typename... args_type>
    int class_memfunc(const char *name, ret_type(class_type::*func)(args_type...))
    {
        return class_mem_func<false, class_type, ret_type, args_type...>(name, func);
    }

    ///ע��һ����ĳ�Ա��������class_mem_fun�������Ǻ������ķ��ػ����lua_yield,����������name��
    ///��ϸ�ĺ���˵������ο�class_mem_fun_all
    template<typename class_type, typename ret_type, typename... args_type>
    int class_mem_yield_func(const char *name, ret_type(class_type::*func)(args_type...))
    {
        return class_mem_func<true, class_type, ret_type, args_type...>(name, func);
    }

    ///����ĳ����������ջ
    template<typename val_type >
    inline void push(val_type val)
    {
        zce::luatie::push_stack<val_type>(lua_state_, val);
    }

    ///��ȡ��ջ�ϵ�ĳ������
    template<typename val_type >
    inline val_type read(int index)
    {
        return zce::luatie::read_stack<val_type>(lua_state_, index);
    }

    template<typename val_type >
    inline val_type pop()
    {
        return zce::luatie::pop_stack<val_type>(lua_state_);
    }

    //
    inline lua_State  *get_state()
    {
        return lua_state_;
    }

protected:

    ///����������չ��kv pari list
    template<typename... pair_tlist>
    void newtable_addkv(pair_tlist ... pair_list)
    {
        return;
    }
    ///����������չ��kv pari list,ͬʱ�ѵ�һ����������table��
    template<typename pair_type, typename... pair_tlist>
    void newtable_addkv(pair_type pair_dat, pair_tlist ... pair_list)
    {
        zce::luatie::push_stack<typename pair_type::first_type>(lua_state_, pair_dat.first);
        zce::luatie::push_stack<typename pair_type::second_type>(lua_state_, pair_dat.second);
        lua_settable(lua_state_, -3);
        newtable_addkv(pair_list...);
        return;
    }

    /*!
    * @brief      ����LUA�ĺ�����û�з���ֵ���������ݲ�ȡ��һ������ֵ��
    * @tparam     args_type ���������б�
    * @return     int == 0 ��ʾ�ɹ�
    * @param      fun_name ��������
    * @param      ret_num  ����ֵ������
    * @param      args     �����б�
    */
    template<typename... args_type>
    int call_luafun(const char *fun_name, size_t ret_num, args_type... args)
    {
        int ret = 0;
        //���������ĺ��������Ҽ�¼��ջ�ĵ�ַ
        lua_pushcclosure(lua_state_, zce::luatie::on_error, 0);
        int errfunc = lua_gettop(lua_state_);

        lua_pushstring(lua_state_, fun_name);
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);
        //������Ƿ��Ǻ���
        if (!lua_isfunction(lua_state_, -1))
        {
            ZCE_LOG(RS_ERROR, "[LUATIE] call_luafun() attempt to call global `%s' (not a function)",
                    fun_name);
            lua_pop(lua_state_, 1);
            return -1;
        }

        size_t arg_num = sizeof...(args);
        if (arg_num)
        {
            //�����ջ������args
            zce::luatie::push_stack(lua_state_, args...);
        }



        //����lua�ĺ�����
        ret = ::lua_pcall(lua_state_,
                          static_cast<int>(arg_num),
                          static_cast<int>(ret_num),
                          errfunc);
        if (ret != 0)
        {
            ZCE_LOG(RS_ERROR, "[LUATIE] lua_pcall ret = %d", ret);
        }

        //�ڶ�ջɾ����������ĺ���
        lua_remove(lua_state_, errfunc);

        //ע���������û��ȡ������ֵ���������������û�з���ֵ

        return 0;
    }

    /*!
    * @brief      ��Lua�п������ݵ�C++�������У��������飬vector��vector��Ҫ��resize
    * @tparam     container_type  �������ͣ�
    * @return     int             �ɹ�����0
    * @param      table_name      �������
    * @param      container_dat   ��������
    * @param      nouse           û��ʹ�õĲ���������������������ʶ��
    */
    template<class container_type>
    int from_luatable(const char *table_name,
                      container_type &container_dat,
                      std::random_access_iterator_tag /*nouse*/)
    {
        //����������ƣ�ȡ�����metatable�ı�����˵ԭ�͡�
        ::lua_pushstring(lua_state_, table_name);
        ::lua_gettable(lua_state_, LUA_GLOBALSINDEX);

        if (!lua_istable(lua_state_, -1))
        {
            ZCE_LOG(RS_ERROR, "[LUATIE] table name[%s] is not tie to lua.",
                    table_name);
            lua_remove(lua_state_, -1);
            return -1;
        }

        //first key ,ferg���Ұ��ⶫ�������ɵ�����
        ::lua_pushnil(lua_state_);
        while (::lua_next(lua_state_, -2) != 0)
        {
            // uses 'key' (at index -2) and 'value' (at index -1)
            int index = zce::luatie::read_stack<int>(lua_state_, -2) - 1;
            container_dat[index] =
                zce::luatie::read_stack
                <container_type::value_type>
                (lua_state_, -1);
            // removes 'value'; keeps 'key' for next iteration
            ::lua_remove(lua_state_, -1);
        }
        return 0;
    }

    //��Lua�п������ݵ�C++�������У��������飬vector��vector��Ҫ��resize
    template<class container_type>
    int from_luatable(const char *table_name,
                      container_type &container_dat,
                      std::bidirectional_iterator_tag /*nouse*/)
    {
        //����������ƣ�ȡ�����metatable�ı�����˵ԭ�͡�
        ::lua_pushstring(lua_state_, table_name);
        ::lua_gettable(lua_state_, LUA_GLOBALSINDEX);

        if (!lua_istable(lua_state_, -1))
        {
            ZCE_LOG(RS_ERROR, "[LUATIE] table name[%s] is not tie to lua.",
                    table_name);
            ZCE_ASSERT(false);
            ::lua_remove(lua_state_, -1);
            return -1;
        }

        //first key ,ferg���Ұ��ⶫ�������ɵ�����
        ::lua_pushnil(lua_state_);
        while (lua_next(lua_state_, -2) != 0)
        {
            // uses 'key' (at index -2) and 'value' (at index -1)
            container_dat[zce::luatie::read_stack<typename container_type::value_type::first_type>(lua_state_, -2)] =
                zce::luatie::read_stack<typename container_type::value_type::second_type>(lua_state_, -1);
            // removes 'value'; keeps 'key' for next iteration
            ::lua_remove(lua_state_, -1);
        }
        return 0;
    }

    /*!
    * @brief      ʹ�õ������������飬vector������,��LUA�е�һ��table�У�
    * @tparam     raiter_type ����ĵ�������random_access_iterator_tag ���͵ĵ�����
    * @param      table_name table������
    * @param      first      ��һ��first������
    * @param      last       ���һ��last�ĵ�����
    * @param      nouse      û��ʹ�õĲ���������������������ʶ��
    * @note
    */
    template<class raiter_type >
    void to_luatable(const char *table_name,
                     const raiter_type first,
                     const raiter_type last,
                     std::random_access_iterator_tag /*nouse*/)
    {
        ::lua_pushstring(lua_state_, table_name);
        ::lua_createtable(lua_state_,
                          static_cast<int>(std::distance(first, last)), 0);
        raiter_type iter_temp = first;
        for (int i = 0; iter_temp != last; iter_temp++, i++)
        {
            //Lua��ʹ��ϰ�������Ǵ�1��ʼ
            ::lua_pushnumber(lua_state_, i + 1);
            //ͨ����������ȡ�õ����ͣ�
            zce::luatie::push_stack<typename std::iterator_traits<raiter_type>::value_type >(
                lua_state_,
                *iter_temp);
            ::lua_settable(lua_state_, -3);
        }
        ::lua_settable(lua_state_, LUA_GLOBALSINDEX);
    }

    /*!
    * @brief      ͨ����������һ��map,����unorder_map����lua table��
    * @tparam     biiter_type ����������,bidirectional_iterator_tag,˫�������
    * @param      table_name  �������
    * @param      first       ��һ����������
    * @param      last        ���һ��������
    * @param      nouse       û��ʹ�õĲ���������������������ʶ��
    * @note       �����ʵҲ֧�ֺ�multi�ļ���map������������������listҲ��bidirectional_iterator_tag
    */
    template<class biiter_type >
    void to_luatable(const char *table_name,
                     const biiter_type  first,
                     const biiter_type last,
                     std::bidirectional_iterator_tag /*nouse*/)
    {
        ::lua_pushstring(lua_state_, table_name);
        ::lua_createtable(lua_state_,
                          0,
                          static_cast<int>(std::distance(first, last)));

        biiter_type iter_temp = first;
        for (; iter_temp != last; iter_temp++)
        {
            //��map��key��Ϊtable��key
            zce::luatie::push_stack <typename std::remove_cv < typename
            std::iterator_traits<biiter_type>::value_type::first_type
            >::type > (lua_state_, iter_temp->first);

            zce::luatie::push_stack <typename std::remove_cv < typename
            std::iterator_traits<biiter_type>::value_type::second_type
            >::type > (lua_state_, iter_temp->second);
            lua_settable(lua_state_, -3);
        }

        lua_settable(lua_state_, LUA_GLOBALSINDEX);
        return;
    }

    /*!
    * @brief      ��LUAע��һ��ȫ�ֺ�����������ľ�̬������lua����
    *             ����last_yieldȷ�����صķ�ʽ���Ƿ����lua_yield
    * @tparam     last_yield ����������Ƿ�ʹ��yield���أ�
    * @tparam     ret_type   ���ز�������
    * @tparam     args_type  �����Ĳ������ͣ����
    * @param      name       ��LUAע��ĺ�������
    * @param      func       ע���C����
    */
    template<bool last_yield, typename ret_type, typename... args_type>
    void reg_g_func(const char *name, ret_type(*func)(args_type...))
    {
        //��������
        ::lua_pushstring(lua_state_, name);
        //������ָ��ת��Ϊvoid * ����Ϊlightuserdata �����ջ����Ϊclosure��upvalue����
        ::lua_pushlightuserdata(lua_state_, (void *)func);
        //functorģ�庯��������closure,
        ::lua_pushcclosure(lua_state_,
                           zce::if_ < std::is_void<ret_type>::value,
                           zce::luatie::g_func_void<last_yield, args_type...>,
                           zce::luatie::g_func_ret<last_yield, ret_type, args_type...>
                           >::type::invoke,
                           1);

        //�������ȫ�ֻ�������
        ::lua_settable(lua_state_, LUA_GLOBALSINDEX);
    }

    /*!
    * @brief      ע����ĳ�Ա����
    * @tparam     last_yield ����Ƿ�ʹ��lua_yield�������أ���Ҫ����Э����
    * @tparam     class_type ��Ա������������
    * @tparam     ret_type   ����ֵ
    * @tparam     args_type  �����б��Ƕ����������
    * @return     int   == 0 ��ʾע��ɹ�
    * @param      name  ����������
    * @param      func  ��Ա����ָ��
    */
    template<bool last_yield, typename class_type, typename ret_type, typename... args_type>
    int class_mem_func(const char *name, ret_type(class_type::*func)(args_type...))
    {
        //����������ƣ�ȡ�����metatable�ı�����˵ԭ�͡�
        ::lua_pushstring(lua_state_, zce::luatie::class_name<class_type>::name());
        ::lua_gettable(lua_state_, LUA_GLOBALSINDEX);

        //
        if (!lua_istable(lua_state_, -1))
        {
            ZCE_LOG(RS_ERROR, "[LUATIE] class name[%s] is not tie to lua.",
                    zce::luatie::class_name<class_type>::name());
            ZCE_ASSERT(false);
            lua_pop(lua_state_, 1);
            return -1;
        }

        ::lua_pushstring(lua_state_, name);
        //�����ĺ���ָ����Ϊupvalue_�ġ�
        //ע���������ĳ�Աָ�루���ӽӽ�size_t����������ʵ�ʵ�ָ�룬�������������light userdata
        //�������д������Ҫ�����񣬷�Ҫ��typedef��תһ��
        typedef ret_type(class_type:: *mem_fun)(args_type...);
        new (lua_newuserdata(lua_state_, sizeof(mem_fun))) mem_fun(func);
        //
        ::lua_pushcclosure(lua_state_,
                           zce::if_ < std::is_void<ret_type>::value,
                           zce::luatie::member_func_void<last_yield, class_type, args_type...>,
                           zce::luatie::member_func_ret<last_yield, class_type, ret_type, args_type...>
                           >::type::invoke,
                           1);
        ::lua_rawset(lua_state_, -3);
        ::lua_remove(lua_state_, -1);
        return 0;
    }

protected:

    //Lua�Ľ�������״̬
    lua_State   *lua_state_;
};



//=======================================================================================================

class ZCE_Lua_Base;

/*!
* @brief      ��lua������﷨�ǣ�ÿ�������᷵��*this�����ã���Ҫ��Ϊ��ʵ������.�����﷨
*             �������﷨���ô�����д���Ӽ�һ�㡣
*             tie.reg_class<TA>("TA",false).mem_var(...).mem_var(...)
*             ��Ȼȱ��Ҳ���У���Ϊ�����Ĳ���û�з���ֵ�����Լ�ʹ�д���Ҳ�޷�����
* @tparam     class_type �󶨵��������
* @note       ���庯���Ľ�����ο�ZCE_Lua_Tie
*/
template <typename class_type>
class Candy_Tie_Class
{
public:
    ///���캯��
    Candy_Tie_Class(ZCE_Lua_Base *lua_tie,
                    bool read_only) :
        lua_tie_(lua_tie),
        read_only_(read_only)
    {
    }
    Candy_Tie_Class(const Candy_Tie_Class &others):
        lua_tie_(others.lua_tie_),
        read_only_(others.read_only_)
    {
    }
    Candy_Tie_Class(Candy_Tie_Class&& others):
        lua_tie_(others.lua_tie_),
        read_only_(others.read_only_)
    {
        others.lua_tie_=nullptr;
    }

    ////�����meta tableע�ṹ�캯��
    template <typename construct_fun >
    Candy_Tie_Class &construct(construct_fun func) 
    {
        lua_tie_->class_constructor<class_type, construct_fun >(func);
        return *this;
    }

    ///�����meta tableע���Ա����
    template <typename var_type >
    Candy_Tie_Class &mem_var(const char *name, var_type class_type::*val)
    {
        lua_tie_->class_mem_var<class_type, var_type >(name, val);
        return *this;
    }

    ///�����meta tableע���Ա����
    template <typename array_type, size_t array_size>
    Candy_Tie_Class &mem_ary(const char *name, array_type(class_type::*ary)[array_size])
    {
        lua_tie_->class_mem_ary<class_type, array_type, array_size >(name, ary, read_only_);
        return *this;
    }

    ///�����meta table�󶨺���
    template<typename ret_type, typename... args_type>
    Candy_Tie_Class &mem_fun(const char *name, ret_type(class_type::*func)(args_type...))
    {
        lua_tie_->class_memfunc<class_type, ret_type, args_type...>(name, func);
        return *this;
    }

    template<typename ret_type, typename... args_type>
    Candy_Tie_Class &mem_yield_fun(const char *name, ret_type(class_type::*func)(args_type...))
    {
        lua_tie_->class_mem_yield_func<class_type, ret_type, args_type...>(name, func);
        return *this;
    }

    //��ĳ����̳�
    template<typename parent_type>
    Candy_Tie_Class &inherit()
    {
        lua_tie_->class_inherit<class_type, parent_type>();
        return *this;
    }

protected:
    ///Lua�Ľ�������״̬
    ZCE_Lua_Base   *lua_tie_ = nullptr;

    ///������Ƿ���ֻ���ķ�ʽ
    bool           read_only_ = false;
};

//=======================================================================================================

/*!
* @brief      Lua��Э�̵ķ�װ������Э�̵�state�Լ����ڶ�ջ��id
*             ��Lua�Ĺ��ܶ���ZCE_Lua_Base �õ�
* @note       Lua�Ĵ��������Գ���Thread������ʵ��Э�̣���Ϊ��ͳһ��Ҳû��ʹ��coroutine������
*             �ҵĴ�������Ҳ��������thread��صĶ�����������⣬ͳһʹ��Lua Thread����������
*/
class ZCE_Lua_Thread : public ZCE_Lua_Base
{
public:

    ZCE_Lua_Thread();

    ///�������룬Lua Thread�Ĵ��벻���Լ��ͷ��Լ���Lua Thread��
    ///��ջ����յ�ʱ�򣬻ᱻGC���յ������ԣ���������ʲôҲ������
    ~ZCE_Lua_Thread();

    ///�����߳���ص�����
    void set_thread(lua_State *lua_thread, int thread_stackidx);

    ///ȡ���߳��ڴ����߶�ջ��λ������
    int get_thread_stackidx();


    ///�ָ��߳�����
    int resume(int narg);

    ///�����߳�����
    int yield(int nresults);

protected:

    ///�߳��ڴ����߶�ջ��λ������
    int luathread_stackidx_ = 0;
};

//=======================================================================================================


/*!
* @brief      LUA Ь�������ڰ����󶨺���C������C++�Ĵ��뵽Lua������ʹ��Lua�ĸ��ֹ��ܡ�
*             ͬʱ������ʹ���̵߳ȹ���
* @note
*/
class ZCE_Lua_Tie : public ZCE_Lua_Base
{
public:

    ZCE_Lua_Tie();
    ~ZCE_Lua_Tie();

    /*!
    * @brief      ��lua state
    * @return     int
    * @param      open_libs  �Ƿ�򿪳��õ�һЩLUA��
    * @param      reg_common �Ƿ�ע��һЩ����
    */
    int open(bool open_libs,
             bool reg_common);
    ///�ر�lua state
    void close();

    ///����һ���µ�lua thread
    int new_thread(ZCE_Lua_Thread *lua_thread);

    ///�رգ�����һ��lua thread
    void del_thread(ZCE_Lua_Thread *lua_thread);

    ///�ָ�һ���̵߳�����
    int resume_thread(ZCE_Lua_Thread *lua_thread, int narg);
};


//=======================================================================================================

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( pop )
#elif defined (ZCE_OS_LINUX)
#pragma GCC diagnostic pop
#endif

#endif //#if ZCE_USE_LUA == 1 && ZCE_SUPPORT_CPP11 ==1

#endif // ZCE_LIB_SCRIPT_LUA_H_

