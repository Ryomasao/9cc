.intel_syntax noprefix
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 208
  mov rax, rbp
  sub rax, 8
  push rax
  push 0
  pop rdi
  pop rax
  mov [rax], rdi

  push rdi
  pop rax

  push 0
  pop rax
  cmp rax, 0
  je .Lend000
#  mov rax, rbp
#  sub rax, 8
#  push rax
#  push 3
#  pop rdi
#  pop rax
#  mov [rax], rdi
#  push rdi
.Lend000:
#  pop rax
#  mov rax, rbp
#  sub rax, 8
#  push rax
#  pop rax
#  mov rax, [rax]
#  push rax
#  pop rax
#  mov rsp, rbp
#  pop rbp
#  ret
#  pop rax

  mov rsp, rbp
  pop rbp
  ret
