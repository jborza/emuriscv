setup:
li a1, 8 # initial value
li t0, 0x100 # initial value address

test_inc:
li a0, 1 # addend
sw a1, 0(t0) # store initial value at 0x100
amoadd.w x1, a0, (t0) # set 0x100 to 8+1
li x29, 9 #expected value
lw x30, 0(t0)
bne x29, x30, fail

test_dec:
li a0, -1 # addend
sw a1, 0(t0) # store initial value at 0x100
amoadd.w x1, a0, (t0) # set 0x100 to 8-1
li x29, 7 #expected value
lw x30, 0(t0)
bne x29, x30, fail

pass:
	li a0, 42
	li a7, 93
	ecall
fail:
	li a0, 0
	li a7, 93
	ecall
