
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


    lua_tie.set_garray("g_array", 20, g_array);

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
    lua_tie.class_member_var<TA>("a_", &TA::a_);
    lua_tie.class_constructor<TA>(ZCE_LUA::constructor<TA,int> );

    TA ta_val(100);
    TA *ta_ptr = new TA(200);
    TA ta_1(300);
    TA &ta_ref = ta_1;

    lua_tie.set_gvar("ta_val",ta_val);
    lua_tie.set_gvar("ta_ptr", ta_ptr);
    lua_tie.set_gvar<TA &>("ta_ref", ta_ref);

    lua_tie.close();

    return 0;
}


