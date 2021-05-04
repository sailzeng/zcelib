#include "zealot_predefine.h"
#include "zealot_test_function.h"

namespace  tie = zce::luatie;

char test_lua_file[PATH_MAX] = "../../test.lua/";

//
int add2_fun(int a,int b)
{
    return a + b;
}

int add3_fun(int a,int b,int c)
{
    std::cout << "para 1 a=" << a << std::endl;
    std::cout << "para 2 b=" << b << std::endl;
    std::cout << "para 3 c=" << c << std::endl;
    return a + b + c;
}

//测试两边函数的互相调用
int test_lua_script1(int,char *[])
{
    ZCE_Lua_Tie lua_tie;
    lua_tie.open(true,true);

    lua_tie.reg_gfunc("add2_fun",add2_fun);

    lua_tie.reg_gfunc("add3_fun",add3_fun);

    lua_tie.do_file(strcat(test_lua_file,"lua_test_01.lua"));

    int ret_a = 0,ret_b = 0,ret_c = 0;
    int var_a = 100,var_b = 200,var_c = 300,var_d = 400;
    lua_tie.call_luafun_3("lua_func",ret_a,ret_b,ret_c,var_a,var_b,var_c,var_d);

    printf("ret a =%d ret b=%d ret c=%d\n",ret_a,ret_b,ret_c);

    lua_tie.close();

    return 0;
}

int g_a = 1111;
int g_b = 2222;

int g_array[20];

//测试全局变量在两边的使用
int test_lua_script2(int,char *[])
{
    for (size_t k = 0; k < 20; ++k)
    {
        g_array[k] = static_cast<int>(k + 1);
    }

    ZCE_Lua_Tie lua_tie;
    lua_tie.open(true,true);

    //绑定引用和指针之前，要注册这个类

    lua_tie.set_gvar("g_b_var",g_b);
    //绑定指针
    int *g_b_ptr1 = &g_b;
    lua_tie.set_gvar("g_b_ptr",g_b_ptr1);
    //重新得到指针，
    int *g_b_ptr2 = lua_tie.get_gvar<int *>("g_b_ptr");

    printf("g_b_ptr1 = %p g_b_ptr2=%p",g_b_ptr1,g_b_ptr2);

    int &ref_gb = g_b;
    lua_tie.set_gvar<int &>("g_a_ref",ref_gb);

    lua_tie.set_garray("g_array",g_array,20);

    lua_tie.do_file(strcat(test_lua_file,"lua_test_02.lua"));

    printf("g_b = %d\n",g_b);

    for (size_t k = 0; k < 20; ++k)
    {
        printf("g_array[%d]=%d\n",(int)k,g_array[k]);
    }

    lua_tie.close();

    return 0;
}

class T3A
{
public:
    T3A(int a): a_(a)
    {
    }
    int set_a(int a)
    {
        a_ = a;
        return a_;
    }
    int a_;
};

class T3B: public T3A
{
public:
    T3B(int b1,int b2,int b3): T3A(0),b1_(b1),b2_(b2),b3_(b3)
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

    int set_b3(int b3_1,int b3_2,int b3_3)
    {
        b3_ = b3_1 + b3_2 + b3_3;
        return b3_;
    }

    int b1_;
    int b2_;
    int b3_;

    double b_array_[120];
};

class T3C
{
public:
    T3C(int c1,const std::string &c2,double c3):
        c1_(c1),
        c2_(c2),
        c3_(c3)
    {
    }

    int set_c1(int c1_1,int c1_2)
    {
        c1_ = c1_1 + c1_2;
        return c1_;
    }

    std::string set_c2(int c2_1,double c2_2,const std::string &c2_3)
    {
        c2_ = std::to_string(c2_1) + std::to_string(c2_2) + (c2_3);
        return c2_;
    }

    double set_c3(double c3_1,double c3_2,double c3_3,int c3_4,const std::string &c3_5)
    {
        c3_ = c3_1 + c3_2 - c3_3 + c3_4 + std::stof(c3_5);
        return c3_;
    }

    int c1_;
    std::string c2_;
    double c3_;
};

