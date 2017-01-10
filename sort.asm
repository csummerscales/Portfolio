#	-------------------------------------------------	#
#								#						
#	Author: Charlie Summerscales				#
#	Purpose:Problem Set 3 - Sort				#
#	Date: 	03/15/2016					#													
#	-------------------------------------------------	#

#
#	Data declarations go here!
.data
	listSize:	.word	0
	zero:		.word	0
	top:		.word	-1
	lo:		.word	0
	hi:		.word	0
	inputSizeMsg:	.asciiz	"Please enter the size of your list: "
	inputMsg:	.asciiz "Enter an integer: "
	errorSizeMsg:	.asciiz "The size must be greater than zero.\n"
	

.text


.ent	main
.globl	main
main:
		#request input list size from user
getSize:	li $v0, 4
		la $a0, inputSizeMsg
		syscall
		#read_int
		li $v0, 5
		syscall
		sw $v0, listSize
		#allocate memory for array
		sll $a0, $v0, 2 
		li $v0, 9
		syscall
		move $s0, $v0	#store starting array address
		#counters and conditions for loop to retrieve integers
		li $t0, 0	#loop counter
		lw $t1, listSize
		
		#prompt for integer
getInts:	li $v0, 4
		la $a0, inputMsg
		syscall
		#read_int
		li $v0, 5
		syscall		
		#store integers in array
		sw $v0, ($s0)		
		#increment counter and check loop condition
		add $t0, $t0, 1
		
		bge $t0, $t1, callSort	 #continue to get more ints
		add $s0, $s0, 4
		j getInts	
		
		#now we call quicksort	
callSort:	li $t0, 1

		#pass the list in the proper order to quicksort
revList:	subu $s0, $s0, 4	#go backwards through the array space
		add $t0, $t0, 1
		blt $t0, $t1, revList
		
		lw $a0, zero
		lw $t0, listSize
		sub $a1, $t0, 1
		jal quicksort

		li $t0, 0
		lw $t1, listSize		
		
		
		#print list
printList:	li $v0, 1
		lw $a0, ($s0)
		syscall
		li $v0, 11
		la $a0, '\n'
		syscall
		addu $s0, $s0, 4	#move up through the array address space
		add $t0, $t0, 1
		blt $t0, $t1, printList
		j term

		#the size was less than zero
errorSize:	li $v0, 4
		la $a0, errorSizeMsg
		syscall
		j getSize
	
		#Terminate
term:		li $v0, 10
		syscall
	

#	-------
#	Done terminate program
	li $v0, 10		# exit system call
	syscall

.end	main

#-----------------------------------------
#
#	Function: quicksort
#
#	Input:
#	$a0	-> lo
#	$a1	-> hi
#
#	This is the iterative form of quicksort. Basically,
#	we store the lo and hi indexes to the stack, then
#	call the partition function.  Partition will return
#	the correctly placed pivot, and we then push the 'bounds'
#	for the unprocessed array elements to the stack.  This
#	way we will ensure we call partition the correct number
#	of times to place all the elements in their necessary 
# 	places.
#
#	A more detailed explanation of the iterative approach
#	is found here: http://www.geeksforgeeks.org/iterative-quick-sort/
.globl quicksort
.ent quicksort
quicksort:
	sw $ra, 0($sp)		#store return address
	subu $sp, $sp, 4	#increase stack by 4 bytes
	sw $a0, ($sp)		#push lo to stack
	subu $sp, $sp, 4	#increase stack by 4 bytes
	sw $a1, ($sp)		#push hi to stack
	lw $t0, top		#load stack counter
	add $t0, $t0, 2		#add 2 to stack counter
	sw $t0, top		#save stack counter

qLoop:
	lw $t2, top		#load stack counter
	bltz $t2, doneQ		#if nothing else in the stack, we terminate

	lw $a1, ($sp)		#retrieve hi from stack
	addu $sp, $sp, 4	#decrease stack space by 4 bytes
	lw $a0, ($sp)		#retrieve low from stack
	addu $sp, $sp, 4	#decrease stack space by 4 bytes
	sub $t0, $t2, 2		#decrease stack counter
	sw $t0, top		#save stack counter
	
	#partition array
	jal partition		#partition is based on array[lo] to array[hi]
	
	move  $t0, $v0		#store returned pivot into $t0 register
	sub $t0, $t0, 1		#subtract one from returned pivot
	
	#if pivot - 1 > lo, push left side to stack
