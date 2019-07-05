#include "9cc.h"

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
  if(node->kind == ND_NUM) {
    // 数値だったらスタックにpush
    printf("  push %d\n", node->val);
    return;
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
      // cmpした結果、フラグレジスタ(ZFとかSF)の結果がかわる
      // seteはフラグレジスタの結果をalにセットする
      // 正確に書くと、ZFレジスタの値を参照する
      // https://www.felixcloutier.com/x86/setcc
      // ZFレジスタは、cmp rax rdiをやったとき、rax rdiが同じ値なら1がセットされる
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LTE:
      // setleはZFが1ならZFをセット
      // そうじゃない場合、SF <> OFとなってるんだけど、 SF <> OFが < になるのかよくわからない
      // note)
      // SF: 計算結果が負のとき0になる。cmp 1 2 は 1-2をしているとのことなので、SFは0になる
      // OF: 符号あり整数の桁あふれが発生した場合に1。 1-2はマイナスになるけど、これは桁あふれ？
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
  }

  printf("  push rax\n");
}