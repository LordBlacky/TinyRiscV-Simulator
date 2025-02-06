.globl _start

.macro push arg
    sw \arg, 0(sp)
    addi sp, sp, -4
.endm

.macro pop arg
    lw \arg, 0(sp)
    addi sp, sp, 4
.endm

.macro save_regs_callee
    nop
.endm

.macro restore_regs_callee
    nop
.endm

.macro restore_regs_caller
    nop
.endm

.macro save_regs_caller
    nop
.endm

.macro call_func arg
    save_regs_caller
    push ra
    call \arg
    pop ra
    restore_regs_caller
.endm

.macro setup_stackframe
    save_regs_callee
    push fp
    mv fp, sp
.endm

.macro end_func
    leave
    restore_regs_callee
    ret
.endm




_start:
    addi sp, x0, 1000
    mv s0, sp
    call_func function

function:
    setup_stackframe

    end_func
