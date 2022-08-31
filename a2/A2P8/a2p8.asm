add $29, $1, $0

count:

sw $1, -4($30)
sw $5, -8($30)
sw $6, -12($30)
sw $20, -16($30)
sw $21, -20($30)
sw $31, -24($30)
lis $31
.word 24
sub $30, $30, $31

lis $4
.word 4
lis $11
.word -1

add $20, $0, $0 ; left
add $21, $0, $0 ; right

add $6, $1, $0

left:
	lw $5, 4($6)
	beq $5, $11, right
	mult $5, $4
	mflo $5
	add $1, $29, $5
	lis $31
	.word count
	jalr $31
	add $20, $3, $0

right:
	lw $5, 8($6)
	beq $5, $11, final
	mult $5, $4
	mflo $5
	add $1, $29, $5
	lis $31
	.word count
	jalr $31
	add $21, $3, $0

final:
	add $3, $20, $0
	slt $25, $20, $21
	beq $25, $0, done
	add $3, $21, $0

done:
	sub $3, $3, $11
	lis $31
	.word 24
	add $30, $30, $31

	lw $1, -4($30)
	lw $5, -8($30)
	lw $6, -12($30)
	lw $20, -16($30)
	lw $21, -20($30)
	lw $31, -24($30)
	
	jr $31


