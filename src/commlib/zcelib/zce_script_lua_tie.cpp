
#include "zce_predefine.h"
#include "zce_trace_log_debug.h"
#include "zce_script_lua_tie.h"

#if defined  lua_h

ZCE_Lua_Tie::ZCE_Lua_Tie() :
    lua_state_(nullptr)
{
}

ZCE_Lua_Tie::~ZCE_Lua_Tie()
{
    close();
}

//打开lua state
int ZCE_Lua_Tie::open()
{
    //如果错误
    if (lua_state_)
    {
        ZCE_LOGMSG(RS_ERROR, "lua_state_ != NULL ,reopen?");
        close();
    }

    lua_state_ = lua_open();
    if (nullptr == lua_state_)
    {
        return -1;
    }
    return 0;
}

//关闭lua state
void ZCE_Lua_Tie::close()
{
    if (lua_state_)
    {
        lua_close(lua_state_);
        lua_state_ = nullptr;
    }
}

// 执行一个LUA的buffer
void ZCE_Lua_Tie::do_buffer(const char *buff, size_t len)
{
    lua_pushcclosure(lua_state_, on_error, 0);
    int errfunc = lua_gettop(lua_state_);

    int ret = luaL_loadbuffer(lua_state_, buff, len, __ZCE_FUNCTION__);
    if (ret == 0)
    {
        lua_pcall(lua_state_, 0, 1, errfunc);
    }
    else
    {
        print_error(lua_state_, "%s", lua_tostring(L, -1));
    }

    lua_remove(lua_state_, errfunc);
    lua_pop(lua_state_, 1);
}

//=======================================================================================================
//read_stack从堆栈中读取一个数据

template<> char *ZCE_Lua_Tie::read_stack(lua_State *state, int index)
{
    return (char *)lua_tostring(state, index);
}

template<> const char *ZCE_Lua_Tie::read_stack(lua_State *state, int index)
{
    return (const char *)lua_tostring(state, index);
}

template<> char ZCE_Lua_Tie::read_stack(lua_State *state, int index)
{
    return (char)lua_tonumber(state, index);
}

template<> unsigned char ZCE_Lua_Tie::read_stack(lua_State *state, int index)
{
    return (unsigned char)lua_tonumber(state, index);
}

template<> short ZCE_Lua_Tie::read_stack(lua_State *state, int index)
{
    return (short)lua_tonumber(state, index);
}

template<> unsigned short ZCE_Lua_Tie::read_stack(lua_State *state, int index)
{
    return (unsigned short)lua_tonumber(state, index);
}

template<> int ZCE_Lua_Tie::read_stack(lua_State *state, int index)
{
    return (int)lua_tonumber(state, index);
}

template<> unsigned int ZCE_Lua_Tie::read_stack(lua_State *state, int index)
{
    return (unsigned int)lua_tonumber(state, index);
}

template<> float ZCE_Lua_Tie::read_stack(lua_State *state, int index)
{
    return (float)lua_tonumber(state, index);
}

template<> double ZCE_Lua_Tie::read_stack(lua_State *state, int index)
{
    return (double)lua_tonumber(state, index);
}

template<> bool ZCE_Lua_Tie::read_stack(lua_State *state, int index)
{
    if (lua_isboolean(state, index))
    {
        return lua_toboolean(state, index) != 0;
    }
    //避免某些程度的转换不成功？
    else
    {
        return lua_tonumber(state, index) != 0;
    }
}

template<> int64_t ZCE_Lua_Tie::read_stack(lua_State *state, int index)
{
    if (lua_isnumber(state, index))
    {
        return (int64_t)lua_tonumber(state, index);
    }
    else
    {
        return *(int64_t *)lua_touserdata(state, index);
    }
}

template<> uint64_t ZCE_Lua_Tie::read_stack(lua_State *state, int index)
{
    if (lua_isnumber(state, index))
    {
        return (uint64_t)lua_tonumber(state, index);
    }
    else
    {
        return *(uint64_t *)lua_touserdata(state, index);
    }
}

