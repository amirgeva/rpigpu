.globl _start
_start:
.word	0xe59ff018, 0xe59ff018,0xe59ff018,0xe59ff018
.word	0xe59ff018, 0xe59ff018,0xe59ff018,0xe59ff018
.word	reset, hang, hang, hang, hang, hang, irq, hang

reset:
    mov r0,#0x8000
    mov r1,#0x0000
	mov r2,#0
copy_loop:
	ldr		r3,[r0,r2]
	str		r3,[r1,r2]
	add		r2,r2,#4
	tst		r2,#64
	beq		copy_loop

	mrs r0,cpsr
	bic r0,r0,#0x1F
    orr r0,r0,#0x12
    msr cpsr_c,r0
    mov sp,#0x1000

	mrs r0,cpsr
	bic r0,r0,#0x1F
    orr r0,r0,#0x13
    msr cpsr_c,r0
    mov sp,#0x6000

    bl gpu_main
hang: b hang

.globl enable_irq
enable_irq:
    mrs r0,cpsr
    bic r0,r0,#0x80
    msr cpsr_c,r0
    bx lr

irq:
    push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
    bl uart_irq_handler
    pop  {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
    subs pc,lr,#4


# Unwind for possible C++ code

.globl __aeabi_unwind_cpp_pr0
.globl __aeabi_unwind_cpp_pr1

__aeabi_unwind_cpp_pr0:
__aeabi_unwind_cpp_pr1:
    b __aeabi_unwind_cpp_pr0

