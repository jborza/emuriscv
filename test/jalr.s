nop
nop
li x1,0x54
jalr x30, x1,0
nop
nop
nop
nop
nop
nop
nop
nop 
nop
nop
nop
nop
nop
nop
nop
nop
beq x0,x0,failure
li x31, 0x10
beq x30, x31, success
failure:	
	li a0, 0
	li a7, 93
	ecall
success:
	li a0, 42
	li a7, 93
	ecall