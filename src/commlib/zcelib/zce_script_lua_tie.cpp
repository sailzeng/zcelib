
#include "zce_predefine.h"
#include "zce_log_logging.h"
#include "zce_script_lua_tie.h"


#if ZCE_USE_LUA == 1 &&  ZCE_SUPPORT_CPP11 == 1


//=======================================================================================================
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

void push_stack(lua_State * /*state*/)
{
}
template<>
void push_stack(lua_State *state, char *const ptr)
{
    lua_pushstring(state, ptr);
}
template<>
void push_stack(lua_State *state, const char *const ptr)
{
    lua_pushstring(state, ptr);
}

//read_stack�Ӷ�ջ�ж�ȡһ������
template<>
void push_stack_val(lua_State *state, char val)
{
    lua_pushnumber(state, val);
}
template<>
void push_stack_val(lua_State *state, unsigned char val)
{
    lua_pushnumber(state, val);
}
template<>
void push_stack_val(lua_State *state, short val)
{
    lua_pushnumber(state, val);
}
template<>
void push_stack_val(lua_State *state, unsigned short val)
{
    lua_pushnumber(state, val);
}
template<>
void push_stack_val(lua_State *state, int val)
{
    lua_pushnumber(state, val);
}
template<>
void push_stack_val(lua_State *state, unsigned int val)
{
    lua_pushnumber(state, val);
}
template<>
void push_stack_val(lua_State *state, float val)
{
    lua_pushnumber(state, val);
}
template<>
void push_stack_val(lua_State *state, double val)
{
    lua_pushnumber(state, val);
}
template<>
void push_stack_val(lua_State *state, bool val)
{
    lua_pushboolean(state, val);
}
template<>
void push_stack_val(lua_State *state, int64_t val)
{
    *(int64_t *)lua_newuserdata(state, sizeof(int64_t)) = val;
    lua_pushstring(state, "int64_t");
    lua_gettable(state, LUA_GLOBALSINDEX);

    //��DEBUG�汾��ǿһЩ��飬�������table
#if defined DEBUG || defined _DEBUG
    if (!lua_istable(state, -1))
    {
        ZCE_LOG(RS_ERROR, "[LUATIE][int64_t] is not a table ? May be you don't register int64_t to lua? type id [%d]",
                lua_type(state, -1));
        lua_pop(state, 1);
        return;
    }
#endif

    lua_setmetatable(state, -2);
    return;
}
template<>
void push_stack_val(lua_State *state, uint64_t val)
{
    *(uint64_t *)lua_newuserdata(state, sizeof(uint64_t)) = val;
    lua_pushstring(state, "uint64_t");
    lua_gettable(state, LUA_GLOBALSINDEX);

    //��DEBUG�汾��ǿһЩ��飬�������table
#if defined DEBUG || defined _DEBUG
    if (!lua_istable(state, -1))
    {
        ZCE_LOG(RS_ERROR, "[LUATIE][uint64_t] is not a table? May be you don't register uint64_t to lua? typeid[%d]",
                lua_type(state, -1));
        lua_pop(state, 1);
        return;
    }
#endif

    lua_setmetatable(state, -2);
    return;
}


//=======================================================================================================
//read_stack�Ӷ�ջ�ж�ȡһ������

template<> char *read_stack_ptr(lua_State *state, int index)
{
    return (char *)lua_tostring(state, index);
}

template<> const char *read_stack_ptr(lua_State *state, int index)
{
    return (const char *)lua_tostring(state, index);
}


template<> char read_stack_val(lua_State *state, int index)
{
    return (char)lua_tonumber(state, index);
}

template<> unsigned char read_stack_val(lua_State *state, int index)
{
    return (unsigned char)lua_tonumber(state, index);
}

template<> short read_stack_val(lua_State *state, int index)
{
    return (short)lua_tonumber(state, index);
}

template<> unsigned short read_stack_val(lua_State *state, int index)
{
    return (unsigned short)lua_tonumber(state, index);
}

template<> int read_stack_val(lua_State *state, int index)
{
    return (int)lua_tonumber(state, index);
}

template<> unsigned int read_stack_val(lua_State *state, int index)
{
    return (unsigned int)lua_tonumber(state, index);
}

