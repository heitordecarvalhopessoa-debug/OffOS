BITS 64
DEFAULT REL

section .text
global monotonic_ns

monotonic_ns:
    sub rsp, 16

    mov eax, 228
    mov edi, 1
    mov rsi, rsp
    syscall

    test rax, rax
    jnz .erro

    mov rax, [rsp]
    imul rax, rax, 1000000000
    add rax, [rsp + 8]

    add rsp, 16
    ret

.erro:
    add rsp, 16
    mov rax, -1
    ret

section .note.GNU-stack noalloc noexec nowrite progbits