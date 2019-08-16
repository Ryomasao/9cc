#include "9cc.h"

int LabelId = 0;
char labelStack [9][100];

int labelCounter() {
  return LabelId++;
}

// 変数名に対応しているスタックのアドレスをスタックに積んどく関数
void gen_lval(Node *node) {
  if(node->kind != ND_LVAR)
    error("代入式の左辺値が変数ではありません");
  
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

// 以下のノードを考えてみよう
//       -
//    +    4
//  +   3 
// 1 2
//
// 最初は、演算子なので、gen(node->lhs) gen(node->rhs)が走る
// 1.スタックには、gen(node->rhs)で右辺の数値の4がpushされる
// 2.左辺は、また演算子なので、gen(node->lhs) gen(node->rhs)が走る
// 3.スタックには、右辺の数値3がpushされる
// 4.左辺はまた演算子なので、gen(node-lhs) gen(node->rhs)が走る
// 5.スタックには、左辺の数値1、右辺の数値2がpushされる
// このときのスタックは以下の通り
//  
// 1
// 2 
// 3
// 4
//
// pop rdi  ※1を取り出す
// pop rdx  ※2を取り出す
// 演算子の型を見て、+ なら、rdiとraxを足して、raxに格納する
// 演算子の型を見て、- なら、略...
// 結果のraxを再度スタックに格納
// 4の再帰が、同様にpopして、結果をスタックに格納
// 2の再帰が、略...

void gen(Node *node) {

  if(node == NULL) return;

  switch (node->kind) {
    case ND_NUM:
      // 数値だったらスタックにpush
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return;
    case ND_RETURN:
      gen(node->lhs);
      // スタックトップに式全体の値が残っているはずなので
      // それをRAXにロードして関数からの返り値とする
      printf("  pop rax\n");
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
      return;
    case ND_IF: {
      // if(node->lhs) node-rhsの形式になってる
      // まずは、node-lhsのコードをつくる
      gen(node->lhs);
      // node->lhsの結果がスタックトップに残っているはずなのでpop
      printf("  pop rax\n");
      // スタックトップの結果と0を比較
      printf("  cmp rax, 0\n");
      // スタックトップが0ならnode->rhsの命令を行わない
      int labelId = labelCounter();
      printf("  je .Lend%d\n", labelId);
      gen(node->rhs);
      printf(".Lend%d:\n", labelId);
      return;
    }
    case ND_IF_ELSE: {
      int elseLabelId = labelCounter();
      int endLabelId = labelCounter();
      // if-elseの場合、if-else-stmtでjmp先のラベルをみる必要がある
      // 今後、ifがネストされることもあるので、スタック機能をつくってそこに格納しておくことにする
      push(elseLabelId);
      push(endLabelId);

      // then
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");

      printf("  je .LifElse%d\n", elseLabelId);
      gen(node->rhs);
      return;
    }
    case ND_IF_ELSE_STMT: {
      int endLabelId = pop();
      int elseLabelId = pop();

      // then
      gen(node->lhs);
      printf("  jmp .LifEnd%d\n", endLabelId);
      // else
      printf(".LifElse%d:\n", elseLabelId);
      gen(node->rhs);
      printf(".LifEnd%d:\n", endLabelId);
      return;
    }
    case ND_WHILE: {
      int whileBeginLabelId = labelCounter();
      int whileEndLabelId = labelCounter();
      printf(".LWhileBegin%d:\n", whileBeginLabelId);
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .LWhileEnd%d\n", whileEndLabelId);
      gen(node->rhs);
      printf("  jmp .LWhileBegin%d\n", whileBeginLabelId);
      printf(".LWhileEnd%d:\n", whileEndLabelId);
      return;
    }
    case ND_FOR: {
      int forBeginLabelId = labelCounter();
      int forEndLabelId = labelCounter();
      push(forBeginLabelId);
      push(forEndLabelId);

      //for(expr; expr; expr;)
      //     👆 

      //printf("#FOR文の初期化処理\n");
      gen(node->lhs);

      printf(".LForBegin%d:\n", forBeginLabelId);
      // to ND_FOR_CONTINUE:
      gen(node->rhs);
      printf(".LForEnd%d:\n", forEndLabelId);
      return;
    }
    case ND_FOR_CONTINUE: {
      int forEndLabelId = pop();
      //for(expr; expr; expr;)
      //           👆 
      //printf("#FOR文の継続判定\n");
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .LForEnd%d\n", forEndLabelId);

      // to ND_FOR_LOOP
      gen(node->rhs);
      return;
    }
    case ND_FOR_LOOP: {
      int forBeginLabelId = pop();

      //printf("#FOR文内で実行される式\n");
      // to ND_FOR_STMT
      gen(node->rhs);

      //for(expr; expr; expr;)
      //                 👆 
      // STMTの処理おわってから、処理を行うので、
      // rhsとlhsの処理順が逆になってる
      //printf("#FOR文の後処理\n");
      gen(node->lhs);
      printf("  jmp .LForBegin%d\n", forBeginLabelId);

      return;
    }
    case ND_FOR_STMT: {
      //for(expr; expr; expr;)
      // stmt()
      //   👆 
      gen(node->lhs);
      return;
    }
    case ND_BLOCK: {
      for(int i = 0; node->vector[i]; i++) {
        gen(node->vector[i]);
        // statementごとに、最後にスタックにpushしている。スタックが枯渇しないようにpopしておく。
        // あんま意識してなかった。一つ前の処理の結果をstackに残しておく発想だからかな？
        printf("  pop rax\n");
      }
      // 最期のstmtの処理結果は、codegenを呼ぶ側で処理するので、スタックに戻す
      printf("  push rax\n");
      return;
    }
    // 関数呼び出し foo()
    case ND_FUNC: {
      //int argc = sizeof(node->argv) / sizeof(int);
      //for(int i = 0; i < argc; i++) {
      //  printf("  push %d\n", node->argv[i]);
      //}

      // 関数呼び出しの際はRSPの値が16の倍数になっていることを前提としている関数がある
      // なので、RSPの値が16の倍数ではない場合、調整する
      int rspLabelId = labelCounter();
      printf("  mov rax, rsp\n");
      printf("  mov r10, 16\n");
      printf("  cqo\n");
      printf("  div r10\n");
      printf("  cmp rdx, 0\n");
      printf("  je .Lrsp%d\n", rspLabelId);
      printf("  sub rsp, 8\n");

      printf(".Lrsp%d:\n", rspLabelId);
      // 第一引数はrdiレジスタ、、、のように決まってるみたい
      // ひとまず、固定でセット
      printf("  mov rdi, %d\n", node->argv[0]);
      printf("  mov rsi, %d\n", node->argv[1]);
      printf("  mov rdx, %d\n", node->argv[2]);

      printf("  call %s\n", node->funcName);
      return;
    }
    // 関数の定義 foo(){ }
    case ND_FUNC_DIF: {
      printf("%s:\n", node->funcName);

      // プロローグ処理
      printf("  push rbp\n");
      printf("  mov rbp, rsp\n");
      // 変数26個分の領域を確保する
      printf("  sub rsp, 208\n");

      return;
    }
   // 関数の終了のブロック構文}のとき
    case ND_FUNC_DIF_END: {
      // ここのアセンブリが実行されるパターンは、関数でreturnしていないとき
      // returnしていないときはNULLを返したほうがいいんだろうけど、
      // ひとまず直前の式の結果がスタックトップにあると思うので、それを返すようにする
      printf("  pop rax\n");

      // エピローグ
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");

      return;
    }
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      //  1 + 2 / 2の場合、構文木はこう
      //
      //    +
      //  1   /
      //     2  3
      //
      // スタックは、以下の順に積まれる
      //  3
      //  2
      //  1
      //
      // idiv rax rdi という命令セットはないとのこと
      // idivはrdx とRDXとRAXを連結して128ビット整数にする
      // それを引数のrdiレジスタの64ビットの値で割り、商をRAXにあまりをRDXにセットする
      // cqoを使うと、RAXを128ビットに伸ばして、RDXとRAXにセットすることができる
      // まとめると、 2 / 3の場合
      // pop rdi ⇨ 3
      // pop rax ⇨ 2
      // raxの2を128ビットに拡張して、 2の値を、 rdiの3でわる。結果がraxにセットされる
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    case ND_LT:
      // SF <> OFの場合、1をセット
      // SF <> OFが < になるのかよくわからない
      // note)
      // SF: 計算結果が負のとき0になる。cmp 1 2 は 1-2をしているとのことなので、SFは0になる
      // OF: 符号あり整数の桁あふれが発生した場合に1。 1-2はマイナスになるけど、これは桁あふれ？
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LTE:
      // ZFが1ならZFをセット
      // または、SF <> OFの場合、1をセット
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_EQ:
      // cmpした結果、フラグレジスタ(ZFとかSF)の結果がかわる
      // seteはフラグレジスタの結果をalにセットする
      // 正確に書くと、ZFレジスタの値を参照する
      // https://www.felixcloutier.com/x86/setcc
      // ZFレジスタは、cmp rax rdiをやったとき、rax rdiが同じ値なら1がセットされる
      // alはraxレジスタの下位8bit
      // sete raxができればいいんだけど、フラグレジスタの値はal経由でしかできないみたい
      // mobzvはraxの上位56bitをゼロクリアする
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NEQ:
      // ZFが0の場合1をセット
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
  }

  printf("  push rax\n");
}

