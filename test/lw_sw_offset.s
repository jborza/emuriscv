#sw and lw with a small negative offset
test1: #zero offset
li x3, 0x2004
li x1, 0x12c0ffee
sw x1, (x3)
lw x30, (x3)
bne x30, x1, failure
test2: #positive offset
li x1, 0x99887766
sw x1, 8 (x3)
lw x30, 8 (x3)
bne x30, x1, failure
test3: #large positive offset
li x1, 0x66778899
sw x1, 2044(x3)
lw x30, 2044(x3)
bne x30, x1, failure
test4: #small negative offset
li x3, 0x2004
li x1, 0xdeadbeef
sw x1, -4(x3)
lw x30, -4(x3)
bne x30, x1, failure
test5: #large negative offset
li x1, 0xabcddcba
li x3, 0x4000
sw x1, -996(x3)
lw x30, -996(x3)
beq x30, x1, success
failure:	
	li a0, 0
	li a7, 93
	ecall
success:
	li a0, 42
	li a7, 93
	ecall
