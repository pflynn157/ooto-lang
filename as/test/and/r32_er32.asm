.intel_syntax noprefix
.data

.text
.global _start
_start:
    mov edi, 15
    mov r10d, 10
    and edi, r10d
    
    mov eax, 60
    syscall
done:
    ret
    
    
