
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
static int tostring_s64(lua_State *lua_state)
{
    char temp[64];
    sprintf_s(temp, "%I64d", *(long long *)lua_topointer(lua_state, 1));
    lua_pushstring(lua_state, temp);
    return 1;
}


static int eq_s64(lua_State *lua_state)
{
    bool bret =(memcmp(lua_topointer(lua_state, 1),
        lua_topointer(lua_state, 2),
        sizeof(int64_t)) == 0);
    lua_pushboolean(lua_state, bret);
    return 1;
}

static int lt_s64(lua_State *lua_state)
{
    bool bret = (memcmp(lua_topointer(lua_state, 1),
        lua_topointer(lua_state, 2),
        sizeof(int64_t)) < 0);
    lua_pushboolean(lua_state, bret);
    return 1;
}

static int le_s64(lua_State *lua_state)
{
    bool bret = (memcmp(lua_topointer(lua_state, 1),
        lua_topointer(lua_state, 2),
        sizeof(int64_t)) <= 0);
    lua_pushboolean(lua_state, bret);
    return 1;
}


void ZCE_Lua_Tie::::register_int64_t(lua_State *L)
{
    const char *name = "__s64";
    lua_pushstring(L, name);
    lua_newtable(L);

    lua_pushstring(L, "__name");
    lua_pushstring(L, name);
    lua_rawset(L, -3);

    lua_pushstring(L, "__tostring");
    lua_pushcclosure(L, tostring_s64, 0);
    lua_rawset(L, -3);

    lua_pushstring(L, "__eq");
    lua_pushcclosure(L, eq_s64, 0);
    lua_rawset(L, -3);

    lua_pushstring(L, "__lt");
    lua_pushcclosure(L, lt_s64, 0);
    lua_rawset(L, -3);

    lua_pushstring(L, "__le");
    lua_pushcclosure(L, le_s64, 0);
    lua_rawset(L, -3);

    lua_settable(L, LUA_GLOBALSINDEX);
}

//=======================================================================================================
//为uint64_t 准备的metatable
static int tostring_u64(lua_State *L)
{
    char temp[64];
    sprintf_s(temp, "%I64u", *(unsigned long long *)lua_topointer(L, 1));
    lua_pushstring(L, temp);
    return 1;
}

/*---------------------------------------------------------------------------*/
static int eq_u64(lua_State *L)
{
    lua_pushboolean(L, memcmp(lua_topointer(L, 1), lua_topointer(L, 2), sizeof(unsigned long long)) == 0);
    return 1;
}

/*---------------------------------------------------------------------------*/
static int lt_u64(lua_State *L)
{
    lua_pushboolean(L, memcmp(lua_topointer(L, 1), lua_topointer(L, 2), sizeof(unsigned long long)) < 0);
    return 1;
}

/*---------------------------------------------------------------------------*/
static int le_u64(lua_State *L)
{
    lua_pushboolean(L, memcmp(lua_topointer(L, 1), lua_topointer(L, 2), sizeof(unsigned long long)) <= 0);
    return 1;
}

/*---------------------------------------------------------------------------*/
void lua_tinker::init_u64(lua_State *L)
{
    const char *name = "__u64";
    lua_pushstring(L, name);
    lua_newtable(L);

    lua_pushstring(L, "__name");
    lua_pushstring(L, name);
    lua_rawset(L, -3);

    lua_pushstring(L, "__tostring");
    lua_pushcclosure(L, tostring_u64, 0);
    lua_rawset(L, -3);

    lua_pushstring(L, "__eq");
    lua_pushcclosure(L, eq_u64, 0);
    lua_rawset(L, -3);

    lua_pushstring(L, "__lt");
    lua_pushcclosure(L, lt_u64, 0);
    lua_rawset(L, -3);

    lua_pushstring(L, "__le");
    lua_pushcclosure(L, le_u64, 0);
    lua_rawset(L, -3);

    lua_settable(L, LUA_GLOBALSINDEX);
}



#endif //
