.intel_syntax noprefix
.data

.text
.global _start
_start:
    mov eax, 60
    mov edi, 10
    syscall
done:
    ret
    
