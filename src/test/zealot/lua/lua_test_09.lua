-- Lua
print("Hello world")

-- const char* T9C::is_t9c() 
print(g_t9c:is_t9c())

print(g_t9c.t9c_v_)
g_t9c.t9c_v_ = 12
g_t9c.t9c_v_ = 15

temp_9b=T9B()

print(g_t9c.t9c_v_)

g_t9c.t9c_v_ = 12
g_t9c.t9c_v_ = 15

-- 
print(g_t9c.t9c_v_)



-- T9C::ret_int()
print(g_t9c:ret_int())

-- 
temp_9c = T9C(1024)

-- 
print(temp_9c.t9c_v_)

-- Lua ，注意a 是TA，TA其实没有注册，
a = g_t9c:get()

-- 
temp_9c:set(a)

-- test::set(A a) 
print(temp_9c.t9c_v_)

-- 
print(temp_9c:is_base())

-- 
print(temp_9c:is_t9c())