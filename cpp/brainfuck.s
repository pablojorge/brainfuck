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
  movq    $1, %rdi # fd
  movq    $1, %rdx # buf_len
  syscall
  popq    %rdi

write:
  pushq   %rdi
  movq    $0x02000004, %rax # SYS_write
  movq    %rdi, %rsi # buf
  movq    $1, %rdi # fd
  movq    $1, %rdx # buf_len
  syscall
  popq    %rdi

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
