
.option norvc
.text
.globl _start
_start:
    addi a0, zero, 0
    addi t0, zero, 10
loop:
    add  a0, a0, t0
    addi t0, t0, -1
    bne  t0, zero, loop
    sw   a0, 256(zero)
    ecall
