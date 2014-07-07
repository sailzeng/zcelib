
#include "zce_predefine.h"
#include "zce_trace_log_debug.h"
#include "zce_script_lua_tie.h"


#if defined  ZCE_USE_LUA && defined ZCE_SUPPORT_CPP11


//=======================================================================================================
//read_stack从堆栈中读取一个数据

template<> char *ZCE_LUA::read_stack(lua_State *state, int index)
{
    return (char *)lua_tostring(state, index);
}

template<> const char *ZCE_LUA::read_stack(lua_State *state, int index)
{
    return (const char *)lua_tostring(state, index);
}

template<> char ZCE_LUA::read_stack(lua_State *state, int index)
{
    return (char)lua_tonumber(state, index);
}

template<> unsigned char ZCE_LUA::read_stack(lua_State *state, int index)
{
    return (unsigned char)lua_tonumber(state, index);
}

template<> short ZCE_LUA::read_stack(lua_State *state, int index)
{
    return (short)lua_tonumber(state, index);
}

template<> unsigned short ZCE_LUA::read_stack(lua_State *state, int index)
{
    return (unsigned short)lua_tonumber(state, index);
}

template<> int ZCE_LUA::read_stack(lua_State *state, int index)
{
    return (int)lua_tonumber(state, index);
}

template<> unsigned int ZCE_LUA::read_stack(lua_State *state, int index)
{
    return (unsigned int)lua_tonumber(state, index);
}

template<> float ZCE_LUA::read_stack(lua_State *state, int index)
{
    return (float)lua_tonumber(state, index);
}

template<> double ZCE_LUA::read_stack(lua_State *state, int index)
{
    return (double)lua_tonumber(state, index);
}

template<> bool ZCE_LUA::read_stack(lua_State *state, int index)
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

template<> int64_t ZCE_LUA::read_stack(lua_State *state, int index)
{
    if (lua_isnumber(state, index))
    {
        return (int64_t)lua_tonumber(state, index);
    }
    else if (lua_isuserdata(state, index))
    {
        return *(int64_t *)lua_touserdata(state, index);
    }
    else
    {
        ZCE_LOGMSG(RS_ERROR, "stack index [%d] can't to be int64_t ,type id [%d]",
                   index,
                   lua_type(state, index));
        return 0;
    }
}

template<> uint64_t ZCE_LUA::read_stack(lua_State *state, int index)
{
    if (lua_isnumber(state, index))
    {
        return (uint64_t)lua_tonumber(state, index);
    }
    else if (lua_isuserdata(state, index))
    {
        return *(uint64_t *)lua_touserdata(state, index);
    }
    else
    {
        ZCE_LOGMSG(RS_ERROR, "stack index [%d] can't to be uint64_t ,type id [%d]",
                   index,
                   lua_type(state, index));
        lua_pop(state, 1);
        return 0;
    }
}

template<> std::string ZCE_LUA::read_stack(lua_State *state, int index)
{
    if (lua_isstring(state, index))
    {
        return std::string (lua_tostring(state, index));
    }
    else if (lua_isuserdata(state, index))
    {
        return *(std::string *) (state, index);
    }
    else
    {
        ZCE_LOGMSG(RS_ERROR, "[ZCELUA]stack index [%d] can't to be std::string ,type id [%d]",
                   index,
                   lua_type(state, index));
        lua_pop(state, 1);
        return std::string("");
    }
}
//=======================================================================================================
//read_stack从堆栈中读取一个数据
template<> void ZCE_LUA::push_stack(lua_State *state, char val)
{
    lua_pushnumber(state, val);
}

template<> void ZCE_LUA::push_stack(lua_State *state, unsigned char val)
{
    lua_pushnumber(state, val);
}

template<> void ZCE_LUA::push_stack(lua_State *state, short val)
{
    lua_pushnumber(state, val);
}

template<> void ZCE_LUA::push_stack(lua_State *state, unsigned short val)
{
    lua_pushnumber(state, val);
}

template<> void ZCE_LUA::push_stack(lua_State *state, int val)
{
    lua_pushnumber(state, val);
}

template<> void ZCE_LUA::push_stack(lua_State *state, unsigned int val)
{
    lua_pushnumber(state, val);
}

template<> void ZCE_LUA::push_stack(lua_State *state, float val)
{
    lua_pushnumber(state, val);
}

template<> void ZCE_LUA::push_stack(lua_State *state, double val)
{
    lua_pushnumber(state, val);
}

