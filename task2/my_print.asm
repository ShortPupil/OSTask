default rel

section .bss
    content: resb 1

section .text
global theprint
theprint:
    mov rcx, rdi
    xor r8, r8
loop:
    xor r9, r9
    mov byte r9b, [rcx + r8]
    cmp byte r9b, 0
    je end
    mov byte [content], r9b
    push rcx
    mov rax, 1
    mov rdi, 1
    mov rsi, content
    mov rdx, 1
    syscall
    pop rcx
    inc r8
    jmp loop
end:
    xor rax, rax
    ret

global printchar
printchar:
    mov byte [content], dil
    mov rax, 1
    mov rdi, 1
    mov rsi, content
    mov rdx, 1
    syscall
    xor rax, rax
    ret