template<> std::string ZCE_Lua_Tie::read_stack(lua_State *state, int index)
{
    if (lua_isstring(state, index))
    {
        return std::string (lua_tostring(state, index));
    }
    else
    {
        return *(std::string *) (state, index);
    }
}
//=======================================================================================================
//read_stack从堆栈中读取一个数据
template<> void ZCE_Lua_Tie::push_stack(lua_State *state, char val)
{
    lua_pushnumber(state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *state, unsigned char val)
{
    lua_pushnumber(state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *state, short val)
{
    lua_pushnumber(state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *state, unsigned short val)
{
    lua_pushnumber(state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *state, long val)
{
    lua_pushnumber(state, val);
}

template<>
void ZCE_Lua_Tie::push_stack(lua_State *state, unsigned long val)
{
    lua_pushnumber(state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *state, int val)
{
    lua_pushnumber(state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *state, unsigned int val)
{
    lua_pushnumber(state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *state, float val)
{
    lua_pushnumber(state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *state, double val)
{
    lua_pushnumber(state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *state, char *val)
{
    lua_pushstring(state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *state, const char *val)
{
    lua_pushstring(state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *state, bool val)
{
    lua_pushboolean(state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *state, int64_t val)
{
    *(int64_t *)lua_newuserdata(state, sizeof(int64_t)) = val;
    lua_pushstring(state, "int64_t");
    lua_gettable(state, LUA_GLOBALSINDEX);
    lua_setmetatable(state, -2);
}
template<> void ZCE_Lua_Tie::push_stack(lua_State *state, uint64_t val)
{
    *(uint64_t *)lua_newuserdata(state, sizeof(uint64_t)) = val;
    lua_pushstring(state, "uint64_t");
    lua_gettable(state, LUA_GLOBALSINDEX);
    lua_setmetatable(state, -2);
}

//=======================================================================================================
//一些公用的特性的LUA注册函数，比如只读的table等
static int newindex_onlyread(lua_State *state)
{
    luaL_error(state, "Table is read only ,can't modify,please check your code.");
    return 1;
}

//=======================================================================================================
//为int64_t 准备的metatable
static int tostring_int64(lua_State *state)
{
    char temp[64];
    snprintf(temp, 63, "%lld", *(int64_t *)lua_touserdata(state, 1));
    lua_pushstring(state, temp);
    return 1;
}

static int eq_int64(lua_State *state)
{
    int64_t a = *(int64_t *)lua_touserdata(state, 1);
    int64_t b = *(int64_t *)lua_touserdata(state, 2);
    lua_pushboolean(state, (a==b));
    return 1;
}

static int lt_int64(lua_State *state)
{
    int64_t a = *(int64_t *)lua_touserdata(state, 1);
    int64_t b = *(int64_t *)lua_touserdata(state, 2);
    lua_pushboolean(state, (a < b));
    return 1;
}

static int le_int64(lua_State *state)
{
    int64_t a = *(int64_t *)lua_touserdata(state, 1);
    int64_t b = *(int64_t *)lua_touserdata(state, 2);
    lua_pushboolean(state, (a <= b));
    return 1;
}

static int add_int64(lua_State *state)
{
    int64_t a = *(int64_t *)lua_touserdata(state, 1);
    int64_t b = *(int64_t *)lua_touserdata(state, 2);
    int64_t c = a + b;
    ZCE_Lua_Tie::push_stack(state,c);
    return 1;
}

static int sub_int64(lua_State *state)
{
    int64_t a = *(int64_t *)lua_touserdata(state, 1);
    int64_t b = *(int64_t *)lua_touserdata(state, 2);
    int64_t c = a - b;
    ZCE_Lua_Tie::push_stack(state, c);
    return 1;
}

static int constructor_int64(lua_State *state)
{
    int64_t data=0;
    sscanf(lua_tostring(state, 1), "%lld", &data);
    ZCE_Lua_Tie::push_stack(state, data);
    return 1;
}

static int selfadd_int64(lua_State *state)
{
    int64_t *data = (int64_t *)lua_touserdata(state, 1);
    ++(*data);
    return 0;
}

static int selfsub_int64(lua_State *state)
{
    int64_t *data = (int64_t *)lua_touserdata(state, 1);
    --(*data);
    return 0;
}

void ZCE_Lua_Tie::reg_int64()
{
    const char *name = "int64_t";
    lua_pushstring(lua_state_, name);
    lua_newtable(lua_state_);
  

    lua_pushstring(lua_state_, "__name");
    lua_pushstring(lua_state_, name);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "__tostring");
    lua_pushcclosure(lua_state_, tostring_int64, 0);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "__eq");
    lua_pushcclosure(lua_state_, eq_int64, 0);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "__lt");
    lua_pushcclosure(lua_state_, lt_int64, 0);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "__le");
    lua_pushcclosure(lua_state_, le_int64, 0);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "__add");
    lua_pushcclosure(lua_state_, add_int64, 0);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "__sub");
    lua_pushcclosure(lua_state_, sub_int64, 0);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "selfadd");
    lua_pushcclosure(lua_state_, selfadd_int64, 0);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "selfsub");
    lua_pushcclosure(lua_state_, selfsub_int64, 0);
    lua_rawset(lua_state_, -3);

    //这样的目的是这样的，__call是对应一个()调用，但实体不是函数式，的调用函数
    //LUA中出现这样的调用，i1 =int64_t("123")
    lua_newtable(lua_state_);

    lua_pushstring(lua_state_, "__call");
    lua_pushcclosure(lua_state_, constructor_int64, 0);
    lua_rawset(lua_state_, -3);

    //设置这个table作为int64_t 原型的metatable.
    lua_setmetatable(lua_state_, -2);

    lua_settable(lua_state_, LUA_GLOBALSINDEX);
}

//=======================================================================================================
//为uint64_t 准备的metatable
static int tostring_uint64(lua_State *state)
{
    char temp[64];
    snprintf(temp, 63, "%llu", *(uint64_t *)lua_touserdata(state, 1));
    lua_pushstring(state, temp);
    return 1;
}

static int eq_uint64(lua_State *state)
{
    uint64_t a = *(uint64_t *)lua_touserdata(state, 1);
    uint64_t b = *(uint64_t *)lua_touserdata(state, 2);
    lua_pushboolean(state, (a == b));
    return 1;
}

static int lt_uint64(lua_State *state)
{
    uint64_t a = *(uint64_t *)lua_touserdata(state, 1);
    uint64_t b = *(uint64_t *)lua_touserdata(state, 2);
    lua_pushboolean(state, (a < b));
    return 1;
}

static int le_uint64(lua_State *state)
{
    uint64_t a = *(uint64_t *)lua_touserdata(state, 1);
    uint64_t b = *(uint64_t *)lua_touserdata(state, 2);
    lua_pushboolean(state, (a <= b));
    return 1;
}

static int add_uint64(lua_State *state)
{
    uint64_t a = *(uint64_t *)lua_touserdata(state, 1);
    uint64_t b = *(uint64_t *)lua_touserdata(state, 2);
    uint64_t c = a + b;
    ZCE_Lua_Tie::push_stack(state, c);
    return 1;
}

static int sub_uint64(lua_State *state)
{
    uint64_t a = *(uint64_t *)lua_touserdata(state, 1);
    uint64_t b = *(uint64_t *)lua_touserdata(state, 2);
    uint64_t c = a - b;
    ZCE_Lua_Tie::push_stack(state, c);
    return 1;
}
//构造函数，让你支持int64_t("123")
static int constructor_uint64(lua_State *state)
{
    uint64_t data = 0;
    sscanf(lua_tostring(state, 1), "%llu", &data);
    ZCE_Lua_Tie::push_stack(state, data);
    return 1;
}

static int selfadd_uint64(lua_State *state)
{
    uint64_t *data = (uint64_t *)lua_touserdata(state, 1);
    ++(*data);
    return 0;
}

static int selfsub_uint64(lua_State *state)
{
    uint64_t *data = (uint64_t *)lua_touserdata(state, 1);
    --(*data);
    return 0;
}

void ZCE_Lua_Tie::reg_uint64()
{
    const char *name = "uint64_t";
    lua_pushstring(lua_state_, name);
    lua_newtable(lua_state_);


    lua_pushstring(lua_state_, "__name");
    lua_pushstring(lua_state_, name);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "__tostring");
    lua_pushcclosure(lua_state_, tostring_uint64, 0);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "__eq");
    lua_pushcclosure(lua_state_, eq_uint64, 0);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "__lt");
    lua_pushcclosure(lua_state_, lt_uint64, 0);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "__le");
    lua_pushcclosure(lua_state_, le_uint64, 0);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "__add");
    lua_pushcclosure(lua_state_, add_uint64, 0);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "__sub");
    lua_pushcclosure(lua_state_, sub_uint64, 0);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "selfadd");
    lua_pushcclosure(lua_state_, selfadd_uint64, 0);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "selfsub");
    lua_pushcclosure(lua_state_, selfsub_uint64, 0);
    lua_rawset(lua_state_, -3);

    //这样的目的是这样的，__call是对应一个()调用，但实体不是函数式，的调用函数
    //LUA中出现这样的调用，i1 =int64_t("123")
    lua_newtable(lua_state_);

    lua_pushstring(lua_state_, "__call");
    lua_pushcclosure(lua_state_, constructor_uint64, 0);
    lua_rawset(lua_state_, -3);

    //设置这个table作为int64_t 原型的metatable.
    lua_setmetatable(lua_state_, -2);

    lua_settable(lua_state_, LUA_GLOBALSINDEX);
}

