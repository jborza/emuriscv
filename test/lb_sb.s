li x3, 0x2004
negative_value:
li x1, 0xdeadbeef
sb x1, -4(x3)
lb x30, -4(x3)
li x31, 0xffffffef
bne x30, x31, failure
negative_value_with_lbu:
lbu x30, -4(x3)
li x31, 0x000000ef
bne x30, x31, failure
positive_value:
li x1, 0x12345678
sb x1, 4(x3)
lb x30, 4(x3)
li x31, 0x00000078
beq x30, x31, success
failure:	
	li a0, 0
	li a7, 93
	ecall
success:
	li a0, 42
	li a7, 93
	ecall
