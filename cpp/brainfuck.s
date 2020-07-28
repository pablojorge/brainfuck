increment:
  movq    $1, %rax
  addl    %eax, (%rdi)

decrement:
  movq    $1, %rax
  subl    %eax, (%rdi)

forward:
  movq    $1, %rax
  addq    %rax, %rdi

backward:
  movq    $1, %rax
  subq    %rax, %rdi

read:
  pushq   %rdi
  movq    $0x02000003, %rax # SYS_read
  movq    %rdi, %rsi # buf
  movq    $0, %rdi # stdin
  movq    $1, %rdx # buf_len
  syscall
  popq    %rdi

write:
  pushq   %rdi              # save RDI
  movq    $0x02000004, %rax # SYS_write
  movq    %rdi, %rsi        # set RDI as the 'buf'
  movq    $1, %rdi          # fd: stdout
  movq    $1, %rdx          # len: 1
  syscall                   # execute the call
  popq    %rdi              # restore RDI

loop_start:
  cmpl    $0, (%rdi)
  je      0

loop_end:
  cmpl    $0, (%rdi)
  jne     0

break:
  int     $3

finish:
  retq
