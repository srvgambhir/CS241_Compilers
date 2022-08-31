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

sub $1, $0, $1 ; convert negative by subtracting from zero
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
	jr $31

