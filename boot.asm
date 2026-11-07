BITS 32

MAGIC    equ 0x1BADB002
FLAGS    equ 0x00000007
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
    dd 0
    dd 800
    dd 600
    dd 32

section .text
global _start
extern kernel_main

_start:
    cli
    mov esp, stack_top
    xor ebp, ebp
    push ebx
    push eax
    
    call kernel_main

.halt:
    cli
    hlt
    jmp .halt

section .bss
align 16

stack_bottom:
    resb 16384
stack_top:

section .note.GNU-stack noalloc noexec nowrite progbits