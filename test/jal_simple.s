j target1
nop
target2:
j target3
target1:
j target2
failure:	
	li a0, 0
	li a7, 93
	ecall
target3:
success:
	li a0, 42
	li a7, 93
	ecall
