    .syntax unified
    .thumb
    .section .stub, "ax"
    .global _start
    .type _start, %function
_start:
    push {r4, lr}          @ save caller r4 + lr across the C call
    bl   openpak_hook      @ r0 = name -> r0 = (match ? ip_string : name)
    pop  {r4}              @ r4 = caller r4
    pop  {r3}              @ r3 = caller lr
    mov  lr, r3
    push {r4, r5, lr}      @ displaced prologue of the resolver entry
    sub  sp, #4
    mov  r1, sp
    adds r4, r0, #0
    ldr  r3, =RESUME_THUMB
    bx   r3                @ continue in the original resolver body
