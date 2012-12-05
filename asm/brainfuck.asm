.data
 program: .ascii "+>+>"
 len: .quad . - program

.section,bss
 .lcomm buffer, 30000 # Memory buffer

.text
.globl _main

# Interpreter implementation:
#     %rbx: program ptr
#     %rsi: data ptr (handy for sys_read and sys_write)
# 
# The program memory will be statically allocated in the BSS section. 
# Initally, the program code be stored in the .data section.
#
# Considerations:
#  * When invoking syscalls, the kernel destroys registers rcx and r11.
#
# To debug in GDB:
#   start
#   display /5i $rip
#   display /b $rsi
#   display /b $rbx
#   display /s $rbx
#   b interpreter_loop
#   cont
#   #disable break 2

_main:
    mov program@GOTPCREL(%rip), %rbx    # Initialize program ptr
    mov buffer@GOTPCREL(%rip), %rsi     # Initialize data ptr

interpreter_loop:
inc_pointer:
    cmpb $0x3e, (%rbx)                  # '>'
    jne dec_pointer
    inc %rsi                            # Increment data pointer
    jmp next_iter
dec_pointer:
    cmpb $0x3c, (%rbx)                  # '<'
inc_data:
    cmpb $0x2b, (%rbx)                  # '+'
    jne dec_data
    incb (%rsi)                         # Increment pointed data
    jmp next_iter
dec_data:
    cmpb $0x2d, (%rbx)                  # '-'
inputb:
    cmpb $0x2c, (%rbx)                  # ','
outputb:
    cmpb $0x2e, (%rbx)                  # '.'
    jne loop_start
    movq $0x2000004, %rax               # SYS_write (4)
    movq $1, %rdi                       # fd 1 (stdout)
    movq $1, %rdx                       # Length of the string
    syscall
    jmp next_iter
loop_start:
    cmpb $0x5b, (%rbx)                  # '['
loop_end:
    cmpb $0x5d, (%rbx)                  # ']'
next_iter:
    inc %rbx                            # Increment program ptr
    # Check if we reached program end: 
    movq program@GOTPCREL(%rip), %r8    # Load addr of program base
    movq len@GOTPCREL(%rip), %r9        # Load addr or program len
    movq (%r9), %r9
    addq %r9, %r8
    cmp %r8, %rbx
    jge interpreter_end
    # Loop
    jmp interpreter_loop

interpreter_end:
    # Invoke the exit syscall:
    movq $0, %rdi                       # we want to call exit(0)
    movq $0x2000001, %rax               # SYS_exit=1
    syscall