template<> float read_stack_val(lua_State *state, int index)
{
    return (float)lua_tonumber(state, index);
}

template<> double read_stack_val(lua_State *state, int index)
{
    return (double)lua_tonumber(state, index);
}

template<> bool read_stack_val(lua_State *state, int index)
{
    if (lua_isboolean(state, index))
    {
        return lua_toboolean(state, index) != 0;
    }
    //����ĳЩ�̶ȵ�ת�����ɹ���
    else
    {
        return lua_tonumber(state, index) != 0;
    }
}

template<> int64_t read_stack_val(lua_State *state, int index)
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
        ZCE_LOG(RS_ERROR, "stack index [%d] can't to be int64_t ,type id [%d]",
                index,
                lua_type(state, index));
        return 0;
    }
}

template<> uint64_t read_stack_val(lua_State *state, int index)
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
        ZCE_LOG(RS_ERROR, "stack index [%d] can't to be uint64_t ,type id [%d]",
                index,
                lua_type(state, index));
        return 0;
    }
}

template<> std::string read_stack_val(lua_State *state, int index)
{
    if (lua_isstring(state, index))
    {
        return std::string (lua_tostring(state, index));
    }
    else if (lua_isuserdata(state, index))
    {
        return *(std::string *) (((lua_udat_base *)lua_touserdata(state, index))->obj_ptr_);
    }
    else
    {
        ZCE_LOG(RS_ERROR, "[ZCELUA]stack index [%d] can't to be std::string ,type id [%d]",
                index,
                lua_type(state, index));
        return std::string("");
    }
}


//=======================================================================================================
//һЩ���õ����Ե�LUAע�ắ����

//ֻ����table��newdindex
int newindex_onlyread(lua_State *state)
{
    luaL_error(state, "Table is read only ,can't modify,please check your code.");
    return 1;
}

//dump lua���еĵĶ�ջ�����ڼ��lua����ʱ�����⣬�������
int dump_luacall_stack(lua_State *state)
{
    lua_Debug ar;
    int stack_lvl = 0;
    while (lua_getstack(state, stack_lvl, &ar) == 1)
    {
        lua_getinfo(state, "nSlu", &ar);
        if (stack_lvl == 0)
        {
            ZCE_LOG(RS_INFO, "[LUASTACK]===========================================" );
        }

        if (ar.name)
        {
            ZCE_LOG(RS_INFO, "%3d.%s() : line %d [%s : line %d]", stack_lvl, ar.name, ar.currentline, ar.source, ar.linedefined);
        }
        else
        {
            ZCE_LOG(RS_INFO, "%3d.unknown : line %d [%s : line %d]", stack_lvl, ar.currentline, ar.source, ar.linedefined);
        }
        ++stack_lvl;
    }
    return 0;
}

//dump C����lua�Ķ�ջ��
int enum_clua_stack(lua_State *state)
{
    int stack_top = lua_gettop(state);
    ZCE_LOG(RS_INFO, "[CLSTACK]C to lua Stack level:%d ====================================", stack_top);
    for (int i = 1; i <= stack_top; ++i)
    {
        int lua_typeid = lua_type(state, i);
        switch (lua_typeid)
        {
            case LUA_TNIL:
                ZCE_LOG(RS_INFO, "[ZCELUA][CLSTACK]%3d.%s",
                        i,
                        lua_typename(state, lua_typeid));
                break;
            case LUA_TBOOLEAN:
                ZCE_LOG(RS_INFO, "[ZCELUA][CLSTACK]%3d.%s [%s]",
                        i,
                        lua_typename(state, lua_typeid),
                        lua_toboolean(state, i) ? "true" : "false");
                break;
            case LUA_TLIGHTUSERDATA:
                ZCE_LOG(RS_INFO, "[ZCELUA][CLSTACK]%3d.%s [0x%p]",
                        i,
                        lua_typename(state, lua_typeid),
                        lua_topointer(state, i));
                break;
            case LUA_TNUMBER:
                ZCE_LOG(RS_INFO, "[ZCELUA][CLSTACK]%3d.%s [%f]",
                        i,
                        lua_typename(state, lua_typeid),
                        lua_tonumber(state, i));
                break;
            case LUA_TSTRING:
                ZCE_LOG(RS_INFO, "[ZCELUA][CLSTACK]%3d.%s[%s]",
                        i,
                        lua_typename(state, lua_typeid),
                        lua_tostring(state, i));
                break;
            case LUA_TTABLE:
                ZCE_LOG(RS_INFO, "[ZCELUA][CLSTACK]%3d.%s[0x%p]",
                        i,
                        lua_typename(state, lua_typeid),
                        lua_topointer(state, i));
                break;
            case LUA_TFUNCTION:
                ZCE_LOG(RS_INFO, "[ZCELUA][CLSTACK]%3d.%s() [0x%p]",
                        i,
                        lua_typename(state, lua_typeid),
                        lua_topointer(state, i));
                break;
            case LUA_TUSERDATA:
                ZCE_LOG(RS_INFO, "[ZCELUA][CLSTACK]%3d.%s[0x%p]",
                        i,
                        lua_typename(state, lua_typeid),
                        lua_topointer(state, i));
                break;
            case LUA_TTHREAD:
                ZCE_LOG(RS_INFO, "[ZCELUA][CLSTACK]%3d.%s",
                        i,
                        lua_typename(state, lua_typeid));
                break;
            default:
                ZCE_LOG(RS_INFO, "[ZCELUA][CLSTACK]%3d.type id %d type name %s",
                        i,
                        lua_typeid,
                        lua_typename(state, lua_typeid));
                break;
        }
    }
    return 0;
}

