//这个代码是参考Tinker实现的，仍然感谢原作者

#ifndef ZCE_LIB_SCRIPT_LUA_H_
#define ZCE_LIB_SCRIPT_LUA_H_

#if defined  lua_h



namespace ZCE_LIB
{ 




};


//LUA 鞋带，用于帮助绑定C++和
class ZCE_Lua_Tie
{
public:
    ZCE_Lua_Tie();
    ~ZCE_Lua_Tie();

    //
    int open();
    //
    void close();

    ///注册int64_t的类型，因为LUA内部的number默认是double，所以其实无法表示。所以要注册这个
    void reg_int64(lua_State *state);
    ///注册uint64_t的类型
    void reg_uint64(lua_State *state);
    ///注册std::string的类型
    void reg_stdstring(lua_State *state);

    //注册枚举值
    void reg_enum(const char *name,size_t item_num, ...);

public:

    //不允许出现long的变量，因为long无法移植，

    ///PUSH一个数据到堆栈，
    template<typename T> static void push_stack(lua_State * /*state*/, T /*val*/)
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

    template<typename T> static  T read_stack(lua_State *L, int index)
    {
        return *(T*)(L, index);
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

protected:

    //
    lua_State   *lua_state_;
};



#endif

#endif // ZCE_LIB_SCRIPT_LUA_H_

