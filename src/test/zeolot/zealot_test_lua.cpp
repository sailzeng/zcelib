
#include "zealot_predefine.h"
#include "zealot_test_function.h"


//
int add2_fun(int a, int b)
{
    return a + b;
}


int add3_fun(int a, int b,int c)
{
    return a + b+ c;
}


//测试两边函数的互相调用
int test_lua_script1(int, char *[])
{
    ZCE_Lua_Tie lua_tie;
    lua_tie.open(true,true);

    

    lua_tie.reg_gfun("add2_fun", add2_fun);

    lua_tie.reg_gfun("add3_fun", add3_fun);
     

    lua_tie.do_file("lua/lua_test_01.lua");

    int ret_a = 0, ret_b = 0, ret_c = 0;
    int var_a = 100, var_b = 200, var_c = 300, var_d = 400;
    lua_tie.call_luafun_3("lua_func", ret_a, ret_b, ret_c, var_a, var_b, var_c, var_d);

    printf("ret a =%d ret b=%d ret c=%d\n", ret_a, ret_b, ret_c);

    lua_tie.close();

    return 0;
}


int g_a = 1111;
int g_b = 2222;


int g_array[20];

//测试全局变量在两边的使用
int test_lua_script2(int ,char *[])
{

    for (size_t k = 0; k < 20; ++k)
    {
        g_array[k] = static_cast<int>(k + 1);
    }

    ZCE_Lua_Tie lua_tie;
    lua_tie.open(true, true);

    //绑定引用和指针之前，要注册这个类

    lua_tie.set_gvar("g_b_var", g_b);
    //绑定指针
    int *g_b_ptr1 = &g_b;
    lua_tie.set_gvar("g_b_ptr", g_b_ptr1);
    //重新得到指针，
    int *g_b_ptr2 = lua_tie.get_gvar<int *>("g_b_ptr");


    printf("g_b_ptr1 = %p g_b_ptr2=%p", g_b_ptr1, g_b_ptr2);

    int &ref_gb = g_b;
    lua_tie.set_gvar<int &>("g_a_ref", ref_gb);


    lua_tie.set_garray("g_array", g_array,20);


    lua_tie.do_file("lua/lua_test_02.lua");

    printf("g_b = %d\n",g_b);



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




struct TB :public TA
{
    TB(int b1, int b2, int b3) :TA(0),b1_(b1), b2_(b2), b3_(b3)
    {
    }

    int set_b1(int b1)
    {
        b1_ = b1;
        return b1_;
    }

    int set_b2(int b2_1,int b2_2)
    {
        b2_ = b2_1 + b2_2;
        return b2_;
    }

    int b1_;
    int b2_;
    int b3_;

    double b_array_[120];
};

int test_lua_script3(int, char *[])
{
    ZCE_Lua_Tie lua_tie;
    lua_tie.open(true, true);
    lua_tie.reg_class<TA>("TA",false);
    lua_tie.class_mem_var<TA>("a_", &TA::a_);
    lua_tie.class_constructor<TA>(ZCE_LUA::constructor<TA,int> );

    TA ta_val(100);
    TA *ta_ptr = new TA(200);
    TA ta_1(300);
    TA &ta_ref = ta_1;

    lua_tie.set_gvar("ta_val",ta_val);
    lua_tie.set_gvar("ta_ptr", ta_ptr);
    lua_tie.set_gvar<TA &>("ta_ref", ta_ref);


    lua_tie.reg_class<TB>("TB", false)
        .construct(ZCE_LUA::constructor<TB, int, int, int>)
        .mem_var("b1_", &TB::b1_)
        .mem_var("b2_", &TB::b2_)
        .mem_var("b3_", &TB::b3_)
        .mem_ary<double,120>("b_array_", &TB::b_array_)
        .mem_fun("set_b2", &TB::set_b2);

    lua_tie.close();
    return 0;
}


int test_lua_script4(int, char *[])
{
    ZCE_Lua_Tie lua_tie;
    lua_tie.open(true, true);
    int array_a[100];
    

    lua_tie.to_luatable("array_a", array_a, array_a + 100);


    std::map<int, double> map_a;
    map_a[9] = 1.0;
    map_a[10] = 2.1;
    map_a[11] = 3.1;
    lua_tie.to_luatable("map_a", map_a.begin(), map_a.end());

    lua_tie.close();

    return 0;
}