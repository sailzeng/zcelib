
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

//
int ZCE_Lua_Tie::open()
{
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

//
void ZCE_Lua_Tie::close()
{
    if (lua_state_)
    {
        lua_close(lua_state_);
        lua_state_ = nullptr;
    }
}


//=======================================================================================================
//read_stack从堆栈中读取一个数据

template<> char *ZCE_Lua_Tie::read_stack(lua_State *lua_state, int index)
{
    return (char *)lua_tostring(lua_state, index);
}

template<> const char *ZCE_Lua_Tie::read_stack(lua_State *lua_state, int index)
{
    return (const char *)lua_tostring(lua_state, index);
}

template<> char ZCE_Lua_Tie::read_stack(lua_State *lua_state, int index)
{
    return (char)lua_tonumber(lua_state, index);
}

template<> unsigned char ZCE_Lua_Tie::read_stack(lua_State *lua_state, int index)
{
    return (unsigned char)lua_tonumber(lua_state, index);
}

template<> short ZCE_Lua_Tie::read_stack(lua_State *lua_state, int index)
{
    return (short)lua_tonumber(lua_state, index);
}

template<> unsigned short ZCE_Lua_Tie::read_stack(lua_State *lua_state, int index)
{
    return (unsigned short)lua_tonumber(lua_state, index);
}

template<> int ZCE_Lua_Tie::read_stack(lua_State *lua_state, int index)
{
    return (int)lua_tonumber(lua_state, index);
}

template<> unsigned int ZCE_Lua_Tie::read_stack(lua_State *lua_state, int index)
{
    return (unsigned int)lua_tonumber(lua_state, index);
}

template<> float ZCE_Lua_Tie::read_stack(lua_State *lua_state, int index)
{
    return (float)lua_tonumber(lua_state, index);
}

template<> double ZCE_Lua_Tie::read_stack(lua_State *lua_state, int index)
{
    return (double)lua_tonumber(lua_state, index);
}

template<> bool ZCE_Lua_Tie::read_stack(lua_State *lua_state, int index)
{
    if (lua_isboolean(lua_state, index))
    {
        return lua_toboolean(lua_state, index) != 0;
    }
    //避免某些程度的转换不成功？
    else
    {
        return lua_tonumber(lua_state, index) != 0;
    }
}

template<> int64_t ZCE_Lua_Tie::read_stack(lua_State *lua_state, int index)
{
    if (lua_isnumber(lua_state, index))
    {
        return (int64_t)lua_tonumber(lua_state, index);
    }
    else
    {
        return *(int64_t *)lua_touserdata(lua_state, index);
    }
}
template<> uint64_t ZCE_Lua_Tie::read_stack(lua_State *lua_state, int index)
{
    if (lua_isnumber(lua_state, index))
    {
        return (uint64_t)lua_tonumber(lua_state, index);
    }
    else
    {
        return *(uint64_t *)lua_touserdata(lua_state, index);
    }
}

//=======================================================================================================
//read_stack从堆栈中读取一个数据
template<> void ZCE_Lua_Tie::push_stack(lua_State *lua_state, char val)
{
    lua_pushnumber(lua_state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *lua_state, unsigned char val)
{
    lua_pushnumber(lua_state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *lua_state, short val)
{
    lua_pushnumber(lua_state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *lua_state, unsigned short val)
{
    lua_pushnumber(lua_state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *lua_state, long val)
{
    lua_pushnumber(lua_state, val);
}

template<>
void ZCE_Lua_Tie::push_stack(lua_State *lua_state, unsigned long val)
{
    lua_pushnumber(lua_state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *lua_state, int val)
{
    lua_pushnumber(lua_state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *lua_state, unsigned int val)
{
    lua_pushnumber(lua_state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *lua_state, float val)
{
    lua_pushnumber(lua_state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *lua_state, double val)
{
    lua_pushnumber(lua_state, val);
}

template<>
void ZCE_Lua_Tie::push_stack(lua_State *lua_state, char *val)
{
    lua_pushstring(lua_state, val);
}

template<>
void ZCE_Lua_Tie::push_stack(lua_State *lua_state, const char *val)
{
    lua_pushstring(lua_state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *lua_state, bool val)
{
    lua_pushboolean(lua_state, val);
}

template<> void ZCE_Lua_Tie::push_stack(lua_State *lua_state, int64_t val)
{
    *(int64_t *)lua_newuserdata(lua_state, sizeof(int64_t)) = val;
    lua_pushstring(lua_state, "int64_t");
    lua_gettable(lua_state, LUA_GLOBALSINDEX);
    lua_setmetatable(lua_state, -2);
}
template<> void ZCE_Lua_Tie::push_stack(lua_State *lua_state, int64_t val)
{
    *(uint64_t *)lua_newuserdata(lua_state, sizeof(uint64_t)) = val;
    lua_pushstring(lua_state, "uint64_t");
    lua_gettable(lua_state, LUA_GLOBALSINDEX);
    lua_setmetatable(lua_state, -2);
}


//=======================================================================================================
//为int64_t 准备的metatable
static int tostring_int64(lua_State *lua_state)
{
    char temp[64];
    snprintf(temp, 63, "%lld", *(int64_t *)lua_touserdata(lua_state, 1));
    lua_pushstring(lua_state, temp);
    return 1;
}

static int eq_int64(lua_State *lua_state)
{
    int64_t a = *(int64_t *)lua_touserdata(lua_state, 1);
    int64_t b = *(int64_t *)lua_touserdata(lua_state, 2);
    lua_pushboolean(lua_state, (a==b));
    return 1;
}

static int lt_int64(lua_State *lua_state)
{
    int64_t a = *(int64_t *)lua_touserdata(lua_state, 1);
    int64_t b = *(int64_t *)lua_touserdata(lua_state, 2);
    lua_pushboolean(lua_state, (a < b));
    return 1;
}

static int le_int64(lua_State *lua_state)
{
    int64_t a = *(int64_t *)lua_touserdata(lua_state, 1);
    int64_t b = *(int64_t *)lua_touserdata(lua_state, 2);
    lua_pushboolean(lua_state, (a <= b));
    return 1;
}

static int add_int64(lua_State *lua_state)
{
    int64_t a = *(int64_t *)lua_touserdata(lua_state, 1);
    int64_t b = *(int64_t *)lua_touserdata(lua_state, 2);
    int64_t c = a + b;
    ZCE_Lua_Tie::push_stack(c);
    return 1;
}

static int sub_int64(lua_State *lua_state)
{
    int64_t a = *(int64_t *)lua_touserdata(lua_state, 1);
    int64_t b = *(int64_t *)lua_touserdata(lua_state, 2);
    int64_t c = a - b;
    ZCE_Lua_Tie::push_stack(c);
    return 1;
}

static int set_int64(lua_State *lua_state)
{
    int64_t *data = (int64_t *)lua_touserdata(lua_state, 1);
    sscanf(lua_tostring(lua_state, 2), "%lld", data);
    return 0;
}

static int selfadd_int64(lua_State *lua_state)
{
    int64_t *data = (int64_t *)lua_touserdata(lua_state, 1);
    ++(*data);
    return 1;
}

static int selfsub_int64(lua_State *lua_state)
{
    int64_t *data = (int64_t *)lua_touserdata(lua_state, 1);
    --(*data);
    return 1;
}


void ZCE_Lua_Tie::register_int64(lua_State *lua_state)
{
    const char *name = "int64_t";
    lua_pushstring(lua_state, name);
    lua_newtable(lua_state);

    lua_pushstring(lua_state, "__name");
    lua_pushstring(lua_state, name);
    lua_rawset(lua_state, -3);

    lua_pushstring(lua_state, "__tostring");
    lua_pushcclosure(lua_state, tostring_int64, 0);
    lua_rawset(lua_state, -3);

    lua_pushstring(lua_state, "__eq");
    lua_pushcclosure(lua_state, eq_int64, 0);
    lua_rawset(lua_state, -3);

    lua_pushstring(lua_state, "__lt");
    lua_pushcclosure(lua_state, lt_int64, 0);
    lua_rawset(lua_state, -3);

    lua_pushstring(lua_state, "__le");
    lua_pushcclosure(lua_state, le_int64, 0);
    lua_rawset(lua_state, -3);

    lua_settable(lua_state, LUA_GLOBALSINDEX);
}

//=======================================================================================================
//为uint64_t 准备的metatable
static int tostring_uint64(lua_State *lua_state)
{
    char temp[64];
    snprintf(temp,63, "%llu", *(uint64_t *)lua_touserdata(lua_state, 1));
    lua_pushstring(lua_state, temp);
    return 1;
}

static int eq_uint64(lua_State *lua_state)
{
    bool bret = (memcmp(lua_touserdata(lua_state, 1),
        lua_touserdata(lua_state, 2),
        sizeof(unsigned long long)) == 0);
    lua_pushboolean(lua_state, bret);
    return 1;
}

static int lt_uint64(lua_State *lua_state)
{
    bool bret = (memcmp(lua_touserdata(lua_state, 1),
        lua_touserdata(lua_state, 2),
        sizeof(unsigned long long)) < 0);
    lua_pushboolean(lua_state, bret);
    return 1;
}

static int le_uint64(lua_State *lua_state)
{
    bool bret = (memcmp(lua_touserdata(lua_state, 1),
        lua_touserdata(lua_state, 2),
        sizeof(unsigned long long)) <= 0);
    lua_pushboolean(lua_state, bret);
    return 1;
}

void ZCE_Lua_Tie::register_uint64(lua_State *lua_state)
{
    const char *name = "uint64_t";
    lua_pushstring(lua_state, name);
    lua_newtable(lua_state);

    lua_pushstring(lua_state, "__name");
    lua_pushstring(lua_state, name);
    lua_rawset(lua_state, -3);

    lua_pushstring(lua_state, "__tostring");
    lua_pushcclosure(lua_state, tostring_uint64, 0);
    lua_rawset(lua_state, -3);

    lua_pushstring(lua_state, "__eq");
    lua_pushcclosure(lua_state, eq_uint64, 0);
    lua_rawset(lua_state, -3);

    lua_pushstring(lua_state, "__lt");
    lua_pushcclosure(lua_state, lt_uint64, 0);
    lua_rawset(lua_state, -3);

    lua_pushstring(lua_state, "__le");
    lua_pushcclosure(lua_state, le_uint64, 0);
    lua_rawset(lua_state, -3);

    lua_settable(lua_state, LUA_GLOBALSINDEX);
}



#endif //
