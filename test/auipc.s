nop
nop
nop
zero_offset:
auipc x30, 0
li x31, 0xc
bne x30, x31, failure
large_offset: #pc now at 0x18
auipc x30, 0xfffff
li x31, 0xfffff018
beq x30, x31, success
failure:	
	li a0, 0
	li a7, 93
	ecall
success:
	li a0, 42
	li a7, 93
	ecall
