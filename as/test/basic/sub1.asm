.intel_syntax noprefix
.data

.text
.global _start
_start:
    mov edx, 6
    mov edi, 15
    sub edi, edx
    
    mov eax, 60
    syscall
    
    