int test_lua_script3(int,char *[])
{
    ZCE_Lua_Tie lua_tie;
    lua_tie.open(true,true);
    lua_tie.reg_class<T3A>("T3A",false);
    lua_tie.class_mem_var<T3A>("a_",&T3A::a_);
    lua_tie.class_memfunc("set_a",&T3A::set_a);
    lua_tie.class_constructor<T3A>(tie::constructor<T3A,int>::invoke);

    T3A ta_val(100);
    T3A *ta_ptr = new T3A(200);
    T3A ta_1(300);
    T3A &ta_ref = ta_1;

    lua_tie.set_gvar("ta_val",ta_val);
    lua_tie.set_gvar("ta_ptr",ta_ptr);
    lua_tie.set_gvar<T3A &>("ta_ref",ta_ref);

    //注册T3B
    lua_tie.reg_class<T3B>("T3B",false)
        .construct(tie::constructor<T3B,int,int,int>::invoke)
        .inherit<T3A>()
        .mem_var("b1_",&T3B::b1_)
        .mem_var("b2_",&T3B::b2_)
        .mem_var("b3_",&T3B::b3_)
        .mem_ary<double,120>("b_array_",&T3B::b_array_)
        .mem_fun("set_b1",&T3B::set_b1)
        .mem_fun("set_b2",&T3B::set_b2)
        .mem_fun("set_b3",&T3B::set_b3);

    T3B tb_val(100,200,300);
    T3B *tb_ptr_1 = new T3B(1000,2000,3000);
    T3B *tb_ptr_2 = new T3B(1000,2000,3000);
    T3B tb_1(10000,20000,30000);
    T3B &tb_ref = tb_1;

    lua_tie.set_gvar("tb_val",tb_val);
    lua_tie.set_gvar("tb_ptr_1",tb_ptr_1);
    lua_tie.set_gvar("tb_ptr_2",tb_ptr_2);
    lua_tie.set_gvar<T3B &>("tb_ref",tb_ref);

    //注册T3C
    lua_tie.reg_class<T3C>("T3C",false)
        .construct(tie::constructor<T3C,int,std::string,double>::invoke)
        .mem_var("c1_",&T3C::c1_)
        .mem_var("c2_",&T3C::c2_)
        .mem_var("c3_",&T3C::c3_)
        .mem_fun("set_c1",&T3C::set_c1)
        .mem_fun("set_c2",&T3C::set_c2)
        .mem_fun("set_c3",&T3C::set_c3);

    lua_tie.do_file(strcat(test_lua_file,"lua_test_03.lua"));

    printf("---------------------------------------------------\n");
    printf("ta_ptr->a_ = %d\n",ta_ptr->a_);
    printf("ta_ref.a_ = %d\n",ta_ref.a_);
    printf("ta_val.a_ = %d\n",ta_val.a_);
    printf("---------------------------------------------------\n");

    printf("tb_ptr_1->a_= %d\n",tb_ptr_1->a_);
    printf("tb_ptr_1->a_= %d\n",tb_ptr_2->a_);
    printf("---------------------------------------------------\n");

    printf("tb_ptr_1->b1_ = %d\n",tb_ptr_1->b1_);
    printf("tb_ptr_1->b2_ = %d\n",tb_ptr_1->b2_);
    printf("tb_ptr_1->b3_ = %d\n",tb_ptr_1->b3_);
    printf("---------------------------------------------------\n");
    for (size_t k = 0; k < 20; ++k)
    {
        printf("tb_ptr_2->b_array_[%d]=%f\n",(int)k,tb_ptr_2->b_array_[k]);
    }
    printf("---------------------------------------------------\n");

    lua_tie.close();

    delete ta_ptr;
    delete tb_ptr_1;
    delete tb_ptr_2;

    return 0;
}