template<> void ZCE_LUA::push_stack(lua_State *state, char *val)
{
    lua_pushstring(state, val);
}

template<> void ZCE_LUA::push_stack(lua_State *state, const char *val)
{
    lua_pushstring(state, val);
}

template<> void ZCE_LUA::push_stack(lua_State *state, bool val)
{
    lua_pushboolean(state, val);
}

template<> void ZCE_LUA::push_stack(lua_State *state, int64_t val)
{
    *(int64_t *)lua_newuserdata(state, sizeof(int64_t)) = val;
    lua_pushstring(state, "int64_t");
    lua_gettable(state, LUA_GLOBALSINDEX);

    //在DEBUG版本增强一些检查，如果不是table
#if defined DEBUG || defined _DEBUG
    if (!lua_istable(state, -1))
    {
        ZCE_LOGMSG(RS_ERROR, "[LUATIE][int64_t] is not a table ? May be you don't register int64_t to lua? type id [%d]",
                   lua_type(state, -1));
        lua_pop(state, 1);
        return;
    }
#endif

    lua_setmetatable(state, -2);
    return;
}

template<> void ZCE_LUA::push_stack(lua_State *state, uint64_t val)
{
    *(uint64_t *)lua_newuserdata(state, sizeof(uint64_t)) = val;
    lua_pushstring(state, "uint64_t");
    lua_gettable(state, LUA_GLOBALSINDEX);

    //在DEBUG版本增强一些检查，如果不是table
#if defined DEBUG || defined _DEBUG
    if (!lua_istable(state, -1))
    {
        ZCE_LOGMSG(RS_ERROR, "[LUATIE][uint64_t] is not a table? May be you don't register uint64_t to lua? typeid[%d]",
                   lua_type(state, -1));
        lua_pop(state, 1);
        return;
    }
#endif

    lua_setmetatable(state, -2);
    return;
}

template<> void ZCE_LUA::push_stack(lua_State *state, std::string val)
{
    *(std::string *)lua_newuserdata(state, sizeof(std::string)) = val;
    lua_pushstring(state, "stdstring");
    lua_gettable(state, LUA_GLOBALSINDEX);

    //在DEBUG版本增强一些检查，如果不是table
#if defined DEBUG || defined _DEBUG
    if (!lua_istable(state, -1))
    {
        ZCE_LOGMSG(RS_ERROR, "[LUATIE][std::string] is not a table? May be you don't register std::string to lua?typeid[%d]",
                   lua_type(state, -1));
        lua_pop(state, 1);
        return;
    }
#endif
    lua_setmetatable(state, -2);
    return;
}

//=======================================================================================================
//一些公用的特性的LUA注册函数，

//只读的table的newdindex
int ZCE_LUA::newindex_onlyread(lua_State *state)
{
    luaL_error(state, "Table is read only ,can't modify,please check your code.");
    return 1;
}

//dump lua运行的的堆栈，用于检查lua运行时的问题，错误处理等
int ZCE_LUA::dump_luacall_stack(lua_State *state)
{
    lua_Debug ar;
    int stack_lvl = 0;
    while (lua_getstack(state, stack_lvl, &ar) == 1)
    {
        lua_getinfo(state, "nSlu", &ar);
        if (stack_lvl == 0)
        {
            ZCE_LOGMSG(RS_INFO, "[LUASTACK]===========================================" );
        }

        if (ar.name)
        {
            ZCE_LOGMSG(RS_INFO, "%3d.%s() : line %d [%s : line %d]", stack_lvl, ar.name, ar.currentline, ar.source, ar.linedefined);
        }
        else
        {
            ZCE_LOGMSG(RS_INFO, "%3d.unknown : line %d [%s : line %d]", stack_lvl, ar.currentline, ar.source, ar.linedefined);
        }
        ++stack_lvl;
    }
    return 0;
}

