stirling:

sw $1, -4($30)
sw $2, -8($30)
sw $4, -12($30)
sw $5, -16($30)
sw $11, -20($30)
sw $31, -24($30)
lis $31
.word 24
sub $30, $30, $31

lis $11
.word 1

add $3, $0, $0 ; block: base case
bne $2, $0, 2
bne $1, $0, done
add $3, $11, $0
beq $1, $0, done

sub $1, $1, $11 ; block: f(n-1, k)
lis $3
.word stirling
jalr $3
add $4, $3, $0

sub $2, $2, $11 ; block: f(n-1, k-1)
lis $3
.word stirling
jalr $3
add $5, $3, $0

multu $1, $4 ; (n-1) * f(n-1, k)
mflo $3

add $3, $3, $5 ; (n-1) * f(n-1, k) + (n-1, k-1)

done:

	lis $31
	.word 24
	add $30, $30, $31

	lw $1, -4($30)
	lw $2, -8($30)
	lw $4, -12($30)
	lw $5, -16($30)
	lw $11, -20($30)
	lw $31, -24($30)

	jr $31



