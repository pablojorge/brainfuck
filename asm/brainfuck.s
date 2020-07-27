##
# Constants declarations
##

# XXX Linux support ??

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

.bss
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

.macro begin_operand
 $0:
  cmpb $1, (%rbx)
  jne $0_end
.endm

.macro end_operand
  jmp next_iter
 $0_end:
.endm

# This macro is used for the loop_start and loop_end operands
# In each case, %rcx holds the counter of brackets that we have to skip
# in order to find the matching one. The difference between matching a closing
# bracket to an opening one and viceversa, is that in one case, the program
# is advanced forward (when looking for the closing bracket), and in the other 
# it's moved backwards (when looking for the opening one). Of course the 
# brackets also have to be swapped (which is the one we started with and it's
# match)
.macro fmatch_bracket
 movq $$1, %rcx # Use %rcx as bracket counter
 fsame_$0:
  $0 %rbx
  cmpb $1, (%rbx) # If we find the same bracket, increment the count
  jne fmatch_$0
  inc %rcx
  jmp fsame_$0
 fmatch_$0:
  cmpb $2, (%rbx) # If we find the matching bracket, decrement the count
  jne fsame_$0
  dec %rcx
 jnz fsame_$0 # Continue matching if count still > 0
.endm

##
# Interpreter implementation:
#
# Register Usage:
#  %rax: holds syscall number and syscall return value
#  %rbx: ** program ptr **
#  %rcx: holds bracket count in fmatch_bracket and argc at the beginning
#  %rdx: third argument to syscalls (usually buf size)
#  %rsp: used to obtain cmdline args
#  %rdi: first argument to syscalls
#  %rsi: ** data ptr ** (handy for sys_read and sys_write). Also used in open()
#  %r12: points to the end of the program
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
  open 16(%rsp), $0x00 # XXX Stack layout # O_RDONLY (0)
  cmp $0x00, %rax # XXX 0x02 on error?
  jg read_program
  print_constant no_such_file
  exit $1

##
# Read file contents in program buffer:
##

 read_program:
  read %rax, program@GOTPCREL(%rip), $50000 # fd = ret from open()
  # XXX handle error (rax < 0)

##
# Initialize interpreter:
##

  mov program@GOTPCREL(%rip), %rbx # Initialize program ptr
  mov buffer@GOTPCREL(%rip), %rsi # Initialize data ptr

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

 begin_operand inc_pointer, $0x3e # '>'
  inc %rsi # Increment data pointer
 end_operand inc_pointer

 begin_operand dec_pointer, $0x3c # '<'
  dec %rsi # Decrement data pointer
 end_operand dec_pointer

 begin_operand inc_data, $0x2b # '+'
  incb (%rsi) # Increment pointed data
 end_operand inc_data

 begin_operand dec_data, $0x2d # '-'
  decb (%rsi) # Decrement pointed data
 end_operand dec_data

 begin_operand inputb, $0x2c # ','
  read $0, %rsi, $1 # fd 0 (stdin), read a single char
  cmp $0x00, %rax # Detect EOF on input (ret == 0)
  je interpreter_end # Abort if EOF was reached
 end_operand inputb

 begin_operand outputb, $0x2e # '.'
  write $1, %rsi, $1                  # fd 1 (stdout), write a single char
 end_operand outputb

 begin_operand loop_start, $0x5b # '['
  cmpb $0x00, (%rsi) # If the current byte is non zero, # XXX testb
  jnz next_iter      # we DON'T have to look for the 
                     # matching ']', just ignore the operand
  fmatch_bracket inc, $0x5b, $0x5d # (inc the program ptr, '[', ']')
 end_operand loop_start

 begin_operand loop_end, $0x5d # ']'
  cmpb $0x00, (%rsi) # If the current byte is zero, # XXX testb
  jz next_iter       # we DON'T have to look for the 
                     # matching '[', just ignore the operand
  fmatch_bracket dec, $0x5d, $0x5b # (dec the program ptr, ']', '[')
 end_operand loop_end

next_iter:
 inc %rbx # Increment program ptr
 jmp interpreter_loop # Loop

##
# End the interpreter
##

interpreter_end:
    # Cleanup?
    exit $0

