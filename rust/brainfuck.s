increment:
  addl    $128, (%rsi)

decrement:
  subl    $128, (%rsi)

forward:
  addq    $128, %rsi

backward:
  subq    $128, %rsi

read:
  movq    $0x02000003, %rax # SYS_read
  movq    $0, %rdi          # fd: 0 (stdin)
  movq    $1, %rdx          # buf_len: 1
  syscall

write:
  movq    $0x02000004, %rax # SYS_write
  movq    $1, %rdi          # fd: 1 (stdout)
  movq    $1, %rdx          # buf_len: 1
  syscall                   # execute the call

loop_start:
  cmpl    $0, (%rsi)
  je      0

loop_end:
  cmpl    $0, (%rsi)
  jne     0

break:
  int     $3

finish:
  retq
