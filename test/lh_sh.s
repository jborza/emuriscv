li x3, 0x2004
negative_value:
li x1, 0xdeadbeef
sh x1, -4(x3)
lh x30, -4(x3)
li x31, 0xffffbeef
bne x30, x31, failure
negative_value_with_lbu:
lhu x30, -4(x3)
li x31, 0x0000beef
bne x30, x31, failure
positive_value:
li x1, 0x12345678
sh x1, 4(x3)
lh x30, 4(x3)
li x31, 0x00005678
beq x30, x31, success
failure:	
	li a0, 0
	li a7, 93
	ecall
success:
	li a0, 42
	li a7, 93
	ecall
