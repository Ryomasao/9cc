.intel_syntax noprefix
.global main

main:
  # 関数を呼び出した時点のrbpをスタックに積んでおく
  push rbp
  # こっからは自分の関数で使うスタックの領域だ！というアドレスをrbpの突っ込む
  mov rbp, rsp
  # rspから208バイト分領域を確保しておく
  # 確保といっても、OS？が確保しているスタックの領域はもともとあるので移動しているだけなのかしら
  sub rsp, 208

  # こっからをトレースしてみると
  #   =
  # a   1 
  #


  # gen_lval 
  # ベースレジスタから、変数名に応じた格納先のスタックのアドレスをraxにセットする
  mov rax, rbp
  sub rax, 8
  push rax

  # gen(node->rhs)
  push 2

  # 実際に格納する
  pop rdi
  pop rax
  mov [rax], rdi
  # このpushはなんでだろう？
  push rdi

  # こっから gen(node->lhs)
  # 今回のケースだと意味がない挙動になる。というのも変数aを参照することはしてないから
  # とはいえ、変数名からスタックのアドレスを取得して
  mov rax, rbp
  sub rax, 8
  push rax

  pop rax 
  # そのアドレスに格納されている値をraxにつっこんでる
  mov rax, [rax]
  push rax

  pop rax

  # エピローグ処理
  # プロローグでやったこのと逆をすることで元に戻す
  # これを行うことで、retしたときにrspが関数の戻り先のアドレスを指している状態になる
  # ret命令はスタックからアドレスをポップして、そこにジャンプする命令
  mov rsp, rbp
  pop rbp
  ret