//dump C调用lua的堆栈，
int ZCE_LUA::dump_clua_stack(lua_State *state)
{
    int stack_top = lua_gettop(state);
    ZCE_LOGMSG(RS_INFO, "[CLSTACK]C to lua Stack level:%d ====================================", stack_top);
    for (int i = 1; i <= stack_top; ++i)
    {
        int lua_typeid = lua_type(state, i);
        switch (lua_typeid)
        {
            case LUA_TNIL:
                ZCE_LOGMSG(RS_INFO, "[ZCELUA][CLSTACK]%3d.%s",
                           i,
                           lua_typename(state, lua_typeid));
                break;
            case LUA_TBOOLEAN:
                ZCE_LOGMSG(RS_INFO, "[ZCELUA][CLSTACK]%3d.%s [%s]",
                           i,
                           lua_typename(state, lua_typeid),
                           lua_toboolean(state, i) ? "true" : "false");
                break;
            case LUA_TLIGHTUSERDATA:
                ZCE_LOGMSG(RS_INFO, "[ZCELUA][CLSTACK]%3d.%s [0x%p]",
                           i,
                           lua_typename(state, lua_typeid),
                           lua_topointer(state, i));
                break;
            case LUA_TNUMBER:
                ZCE_LOGMSG(RS_INFO, "[ZCELUA][CLSTACK]%3d.%s [%f]",
                           i,
                           lua_typename(state, lua_typeid),
                           lua_tonumber(state, i));
                break;
            case LUA_TSTRING:
                ZCE_LOGMSG(RS_INFO, "[ZCELUA][CLSTACK]%3d.%s[%s]",
                           i,
                           lua_typename(state, lua_typeid),
                           lua_tostring(state, i));
                break;
            case LUA_TTABLE:
                ZCE_LOGMSG(RS_INFO, "[ZCELUA][CLSTACK]%3d.%s[0x%p]",
                           i,
                           lua_typename(state, lua_typeid),
                           lua_topointer(state, i));
                break;
            case LUA_TFUNCTION:
                ZCE_LOGMSG(RS_INFO, "[ZCELUA][CLSTACK]%3d.%s() [0x%p]",
                           i,
                           lua_typename(state, lua_typeid),
                           lua_topointer(state, i));
                break;
            case LUA_TUSERDATA:
                ZCE_LOGMSG(RS_INFO, "[ZCELUA][CLSTACK]%3d.%s[0x%p]",
                           i,
                           lua_typename(state, lua_typeid),
                           lua_topointer(state, i));
                break;
            case LUA_TTHREAD:
                ZCE_LOGMSG(RS_INFO, "[ZCELUA][CLSTACK]%3d.%s",
                           i,
                           lua_typename(state, lua_typeid));
                break;
            default:
                ZCE_LOGMSG(RS_INFO, "[ZCELUA][CLSTACK]%3d.type id %d type name %s",
                           i,
                           lua_typeid,
                           lua_typename(state, lua_typeid));
                break;
        }
    }
    return 0;
}

//
int ZCE_LUA::on_error(lua_State *state)
{
    ZCE_LOGMSG(RS_ERROR, "[ZCELUA]error msg =%s", lua_tostring(state, 1));
    lua_pop(state, 1);
    dump_luacall_stack(state);
    return 0;
}


//LUA的程序通过这个函数完定义类的__index
int ZCE_LUA::class_meta_get(lua_State *state)
{
    int meta_ret = 0;
    //要检查的数据在栈底部，取出得到其metatable放入栈顶
    lua_getmetatable(state, 1);
    //复制栈底倒数第二个参数key，放入栈顶
    lua_pushvalue(state, 2);
    //在metatable里面寻找key，
    lua_rawget(state, -2);

    //如果是一个userdata，其实其就是我们扔进去的类的成员指针
    if (lua_isuserdata(state, -1))
    {
        //进行调用
        ((memvar_base *)lua_touserdata(state, -1))->get(state);
        //从堆栈移除这个key，
        lua_remove(state, -2);
        meta_ret = 0;
    }
    //如果没有找到
    else if (lua_isnil(state, -1))
    {
        //去掉nil
        lua_remove(state, -1);
        //检查的他的父类里面是否有可以调用的
        meta_ret = class_parent(state);
        if (0 == meta_ret )
        {
            ((memvar_base *)lua_touserdata(state, -1))->get(state);
        }
    }
    else
    {
        lua_remove(state, -1);
        meta_ret = -1;
    }

    //如果返回值告诉我们没有得到
    if (0 != meta_ret)
    {
        lua_pushfstring(state, "[LUATIE]%s can't find '%s' class variable. (forgot registering class variable ?)",
                        __ZCE_FUNCTION__,
                        lua_tostring(state, 2));
        lua_error(state);
        return meta_ret;
    }

    //删除掉metatable，
    lua_remove(state, -2);

    return 1;
}


