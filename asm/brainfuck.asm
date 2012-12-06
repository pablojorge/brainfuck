.data
 program: .incbin "../programs/cat.bf"
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

_main:
    mov program@GOTPCREL(%rip), %rbx    # Initialize program ptr
    mov buffer@GOTPCREL(%rip), %rsi     # Initialize data ptr

interpreter_loop:
    
inc_pointer:
    cmpb $0x3e, (%rbx)                  # '>'
    jne dec_pointer
   ##
    inc %rsi                            # Increment data pointer
   ##
    jmp next_iter
dec_pointer:
    cmpb $0x3c, (%rbx)                  # '<'
    jne inc_data
   ##
    dec %rsi                            # Decrement data pointer
   ##
    jmp next_iter
inc_data:
    cmpb $0x2b, (%rbx)                  # '+'
    jne dec_data
   ##
    incb (%rsi)                         # Increment pointed data
   ##
    jmp next_iter
dec_data:
    cmpb $0x2d, (%rbx)                  # '-'
    jne inputb
   ##
    decb (%rsi)                         # Decrement pointed data
   ##
    jmp next_iter
inputb:
    cmpb $0x2c, (%rbx)                  # ','
    jne outputb
   ##
    movq $0x2000003, %rax               # SYS_read (3)
    xor %rdi, %rdi                      # fd 0 (stdin)
    movq $1, %rdx                       # Buffer size (read a single char)
    syscall
    cmp $0x00, %rax                     # Detect EOF on input (ret == 0)
    je interpreter_end
   ##
    jmp next_iter
outputb:
    cmpb $0x2e, (%rbx)                  # '.'
    jne loop_start
   ##
    movq $0x2000004, %rax               # SYS_write (4)
    movq $1, %rdi                       # fd 1 (stdout)
    movq $1, %rdx                       # Length of the string
    syscall
   ##
    jmp next_iter
loop_start:
    cmpb $0x5b, (%rbx)                  # '['
    jne loop_end
   ##
    cmpb $0x00, (%rsi) # XXX testb      # If the current byte is non zero, 
    jnz next_iter                       # we DON'T have to look for the 
                                        # matching ']', just ignore the operand
    movq $1, %rcx                       # Use %rcx as bracket counter
  match_fwd_open:
    inc %rbx
    cmpb $0x5b, (%rbx)                  # If we find another '[', increment the 
    jne match_fwd_close                 # bracket count
    inc %rcx
    jmp match_fwd_open
  match_fwd_close:
    cmpb $0x5d, (%rbx)                  # If we find a ']', decrement the 
                                        # bracket count
    jne match_fwd_open
    dec %rcx
    jnz match_fwd_open                  # Continue matching if count still > 0
   ##
    jmp next_iter
loop_end:
    cmpb $0x5d, (%rbx)                  # ']'
    jne next_iter
   ##
    cmpb $0x00, (%rsi) # XXX testb      # If the current byte is zero, 
    jz next_iter                        # we DON'T have to look for the 
                                        # matching '[', just ignore the operand
    movq $1, %rcx                       # Use %rcx as bracket counter
  match_bwd_open:
    dec %rbx
    cmpb $0x5d, (%rbx)                  # If we find another ']', increment the 
    jne match_bwd_close                 # bracket count
    inc %rcx
    jmp match_bwd_open
  match_bwd_close:
    cmpb $0x5b, (%rbx)                  # If we find a '[', decrement the 
                                        # bracket count
    jne match_bwd_open
    dec %rcx
    jnz match_bwd_open                  # Continue matching if count still > 0
   ##
    #jmp next_iter

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
