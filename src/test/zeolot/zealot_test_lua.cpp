
#include "zealot_predefine.h"
#include "zealot_test_function.h"


//
int int32_add_fun(int a, int b)
{
    return a + b;
}


int test_lua_script()
{
    ZCE_Lua_Tie lua_tie;
    lua_tie.open();
    lua_tie.reg_fun("int32_add_fun", int32_add_fun);
    lua_tie.close();

    return 0;
}
