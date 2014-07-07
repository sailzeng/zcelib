tb_val:set_b1(1)

print("ta_ptr.a_ = "..ta_ptr.a_)
print("ta_ref.a_ = "..ta_ref.a_)
print("ta_val.a_ = "..ta_val.a_)

ta_ptr.a_ = 20000
ta_ref.a_ = 30000
ta_val.a_ = 40000


print("tb_ptr_1.b1_ = "..tb_ptr_1.b1_)
print("tb_ptr_1.b2_ = "..tb_ptr_1.b2_)
print("tb_ptr_1.b3_ = "..tb_ptr_1.b3_)


tb_ptr_1:set_b1(1)
tb_ptr_1:set_b2(2,3)
tb_ptr_1:set_b3(4,5,6)