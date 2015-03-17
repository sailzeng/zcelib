
-- lua 函数，用于表述不同的集成方式以及测试性能数据，

lobj_x={}
lobj_y={}


function read_objx(a,b,c,d,e,f)
	lobj_x.a_ = a
	lobj_x.b_ = b
	lobj_x.c_ = c
	lobj_x.d_ = d
	lobj_x.e_ = e
	lobj_x.f_ = f
end

function read_objy(a,b,c,d,e,f)
	lobj_y.a_ = a
	lobj_y.b_ = b
	lobj_y.c_ = c
	lobj_y.d_ = d
	lobj_y.e_ = e
	lobj_y.f_ = f
end


function obj_add()
	r_a = lobj_x.a_ + lobj_y.a_
	r_b = lobj_x.b_ + lobj_y.b_
	r_c = lobj_x.c_ + lobj_y.c_
	r_d = lobj_x.d_ + lobj_y.d_
	r_e = lobj_x.e_ + lobj_y.e_
	r_f = lobj_x.f_ + lobj_y.f_
	--调用C函数去传递结果给C
	set_result(r_a,r_b,r_c,r_d,r_e,r_f)
end