//
int on_error(lua_State *state)
{
    ZCE_LOG(RS_ERROR, "[LUATIE]error msg =%s", lua_tostring(state, -1));
    dump_luacall_stack(state);
    return 0;
}


//LUA�ĳ���ͨ����������궨�����__index
int class_meta_get(lua_State *state)
{
    int ret = 0;

    //Ҫ����������ջ�ײ���ȡ���õ���metatable����ջ��
    lua_getmetatable(state, 1);
    //����ջ�׵����ڶ�������key������ջ��
    lua_pushvalue(state, 2);

    //��metatable����Ѱ��key��
    lua_rawget(state, -2);

    //���û���ҵ�
    if (lua_isnil(state, -1))
    {
        //ȥ��nil
        lua_remove(state, -1);
        //�������ĸ��������Ƿ��п��Ե��õ�
        ret = class_parent(state);
    }

    if (ret == 0)
    {
        //�����һ��userdata����ʵ����������ӽ�ȥ����ĳ�Աָ��
        if (lua_isuserdata(state, -1))
        {
            //���е���,ȡ�������Ա���󣬷����ջ
            ((memvar_base *)lua_touserdata(state, -1))->get(state);
            //�Ӷ�ջ�Ƴ����usedata������Ҫ����stack��ֵ���Ѿ���get������ʹ����.
            lua_remove(state, -2);
        }
        //�����һ�����������أ�
        else if (lua_isfunction(state, -1))
        {
            //ʲôҲ����
        }
        else
        {
            lua_remove(state, -1);
            ret = -1;
        }
    }

    //�������ֵ��������û�еõ�
    if (0 != ret)
    {
        lua_pushfstring(state, "[LUATIE]%s can't find '%s' class variable. (forgot registering class variable ?)",
                        __ZCE_FUNC__,
                        lua_tostring(state, 2));
        lua_error(state);
        return 0;
    }

    //ɾ����metatable��
    lua_remove(state, -2);

    return 1;
}


//LUA�ĳ���ͨ�����������ɶ������__newindex
int class_meta_set(lua_State *state)
{
    int ret = 0;
    //�õ�table��meta table�����ݱ������Ƶõ���Ӧ�ı�����set����
    lua_getmetatable(state, 1);
    lua_pushvalue(state, 2);
    lua_rawget(state, -2);


    if (lua_isnil(state, -1))
    {
        //ȥ��nil
        lua_remove(state, -1);
        //�������ĸ��������Ƿ��п��Ե��õ�
        ret = class_parent(state);
    }

    if (ret == 0)
    {
        if (lua_isuserdata(state, -1))
        {
            ((memvar_base *)lua_touserdata(state, -1))->set(state);
        }
        else
        {
            //ȥ��nil
            lua_remove(state, -1);
            ret = -1;
        }
    }

    //�������ֵ��������û�еõ�
    if (0 != ret)
    {
        lua_pushfstring(state, "[LUATIE]%s can't find '%s' class variable. (forgot registering class variable ?)",
                        __ZCE_FUNC__,
                        lua_tostring(state, 2));
        lua_error(state);
        return 0;
    }

    return 0;
}


