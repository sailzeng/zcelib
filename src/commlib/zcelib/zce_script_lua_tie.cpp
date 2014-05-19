
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

    lua_state_ = luaL_newstate();
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
//为int64_t 准备的metatable
static int tostring_int64(lua_State *lua_state)
{
    char temp[64];
    sprintf_s(temp, "%I64d", *(long long *)lua_topointer(lua_state, 1));
    lua_pushstring(lua_state, temp);
    return 1;
}


static int eq_int64(lua_State *lua_state)
{
    bool bret =(memcmp(lua_topointer(lua_state, 1),
        lua_topointer(lua_state, 2),
        sizeof(int64_t)) == 0);
    lua_pushboolean(lua_state, bret);
    return 1;
}

static int lt_int64(lua_State *lua_state)
{
    bool bret = (memcmp(lua_topointer(lua_state, 1),
        lua_topointer(lua_state, 2),
        sizeof(int64_t)) < 0);
    lua_pushboolean(lua_state, bret);
    return 1;
}

static int le_int64(lua_State *lua_state)
{
    bool bret = (memcmp(lua_topointer(lua_state, 1),
        lua_topointer(lua_state, 2),
        sizeof(int64_t)) <= 0);
    lua_pushboolean(lua_state, bret);
    return 1;
}

static int set_int64(lua_State *lua_state)
{
    bool bret = (memcmp(lua_topointer(lua_state, 1),
        lua_topointer(lua_state, 2),
        sizeof(int64_t)) <= 0);
    lua_pushboolean(lua_state, bret);
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
    sprintf_s(temp, "%I64u", *(unsigned long long *)lua_topointer(lua_state, 1));
    lua_pushstring(lua_state, temp);
    return 1;
}

static int eq_uint64(lua_State *lua_state)
{
    bool bret = (memcmp(lua_topointer(lua_state, 1),
        lua_topointer(lua_state, 2),
        sizeof(unsigned long long)) == 0);
    lua_pushboolean(lua_state, bret);
    return 1;
}

static int lt_uint64(lua_State *lua_state)
{
    bool bret = (memcmp(lua_topointer(lua_state, 1),
        lua_topointer(lua_state, 2),
        sizeof(unsigned long long)) < 0);
    lua_pushboolean(lua_state, bret);
    return 1;
}

static int le_uint64(lua_State *lua_state)
{
    bool bret = (memcmp(lua_topointer(lua_state, 1),
        lua_topointer(lua_state, 2),
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
