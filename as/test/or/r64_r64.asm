.intel_syntax noprefix
.data

.text
.global _start
_start:
    mov rbx, 5
    mov rdi, 10
    or rdi, rbx
    
    mov eax, 60
    syscall
done:
    ret
    
    
