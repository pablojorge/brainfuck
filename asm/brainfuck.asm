##
# Constants declarations
##

.data
 missing_args: .ascii "Missing arguments!\n"
 missing_args_len: .quad . - missing_args
 
 usage: .ascii "Usage:\n\t./brainfuck <program name>\n"
 usage_len: .quad . - usage

 no_such_file: .ascii "No such file or directory"
 no_such_file_len: .quad . - no_such_file

## 
# Buffers
##

.section,bss
 .lcomm program, 50000 # Program
 .lcomm buffer, 30000 # Memory buffer

##
# Text section
##

.text
.globl _main

##
# Macros
##

## Syscalls

.macro exit
 # void exit(int status);
 #           rdi
 movq $0, %rdi
 movq $$0x2000001, %rax # SYS_exit (1)
 syscall
.endm

.macro read
 # ssize_t read(int fildes, void *buf, size_t nbyte);
 #              rdi         rsi        rdx
 movq $0, %rdi
 movq $1, %rsi
 movq $2, %rdx
 movq $$0x2000003, %rax # SYS_read (3)
 syscall
.endm

.macro write
 # ssize_t write(int fildes, const void *buf, size_t nbyte);
 #               rdi         rsi              rdx
 movq $0, %rdi
 movq $1, %rsi
 movq $2, %rdx
 movq $$0x2000004, %rax # SYS_open (4)
 syscall
.endm

.macro open
 # int open(const char* path, int oflag, ...);
 #          rdi               rsi        rdx
 movq $0, %rdi
 movq $1, %rsi
 movq $$0x2000005, %rax # SYS_open (5)
 syscall
.endm

## Convenience macros

.macro print_constant
 mov $0_len @GOTPCREL(%rip), %rdx
 write $$0x1, $0 @GOTPCREL(%rip), (%rdx)
.endm

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
    movq (%rsp), %rcx # "argc"
    cmp $0x02, %rcx
    jge get_filename
    print_constant missing_args
    print_constant usage
    exit $1

 get_filename:
    # int open(const char* path, int oflag, ...);
    #          rdi               rsi        rdx
    open 16(%rsp), $0x00 # XXX Stack layout # O_RDONLY (0)
    cmp $0x00, %rax # XXX 0x02 on error?
    jg read_program
    print_constant no_such_file
    exit $1

##
# Read file contents in program buffer:
##

 read_program:
    # ssize_t read(int fildes, void *buf, size_t nbyte);
    #              rdi         rsi        rdx
    read %rax, program@GOTPCREL(%rip), $50000 # fd = ret from open()
    # XXX handle error (rax < 0)

##
# Initialize interpreter:
##

    mov program@GOTPCREL(%rip), %rbx    # Initialize program ptr
    mov buffer@GOTPCREL(%rip), %rsi     # Initialize data ptr

    # Point %r12 to the end of the program:
    mov %rbx, %r12
    add %rax, %r12

##
# Main interpreter loop:
##

interpreter_loop:
    # Check if we reached program end: 
    cmp %r12, %rbx
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
    read $0, %rsi, $1                   # fd 0 (stdin), read a single char
    cmp $0x00, %rax                     # Detect EOF on input (ret == 0)
    je interpreter_end
    jmp next_iter
outputb:
    cmpb $0x2e, (%rbx)                  # '.'
    jne loop_start
    write $1, %rsi, $1                  # fd 1 (stdout), write a single char
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
# End the interpreter
##

interpreter_end:
    # Cleanup?
    exit $0

