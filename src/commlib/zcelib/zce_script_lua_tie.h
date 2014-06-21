//这个代码是参考Tinker实现的，仍然感谢原作者
//我等了四年，就是要等一个机会，我要争一口气，不是想证明我了不起，我是要告诉人家，我失去的东西一定要亲手拿回来！ -- 《英雄本色》 小马哥
//2014年6月13日早上，荷兰干净利落的爆了西班牙5：1，



#ifndef ZCE_LIB_SCRIPT_LUA_H_
#define ZCE_LIB_SCRIPT_LUA_H_




//LUA目前的包装代码使用C11的新特效，必须用新的编译器
#if defined  ZCE_USE_LUA && defined ZCE_SUPPORT_CPP11

//LUA 5.02版本目前而言，大部分组件都还不支持，所以……
#if LUA_VERSION_NUM != 501
#error "[Error] please check your lua libary version,only support 5.1,Lua 5.2 is not mature. LUA_VERSION_NUM != 501."
#endif

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( push )
#pragma warning ( disable : 4127)
#endif

//最后还是使用用了ZCE_LUA的名字空间，因为用ZCE_LIB感觉不能完全避免冲突性
namespace ZCE_LUA
{
///只读的table的newdindex
int newindex_onlyread(lua_State *state);

///dump lua运行的的堆栈，用于检查lua运行时的问题，错误处理等
int dump_luacall_stack(lua_State *state);

///dump C调用lua的堆栈，
int dump_clua_stack(lua_State *state);

///
int on_error(lua_State *state);


//用模板函数辅助帮忙实现一个方法，可以通过class 找到对应的类名称（注册到LUA的名称），
template<typename class_type>
struct class_name
{
    //对象记录一个全局的名称
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

//USER DATA的基类，保存一个void *的指针，用于记录数据的指针，通过
//继承实现对于val，ptr，ref的对象管理
struct lua_udat_base
{
    lua_udat_base(void *p)
        : obj_ptr_(p)
    {
    }
    //必须是virtual
    virtual ~lua_udat_base()
    {
    }
    void *obj_ptr_;
};

///把一个val放入LUA USER DATA的封装
template<typename val_type>
struct val_2_udat : lua_udat_base
{
    val_2_udat() 
        : lua_udat_base(new val_type) 
    {
    } 

