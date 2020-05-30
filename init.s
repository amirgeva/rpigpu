.globl _start
_start:
    mov sp,#0x8000
    bl gpu_main
hang: b hang


# Unwind for possible C++ code

.globl __aeabi_unwind_cpp_pr0
.globl __aeabi_unwind_cpp_pr1

__aeabi_unwind_cpp_pr0:
__aeabi_unwind_cpp_pr1:
    b __aeabi_unwind_cpp_pr0

