.data
 missing_args: .ascii "Missing arguments!\n"
 missing_args_len: .quad . - missing_args

.section,bss
 .lcomm program, 50000 # Program
 .lcomm buffer, 30000 # Memory buffer

.text
.globl _main

##
# Interpreter implementation:
#     %rbx: program ptr
#     %rsi: data ptr (handy for sys_read and sys_write)
#     XXX missing regs
# 
# The program memory is statically allocated in the BSS section. The BSS 
# section is zero-filled at program start, so there's no need to clean it.
# The program itself is read from a file, obtained via command-line arguments,
# and also stored in a pre-allocated buffer in the BSS section.
#
# Considerations:
#  * Syscall numbers taken from /usr/include/sys/syscall.h
#  * Constants taken from /usr/include/sys/fcntl.h
#  * When invoking syscalls, the kernel destroys registers rcx and r11.
#  * x86_64 calling convention: sequence %rdi, %rsi, %rdx, %rcx, %r8 and %r9
##

_main:

##
# Obtain filename from stack and open it:
##
    # Check arguments count first:
    movq (%rsp), %rcx                   # "argc"
    cmp $0x02, %rcx
    jge get_filename
    # ssize_t write(int fildes, const void *buf, size_t nbyte);
    #               rdi         rsi              rdx
    movq $0x1, %rdi
    mov missing_args@GOTPCREL(%rip), %rsi
    mov missing_args_len@GOTPCREL(%rip), %rdx
    mov (%rdx), %rdx
    movq $0x2000004, %rax
    syscall
    jmp exit 

 get_filename:
    # int open(const char* path, int oflag, ...);
    #          rdi               rsi        rdx
    movq 16(%rsp), %rdi                 # XXX Stack layout
    xor %rsi, %rsi                      # O_RDONLY (0)
    movq $0x2000005, %rax               # SYS_open (5)
    syscall
    # XXX handle error (rax < 0)

 read_program:
##
# Read file contents in program buffer:
##

    # ssize_t read(int fildes, void *buf, size_t nbyte);
    #              rdi         rsi        rdx
    mov %rax, %rdi                      # fd (ret from open())
    mov program@GOTPCREL(%rip), %rsi    # Buffer to read into
    movq $50000, %rdx                   # Bufsize
    movq $0x2000003, %rax               # SYS_read (3)
    syscall
    # XXX handle error (rax < 0)

##
# Initialize interpreter:
##

    mov %rax, %r12                      # Store program length in %r12

    # XXX Store base address + length to speed up end-of-program check
    mov program@GOTPCREL(%rip), %rbx    # Initialize program ptr
    mov buffer@GOTPCREL(%rip), %rsi     # Initialize data ptr

##
# Main interpreter loop:
##

interpreter_loop:
    # Check if we reached program end: 
    movq program@GOTPCREL(%rip), %r8    # Load addr of program base
    addq %r12, %r8
    cmp %r8, %rbx
    jge interpreter_end

inc_pointer:
    cmpb $0x3e, (%rbx)                  # '>'
    jne dec_pointer
    inc %rsi                            # Increment data pointer
    jmp next_iter
dec_pointer:
    cmpb $0x3c, (%rbx)                  # '<'
    jne inc_data
    dec %rsi                            # Decrement data pointer
    jmp next_iter
inc_data:
    cmpb $0x2b, (%rbx)                  # '+'
    jne dec_data
    incb (%rsi)                         # Increment pointed data
    jmp next_iter
dec_data:
    cmpb $0x2d, (%rbx)                  # '-'
    jne inputb
    decb (%rsi)                         # Decrement pointed data
    jmp next_iter
inputb:
    cmpb $0x2c, (%rbx)                  # ','
    jne outputb
    # ssize_t read(int fildes, void *buf, size_t nbyte);
    #              rdi         rsi        rdx
    movq $0x2000003, %rax               # SYS_read (3)
    xor %rdi, %rdi                      # fd 0 (stdin)
    movq $1, %rdx                       # Buffer size (read a single char)
    syscall
    cmp $0x00, %rax                     # Detect EOF on input (ret == 0)
    je interpreter_end
    jmp next_iter
outputb:
    cmpb $0x2e, (%rbx)                  # '.'
    jne loop_start
    # ssize_t write(int fildes, const void *buf, size_t nbyte);
    #               rdi         rsi              rdx
    movq $0x2000004, %rax               # SYS_write (4)
    movq $1, %rdi                       # fd 1 (stdout)
    movq $1, %rdx                       # Length of the string
    syscall
    jmp next_iter
loop_start:
    cmpb $0x5b, (%rbx)                  # '['
    jne loop_end
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
    jmp next_iter
loop_end:
    cmpb $0x5d, (%rbx)                  # ']'
    jne next_iter
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

next_iter:
    inc %rbx                            # Increment program ptr
    jmp interpreter_loop                # Loop

##
# End the interpreter (invoke sys_exit)
##

interpreter_end:
    # Cleanup?
    jmp exit

exit:
    movq $0, %rdi                       # we want to call exit(0)
    movq $0x2000001, %rax               # SYS_exit=1
    syscall
