
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

    for (size_t k = 0; k < 20; ++k)
    {
        printf("g_array[%d]=%d\n", k, g_array[k]);
    }

    lua_tie.close();

    return 0;
}

struct TA
{
    TA(int a) :a_(a)
    {
    }
    int set_a(int a)
    {
        a_ = a;
        return a_;
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

    int set_b3(int b3_1, int b3_2,int b3_3)
    {
        b3_ = b3_1 + b3_2 + b3_3;
        return b3_;
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
    lua_tie.class_mem_fun("set_a", &TA::set_a);
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
        .inherit<TA>()
        .mem_var("b1_", &TB::b1_)
        .mem_var("b2_", &TB::b2_)
        .mem_var("b3_", &TB::b3_)
        .mem_ary<double, 120>("b_array_", &TB::b_array_)
        .mem_fun("set_b1", &TB::set_b1)
        .mem_fun("set_b2", &TB::set_b2)
        .mem_fun("set_b3", &TB::set_b3);

    TB tb_val(100, 200, 300);
    TB *tb_ptr_1 = new TB(1000, 2000, 3000);
    TB *tb_ptr_2 = new TB(1000, 2000, 3000);
    TB tb_1(10000, 20000, 30000);
    TB &tb_ref = tb_1;


    lua_tie.set_gvar("tb_val", tb_val);
    lua_tie.set_gvar("tb_ptr_1", tb_ptr_1);
    lua_tie.set_gvar("tb_ptr_2", tb_ptr_2);
    lua_tie.set_gvar<TB &>("tb_ref", tb_ref);


    lua_tie.do_file("lua/lua_test_03.lua");

    printf("---------------------------------------------------\n");
    printf("ta_ptr->a_ = %d\n", ta_ptr->a_);
    printf("ta_ref.a_ = %d\n", ta_ref.a_);
    printf("ta_val.a_ = %d\n", ta_val.a_);
    printf("---------------------------------------------------\n");

    printf("tb_ptr_1->a_= %d\n", tb_ptr_1->a_);
    printf("tb_ptr_1->a_= %d\n", tb_ptr_2->a_);
    printf("---------------------------------------------------\n");

    printf("tb_ptr_1->b1_ = %d\n", tb_ptr_1->b1_);
    printf("tb_ptr_1->b2_ = %d\n", tb_ptr_1->b2_);
    printf("tb_ptr_1->b3_ = %d\n", tb_ptr_1->b3_);
    printf("---------------------------------------------------\n");
    for (size_t k = 0; k < 20; ++k)
    {
        printf("tb_ptr_2->b_array_[%d]=%f\n", k, tb_ptr_2->b_array_[k]);
    }
    printf("---------------------------------------------------\n");

    lua_tie.close();

    delete ta_ptr;
    delete tb_ptr_1;
    delete tb_ptr_2;

    return 0;
}


int test_lua_script4(int, char *[])
{
    ZCE_Lua_Tie lua_tie;
    lua_tie.open(true, true);


    //
    printf("%s\n", "-------------------------- current stack");
    lua_tie.enum_stack();

    // 泅犁 胶琶狼 郴侩阑 促矫 免仿茄促.
    printf("%s\n", "-------------------------- stack after push '1'");
    lua_tie.push(1);
    lua_tie.push(2);
    lua_tie.push("333333");
    lua_tie.enum_stack();

    // sample5.lua 颇老阑 肺靛/角青茄促.
    lua_tie.do_file("sample5.lua");

    // test_error() 
    // test_error() 会调用到 test_error_3() 
    printf("%s\n", "-------------------------- calling test_error()");
    int abc = 0;
    lua_tie.call_luafun_0("test_error", abc);

    // test_error_3()
    printf("%s\n", "-------------------------- calling test_error_3()");
    lua_tie.call_luafun_0("test_error_3");

    lua_tie.close();

    return 0;
}


int test_lua_script5(int, char *[])
{
    ZCE_Lua_Tie lua_tie;
    lua_tie.open(true, true);
    int array_a[100];
    for (size_t i = 0; i < 100; ++i)
    {
        array_a[i] = static_cast<int>(9009000 + i);
    }

    lua_tie.to_luatable("array_a", array_a, array_a + 100);


    std::map<std::string, double> map_a;
    map_a[std::string("Wuhan")] = 1.0;
    map_a[std::string("ZhongGuo")] = 2.1;
    map_a[std::string("Miaomiao")] = 3.1;
    lua_tie.to_luatable("map_a", map_a.begin(), map_a.end());

    lua_tie.do_file("lua/lua_test_05.lua");

    lua_tie.close();

    return 0;
}


