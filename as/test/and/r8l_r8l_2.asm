.intel_syntax noprefix
.data

.text
.global _start
_start:
    mov dil, 7
    mov cl, 2
    and dil, cl
    
    mov eax, 60
    syscall
final:
    ret
    
