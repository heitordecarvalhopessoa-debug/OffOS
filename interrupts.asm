BITS 32

section .text
extern timer_irq_handler
extern keyboard_irq_handler
extern mouse_irq_handler

global irq0_stub
global irq1_stub
global irq12_stub

irq0_stub:
    pusha
    cld
    call timer_irq_handler
    popa
    iretd

irq1_stub:
    pusha
    cld
    call keyboard_irq_handler
    popa
    iretd

irq12_stub:
    pusha
    cld
    call mouse_irq_handler
    popa
    iretd

section .note.GNU-stack noalloc noexec nowrite progbits