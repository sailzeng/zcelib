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

    //
    void register_int64(lua_State *lua_state);
    //
    void register_uint64(lua_State *lua_state);

protected:

    //不允许出现long的变量，因为long无法移植，

    ///PUSH一个数据到堆栈，
    static template<>  void push_stack(lua_State *lua_state, char ret);
    static template<>  void push_stack(lua_State *lua_state, unsigned char ret);
    static template<>  void push_stack(lua_State *lua_state, short ret);
    static template<>  void push_stack(lua_State *lua_state, unsigned short ret);
    static template<>  void push_stack(lua_State *lua_state, int ret);
    static template<>  void push_stack(lua_State *lua_state, unsigned int ret);
    static template<>  void push_stack(lua_State *lua_state, float ret);
    static template<>  void push_stack(lua_State *lua_state, double ret);
    static template<>  void push_stack(lua_State *lua_state, char *ret);
    static template<>  void push_stack(lua_State *lua_state, const char *ret);
    static template<>  void push_stack(lua_State *lua_state, bool ret);
    static template<>  void push_stack(lua_State *lua_state, int64_t ret);
    static template<>  void push_stack(lua_State *lua_state, uint64_t ret);

    static template<>  char               *read_stack(lua_State *lua_state, int index);
    static template<>  const char         *read_stack(lua_State *lua_state, int index);
    static template<>  char                read_stack(lua_State *lua_state, int index);
    static template<>  unsigned char       read_stack(lua_State *lua_state, int index);
    static template<>  short               read_stack(lua_State *lua_state, int index);
    static template<>  unsigned short      read_stack(lua_State *lua_state, int index);
    static template<>  int                 read_stack(lua_State *lua_state, int index);
    static template<>  unsigned int        read_stack(lua_State *lua_state, int index);
    static template<>  float               read_stack(lua_State *lua_state, int index);
    static template<>  double              read_stack(lua_State *lua_state, int index);
    static template<>  bool                read_stack(lua_State *lua_state, int index);
    static template<>  int64_t             read_stack(lua_State *lua_state, int index);
    static template<>  uint64_t            read_stack(lua_State *lua_state, int index);

protected:

    //
    lua_State   *lua_state_;
};



#endif

#endif // ZCE_LIB_SCRIPT_LUA_H_

