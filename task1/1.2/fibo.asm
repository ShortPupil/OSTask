global 	_start:
section 	.data

	over: 	dq 	0x7FFFFFFFFFFFFFFF
	overLOck:	dq 	0x7FFFFFFFFFFFFFFF
	count:	        dq 	1
	count1: 	        dq 	0
    	count2: 	        dq 	0
    	count3:	        dq	1
	changeLine	db  	0xa
	changeLine_length equ 	$-changeLine
        justInput:	db	"Input x and y:",	0xa
	justInputLen:	equ 	$-justInput
        one:            db      "1"
        zero:           db      "0",                    0xa
        oneLen:        db      $-one

	c:	db	1

	setcolor :	db 	1Bh, '['
	c1:		db	'0'
			db	';3'
	c2:		db 	'1'
			db	'm'
	n_p:		db	'0'  
    	len 		equ 	$ - setcolor
section .bss

	temp: 	resq 	1		
    	fib1: 	resq 	1
    	fib2: 	resq 	1
    	fib2_1: 	resq 	1
    	result:	resb 	64
    	input : 	resb 	64
    	num: 	resw 	64

;-------定义宏-------
   	;------读------
 	%macro read 2
        mov 	rax, 	3
        mov 	rbx, 	1
        mov 	rcx, 	%1
        mov 	rdx, 	%2
        int 	80h
    	%endmacro

       ;------写------
	%macro	write 	 2
	mov 	rax, 	4
        mov 	rbx, 	1
        mov 	rcx, 	%1
        mov 	rdx, 	%2
        int 	80h
    	%endmacro

        ;-------颜色-------
	%macro	color 	 1
	push 	rdx
    	push 	rdi
    	push 	rsi	
    	mov 	rsi,	%1
    	mov 	rdi, 	n_p
    	movsb

        ;改变颜色
 	mov 	rsi,	c
 	lodsb
 	cmp 	rax,	4
 	jb 	fcol%1
 	mov 	rdi,	8
 	mov 	rdx, 	0
 	div 	rdi
 	mov 	rdi,	c1
 	add 	rax, 	'0'
 	stosb
 	mov 	rax,	rdx
 fcol%1:
 	mov 	rdi,	c2
 	add 	rax, 	'0'
 	stosb
        ;写
	mov 	rax, 	4
        mov 	rbx, 	1
        mov 	rcx, 	setcolor
        mov 	rdx, 	len
        int 	80h
        pop 	rsi
        pop 	rdi
        pop 	rdx
    	%endmacro

        ;------乘10------
     	%macro multen 1
     	mov 	rax,	%1
     	push	r10
     	mov	r10,	10
     	mul 	r10
     	pop	r10
   	%endmacro

        ;------除10------
     	%macro divten 1
     	mov 	rax,	%1  
     	push 	rdx
     	push 	r10
     	mov 	r10,	10    ;r10=10 
     	mov 	rdx,	0
     	div 	r10
     	pop 	r10
     	pop 	rdx
   	%endmacro

section 	.text
_start:
	write 	justInput,	justInputLen

;-----------------start reading---------------
	mov 	rax,	3
	mov 	rbx,	1
	mov 	rcx, 	input
	mov 	rdx, 	64
	int 	80h

	mov 	rsi, 	input
	mov 	rdi,	num
	push 	rdi
	mov 	r13,	" "
	push 	r13
readInput:
	;mov rax,rsi
	lodsb 
	cmp 	rax,	0xa   
	je    	end
	
	cmp 	rax,	" "	
	je 	update
	cmp 	rax, 	'0'    
	jb      	readInput     	
	cmp 	rax, 	'9'
	ja     	readInput     		
	sub 	rax,	'0'
	push 	rax
	jmp 	readInput
end:
	mov 	r12,	"e"
update:
	mov 	rax,	" "
	;push rax
	mov 	r9,	1
	mov 	r10,	0 
cons:
	pop 	rax
	cmp 	rax ,	" "
	je 	exitcon
	mul 	r9
	add 	r10,	rax
	multen 	r9
	mov 	r9,	rax
	jmp 	cons
