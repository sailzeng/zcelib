//这个代码是参考Tinker实现的，仍然感谢原作者

#ifndef ZCE_LIB_SCRIPT_LUA_H_
#define ZCE_LIB_SCRIPT_LUA_H_

//LUA目前的包装代码使用C11的新特效，必须用新的编译器
#if defined  ZCE_USE_LUA && defined ZCE_SUPPORT_CPP11

#if LUA_VERSION_NUM != 501
#error "[Error] please check your lua libary version,Only support 501. LUA_VERSION_NUM != 501."
#endif

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( push )
#pragma warning ( disable : 4127)
#endif

//最后还是同意用了ZCE_LIB的名字空间
namespace ZCE_LIB
{
    ///只读的table的newdindex
    int newindex_onlyread(lua_State *state);

    ///dump lua运行的的堆栈，用于检查lua运行时的问题，错误处理等
    int dump_luacall_stack(lua_State *state);

    ///dump C调用lua的堆栈，
    int dump_clua_stack(lua_State *state);
    
    ///
    int on_error(lua_State *state);


    ///为变参模板函数递归准备的函数
    template<typename val_type, typename... val_tlist>
    void push_stack(lua_State * state, val_type val, val_tlist ... val_s)
    {
        push_stack(state, val, val_s...);

    }

    
    ///PUSH一个数据到堆栈，
    //不允许出现long的变量，因为long无法移植，
    template<typename val_type> 
    void push_stack(lua_State * /*state*/, val_type /*val*/)
    {
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

    ///
    template<typename val_type> static  val_type read_stack(lua_State *L, int index)
    {
        return *(val_type*)(L, index);
    }
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
        lua_pop(L, 1);
        return t;
    }


    ///用C11的新特效，变参实现
    template<typename ret_type, typename... args_type>
    struct g_functor
    {
        static int invoke(lua_State *state)
        {
            //push是将结果放入堆栈
            void *upvalue_1 = lua_touserdata(state, lua_upvalueindex(1));
            ret_type(*fun_ptr)(args_type...) = (ret_type(*)(args_type...)) (upvalue_1);

            if (std::is_void<ret_type>::value)
            {
                int para_idx = 0;
                fun_ptr(read_stack<args_type>(state, para_idx++)...);
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
    * @tparam     val_type
    * @param      name
    * @param      val
    */
    template<typename val_type>
    void set_gval(const char *name, typename val_type val)
    {
        //名称对象，
        lua_pushstring(lua_state_, name);
        //模板函数，根据val_type绝对如何push
        push_stack(lua_state_, val);
        lua_settable(lua_state_, LUA_GLOBALSINDEX);
    }

    ///根据名称，从LUA读取一个变量
    template<typename val_type>
    typename val_type get_gval(const char *name)
    {
        lua_pushstring(lua_state_, name);
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);
        return pop_stack<val_type>(lua_state_);
    }

    ///向LUA设置一个全局的数组
    template<typename val_type>
    void set_gary(const char *name, 
        size_t ary_num,
        typename const val_type ary_data[],
        bool read_only = false)
    {
        //名称对象，
        lua_pushstring(lua_state_, name);
        lua_createtable(lua_state_, ary_num, 0);

        for (size_t i = 0; i<ary_num; ++i)
        {
            //相当于lua_rawseti.只是lua_rawseti的内部其实挑换了堆栈顺序，理解哟点怪，
            //算了,但的确不知道lua_rawseti是否有一些优化处理，因为感觉lua的hashtable是有一些特殊处理的，
            lua_pushnumber(lua_state_,static_cast<int>(i) );
            push_stack(lua_state_, ary_data[i]);
            lua_rawset(lua_state_, -3);
        }
        //如果希望其只读
        if (read_only)
        {
            //让这个表格只读
            lua_newtable(lua_state_);

            lua_pushstring(lua_state_, "__newindex");
            lua_pushcclosure(lua_state_, ZCE_LIB::newindex_onlyread, 0);
            lua_rawset(lua_state_, -3);

            lua_setmetatable(lua_state_, -2);
        }

        lua_settable(lua_state_, LUA_GLOBALSINDEX);
    }

    ///从LUA中获取一个全局的数组
    template<typename ary_type>
    int get_gary(const char *name,
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

        for (size_t i = 0; i < ary_num; ++i)
        {
            lua_pushnumber(lua_state_, static_cast<int>(i));
            lua_gettable(lua_state_,-2)
            if (lua_isnil(lua_state_,-1))
            {
                return -1;
            }
            ary_data[i] = pop_stack<ary_type>(lua_state_ );
        }
    }
    
    /*!
    * @brief      向LUA注册一个全局函数，或者类的静态函数给lua调用
    * @tparam     ret_type  返回参数类型
    * @tparam     args_type 函数的参数类型，变参
    * @param      name      向LUA注册的函数名称
    * @param      func      注册的C函数
    */
    template<typename ret_type, typename... args_type>
    void reg_gfun(const char *name, ret_type(*func)(args_type...))
    {
        //函数名称
        lua_pushstring(lua_state_, name);
        //将函数指针转换为void * ，作为lightuserdata 放入堆栈，作为closure的upvalue放入
        lua_pushlightuserdata(lua_state_, (void *)func);
        //functor模板函数，放入closure,
        lua_pushcclosure(lua_state_, ZCE_LIB::g_functor<ret_type, args_type...>::invoke, 1);
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
        lua_pushcclosure(lua_state_, ZCE_LIB::on_error, 0);
        int errfunc = lua_gettop(lua_state_);

        lua_pushstring(lua_state_, name);
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);
        //检查其是否是函数
        if (!lua_isfunction(lua_state_, -1))
        {
            ZCE_LOGMSG(RS_ERROR, "call_luafun() attempt to call global `%s' (not a function)", name);
            lua_pop(lua_state_,1);
            return -1;
        }

        //放入堆栈参数，args
        ZCE_LIB::push_stack(lua_state_, args...);

        size_t arg_num = sizeof...(args);
        //调用lua的函数，默认只有一个返回值，
        ret = lua_pcall(lua_state_,
            static_cast<int>(arg_num),
            ,
            errfunc);
        if (ret != 0)
        {
            ZCE_LOGMSG(RS_ERROR, "lua_pcall ret = %d", ret);
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
        ret_val1 = ZCE_LIB::pop_stack<ret_type1>(lua_state_);
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
        ret_val1 = ZCE_LIB::pop_stack<ret_type1>(lua_state_);
        ret_val2 = ZCE_LIB::pop_stack<ret_type2>(lua_state_);
        return 0;
    }

    //template<typename ret_type1, typename ret_type2, typename... args_type>
    

protected:

    //
    lua_State   *lua_state_;
};

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( pop )
#endif

#endif //#if defined  ZCE_USE_LUA && defined ZCE_SUPPORT_CPP11

#endif // ZCE_LIB_SCRIPT_LUA_H_

