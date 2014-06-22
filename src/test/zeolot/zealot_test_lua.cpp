
#include "zealot_predefine.h"
#include "zealot_test_function.h"


//
int int32_add_fun(int a, int b)
{
    return a + b;
}


int test_lua_script1()
{
    ZCE_Lua_Tie lua_tie;
    lua_tie.open(true,true);
    lua_tie.tie_gfun("int32_add_fun", int32_add_fun);
    lua_tie.close();

    return 0;
}


int g_a = 1;
int g_b = 2;


int g_array[20];

int test_lua_script2(int ,char *[])
{
    ZCE_Lua_Tie lua_tie;
    lua_tie.open(true, true);
    lua_tie.set_gvar("g_b_var", g_b);
    lua_tie.set_gvar("g_b_ptr", &g_b);
    int &ref_gb = g_b;
    lua_tie.set_gvar<int &>("g_a_ref", ref_gb);


    lua_tie.set_garray("g_array", g_array,20);

    lua_tie.close();

    return 0;
}

struct TA
{
    TA(int a) :a_(a)
    {
    }

    int a_;
};

int test_lua_script3(int, char *[])
{
    ZCE_Lua_Tie lua_tie;
    lua_tie.open(true, true);
    lua_tie.tie_class<TA>("TA",false);
    lua_tie.tie_member_var<TA>("a_", &TA::a_);
    lua_tie.tie_constructor<TA>(ZCE_LUA::constructor<TA,int> );

    TA ta_val(100);
    TA *ta_ptr = new TA(200);
    TA ta_1(300);
    TA &ta_ref = ta_1;

    lua_tie.set_gvar("ta_val",ta_val);
    lua_tie.set_gvar("ta_ptr", ta_ptr);
    lua_tie.set_gvar<TA &>("ta_ref", ta_ref);

    int ret_a = 0, ret_b = 0;
    int var_a = 100, var_b = 200, var_c = 300;
    lua_tie.call_luafun_2("add_abc", ret_a, ret_b, var_a, var_b, var_c);

    lua_tie.close();

    return 0;
}