///���ø�ĸ�Ķ�Ӧ��meta table�����Ƿ�����Ӧ�� get
int class_parent(lua_State *state)
{
    int ret = 0;
    //
    lua_pushstring(state, "__parent");
    lua_rawget(state, -2);

    //�������table����ʾû�и�ĸ����ʧ
    if (!lua_istable(state, -1))
    {
        lua_remove(state, -1);
        return -1;
    }

    //�����ĸ��Ӧ����һ��table
    //����ջ�׵����ڶ�������key������ջ��,��metatable����Ѱ��key��
    lua_pushvalue(state, 2);
    lua_rawget(state, -2);

    //�����һ��userdata����ʵ����������ӽ�ȥ����ĳ�Աָ��
    if (lua_isnil(state, -1))
    {
        //����nil
        lua_remove(state, -1);
        ret = class_parent(state);
    }
    //
    lua_remove(state, -2);

    return ret;

}


int destroyer(lua_State *state)
{
    ((lua_udat_base *)(lua_touserdata(state, 1)))->~lua_udat_base();
    return 0;
}

};
};// namespace

//=======================================================================================================
//Ϊint64_t ׼����metatable
static int tostring_int64(lua_State *state)
{
    char temp[64];
    snprintf(temp, 63, "%" PRId64, *(int64_t *)lua_touserdata(state, 1));
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
    int64_t a = 0, b = 0;
    //read a
    if (lua_isuserdata(state, 1))
    {
        a = *(int64_t *)lua_touserdata(state, 1);
    }
    else if (lua_isnumber(state, 1))
    {
        a = static_cast<int64_t>(lua_tonumber(state, 1));
    }
    //read b
    if (lua_isuserdata(state, 2))
    {
        b = *(int64_t *)lua_touserdata(state, 2);
    }
    else if (lua_isnumber(state, 2))
    {
        b = static_cast<int64_t>(lua_tonumber(state, 2));
    }

    int64_t c = a + b;
    zce::luatie::push_stack(state, c);
    return 1;
}

static int sub_int64(lua_State *state)
{
    int64_t a = 0, b = 0;
    //read a
    if (lua_isuserdata(state, 1))
    {
        a = *(int64_t *)lua_touserdata(state, 1);
    }
    else if (lua_isnumber(state, 1))
    {
        a = static_cast<int64_t>(lua_tonumber(state, 1));
    }
    //read b
    if (lua_isuserdata(state, 2))
    {
        b = *(int64_t *)lua_touserdata(state, 2);
    }
    else if (lua_isnumber(state, 2))
    {
        b = static_cast<int64_t>(lua_tonumber(state, 2));
    }

    int64_t c = a - b;
    zce::luatie::push_stack(state, c);
    return 1;
}

