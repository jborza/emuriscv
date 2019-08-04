test2:
li x1, 0x00000000
li x2, 0x00000001
bltu x1, x2, test3
beq x0,x0,failure

test3:
li x1, 0xfffffffe
li x2, 0xffffffff
bltu x1, x2, test4
beq x0,x0,failure

test4:
li x1, 0x00000000
li x2, 0xffffffff
bltu x1, x2, test5
beq x0,x0,failure

test5:
li x1, 0x00000001
li x2, 0x00000000
bltu x1, x2, failure

test6:
li x1, 0xffffffff
li x2, 0xfffffffe
bltu x1, x2, failure

test7:
li x1, 0xffffffff
li x2, 0x00000000
bltu x1, x2, failure

test8:
li x1, 0x80000000
li x2, 0x7fffffff
bltu x1, x2, failure
beq x0,x0,success
  
failure:	
	li a0, 0
	li a7, 93
	ecall
success:
	li a0, 42
	li a7, 93
	ecall