pivLt:	ble $t0, $a0, pivRt	#otherwise, move on to pivRt
	subu $sp, $sp, 4	#increase stack by 4 bytes
	sw $a0, ($sp)		#push lo to stack
	subu $sp, $sp, 4	#increase stack by 4 bytes
	sw $t0, ($sp)		#push hi to stack
	lw $t1, top		#load stack counter
	add $t1, $t1, 2		#add 2 to stack counter
	sw $t1, top		#save stack counter

	#if pivot + 1 < hi, push right side to stack
pivRt:  add $t0, $t0, 2		#pivot - 1 + 2 = pivot + 1
	bge $t0, $a1, back	#if pivot + 1 < hi, continue, otherwise back through the loop
	subu $sp, $sp, 4	#increase stack by 4 bytes
	sw $t0, ($sp)		#push lo to stack
	subu $sp, $sp, 4	#increase stack by 4 bytes
	sw $a1, ($sp)		#push hi to stack
	lw $t1, top		#load stack counter
	add $t1, $t1, 2		#add 2 to stack counter
	sw $t1, top		#save stack counter

back:	j qLoop
	
	#end of function
doneQ:	lw $ra, 0($sp)
	jr $ra
.end quicksort


#-----------------------------------------
#
#	Function: partition
#
#	Input:
#	$a0	-> lo
#	$a1	-> hi
#
#
#	Output:
#	$v0	-> pivot element
#
.globl partition
.ent partition
partition:
		#set the pivot
		li $t0, 0		#used to set the pivot for the first time
		sw $a0, lo
		sw $a1, hi
		
		#compare pivot against rest of the list
		lw $t2, lo		#compare loop counter, j
		
		#check to see if we need to terminate
parLoop:	lw $t7, hi		#load hi
		sub $t7, $t7, 1		#use hi-1 as the stopping point
		bgt $t2, $t7, finalSwap	
		bnez $t0, compare

		la $t0, ($s0)		#set $t0 to the starting array address
		li $t3, 0		#pivSet counter
pivLoop:	beq $a1, $t3, pivSet	#if hi = pivSet counter then we set it at as the pivot element
		add $t3, $t3, 1		#otherwise, increment counter
		add $t0, $t0, 4		#increment stack space
		j pivLoop		#back to beginning
pivSet:		lw $t1, ($t0)		#set $t1 = array[hi]

compare:	mul $t3, $t2, 4 	#address offset calculated by j * 4
		add $t2, $t2, 1		#j = j + 1
		la $t4, ($s0)		#set $t4 = array[0], then we add the offset in the next step
		add $t4, $t4, $t3	#increment $t4 to array[0 + j-offset] == array[j]
		lw $t5, ($t4)		#load int at array[j]
		ble $t5, $t1, swap	#if array[j] <= array[pivot], swap
		ble $t2, $t7, compare	# for j to listSize, compare
		j finalSwap
		
		#swap elements located at array[$t0] and array[$t4]	
swap:		mul $t9, $a0, 4		#address offset for array[i] -> $t9 = i*4
		add $a0, $a0, 1 	#i = i + 1		
		la $t7, ($s0)		#set $t7 equal to array[0]
		add $t7, $t7, $t9	#increment $t7 to array[0 + i-offset] == array[i]
		lw $t8, ($t7)		#value at array[i] is stored in $t8
		
		add $s0, $s0, $t9 	#offset array address stored in $s0
		sw $t5, ($s0)		#store element from array[j] into array[i]		
		
		sub $t4, $t3, $t9	#j-offset minus i-offset
		add $s0, $s0, $t4	#offset array address in $s0 by difference from i to j
		sw $t8, ($s0)		#store element from array[i] into array[j]
		sub $s0, $s0, $t9
		sub $s0, $s0, $t4
		j parLoop	#go back to loop

	
finalSwap:	mul $t3, $a1, 4		#finds last element pivot offset
		mul $t2, $a0, 4		#finds i index offset
		sub $t3, $t3, $t2	#offset needed to navigate to last element
		add $s0, $s0, $t2	#navigate to array[i]
		lw $t8, ($s0)		#gets element stored at array[i]
		sw $t1, ($s0)		#stores pivot in its proper location
		add $s0, $s0, $t3	#increments up to last element
		sw $t8, ($s0)		#stores array[i] in last place
		sub $s0, $s0, $t3	
		sub $s0, $s0, $t2
		
		move $v0, $a0		#return pivot index
		lw $a0, lo		#reload lo and hi indexes
		lw $a1, hi
		jr $ra
	
.end partition
