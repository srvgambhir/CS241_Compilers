lis $4
.word 4
lis $11
.word 1

lw $3, 0($1)
add $21, $1, $0
add $22, $2, $0

loop:
	add $21, $21, $4 ; move pointer to next element in array
	sub $22, $22, $11 

	lw $5, 0($21)
	slt $6, $5, $3
	bne $6, $0, 1
	lw $3, 0($21)
	
	bne $22, $0, loop

jr $31
