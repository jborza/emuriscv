li x3, 0x2004
li x1, 0xdeadbeef
sw x1, -444(x3)
lw x30, -444(x3)
li x31, 0xdeadbeef
beq x30, x31, success
failure:	
	li a0, 0
	li a7, 93
	ecall
success:
	li a0, 42
	li a7, 93
	ecall