exitcon:
	pop 	rdi
	mov 	rax,	r14       
        cmp     r14,    0
        jne     putin
        jmp     out
putin:                      
        mov     rax,    r14
	stosw
        add     r14,    1
        cmp     r14,    r10
        jne     putin
finalputin:
        mov     rax,    r10
        stosw
        jmp     out 
out: 
        mov     r15,    rax
        ;push    rbx
        ;push    rcx
        ;push    rdx
        ;push    rdi
        cmp     r10,    0
        je      writeZero
        cmp     r10,    1
        je      writeOne
        jmp     not
writeZero:
	color   zero
	write 	changeLine,	changeLine_length
writeOne:
	color   one
	write 	changeLine,	changeLine_length
        mov     r10,    1
not:
        mov     rax,    r15
        ;pop     rbx
        ;pop     rcx
        ;pop     rdx
        ;pop      rdi
        mov     r14,    r10
	push 	rdi
	cmp 	r12,	'e'
	je 	exitRead

	push 	rdi

	mov 	rax,	 " "
	push 	rax
	jmp 	readInput

exitRead:
	pop 	rdi
	mov 	rax, 	's'
	stosw


;----------------start fb-------------------
	mov 	qword[fib1],	0
	mov 	qword[fib2],	1
	
	jmp	calLong 
		
exit:
	mov 	rax, 	1
    	mov 	rbx, 	0
    	int 	80h

calLong:
	mov 	rax,	qword[overLOck]
	mov 	qword[over],	rax

	mov 	rcx,	qword[count2]
	mov 	qword[count3],	rcx	
        mov 	rbx, 	qword[fib2]
	mov 	qword[fib2_1],	rbx	
	mov 	rcx, 	qword[fib1]	

	mov 	r10,	rbx

	add 	rbx,	rcx
	cmp 	rbx,	qword[overLOck]
	jb 	backover


handleoverflow:
	mov 	rbx,	r10
	inc 	qword[count2]	
	sub 	rbx,	qword[overLOck]
	; mov 	qword[fib1],	rbx
	add 	rbx,	rcx
	
backover:
	mov 	qword[fib2],	rbx
	mov 	rax,	qword[fib2]
	mov 	qword[result],	rax
	mov 	rax,	qword[fib2_1]
	mov 	qword[fib1],	rax
	mov 	rax,	qword[count3]
	mov 	rbx,	qword[count1]
	add 	qword[count2],	rbx
	mov 	qword[count1],	rax

	mov 	rax,	'e'
	push 	rax		;a sign of end
	cmp 	qword[count2],	0
	jne 	notok
	mov 	qword[over],	0

notok:

	add 	qword[count],		1

	mov 	rsi,	qword[count]

back:	
	cmp 	rsi, 	300
	je 	exit  		 
	mov 	r10,	rsi	

	mov 	rsi, 	num


        
judge:	    
        lodsw
	cmp 	rax,	's' ;字符结束
	je 	calLong
	cmp 	r10, 	rax
        jne     judge	

divi:
	mov 	rax, 	qword[over]
	mov 	rbx,	10
	mov 	rdx, 	0
	div 	rbx
	mov 	qword[over], 	rax
	push 	rdx		

	
	mov 	rax,	qword[result]
	mov 	r8,	10
	mov 	rdx,	0
	div 	r8
	mov 	qword[result],	rax

	pop 	rax
	mov 	r8,	rdx
	mul 	qword[count2]
	add 	rax,	r8
	mov 	r8,	10
	mov 	rdx,	0
	div 	r8

	push 	rdx		;save remaider
	add 	qword[result],	rax
		
	cmp 	qword[over],	0
	jne 	divi

	cmp 	qword[result],	0
	jne 	divi

;---------------printFb------------
print:
	pop 	qword[temp]
	cmp 	qword[temp],	'e'
	je 	next
	add 	qword[temp],	'0'
	color   temp
	jmp 	print
next:
	write 	changeLine,	changeLine_length
	inc 	qword[c]
	jmp 	calLong
