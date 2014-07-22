
-- lua coroutine 
function ThreadTest()
	print("ThreadTest invoke start.")
	
	print("thread_func and thread_func2 invoke start.")
	--  lua_yield() 
	thread_func(thread_handle)
	thread_func2(thread_handle,1.2)
	print("thread_func and thread_func2 invoke end.")
	
	print("g_test:thread_men_fun() and g_test:thread_men_fun2() invoke start.")
	--  lua_yield() .
	g_test:thread_men_fun()
	g_test:thread_men_fun2(2.3)
	print("g_test::thread_men_fun() and g_test::thread_men_fun2() invoke end.")
	
	print("ThreadTest  invok end.")
end