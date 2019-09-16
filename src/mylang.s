.intel_syntax noprefix
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 208
  mov rax, rbp
  sub rax, 8
  push rax
  push 10
  pop rdi
  pop rax
  mov [rax], rdi
  mov rax, rbp
  sub rax, 16
  push rax
  push 11
  pop rdi
  pop rax
  mov [rax], rdi
  mov rax, rbp
  sub rax, 24
  push rax
  push 12
  pop rdi
  pop rax
  mov [rax], rdi
  mov rax, rbp
  sub rax, 32
  push rax
  mov rax, rbp
  sub rax, 16
  push rax
  push 8
  pop rdi
  pop rax
  sub rax, rdi
  push rax
  pop rdi
  pop rax
  mov [rax], rdi
  mov rax, rbp
  sub rax, 32
  push rax
  pop rax
  mov rax, [rax]
  push rax
  pop rax
  mov rax, [rax]
  push rax
  pop rax
  mov rsp, rbp
  pop rbp
  ret
  pop rax
  mov rsp, rbp
  pop rbp
  ret
