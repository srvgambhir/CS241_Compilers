lis $4
.word 4
lis $6
.word 0xffff000c ; print address
lis $7
.word 32 ; space
lis $8
.word 64 ; letter offset
lis $11
.word 1

add $21, $1, $0
add $22, $2, $0

loop:
	beq $22, $0, done

	lw $5, 0($21)

	add $21, $21, $4 ; move pointer to next element
	sub $22, $22, $11

	beq $5, $0, space

	add $5, $5, $8
	sw $5, 0($6)

	beq $0, $0, loop

space:
	sw $7, 0($6)
	beq $0, $0, loop

done:
	jr $31


