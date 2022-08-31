print:
sw $1, -4($30)
sw $2, -8($30)
sw $3, -12($30)
sw $4, -16($30)
sw $5, -20($30)
sw $6, -24($30)
sw $7, -28($30)
sw $10, -32($30)
sw $11, -36($30)
sw $31, -40($30)
lis $31
.word 40
sub $30, $30, $31

lis $4
.word 48 ; number offset
lis $5
.word 1000000000 ; divisor
lis $6
.word 0xffff000c ; print address
lis $7
.word 45 ; "-"
lis $10
.word 10 ; newline and #10
lis $11
.word 1 ; #1 and lead zero boolean

bne $1, $0, 2 
sw $4, 0($6)
beq $0, $0, done

slt $3, $1, $0
beq $3, $0, loop

sub $1, $0, $1 ; canvert negative by subtracting from zero
sw $7, 0($6)

loop:
	beq $5, $0, done
	div $1, $5
	mflo $3
	mfhi $1

	div $5, $10
	mflo $5

	beq $3, $0, 1
	add $11, $0, $0 ; set to 1 if non-zero digit encountered

	bne $3, $0, 1
	bne $11, $0, 2
	add $3, $3, $4
	sw $3, 0($6)
	beq $0, $0, loop

done:
	sw $10, 0($6)

	lis $31
	.word 40
	add $30, $30, $31

	lw $1, -4($30)
	lw $2, -8($30)
	lw $3, -12($30)
	lw $4, -16($30)
	lw $5, -20($30)
	lw $6, -24($30)
	lw $7, -28($30)
	lw $10, -32($30)
	lw $11, -36($30)
	lw $31, -40($30)

	jr $31