    //args_type是构造函数的参数,args_type1 存在的目的是和默认构造函数分开，
    template<typename args_type1, typename ...args_tlist>
    val_2_udat(args_type1 arg1, args_tlist ...arg) : lua_udat_base(new val_type(arg1,args_tlist ...))
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

/// 指针到
template<typename val_type>
struct ptr_2_udat : lua_udat_base
{
    ptr_2_udat(val_type *t)
        : lua_udat_base((void *)t)
    {
    }
};

///
template<typename val_type>
struct ref_2_udat : lua_udat_base
{
    //注意第一个&t表示是引用参数，第二个是标示传递指针给lua_udat_base基类
    ref_2_udat(val_type &t) :
        lua_udat_base(&t)
    {
    }
};

/// 数组引用 到 userdata
template<typename ary_type>
struct arrayref_2_udat : lua_udat_base
{
    ///构造函数
    arrayref_2_udat(ary_type *ary_ptr, size_t ary_size,bool read_only) :
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



template<typename val_type  >
void push_stack(lua_State * /*state*/, typename val_type /*val*/)
{
    ZCE_LOGMSG(RS_ERROR, "[LUATIE]Type[%s] not support in this code?", typeid(val_type).name());
    return;
}


template<typename val_type  >
void push_stack(lua_State * state, typename arrayref_2_udat<val_type> & ary_dat )
{
    new (lua_newuserdata(state, sizeof(arrayref_2_udat<array_type>)))
        (ary_dat);
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
    return;
}




///PUSH一个数据到堆栈，
template<typename val_type  >
void push_stack(lua_State *state,
    typename std::enable_if<std::is_object<val_type>::value>::type val)
{
    new(lua_newuserdata(state,
        sizeof(val_2_udat<val_type>))) val_2_udat<val_type>(val);
    
    //根据类的名称，设置metatable
    lua_pushstring(state, class_name<val_type >::name());
    lua_gettable(state, LUA_GLOBALSINDEX);
    if (lua_istable(state, -1))
    {
        ZCE_LOGMSG(RS_ERROR, "[LUATIE][%s][%s] is not tie to lua,[%d][%s]? May be you don't register or name conflict? "
            typeid(val).name(),
            class_name<val_type >::name());
        lua_pop(state, 1);
        return;
    }
    lua_setmetatable(state, -2);
    return;

}

///为变参模板函数递归准备的函数
template<typename val_type, typename... val_tlist>
void push_stack(lua_State *state, val_type val, val_tlist ... val_s)
{
    push_stack(state, val, val_s...);
    return;
}

//放入一个引用
template<typename val_type  >
void push_stack(lua_State *state,
    typename std::enable_if<std::is_reference<val_type>::value>::type val)
{
    new(lua_newuserdata(state,
        sizeof(ref_2_udat<val_type>))) ref_2_udat<val_type>(val);

    //如果有可能为const多做一些检查
    //if (std::is_const<val_type>::value)

    //根据类的名称，设置metatable
    lua_pushstring(state, class_name<val_type >::name());
    lua_gettable(state, LUA_GLOBALSINDEX);
    if (lua_istable(state, -1))
    {
        ZCE_LOGMSG(RS_ERROR, "[LUATIE][%s][%s] is not tie to lua,[%d][%s]? May be you don't register or name conflict? "
            typeid(val).name(),
            class_name<val_type >::name());
        lua_pop(state, 1);
        return;
    }
    lua_setmetatable(state, -2);
    return;
}

//放入一个指针
template<typename val_type  >
void push_stack(lua_State *state,
    typename  std::enable_if<std::is_pointer<val_type>::value>::type val)
{
    if (val)
    {
        new(lua_newuserdata(state, sizeof(ptr_2_udat<val_type>))) ptr_2_udat<val_type>(val);

        lua_pushstring(state, class_name<val_type >::name());
        lua_gettable(state, LUA_GLOBALSINDEX);
        if (!lua_istable(state, -1))
        {
            ZCE_LOGMSG(RS_ERROR, "[LUATIE][%s][%s] is not tie to lua,[%d][%s]? May be you don't register or name conflict? "
                typeid(val).name(),
                class_name<val_type >::name());
            lua_pop(state, 1);
            return;
        }
        lua_setmetatable(state, -2);
    }
    else
    {
        lua_pushnil(state);
    }
    return;
}

//枚举
template<typename val_type  >
int push_stack(lua_State *state,
    typename  std::enable_if<std::is_enum<val_type>::value>::type val)
{
    lua_pushnumber(state, val);
    return 0;
}

template<> void push_stack(lua_State *state, char val);
template<> void push_stack(lua_State *state, unsigned char val);
template<> void push_stack(lua_State *state, short val);
template<> void push_stack(lua_State *state, unsigned short val);
template<> void push_stack(lua_State *state, int val);
template<> void push_stack(lua_State *state, unsigned int val);
template<> void push_stack(lua_State *state, float val);
template<> void push_stack(lua_State *state, double val);
template<> void push_stack(lua_State *state, char *val);
template<> void push_stack(lua_State *state, const char *val);
template<> void push_stack(lua_State *state, bool val);
template<> void push_stack(lua_State *state, int64_t val);
template<> void push_stack(lua_State *state, uint64_t val);
template<> void push_stack(lua_State *state, const std::string &val);

//不允许出现long的变量，因为long无法移植，只定义，所以不实现
template<> void push_stack(lua_State *state, long val);
template<> void push_stack(lua_State *state, unsigned long val);




//对于非object类型的数据，如果要放入引用和指针，必须先声明这些对象
//这个请注意！！！

template<typename ret_type>
typename ret_type read_stack(lua_State *state, int index)
{
    if (std::is_pointer<ret_type>::value)
    {
        return (std::remove_pointer<ret_type>::type *)(((lua_udat_base *)lua_touserdata(state, index))->obj_ptr_);
    }
    else if (std::is_reference<ret_type>::value)
    {
        return *( std::remove_reference<ret_type>::type *)(((lua_udat_base *)lua_touserdata(state, index))->obj_ptr_);
    }
    else
    {
        return *(ret_type *)(((lua_udat_base *)lua_touserdata(state, index))->obj_ptr_);
    }
}

///读取枚举值
template<typename val_type> 
static 
typename std::enable_if<std::is_enum<val_type>::value, val_type>::type 
read_stack(lua_State *state, int index)
{
    return (val_type)lua_tonumber(state, index);
}

///从堆栈中读取某个类型
template<> char               *read_stack(lua_State *state, int index);
template<> const char         *read_stack(lua_State *state, int index);
template<> char                read_stack(lua_State *state, int index);
template<> unsigned char       read_stack(lua_State *state, int index);
template<> short               read_stack(lua_State *state, int index);
template<> unsigned short      read_stack(lua_State *state, int index);
template<> int                 read_stack(lua_State *state, int index);
template<> unsigned int        read_stack(lua_State *state, int index);
template<> float               read_stack(lua_State *state, int index);
template<> double              read_stack(lua_State *state, int index);
template<> bool                read_stack(lua_State *state, int index);
template<> int64_t             read_stack(lua_State *state, int index);
template<> uint64_t            read_stack(lua_State *state, int index);
template<> std::string         read_stack(lua_State *state, int index);


///
template<typename val_type>
val_type pop_stack(lua_State *state)
{
    val_type t = read_stack<val_type>(state, -1);
    lua_pop(state, 1);
    return t;
}

///让LUA从一个绑定的数组（指针）那里得到数组的的值
template<typename array_type>
int array_meta_get(lua_State *state)
{
    //如果不是
    int index = static_cast<int>( lua_tonumber(state, -1));
    arrayref_2_udat<array_type> *ptr = ((arrayref_2_udat<array_type> *)lua_touserdata(state, -2));
    if (index < 1 && index > ptr->ary_size_)
    {
        ZCE_LOGMSG(RS_ERROR, "Lua script use error index [%d] to visit array %s[] size[%u].",
            index,
            typeid(ptr->obj_ptr_).name(),
            static_cast<uint32_t>(ptr->ary_size_));
        ZCE_ASSERT(false);
        lua_pushnil(state);

    }
    else
    {
        push_stack<array_type>(state, ptr->obj_ptr_[index]);
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
    if (index < 1 && index > ptr->ary_size_)
    {
        ZCE_LOGMSG(RS_ERROR, "Lua script use error index [%d] to visit array %s[] size[%u].",
            index, 
            typeid(ptr->obj_ptr_).name(),
            static_cast<uint32_t>(ptr->ary_size_));
        ZCE_ASSERT(false);
        
    }
    else
    {
        ptr->obj_ptr_[index] = read_stack<array_type>(state, -1);
    }

    return 0;
}

//拷贝数组，迭代器，的数据,到LUA中的一个table中，
int to_luatable(lua_State *state)
{
    return 0;
}

int from_luatable(lua_State *state)
{
    return 0;
}


///
int class_meta_get(lua_State *state);

///
int class_meta_set(lua_State *state);


///用C++11的新特效，变参实现的全局(包括static)函数桥接
template<typename ret_type, typename... args_type>
struct g_functor
{
    static int invoke(lua_State *state)
    {
        //push是将结果放入堆栈
        void *upvalue_1 = lua_touserdata(state, lua_upvalueindex(1));
        ret_type(*fun_ptr)(args_type...) = (ret_type( *)(args_type...)) (upvalue_1);

        if (std::is_void<ret_type>::value)
        {
            int para_idx = 0;
            fun_ptr(read_stack<args_type>(state, ++para_idx)...);
            return 0;
        }
        else
        {
            int para_idx = 0;
            push_stack<ret_type>(state, fun_ptr(read_stack<args_type>(state, para_idx++)...));
            return 1;
        }
    }
};


///用C++11的新特效，变参实现的类函数桥接
template<typename class_type,typename ret_type,typename... args_type>
struct member_functor
{
    static int invoke(lua_State *state)
    {
        //push是将结果放入堆栈
        void *upvalue_1 = lua_touserdata(state, lua_upvalueindex(1));
        ret_type(*fun_ptr)(args_type...) = (ret_type(class_type::*)(args_type...)) (upvalue_1);

        class_type *obj_ptr = read_stack<class_type *>(state, -1);

        if (std::is_void<ret_type>::value)
        {
            
            int para_idx = 1;
            obj_ptr->fun_ptr(read_stack<args_type>(state, ++para_idx)...);
            return 0;
        }
        else
        {
            int para_idx = 1;
            push_stack<ret_type>(state, 
                obj_ptr->fun_ptr(read_stack<args_type>(state, para_idx++)...));
            return 1;
        }
    }
};


///成员变量的基类，用于初始化
///memvar_base 注意其不是一个有模板的函数，
///这样就可以保证通过LUA user data里面的void * 转换为 memvar_base *
///而通过memvar_base *的调用 get, set 帮忙恰恰能直接调用到真正的
///member_var <class_type,var_type>
struct memvar_base
{
    virtual void get(lua_State *state) = 0;
    virtual void set(lua_State *state) = 0;
};


//class_type 为class 类型，
//var_type 为变量类型
template<typename class_type, typename var_type>
struct member_var : memvar_base
{
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

//
template<typename class_type, typename ary_type>
struct member_array : memvar_base
{
    ///对于成员数组，设置没有任何意义，
    void set(lua_State *state)
    {

    }

    //get是LUA读取的操作，也就是把C++的数据读取到LUA里面，所以是PUSH
    void get(lua_State *state)
    {
        //read_stack其实就是把类的对象的指针读取出来。
        //然后通过类成员指针，把成员获取出来
        ary_type *ary_ptr = read_stack<class_type *>(state, 1)->*(var_ptr_);
        push_stack<var_type>(state,
            );
    }

    ///
    var_type class_type::**ary_ptr_;
    ///
    size_t                 ary_size_;
    ///
    bool                   read_only_;
}

//封装类的构造函数给LUA使用
template<typename class_type, typename... args_type>
int constructor(lua_State *state)
{
    //new 一个user data，用<T>的大小,同时，同时用placement new 的方式，
    //（指针式lua_newuserdata分配的）完成构造函数
    int para_idx = 1;
    new(lua_newuserdata(state,
                        sizeof(val_2_udat<class_type> )))
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



};

//=======================================================================================================
//LUA 鞋带，用于帮助绑定C++和
class ZCE_Lua_Tie
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


    ///执行一个lua文件
    int do_file(const char *filename);
    ///执行一个LUA的buffer
    int do_buffer(const char *buff, size_t sz);

    ///dump C调用lua的堆栈，
    void dump_clua_stack();
    ///dump lua运行的的堆栈，用于检查lua运行时的问题，错误处理等
    void dump_luacall_stack();


    ///向LUA注册int64_t的类型，因为LUA内部的number默认是double，所以其实无法表示。所以要注册这个
    void reg_int64();
    ///向LUA注册uint64_t的类型
    void reg_uint64();
    ///向LUA注册std::string的类型
    void reg_stdstring();

    ///向LUA注册枚举值
    void reg_enum(const char *name, size_t item_num, ...);


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
        ZCE_LUA::push_stack(lua_state_, var);
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



    ///向LUA设置一个数组的引用,在LUA内部保存一个相关的userdata，
    template<typename array_type>
    void set_garray(const char *name,
                    typename val_type ary_data[],
                    size_t ary_size,
                    bool read_only = false)
    {
        arrayref_2_udat<array_type> aux_ary(ary_data, ary_size, read_only);
        //名称对象，
        lua_pushstring(lua_state_, name);
        push_stack(aux_ary);
        lua_settable(lua_state_, LUA_GLOBALSINDEX);
    }

    ///从LUA中获取一个全局的数组
    template<typename ary_type>
    int get_garray(const char *name,
                 size_t ary_num,
                 typename const ary_type ary_data[]  )
    {
        //名称对象，
        lua_pushstring(lua_state_, name);
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);

        //如果不是一个table，错误哦
        if (!lua_istable(lua_state_, -1))
        {
            lua_pop(lua_state_, 1);
            return -1;
        }

        return 0;
    }

    /*!
    * @brief      向LUA注册一个全局函数，或者类的静态函数给lua调用
    * @tparam     ret_type  返回参数类型
    * @tparam     args_type 函数的参数类型，变参
    * @param      name      向LUA注册的函数名称
    * @param      func      注册的C函数
    */
    template<typename ret_type, typename... args_type>
    void tie_gfun(const char *name, ret_type(*func)(args_type...))
    {
        //函数名称
        lua_pushstring(lua_state_, name);
        //将函数指针转换为void * ，作为lightuserdata 放入堆栈，作为closure的upvalue放入
        lua_pushlightuserdata(lua_state_, (void *)func);
        //functor模板函数，放入closure,
        lua_pushcclosure(lua_state_, ZCE_LUA::g_functor<ret_type, args_type...>::invoke, 1);
        //将其放入全局环境表中
        lua_settable(lua_state_, LUA_GLOBALSINDEX);
    }

    /*!
    * @brief      调用LUA的函数，没有返回值，（或者暂不取回一个返回值）
    * @tparam     args_type 参数类型列表
    * @return     int == 0 表示成功
    * @param      name 函数名称
    * @param      args 参数列表
    */
    template<typename... args_type>
    int call_luafun_0(const char *name, args_type... args)
    {
        int ret = 0;
        //放入错误处理的函数，并且记录堆栈的地址
        lua_pushcclosure(lua_state_, ZCE_LUA::on_error, 0);
        int errfunc = lua_gettop(lua_state_);

        lua_pushstring(lua_state_, name);
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);
        //检查其是否是函数
        if (!lua_isfunction(lua_state_, -1))
        {
            ZCE_LOGMSG(RS_ERROR, "[LUATIE] call_luafun() attempt to call global `%s' (not a function)", name);
            lua_pop(lua_state_, 1);
            return -1;
        }

        //放入堆栈参数，args
        ZCE_LUA::push_stack(lua_state_, args...);

        size_t arg_num = sizeof...(args);
        //调用lua的函数，默认只有一个返回值，
        ret = lua_pcall(lua_state_,
                        static_cast<int>(arg_num),
                        ,
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

    ///调用LUA的函数，只有一个返回值
    template<typename ret_type1, typename... args_type>
    int call_luafun_1(const char *name, ret_type1 ret_val1, args_type... args)
    {
        int ret = 0;
        ret = call_luafun_0(args...);
        if (ret != 0)
        {
            return ret;
        }
        //在堆栈弹出返回值
        ret_val1 = ZCE_LUA::pop_stack<ret_type1>(lua_state_);
        return 0;
    }

    ///调用LUA的函数，有两个返回值
    template<typename ret_type1, typename ret_type2, typename... args_type>
    int call_luafun_2(const char *name, ret_type1 ret_val1, ret_type2 ret_val2, args_type... args)
    {
        int ret = 0;
        ret = call_luafun_0(args...);
        if (ret != 0)
        {
            return ret;
        }
        //在堆栈弹出返回值
        ret_val1 = ZCE_LUA::pop_stack<ret_type1>(lua_state_);
        ret_val2 = ZCE_LUA::pop_stack<ret_type2>(lua_state_);
        return 0;
    }

    // class init
    //类的初始化，让class能在lua中使用
    //定义类的metatable的表，或者说原型的表。
    template<typename class_type>
    void tie_class(const char *name,
        bool read_only)
    {
        //绑定T和名称
        ZCE_LUA::class_name<class_type>::name(name);

        //类的名称
        lua_pushstring(lua_state_, name);
        //new 一个table，这个table是作为其他的类的metatable的（某种程度上也可以说是原型），
        lua_newtable(lua_state_);

        //__name不是标准的元方法，但在例子中有使用
        lua_pushstring(lua_state_, "__name");
        lua_pushstring(lua_state_, name);
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
    }



    // class_type 是类
    // constructor_fun 是构造函数的封装，ZCE_LUA::constructor
    template<typename class_type, typename constructor_fun>
    int tie_constructor(constructor_fun func)
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
    * @brief      
    * @tparam     class_type 绑定的类的类型
    * @tparam     base_type  成员所属的类，一般我认为T和BASE是一样的,但你也可以把一个基类的成员绑定在子类的Lua的meta table里面，
    * @tparam     var_type   成员类型
    * @return     int        是否绑定成功
    * @param      name       绑定的名称
    * @param      base_type::*val 绑定的成员变量的指针
    */
    template<typename class_type, typename base_type, typename var_type>
    int tie_member_var(const char *name, var_type base_type::*val)
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
        new(lua_newuserdata(lua_state_, sizeof(ZCE_LUA::member_var<base_type, var_type>)))  \
            ZCE_LUA::member_var<base_type, var_type>(val);
        lua_rawset(lua_state_, -3);

        lua_pop(lua_state_, 1);
        return 0;
    }



    template<typename class_type, typename ret_type, typename... args_type>
    int tie_member_fun(const char *name, typename ret_type( class_type::*func)(args_type...))
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
        new(lua_newuserdata(lua_state_, sizeof(ret_type(class_type::*)(args_type...)))) \
            ret_type(class_type::*func)(args_type...);
        //
        lua_pushcclosure(lua_state_, 
            ZCE_LUA::member_functor<class_type, ret_type, args_type...>::invoke, 1);
        lua_rawset(lua_state_, -3);

        lua_pop(lua_state_, 1);
        return 0;
    }


protected:

    //
    lua_State   *lua_state_;
};

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( pop )
#endif

#endif //#if defined  ZCE_USE_LUA && defined ZCE_SUPPORT_CPP11

#endif // ZCE_LIB_SCRIPT_LUA_H_

