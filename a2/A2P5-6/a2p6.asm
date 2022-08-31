main:
sw $1, -4($30)
sw $2, -8($30)
sw $3, -12($30)
sw $4, -16($30)
sw $11, -20($30)
sw $31, -24($30)
lis $31
.word 24
sub $30, $30, $31

lis $4
.word 4
lis $11
.word 1

add $3, $1, $0

loop:
	beq $2, $0, done
	
	lw $1, 0($3)

	lis $31
	.word print
	jalr $31

	add $3, $3, $4
	sub $2, $2, $11
	
	beq $0, $0, loop

done:

	lis $31
	.word 24
	add $30, $30, $31
	
	lw $1, -4($30)
	lw $2, -8($30)
	lw $3, -12($30)
	lw $4, -16($30)
	lw $11, -20($30)
	lw $31, -24($30)

	jr $31