//=======================================================================================================
//为std::string 准备的metatable

static int tostring_stdstring(lua_State *state)
{
    lua_pushstring(state, ((std::string *)lua_touserdata(state, 1))->c_str());
    return 1;
}

static int eq_stdstring(lua_State *state)
{
    std::string *a = (std::string *)lua_touserdata(state, 1);
    std::string *b = (std::string *)lua_touserdata(state, 2);
    lua_pushboolean(state, (a->c_str() == b->c_str()));
    return 1;
}

static int lt_stdstring(lua_State *state)
{
    std::string *a = (std::string *)lua_touserdata(state, 1);
    std::string *b = (std::string *)lua_touserdata(state, 2);
    lua_pushboolean(state, (a->c_str() < b->c_str()));
    return 1;
}

static int le_stdstring(lua_State *state)
{
    std::string *a = (std::string *)lua_touserdata(state, 1);
    std::string *b = (std::string *)lua_touserdata(state, 2);
    lua_pushboolean(state, (a->c_str() <= b->c_str()));
    return 1;
}

static int add_stdstring(lua_State *state)
{
    std::string *a = (std::string *)lua_touserdata(state, 1);
    std::string *b = (std::string *)lua_touserdata(state, 2);
    std::string c = *a + *b;
    ZCE_Lua_Tie::push_stack(state, c);
    return 1;
}

