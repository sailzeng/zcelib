
#include "zealot_predefine.h"
#include "zealot_test_function.h"


//
int add2_fun(int a, int b)
{
    return a + b;
}


int add3_fun(int a, int b, int c)
{
    return a + b + c;
}


//测试两边函数的互相调用
int test_lua_script1(int, char *[])
{
    ZCE_Lua_Tie lua_tie;
    lua_tie.open(true, true);



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
int test_lua_script2(int , char *[])
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


    lua_tie.set_garray("g_array", g_array, 20);


    lua_tie.do_file("lua/lua_test_02.lua");

    printf("g_b = %d\n", g_b);

    for (size_t k = 0; k < 20; ++k)
    {
        printf("g_array[%d]=%d\n", k, g_array[k]);
    }

    lua_tie.close();

    return 0;
}

struct TA
{
    TA(int a) : a_(a)
    {
    }
    int set_a(int a)
    {
        a_ = a;
        return a_;
    }
    int a_;
};




struct TB : public TA
{
    TB(int b1, int b2, int b3) : TA(0), b1_(b1), b2_(b2), b3_(b3)
    {
    }

    int set_b1(int b1)
    {
        b1_ = b1;
        return b1_;
    }

    int set_b2(int b2_1, int b2_2)
    {
        b2_ = b2_1 + b2_2;
        return b2_;
    }

    int set_b3(int b3_1, int b3_2, int b3_3)
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
    lua_tie.reg_class<TA>("TA", false);
    lua_tie.class_mem_var<TA>("a_", &TA::a_);
    lua_tie.class_mem_fun("set_a", &TA::set_a);
    lua_tie.class_constructor<TA>(ZCE_LUA::constructor<TA, int> );

    TA ta_val(100);
    TA *ta_ptr = new TA(200);
    TA ta_1(300);
    TA &ta_ref = ta_1;

    lua_tie.set_gvar("ta_val", ta_val);
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


enum TO_LUA_ENUM
{
    ENUM_0001,
    ENUM_0002,
    ENUM_0003,
    ENUM_0004,
};
//展现table的几种使用方式
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


    std::map<double, std::string> map_a;
    map_a[10.2] = std::string("Wuhan");
    map_a[20.3] = std::string("ZhongGuo");
    map_a[30.4] = std::string("Miaomiao");
    lua_tie.to_luatable("map_a", map_a.begin(), map_a.end());

    std::map<const char *, std::string> map_b;
    const char *KEY_1 = "B_KEY_1";
    const char *KEY_2 = "B_KEY_2";
    const char *KEY_3 = "B_KEY_3";

    map_b[KEY_1] = std::string("GuangDong");
    map_b[KEY_2] = std::string("Shenzhen");
    map_b[KEY_3] = std::string("LuaTie");

    lua_tie.to_luatable("map_b", map_b.begin(), map_b.end());

    //轻轻松松完成吧枚举值注册给lua的任务
    lua_tie.new_table("tolua_enum",
        std::make_pair("ENUM_0001", ENUM_0001),
        std::make_pair("ENUM_0002", ENUM_0002),
        std::make_pair("ENUM_0003", ENUM_0003),
        std::make_pair("ENUM_0004", ENUM_0004)
        );


    lua_tie.do_file("lua/lua_test_05.lua");

    printf("%s\n", "-------------------------- ");
    int read_ary_a[10];
    lua_tie.from_luatable("table_array", read_ary_a);
    for (size_t i = 0; i < 10; ++i)
    {
        printf("read_ary_a[%d]=%d\n", (int)i, read_ary_a[i]);
    }
    printf("%s\n", "-------------------------- ");
    std::map<int, double> read_map_a;
    lua_tie.from_luatable("table_map", read_map_a);

    auto iter = read_map_a.begin();
    for (; iter != read_map_a.end(); iter++)
    {
        printf("read_map_a[%d]=%f\n",
               iter->first,
               iter->second);
    }

    lua_tie.close();

    return 0;
}



void thread_func()
{
    printf("# thread_func is invoke.\n");
    return;
}

double thread_func2(double a, double b)
{
    printf("# thread_func2(%f,%f) is invoke. \n", a,b);
    return a+b;
}

class Test_Thread_Class
{
public:

    //
    void thread_men_fun()
    {
        printf("# Test_Thread_Class::thread_men_fun is invoke.\n");
        return;
    }

    double thread_men_fun2(double a,double b)
    {
        printf("# Test_Thread_Class::thread_men_fun2(%f,%f) is invoke.\n",  a, b);
        return a+b;
    }

};

//这个测试是和tinker类似的，但仔细你会发现我的实现比tinker高明了很多！！！！
//我不需要你的函数写成特定的方式。

//测试Lua Thread 协程
int test_lua_script6(int, char *[])
{
    ZCE_Lua_Tie lua_tie;
    lua_tie.open(true, true);

    //请注意这个地方，注册函数用的是reg_yeild_gfun，这样thread_func执行
    //完毕会，调用lua_yield
    lua_tie.reg_yeild_gfun("thread_func", &thread_func);
    lua_tie.reg_yeild_gfun("thread_func2", &thread_func2);
    

    lua_tie.reg_class<Test_Thread_Class>("TestClass").
        mem_yield_fun("thread_men_fun", &Test_Thread_Class::thread_men_fun).
        mem_yield_fun("thread_men_fun2", &Test_Thread_Class::thread_men_fun2);
   

    ZCE_Lua_Thread thread_hdl;
    int ret = lua_tie.new_thread(&thread_hdl);
    if (ret != 0)
    {
        return ret;
    }

    Test_Thread_Class g_test;
    lua_tie.set_gvar("g_test", &g_test);

    thread_hdl.do_file("lua/lua_test_06.lua");


    ret = thread_hdl.get_luaobj("ThreadTest", LUA_TFUNCTION);
    if (ret != 0)
    {
        return ret;
    }
    printf("* lua_resume() to.. ThreadTest start. =============================\n");
    thread_hdl.resume(0);


    printf("* lua_resume() to.. \n");
    thread_hdl.resume(0);
    printf("* pop ret %f\n", thread_hdl.pop<double>());

    printf("* lua_resume() to.. \n");
    thread_hdl.resume(0);
    
    printf("* lua_resume() to.. \n");
    thread_hdl.resume(0);
    printf("* pop ret %f\n", thread_hdl.pop<double>());

    printf("* lua_resume() to.. \n");
    thread_hdl.resume(0);

    printf("*  ThreadTest end. =================================\n");



    ret = thread_hdl.get_luaobj("ThreadTest2", LUA_TFUNCTION);
    if (ret != 0)
    {
        return ret;
    }
    printf("* lua_resume() to.. ThreadTest2 start. ==============================\n");
    thread_hdl.resume(0);
    printf("* lua_resume() to.. \n");
    printf("* pop %d\n", thread_hdl.pop<int>());
    thread_hdl.resume(0);
    printf("* lua_resume() to.. \n");
    printf("* pop %d\n", thread_hdl.pop<int>());
    thread_hdl.resume(0);

    printf("* lua_resume() to.. \n");
    printf("* pop %d\n", thread_hdl.pop<int>());
    thread_hdl.resume(0);
    printf("* lua_resume() to.. \n");
    printf("* pop %d\n", thread_hdl.pop<int>());
    thread_hdl.resume(0);

    printf("*  ThreadTest2 end. =================================\n");

    lua_tie.close();

    return 0;
}

