test1:
li x1, 1
li x2, 1
li x3, 1
bge x1, x2, test2
beq x0, x0, failure

test2:
li x1, 0x00000000
li x2, 0x00000001
li x3,2
bge x1, x2, failure

test3:
li x1, -1
li x2, 1
li x3,3
bge x1, x2, failure

test4:
li x1, -2
li x2, -1
li x3,4
bge x1, x2, failure

test5:
li x1, 1
li x2, 0
li x3,5
bge x1, x2, test6
beq x0,x0,failure

test6:
li x1, 1
li x2, -2
li x3,6
bge x1, x2, test7
beq x0,x0,failure

test7:
li x1, -1
li x2, -2
li x3,7
bge x1, x2, test8
beq x0,x0,failure

test8:
li x1, 1
li x2, -2
li x3,8
bge x1, x2, success
  
failure:	
	li a0, 0
	li a7, 93
	ecall
success:
	li a0, 42
	li a7, 93
	ecall