//LUA的程序通过这个函数完成定义类的__newindex
int ZCE_LUA::class_meta_set(lua_State *state)
{
    int meta_ret = -1;
    //得到table的meta table，根据变量名称得到对应的变量的set函数
    lua_getmetatable(state, 1);
    lua_pushvalue(state, 2);
    lua_rawget(state, -2);

    if (lua_isuserdata(state, -1))
    {
        ((memvar_base *)lua_touserdata(state, -1))->set(state);
    }
    else if (lua_isnil(state, -1))
    {
        //去掉nil
        lua_remove(state, -1);
        //检查的他的父类里面是否有可以调用的
        meta_ret = class_parent(state);
        if (0 == meta_ret)
        {
            ((memvar_base *)lua_touserdata(state, -1))->set(state);
        }
    }
    else
    {
        //去掉nil
        lua_remove(state, -1);
        meta_ret = -1;
    }

    //如果返回值告诉我们没有得到
    if (0 != meta_ret)
    {
        lua_pushfstring(state, "[LUATIE]%s can't find '%s' class variable. (forgot registering class variable ?)",
                        __ZCE_FUNCTION__,
                        lua_tostring(state, 2));
        lua_error(state);
        return meta_ret;
    }

    return 0;
}


///调用父母的对应的meta table里面是否有相应的 get
int ZCE_LUA::class_parent(lua_State *state)
{

    //
    lua_pushstring(state, "__parent");
    lua_rawget(state, -2);

    //如果不是table，表示没有父母，消失
    if (!lua_istable(state, -1))
    {
        lua_pop(state, 1);
        return -1;
    }

    //如果父母对应的是一个table
    //复制栈底倒数第二个参数key，放入栈顶,在metatable里面寻找key，
    lua_pushvalue(state, 2);
    lua_rawget(state, -2);

    //如果是一个userdata，其实其就是我们扔进去的类的成员指针
    if (lua_isuserdata(state, -1))
    {
        //进行调用,会把取出的数据push stack，
        ((memvar_base *)lua_touserdata(state, -1))->get(state);
        //从堆栈移除这个meta table
        lua_remove(state, -2);
        return 0;
    }
    else if (lua_isnil(state, -1))
    {
        //弹出nil
        lua_pop(state, 1);
        int pk_ret = class_parent(state);
        //如果正确
        if (pk_ret == 0)
        {
            lua_remove(state, -2);

        }
        else
        {
            lua_remove(state, -1);
        }
        return pk_ret;

    }
    else
    {
        lua_pop(state, 2);
        return -1;
    }
}


int ZCE_LUA::destroyer(lua_State *state)
{
    ((lua_udat_base *)(lua_touserdata(state, 1)))->~lua_udat_base();
    return 0;
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
    lua_pushboolean(state, (a == b));
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
    ZCE_LUA::push_stack(state, c);
    return 1;
}

static int sub_int64(lua_State *state)
{
    int64_t a = *(int64_t *)lua_touserdata(state, 1);
    int64_t b = *(int64_t *)lua_touserdata(state, 2);
    int64_t c = a - b;
    ZCE_LUA::push_stack(state, c);
    return 1;
}

static int constructor_int64(lua_State *state)
{
    int64_t data = 0;
    sscanf(lua_tostring(state, 1), "%lld", &data);
    ZCE_LUA::push_stack(state, data);
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
    ZCE_LUA::push_stack(state, c);
    return 1;
}

static int sub_uint64(lua_State *state)
{
    uint64_t a = *(uint64_t *)lua_touserdata(state, 1);
    uint64_t b = *(uint64_t *)lua_touserdata(state, 2);
    uint64_t c = a - b;
    ZCE_LUA::push_stack(state, c);
    return 1;
}
//构造函数，让你支持int64_t("123")
static int constructor_uint64(lua_State *state)
{
    uint64_t data = 0;
    sscanf(lua_tostring(state, 1), "%llu", &data);
    ZCE_LUA::push_stack(state, data);
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
    lua_pushboolean(state, (*a == *b));
    return 1;
}

static int lt_stdstring(lua_State *state)
{
    std::string *a = (std::string *)lua_touserdata(state, 1);
    std::string *b = (std::string *)lua_touserdata(state, 2);
    lua_pushboolean(state, (*a < *b));
    return 1;
}

static int le_stdstring(lua_State *state)
{
    std::string *a = (std::string *)lua_touserdata(state, 1);
    std::string *b = (std::string *)lua_touserdata(state, 2);
    lua_pushboolean(state, (*a <= *b ));
    return 1;
}