static int constructor_int64(lua_State *state)
{
    int64_t data = 0;
    int fields = sscanf(lua_tostring(state, 1), "%" PRId64, &data);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    zce::luatie::push_stack(state, data);
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

void ZCE_Lua_Base::reg_int64()
{
    const char *int64_name = "int64_t";
    zce::luatie::class_name<int64_t>::name(int64_name);
    lua_pushstring(lua_state_, int64_name);
    lua_newtable(lua_state_);


    lua_pushstring(lua_state_, "__name");
    lua_pushstring(lua_state_, int64_name);
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

    //������Ŀ���������ģ�__call�Ƕ�Ӧһ��()���ã���ʵ�岻�Ǻ���ʽ���ĵ��ú���
    //LUA�г��������ĵ��ã�i1 =int64_t("123")
    lua_newtable(lua_state_);

    lua_pushstring(lua_state_, "__call");
    lua_pushcclosure(lua_state_, constructor_int64, 0);
    lua_rawset(lua_state_, -3);

    //�������table��Ϊint64_t ԭ�͵�metatable.
    lua_setmetatable(lua_state_, -2);

    lua_settable(lua_state_, LUA_GLOBALSINDEX);
}

//=======================================================================================================
//Ϊuint64_t ׼����metatable
static int tostring_uint64(lua_State *state)
{
    char temp[64];
    snprintf(temp, 63, "%" PRIu64, *(uint64_t *)lua_touserdata(state, 1));
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
    uint64_t a = 0, b = 0;
    //read a
    if (lua_isuserdata(state, 1))
    {
        a = *(uint64_t *)lua_touserdata(state, 1);
    }
    else if (lua_isnumber(state, 1))
    {
        a = static_cast<uint64_t>(lua_tonumber(state, 1));
    }
    //read b
    if (lua_isuserdata(state, 2))
    {
        b = *(uint64_t *)lua_touserdata(state, 2);
    }
    else if (lua_isnumber(state, 2))
    {
        b = static_cast<uint64_t>(lua_tonumber(state, 2));
    }

    uint64_t c = a + b;
    zce::luatie::push_stack(state, c);
    return 1;
}

static int sub_uint64(lua_State *state)
{
    uint64_t a = 0, b = 0;
    //read a
    if (lua_isuserdata(state, 1))
    {
        a = *(uint64_t *)lua_touserdata(state, 1);
    }
    else if (lua_isnumber(state, 1))
    {
        a = static_cast<uint64_t>(lua_tonumber(state, 1));
    }
    //read b
    if (lua_isuserdata(state, 2))
    {
        b = *(uint64_t *)lua_touserdata(state, 2);
    }
    else if (lua_isnumber(state, 2))
    {
        b = static_cast<uint64_t>(lua_tonumber(state, 2));
    }
    uint64_t c = a - b;
    zce::luatie::push_stack(state, c);
    return 1;
}
//���캯��������֧��int64_t("123")
static int constructor_uint64(lua_State *state)
{
    uint64_t data = 0;
    int fields = sscanf(lua_tostring(state, 1), "%" PRIu64, &data);
    if (fields != 1)
    {
        ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
    }
    zce::luatie::push_stack(state, data);
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

void ZCE_Lua_Base::reg_uint64()
{
    const char *uint64_name = "uint64_t";
    zce::luatie::class_name<uint64_t>::name(uint64_name);
    lua_pushstring(lua_state_, uint64_name);
    lua_newtable(lua_state_);


    lua_pushstring(lua_state_, "__name");
    lua_pushstring(lua_state_, uint64_name);
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

    //������Ŀ���������ģ�__call�Ƕ�Ӧһ��()���ã���ʵ�岻�Ǻ���ʽ���ĵ��ú���
    //LUA�г��������ĵ��ã�i1 =int64_t("123")
    lua_newtable(lua_state_);

    lua_pushstring(lua_state_, "__call");
    lua_pushcclosure(lua_state_, constructor_uint64, 0);
    lua_rawset(lua_state_, -3);

    //�������table��Ϊint64_t ԭ�͵�metatable.
    lua_setmetatable(lua_state_, -2);

    lua_settable(lua_state_, LUA_GLOBALSINDEX);
}

//=======================================================================================================
//Ϊstd::string ׼����metatable

static int tostring_stdstring(lua_State *state)
{
    lua_pushstring(state, ((std::string *)
                           (((zce::luatie::lua_udat_base *)lua_touserdata(state, 1))->obj_ptr_))->c_str() );
    return 1;
}

static int eq_stdstring(lua_State *state)
{
    std::string *a = (std::string *)(((zce::luatie::lua_udat_base *)lua_touserdata(state, 1))->obj_ptr_);
    std::string *b = (std::string *)(((zce::luatie::lua_udat_base *)lua_touserdata(state, 2))->obj_ptr_);
    lua_pushboolean(state, (*a == *b));
    return 1;
}

static int lt_stdstring(lua_State *state)
{
    std::string *a = (std::string *)(((zce::luatie::lua_udat_base *)lua_touserdata(state, 1))->obj_ptr_);
    std::string *b = (std::string *)(((zce::luatie::lua_udat_base *)lua_touserdata(state, 2))->obj_ptr_);
    lua_pushboolean(state, (*a < *b));
    return 1;
}

static int le_stdstring(lua_State *state)
{
    std::string *a = (std::string *)(((zce::luatie::lua_udat_base *)lua_touserdata(state, 1))->obj_ptr_);
    std::string *b = (std::string *)(((zce::luatie::lua_udat_base *)lua_touserdata(state, 2))->obj_ptr_);
    lua_pushboolean(state, (*a <= *b ));
    return 1;
}

static int add_stdstring(lua_State *state)
{
    std::string *ptr_a = nullptr, *ptr_b = nullptr, val_a, val_b;
    if (lua_isuserdata(state, 1))
    {
        ptr_a = (std::string *)(((zce::luatie::lua_udat_base *)lua_touserdata(state, 1))->obj_ptr_);
    }
    else if (lua_isstring(state, 1))
    {
        val_a = lua_tostring(state, 1);
        ptr_a = &val_a;
    }
    if (lua_isuserdata(state, 2))
    {
        ptr_b = (std::string *)(((zce::luatie::lua_udat_base *)
                                 lua_touserdata(state, 2))->obj_ptr_);
    }
    else if (lua_isstring(state, 2))
    {
        val_b = lua_tostring(state, 2);
        ptr_b = &val_a;
    }

    if (!ptr_a || !ptr_b)
    {
        lua_pushfstring(state, "[LUATIE] %s std::string .. operator is fail. "
                        "var a[%d][%s] or var b[%d][%s] is not string",
                        __ZCE_FUNC__,
                        lua_type(state, 1),
                        lua_typename(state, 1),
                        lua_type(state, 2),
                        lua_typename(state, 2)
                       );
        lua_error(state);
    }
    else
    {
        std::string c = *ptr_a + *ptr_b;
        zce::luatie::push_stack(state, c);
    }
    return 1;
}

static int constructor_stdstring(lua_State *state)
{
    std::string data(lua_tostring(state, 1));
    zce::luatie::push_stack(state, data);
    return 1;
}

//ע��std::string
void ZCE_Lua_Base::reg_stdstring()
{
    const char *stdstring_name = "stdstring";
    zce::luatie::class_name<std::string>::name(stdstring_name);
    lua_pushstring(lua_state_, stdstring_name);
    lua_newtable(lua_state_);


    lua_pushstring(lua_state_, "__name");
    lua_pushstring(lua_state_, stdstring_name);
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
    lua_pushcclosure(lua_state_, add_stdstring, 0);
    lua_rawset(lua_state_, -3);

    //������Ŀ���������ģ�__call�Ƕ�Ӧһ��()���ã���ʵ�岻�Ǻ���ʽ���ĵ��ú���
    //LUA�г��������ĵ��ã�i1 =stdstring("123")
    lua_newtable(lua_state_);

    lua_pushstring(lua_state_, "__call");
    lua_pushcclosure(lua_state_, constructor_stdstring, 0);
    lua_rawset(lua_state_, -3);

    //�������table��Ϊint64_t ԭ�͵�metatable.
    lua_setmetatable(lua_state_, -2);

    lua_settable(lua_state_, LUA_GLOBALSINDEX);
}






//=======================================================================================================
ZCE_Lua_Base::ZCE_Lua_Base(lua_State *lua_state) :
    lua_state_(lua_state)
{
}

ZCE_Lua_Base::~ZCE_Lua_Base()
{
}

// ִ��һ��LUA��buffer
int ZCE_Lua_Base::do_buffer(const char *buff, size_t len)
{
    int ret = 0;

    ::lua_pushcclosure(lua_state_, zce::luatie::on_error, 0);
    int errfunc = ::lua_gettop(lua_state_);

    ret = ::luaL_loadbuffer(lua_state_, buff, len, __ZCE_FUNC__);
    if (0 != ret)
    {
        ZCE_LOG(RS_ERROR, "luaL_loadbuffer ret= %d error msg= %s",
                ret,
                lua_tostring(lua_state_, -1));
        lua_pop(lua_state_, 1);
        lua_remove(lua_state_, errfunc);
        return ret;
    }

    //lua_pcall�Ĵ������Ҵ�����������ģ�
    ret = ::lua_pcall(lua_state_, 0, 0, errfunc);
    if (0 != ret)
    {
        ZCE_LOG(RS_ERROR, "lua_pcall ret = %d", ret);
    }

    ::lua_remove(lua_state_, errfunc);
    return 0;
}

// ִ��һ��LUA���ļ�
int ZCE_Lua_Base::do_file(const char *filename)
{
    int ret = 0;

    lua_pushcclosure(lua_state_, zce::luatie::on_error, 0);
    int errfunc = lua_gettop(lua_state_);

    ret = luaL_loadfile(lua_state_, filename);
    if (0 != ret)
    {
        ZCE_LOG(RS_ERROR, "luaL_loadbuffer ret= %d error msg= %s",
                ret,
                lua_tostring(lua_state_, -1));
        lua_pop(lua_state_, 1);
        ::lua_remove(lua_state_, errfunc);
        return ret;
    }

    //lua_pcall�Ĵ������Ҵ�����������ģ�
    ret = lua_pcall(lua_state_, 0, 0, errfunc);
    if (0 != ret)
    {
        ZCE_LOG(RS_ERROR, "lua_pcall ret = %d", ret);
    }

    ::lua_remove(lua_state_, errfunc);
    return 0;
}


///dump C����lua�Ķ�ջ��
void ZCE_Lua_Base::enum_stack()
{
    zce::luatie::enum_clua_stack(lua_state_);
}
///dump lua���еĵĶ�ջ�����ڼ��lua����ʱ�����⣬�������
void ZCE_Lua_Base::dump_stack()
{
    zce::luatie::dump_luacall_stack(lua_state_);
}

//=======================================================================================================
//Lua Thread �ķ�װ
ZCE_Lua_Thread::ZCE_Lua_Thread():
    ZCE_Lua_Base(nullptr)
{
}


ZCE_Lua_Thread::~ZCE_Lua_Thread()
{
}


//�����߳���ص�����
void ZCE_Lua_Thread::set_thread(lua_State *lua_thread, int thread_stackidx)
{
    lua_state_ = lua_thread;
    luathread_stackidx_ = thread_stackidx;
}

//ȡ���߳��ڴ����߶�ջ��λ������
int ZCE_Lua_Thread::get_thread_stackidx()
{
    return luathread_stackidx_;
}


//�ָ��߳�����
int ZCE_Lua_Thread::resume(int narg)
{
    return ::lua_resume(lua_state_, narg);
}

//�����߳�����
int ZCE_Lua_Thread::yield(int nresults)
{
    return ::lua_yield(lua_state_, nresults);
}


//=======================================================================================================
ZCE_Lua_Tie::ZCE_Lua_Tie():
    ZCE_Lua_Base(nullptr)
{
}

ZCE_Lua_Tie::~ZCE_Lua_Tie()
{
    close();
}

//��lua state
int ZCE_Lua_Tie::open(bool open_libs,
                      bool reg_common_use)
{
    //�������
    if (lua_state_)
    {
        ZCE_LOG(RS_ERROR, "lua_state_ != NULL ,reopen?");
        close();
    }

    //lua_open �����������luaL_newstate�����������Ϊ����ǰ����
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

//�ر�lua state
void ZCE_Lua_Tie::close()
{
    if (lua_state_)
    {
        lua_close(lua_state_);
        lua_state_ = nullptr;
    }
}


//����һ���µ�lua thread
int ZCE_Lua_Tie::new_thread(ZCE_Lua_Thread *lua_thread)
{
    lua_State *tread_state = lua_newthread(lua_state_);
    if (!tread_state)
    {
        return -1;
    }
    lua_thread->set_thread(tread_state, lua_gettop(lua_state_));
    return 0;
}

//ȡ���߳��ڴ����߶�ջ��λ������
void ZCE_Lua_Tie::del_thread(ZCE_Lua_Thread *lua_thread)
{
    ///Lua Thread�Ĵ��벻���Լ��ͷ��Լ���Lua Thread�ڶ�ջ����յ�ʱ�򣬻ᱻGC���յ�
    int idx = lua_thread->get_thread_stackidx();
    if (idx != 0)
    {
        lua_remove(lua_state_, idx);
    }
}

//�ָ�һ���̵߳�����
int ZCE_Lua_Tie::resume_thread(ZCE_Lua_Thread *lua_thread, int narg)
{
    return lua_thread->resume(narg);
}

#if defined (ZCE_OS_WINDOWS)
#pragma warning ( pop )
#elif defined (ZCE_OS_LINUX)
#pragma GCC diagnostic pop
#endif


#endif //#if ZCE_USE_LUA == 1 && ZCE_SUPPORT_CPP11 == 1

