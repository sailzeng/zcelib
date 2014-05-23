//这个代码是参考Tinker实现的，仍然感谢原作者

#ifndef ZCE_LIB_SCRIPT_LUA_H_
#define ZCE_LIB_SCRIPT_LUA_H_

#if defined  lua_h




//LUA 鞋带，用于帮助绑定C++和
class ZCE_Lua_Tie
{
    //很多基本的模版函数
public:

    ///用C11的新特效，变参实现
    template<typename ret_type, typename... args>
    struct functor
    {
        static int invoke(lua_State *state)
        {
            //push是将结果放入堆栈
            void *upvalue_1 = lua_touserdata(state,lua_upvalueindex(1));
            ret_type(*)((args)...) fun_ptr = (ret_type(*)(args)...) (upvalue_1);

            if (std::is_void<ret_type>::value)
            {
                int para_idx = 0;
                fun_ptr(read_stack(state, para_idx++)...);
                return 0;
            }
            else
            {
                int para_idx = 0;
                push_stack<ret_type>(fun_ptr(read_stack(state, para_idx++)...));
                return 1;
            }

        }
    };

    //不允许出现long的变量，因为long无法移植，

    ///PUSH一个数据到堆栈，
    template<typename val_type> static void push_stack(lua_State * /*state*/, val_type /*val*/)
    {
    }
    template<> static void push_stack(lua_State *state, char val);
    template<> static void push_stack(lua_State *state, unsigned char val);
    template<> static void push_stack(lua_State *state, short val);
    template<> static void push_stack(lua_State *state, unsigned short val);
    template<> static void push_stack(lua_State *state, int val);
    template<> static void push_stack(lua_State *state, unsigned int val);
    template<> static void push_stack(lua_State *state, float val);
    template<> static void push_stack(lua_State *state, double val);
    template<> static void push_stack(lua_State *state, char *val);
    template<> static void push_stack(lua_State *state, const char *val);
    template<> static void push_stack(lua_State *state, bool val);
    template<> static void push_stack(lua_State *state, int64_t val);
    template<> static void push_stack(lua_State *state, uint64_t val);
    template<> static void push_stack(lua_State *state, std::string val);

    ///
    template<typename val_type> static  val_type read_stack(lua_State *L, int index)
    {
        return *(val_type*)(L, index);
    }
    template<> static char               *read_stack(lua_State *state, int index);
    template<> static const char         *read_stack(lua_State *state, int index);
    template<> static char                read_stack(lua_State *state, int index);
    template<> static unsigned char       read_stack(lua_State *state, int index);
    template<> static short               read_stack(lua_State *state, int index);
    template<> static unsigned short      read_stack(lua_State *state, int index);
    template<> static int                 read_stack(lua_State *state, int index);
    template<> static unsigned int        read_stack(lua_State *state, int index);
    template<> static float               read_stack(lua_State *state, int index);
    template<> static double              read_stack(lua_State *state, int index);
    template<> static bool                read_stack(lua_State *state, int index);
    template<> static int64_t             read_stack(lua_State *state, int index);
    template<> static uint64_t            read_stack(lua_State *state, int index);
    template<> static std::string         read_stack(lua_State *state, int index);

    ///
    template<typename val_type>
    val_type pop_stack(lua_State *state)
    {
        val_type t = read_stack<val_type>(state, -1);
        lua_pop(L, 1);
        return t;
    }


public:
    ZCE_Lua_Tie();
    ~ZCE_Lua_Tie();

    ///打开lua state
    int open();
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

    ///向LUA设置一个全局变量（的名称对应值得拷贝）
    template<typename val_type>
    void set_tolua(const char *name, typename val_type val)
    {
        //名称对象，
        lua_pushstring(lua_state_, name);
        //模板函数，根据val_type绝对如何push
        push_stack(lua_state_, val);
        lua_settable(lua_state_, LUA_GLOBALSINDEX);
    }
    ///
    template<typename val_type>
    typename val_type get_fromlua(const char *name)
    {
        lua_pushstring(lua_state_, name);
        lua_gettable(lua_state_, LUA_GLOBALSINDEX);
        return pop_stack<val_type>(lua_state_);
    }

    ///注册一个全局函数，或者类的静态函数给lua调用
    template<typename ret_type, typename... args>
    void reg_fun(const char *name, ret_type(*func)(args...))
    {
        //函数名称
        lua_pushstring(lua_state_, name);
        //将函数指针转换为void * ，作为lightuserdata 放入堆栈，作为closure的upvalue放入
        lua_pushlightuserdata(lua_state_, (void *)func);
        //functor模板函数，放入closure,
        lua_pushcclosure(lua_state_, functor<ret_type, args...>::invoke, 1);
        //将其放入全局环境表中
        lua_settable(lua_state_, LUA_GLOBALSINDEX);
    }

protected:

    //
    lua_State   *lua_state_;
};



#endif

#endif // ZCE_LIB_SCRIPT_LUA_H_