static int concat_stdstring(lua_State *state)
{
    std::string *a = (std::string *)lua_touserdata(state, 1);
    std::string *b = (std::string *)lua_touserdata(state, 2);
    std::string c = *a + *b;
    ZCE_Lua_Tie::push_stack(state, c);
    return 1;
}

static int constructor_stdstring(lua_State *state)
{
    std::string data(lua_tostring(state, 1));
    ZCE_Lua_Tie::push_stack(state, data);
    return 1;
}

//注册std::string
void ZCE_Lua_Tie::reg_stdstring()
{
    const char *name = "stdstring";
    lua_pushstring(lua_state_, name);
    lua_newtable(lua_state_);


    lua_pushstring(lua_state_, "__name");
    lua_pushstring(lua_state_, name);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "__tostring");
    lua_pushcclosure(lua_state_, tostring_stdstring, 0);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "__eq");
    lua_pushcclosure(lua_state_, eq_stdstring, 0);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "__lt");
    lua_pushcclosure(lua_state_, lt_stdstring, 0);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "__le");
    lua_pushcclosure(lua_state_, le_stdstring, 0);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "__add");
    lua_pushcclosure(lua_state_, add_stdstring, 0);
    lua_rawset(lua_state_, -3);

    lua_pushstring(lua_state_, "__concat");
    lua_pushcclosure(lua_state_, concat_stdstring, 0);
    lua_rawset(lua_state_, -3);
    
    //这样的目的是这样的，__call是对应一个()调用，但实体不是函数式，的调用函数
    //LUA中出现这样的调用，i1 =stdstring("123")
    lua_newtable(lua_state_);

    lua_pushstring(lua_state_, "__call");
    lua_pushcclosure(lua_state_, constructor_uint64, 0);
    lua_rawset(lua_state_, -3);

    //设置这个table作为int64_t 原型的metatable.
    lua_setmetatable(lua_state_, -2);

    lua_settable(lua_state_, LUA_GLOBALSINDEX);
}

//=======================================================================================================
//为std::string 准备的metatable

void ZCE_Lua_Tie::reg_enum(const char *name, size_t item_num, ...)
{
    lua_pushstring(lua_state_, name);
    lua_newtable(lua_state_);

    va_list argp;
    va_start(argp, item_num);
    //将枚举值和字符串一一做好绑定，
    for (size_t i = 0; i < item_num; ++i)
    {
        lua_pushstring(lua_state_, va_arg(argp, char *));
        lua_pushnumber(lua_state_, va_arg(argp, int));
        lua_rawset(lua_state_, -3);
    }
    va_end(argp);

    //让这个表格只读
    lua_newtable(lua_state_);

    lua_pushstring(lua_state_, "__newindex");
    lua_pushcclosure(lua_state_, newindex_onlyread, 0);
    lua_rawset(lua_state_, -3);
    
    lua_settable(lua_state_, LUA_GLOBALSINDEX);
}



#endif //