int test_lua_script4(int,char *[])
{
    ZCE_Lua_Tie lua_tie;
    lua_tie.open(true,true);

    //
    printf("%s\n","-------------------------- current stack");
    lua_tie.enum_stack();

    //
    printf("%s\n","-------------------------- stack after push '1'");
    lua_tie.push(1);
    lua_tie.push(2);
    lua_tie.push("333333");
    lua_tie.enum_stack();

    // sample5.lua 颇老阑 肺靛/角青茄促.
    lua_tie.do_file(strcat(test_lua_file,"lua_test_04.lua"));

    // test_error()
    // test_error() 会调用到 test_error_3()
    printf("%s\n","-------------------------- calling test_error()");
    int abc = 0;
    lua_tie.call_luafun_0("test_error",abc);

    // test_error_3()
    printf("%s\n","-------------------------- calling test_error_3()");
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
int test_lua_script5(int,char *[])
{
    ZCE_Lua_Tie lua_tie;
    lua_tie.open(true,true);
    int array_a[100];
    for (size_t i = 0; i < 100; ++i)
    {
        array_a[i] = static_cast<int>(9009000 + i);
    }

    lua_tie.to_luatable("array_a",array_a,array_a + 100);

    std::map<double,std::string> map_a;
    map_a[10.2] = std::string("Wuhan");
    map_a[20.3] = std::string("ZhongGuo");
    map_a[30.4] = std::string("Miaomiao");
    lua_tie.to_luatable("map_a",map_a.begin(),map_a.end());

    /*std::list<int> list_a;
    list_a.push_back(1);
    list_a.push_back(2);
    list_a.push_back(3);
    lua_tie.to_luatable("list_a", list_a.begin(), list_a.end());*/

    std::map<const char *,std::string> map_b;
    const char *KEY_1 = "B_KEY_1";
    const char *KEY_2 = "B_KEY_2";
    const char *KEY_3 = "B_KEY_3";

    map_b[KEY_1] = std::string("GuangDong");
    map_b[KEY_2] = std::string("Shenzhen");
    map_b[KEY_3] = std::string("LuaTie");

    lua_tie.to_luatable("map_b",map_b.begin(),map_b.end());

    //轻轻松松完成吧枚举值注册给lua的任务
    lua_tie.new_table("tolua_enum",
                      std::make_pair("ENUM_0001",ENUM_0001),
                      std::make_pair("ENUM_0002",ENUM_0002),
                      std::make_pair("ENUM_0003",ENUM_0003),
                      std::make_pair("ENUM_0004",ENUM_0004)
    );

    lua_tie.do_file(strcat(test_lua_file,"lua_test_05.lua"));

    printf("%s\n","-------------------------- ");
    int read_ary_a[10];
    lua_tie.from_luatable("table_array",read_ary_a);
    for (size_t i = 0; i < 10; ++i)
    {
        printf("read_ary_a[%d]=%d\n",(int)i,read_ary_a[i]);
    }
    printf("%s\n","-------------------------- ");
    std::map<int,double> read_map_a;
    lua_tie.from_luatable("table_map",read_map_a);

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

double thread_func2(double a,double b)
{
    printf("# thread_func2(%f,%f) is invoke. \n",a,b);
    return a + b;
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
        printf("# Test_Thread_Class::thread_men_fun2(%f,%f) is invoke.\n",a,b);
        return a + b;
    }
};

//这个测试是和tinker类似的，但仔细你会发现我的实现比tinker高明了很多！！！！
//我不需要你的函数写成特定的方式。

//测试Lua Thread 协程
int test_lua_script6(int,char *[])
{
    ZCE_Lua_Tie lua_tie;
    lua_tie.open(true,true);

    //请注意这个地方，注册函数用的是reg_yeild_gfun，这样thread_func执行
    //完毕会，调用lua_yield
    lua_tie.reg_yeild_gfunc("thread_func",&thread_func);
    lua_tie.reg_yeild_gfunc("thread_func2",&thread_func2);

    lua_tie.reg_class<Test_Thread_Class>("TestClass").
        mem_yield_fun("thread_men_fun",&Test_Thread_Class::thread_men_fun).
        mem_yield_fun("thread_men_fun2",&Test_Thread_Class::thread_men_fun2);

    ZCE_Lua_Thread thread_hdl;
    int ret = lua_tie.new_thread(&thread_hdl);
    if (ret != 0)
    {
        return ret;
    }

    Test_Thread_Class g_test;
    lua_tie.set_gvar("g_test",&g_test);

    thread_hdl.do_file(strcat(test_lua_file,"lua_test_06.lua"));

    ret = thread_hdl.get_luaobj("ThreadTest",LUA_TFUNCTION);
    if (ret != 0)
    {
        return ret;
    }
    printf("* lua_resume() to.. ThreadTest start. =============================\n");
    thread_hdl.resume(0);

    printf("* lua_resume() to.. \n");
    thread_hdl.resume(0);
    printf("* pop ret %f\n",thread_hdl.pop<double>());

    printf("* lua_resume() to.. \n");
    thread_hdl.resume(0);

    printf("* lua_resume() to.. \n");
    thread_hdl.resume(0);
    printf("* pop ret %f\n",thread_hdl.pop<double>());

    printf("* lua_resume() to.. \n");
    thread_hdl.resume(0);

    printf("*  ThreadTest end. =================================\n");

    ret = thread_hdl.get_luaobj("ThreadTest2",LUA_TFUNCTION);
    if (ret != 0)
    {
        return ret;
    }
    printf("* lua_resume() to.. ThreadTest2 start. ==============================\n");
    thread_hdl.resume(0);
    printf("* lua_resume() to.. \n");
    printf("* pop %d\n",thread_hdl.pop<int>());
    thread_hdl.resume(0);
    printf("* lua_resume() to.. \n");
    printf("* pop %d\n",thread_hdl.pop<int>());
    thread_hdl.resume(0);

    printf("* lua_resume() to.. \n");
    printf("* pop %d\n",thread_hdl.pop<int>());
    thread_hdl.resume(0);
    printf("* lua_resume() to.. \n");
    printf("* pop %d\n",thread_hdl.pop<int>());
    thread_hdl.resume(0);

    printf("*  ThreadTest2 end. =================================\n");

    lua_tie.close();

    return 0;
}

//表述不同的集成方式，以及测试性能数据，

struct Woo_Struct
{
    int a_;
    int b_;
    int c_;
    int d_;
    int e_;
    int f_;
};

Woo_Struct woo_result;

void set_woo(int a,int b,int c,int d,int e,int f)
{
    woo_result.a_ = a;
    woo_result.b_ = b;
    woo_result.c_ = c;
    woo_result.d_ = d;
    woo_result.e_ = e;
    woo_result.f_ = f;
}

const size_t LUA_TEST_COUNT = 10240;

int test_lua_script7(int,char *[])
{
    ZCE_Lua_Tie lua_tie;
    lua_tie.open(true,true);

    Woo_Struct obj_x,obj_y;

    obj_x.a_ = 1;
    obj_x.b_ = 2;
    obj_x.c_ = 3;
    obj_x.d_ = 4;
    obj_x.e_ = 5;
    obj_x.f_ = 6;

    obj_y.a_ = 10;
    obj_y.b_ = 20;
    obj_y.c_ = 30;
    obj_y.d_ = 40;
    obj_y.e_ = 50;
    obj_y.f_ = 60;

    lua_tie.reg_gfunc("set_result",&set_woo);

    lua_tie.do_file(strcat(test_lua_file,"lua_test_07.lua"));

    lua_tie.call_luafun_0("read_objx",
                          obj_x.a_,
                          obj_x.b_,
                          obj_x.c_,
                          obj_x.d_,
                          obj_x.e_,
                          obj_x.f_);

    lua_tie.call_luafun_0("read_objy",
                          obj_y.a_,
                          obj_y.b_,
                          obj_y.c_,
                          obj_y.d_,
                          obj_y.e_,
                          obj_y.f_);

    lua_tie.call_luafun_0("obj_add");
    printf("result a=%d b=%d c=%d d=%d e=%d f=%d \n",
           woo_result.a_,
           woo_result.b_,
           woo_result.c_,
           woo_result.d_,
           woo_result.e_,
           woo_result.f_);

    //做一下性能测试。
    const uint32_t TEST_SEED = 120825;
    zce::random_mt11213b  mt11231b_gen(TEST_SEED);

    ZCE_Progress_Timer timer;
    timer.restart();
    for (size_t i = 0; i < LUA_TEST_COUNT; ++i)
    {
        obj_x.a_ = mt11231b_gen.get_uint32();
        obj_x.b_ = mt11231b_gen.get_uint32();
        obj_x.c_ = mt11231b_gen.get_uint32();
        obj_x.d_ = mt11231b_gen.get_uint32();
        obj_x.e_ = mt11231b_gen.get_uint32();
        obj_x.f_ = mt11231b_gen.get_uint32();

        obj_y.a_ = mt11231b_gen.get_uint32();
        obj_y.b_ = mt11231b_gen.get_uint32();
        obj_y.c_ = mt11231b_gen.get_uint32();
        obj_y.d_ = mt11231b_gen.get_uint32();
        obj_y.e_ = mt11231b_gen.get_uint32();
        obj_y.f_ = mt11231b_gen.get_uint32();

        lua_tie.call_luafun_0("read_objx",
                              obj_x.a_,
                              obj_x.b_,
                              obj_x.c_,
                              obj_x.d_,
                              obj_x.e_,
                              obj_x.f_);

        lua_tie.call_luafun_0("read_objy",
                              obj_y.a_,
                              obj_y.b_,
                              obj_y.c_,
                              obj_y.d_,
                              obj_y.e_,
                              obj_y.f_);

        lua_tie.call_luafun_0("obj_add");
    }

    timer.end();
    std::cout << " elapsed: " << std::setprecision(6) << timer.elapsed_sec() << std::endl;

    lua_tie.close();

    return 0;
}

int test_lua_script8(int,char *[])
{
    Woo_Struct obj_x,obj_y,obj_result;

    ZCE_Lua_Tie lua_tie;
    lua_tie.open(true,true);
    lua_tie.reg_class<Woo_Struct>("Woo_Struct").
        mem_var("a_",&Woo_Struct::a_).
        mem_var("b_",&Woo_Struct::b_).
        mem_var("c_",&Woo_Struct::c_).
        mem_var("d_",&Woo_Struct::d_).
        mem_var("e_",&Woo_Struct::e_).
        mem_var("f_",&Woo_Struct::f_);

    obj_x.a_ = 1;
    obj_x.b_ = 2;
    obj_x.c_ = 3;
    obj_x.d_ = 4;
    obj_x.e_ = 5;
    obj_x.f_ = 6;

    obj_y.a_ = 10;
    obj_y.b_ = 20;
    obj_y.c_ = 30;
    obj_y.d_ = 40;
    obj_y.e_ = 50;
    obj_y.f_ = 60;

    lua_tie.set_gvar("obj_x",&obj_x);
    lua_tie.set_gvar("obj_y",&obj_y);
    lua_tie.set_gvar("obj_result",&obj_result);

    lua_tie.do_file(strcat(test_lua_file,"lua_test_08.lua"));
    lua_tie.call_luafun_0("obj_add");
    printf("result a=%d b=%d c=%d d=%d e=%d f=%d \n",
           obj_result.a_,
           obj_result.b_,
           obj_result.c_,
           obj_result.d_,
           obj_result.e_,
           obj_result.f_);

    //做一下性能测试。
    const uint32_t TEST_SEED = 120825;
    zce::random_mt11213b  mt11231b_gen(TEST_SEED);

    ZCE_Progress_Timer timer;

    //使用指针对应数据，速度比完全用Lua 栈交互快，但这个测试，数据没有跟换过，
    //一直使用obj_x，obj_y，obj_result
    timer.restart();
    for (size_t i = 0; i < LUA_TEST_COUNT; ++i)
    {
        obj_x.a_ = mt11231b_gen.get_uint32();
        obj_x.b_ = mt11231b_gen.get_uint32();
        obj_x.c_ = mt11231b_gen.get_uint32();
        obj_x.d_ = mt11231b_gen.get_uint32();
        obj_x.e_ = mt11231b_gen.get_uint32();
        obj_x.f_ = mt11231b_gen.get_uint32();

        obj_y.a_ = mt11231b_gen.get_uint32();
        obj_y.b_ = mt11231b_gen.get_uint32();
        obj_y.c_ = mt11231b_gen.get_uint32();
        obj_y.d_ = mt11231b_gen.get_uint32();
        obj_y.e_ = mt11231b_gen.get_uint32();
        obj_y.f_ = mt11231b_gen.get_uint32();

        lua_tie.call_luafun_0("obj_add");
    }
    timer.end();
    std::cout << " elapsed  01: " << std::setprecision(6) << timer.elapsed_sec() << std::endl;

    //如果每次使用的是，都跟换数据，你会发现性能会下降很多，其实也容易理解，绑定是有new操作的
    timer.restart();
    for (size_t i = 0; i < LUA_TEST_COUNT; ++i)
    {
        obj_x.a_ = mt11231b_gen.get_uint32();
        obj_x.b_ = mt11231b_gen.get_uint32();
        obj_x.c_ = mt11231b_gen.get_uint32();
        obj_x.d_ = mt11231b_gen.get_uint32();
        obj_x.e_ = mt11231b_gen.get_uint32();
        obj_x.f_ = mt11231b_gen.get_uint32();

        obj_y.a_ = mt11231b_gen.get_uint32();
        obj_y.b_ = mt11231b_gen.get_uint32();
        obj_y.c_ = mt11231b_gen.get_uint32();
        obj_y.d_ = mt11231b_gen.get_uint32();
        obj_y.e_ = mt11231b_gen.get_uint32();
        obj_y.f_ = mt11231b_gen.get_uint32();

        //每次都重新绑定，
        lua_tie.set_gvar("obj_x",&obj_x);
        lua_tie.set_gvar("obj_y",&obj_y);
        lua_tie.set_gvar("obj_result",&obj_result);

        lua_tie.call_luafun_0("obj_add");
    }
    timer.end();
    std::cout << " elapsed  02: " << std::setprecision(6) << timer.elapsed_sec() << std::endl;

    lua_tie.close();

    return 0;
}

//按照Lua Tinker 的例子3复刻的一个例子，用于测试OO部分
struct T9A
{
    T9A(int v):
        t9a_val_(v)
    {
    }
    int t9a_val_;
};

struct T9B
{
    int t9b_val_;
};

struct T9C_base
{
    T9C_base() {}

    const char *is_base()
    {
        return "this is base";
    }
};

class T9C: public T9C_base
{
public:
    T9C(int val):
        T9C_base(),
        t9c_v_(val)
    {
    }

    ~T9C()
    {
    }

    const char *is_t9c()
    {
        return "this is test";
    }

    void ret_void()
    {
    }

    int ret_int()
    {
        return t9c_v_;
    }
    int ret_mul(int m) const
    {
        return t9c_v_ * m;
    }
    T9A get()
    {
        return T9A(t9c_v_);
    }
    void set(T9A a)
    {
        t9c_v_ = a.t9a_val_;
    }

    int t9c_v_;

    T9B t9c_b_;
};

T9C g_t9c(888999);

int test_lua_script9(int,char *[])
{
    ZCE_Lua_Tie lua_tie;
    lua_tie.open(true,true);
    //注册T9B
    lua_tie.reg_class<T9B>("T9B").
        construct(tie::constructor<T9B>::invoke).
        mem_var("t9b_val_",&T9B::t9b_val_);

    lua_tie.reg_class<T9C_base>("T9C_base").
        mem_fun("is_base",&T9C_base::is_base);

    lua_tie.reg_class<T9C>("T9C").
        construct(tie::constructor<T9C,int >::invoke).
        inherit<T9C_base>().
        mem_fun("is_t9c",&T9C::is_t9c).
        mem_fun("ret_int",&T9C::ret_int).
        mem_fun("get",&T9C::get).
        mem_fun("set",&T9C::set).
        mem_var("t9c_v_",&T9C::t9c_v_).
        mem_var("t9c_b_",&T9C::t9c_b_);

    lua_tie.set_gvar("g_t9c",&g_t9c);

    lua_tie.do_file(strcat(test_lua_file,"lua_test_09.lua"));

    lua_tie.close();
    return 0;
}