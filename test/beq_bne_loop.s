li a0, 7
beq x0,x0,target
li x2, 1
target:
addi x1,x1,1 
bne x1,a0,target
li a0, 42
li a7, 93
ecall