
-- lua coroutine ²âÊÔº¯Êı1
function ThreadTest()
	print("#ThreadTest invoke start.")
	
	print("#thread_func and thread_func2 invoke start.")
	--  lua_yield() 
	thread_func()
	print("#thread_func invoke end.")
	thread_func2(1.2,2.3)
	print("#thread_func2 invoke end.")
	
	print("#g_test:thread_men_fun() and g_test:thread_men_fun2() invoke start.")
	--  lua_yield() .
	g_test:thread_men_fun()
	g_test:thread_men_fun2(2.3,3.4)
	print("#g_test::thread_men_fun() and g_test::thread_men_fun2() invoke end.")
	
	print("#ThreadTest  invok end.")
end

-- lua coroutine ²âÊÔº¯Êı2
function ThreadTest2()
	print("#ThreadTest2 invoke start.")
	
	--  lua_yield() 
	
	coroutine.yield(10)

	coroutine.yield(20)

	coroutine.yield(30)

	coroutine.yield(40)
	
	print("#ThreadTest2  invok end.")
end