static int add_stdstring(lua_State *state)
{
    std::string *a = (std::string *)lua_touserdata(state, 1);
    std::string *b = (std::string *)lua_touserdata(state, 2);
    std::string c = *a + *b;
    ZCE_LUA::push_stack(state, c);
    return 1;
}

static int concat_stdstring(lua_State *state)
{
    std::string *a = (std::string *)lua_touserdata(state, 1);
    std::string *b = (std::string *)lua_touserdata(state, 2);
    std::string c = *a + *b;
    ZCE_LUA::push_stack(state, c);
    return 1;
}

static int constructor_stdstring(lua_State *state)
{
    std::string data(lua_tostring(state, 1));
    ZCE_LUA::push_stack(state, data);
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
    lua_pushcclosure(lua_state_, constructor_stdstring, 0);
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
    //由于不知道你的枚举值是否是array，所以这样申请的，
    lua_createtable(lua_state_, 0, static_cast<int>(item_num));

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
    lua_pushcclosure(lua_state_, ZCE_LUA::newindex_onlyread, 0);
    lua_rawset(lua_state_, -3);

    lua_setmetatable(lua_state_, -2);

    lua_settable(lua_state_, LUA_GLOBALSINDEX);
}







//=======================================================================================================
ZCE_Lua_Tie::ZCE_Lua_Tie() :
    lua_state_(nullptr)
{
}

ZCE_Lua_Tie::~ZCE_Lua_Tie()
{
    close();
}

//打开lua state
int ZCE_Lua_Tie::open(bool open_libs,
                      bool reg_common_use)
{
    //如果错误
    if (lua_state_)
    {
        ZCE_LOGMSG(RS_ERROR, "lua_state_ != NULL ,reopen?");
        close();
    }

    //lua_open 这个东东就是luaL_newstate，这个好像是为了向前兼容
    lua_state_ = lua_open();
    if (nullptr == lua_state_)
    {
        return -1;
    }

    if (open_libs)
    {
        luaL_openlibs(lua_state_);
    }

    if (reg_common_use)
    {
        reg_int64();
        reg_uint64();
        reg_stdstring();
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
int ZCE_Lua_Tie::do_buffer(const char *buff, size_t len)
{
    int ret = 0;

    lua_pushcclosure(lua_state_, ZCE_LUA::on_error, 0);
    int errfunc = lua_gettop(lua_state_);

    ret = luaL_loadbuffer(lua_state_, buff, len, __ZCE_FUNCTION__);
    if (0 != ret)
    {
        ZCE_LOGMSG(RS_ERROR, "luaL_loadbuffer ret= %d error msg= %s",
                   ret,
                   lua_tostring(lua_state_, -1));
        lua_pop(lua_state_, 1);
        lua_remove(lua_state_, errfunc);
        return ret;
    }

    //lua_pcall的错误是右错误处理函数处理的，
    ret = lua_pcall(lua_state_, 0, 0, errfunc);
    if (0 != ret)
    {
        ZCE_LOGMSG(RS_ERROR, "lua_pcall ret = %d", ret);
    }

    lua_remove(lua_state_, errfunc);
    return 0;
}

// 执行一个LUA的文件
int ZCE_Lua_Tie::do_file(const char *filename)
{
    int ret = 0;

    lua_pushcclosure(lua_state_, ZCE_LUA::on_error, 0);
    int errfunc = lua_gettop(lua_state_);

    ret = luaL_loadfile(lua_state_, filename);
    if (0 != ret)
    {
        ZCE_LOGMSG(RS_ERROR, "luaL_loadbuffer ret= %d error msg= %s",
                   ret,
                   lua_tostring(lua_state_, -1));
        lua_pop(lua_state_, 1);
        lua_remove(lua_state_, errfunc);
        return ret;
    }

    //lua_pcall的错误是右错误处理函数处理的，
    ret = lua_pcall(lua_state_, 0, 0, errfunc);
    if (0 != ret)
    {
        ZCE_LOGMSG(RS_ERROR, "lua_pcall ret = %d", ret);
    }

    lua_remove(lua_state_, errfunc);
    return 0;
}

///dump C调用lua的堆栈，
void ZCE_Lua_Tie::dump_clua_stack()
{
    ZCE_LUA::dump_clua_stack(lua_state_);
}
///dump lua运行的的堆栈，用于检查lua运行时的问题，错误处理等
void ZCE_Lua_Tie::dump_luacall_stack()
{
    ZCE_LUA::dump_luacall_stack(lua_state_);
}


#endif //#if defined  ZCE_USE_LUA && defined ZCE_SUPPORT_CPP11

