#include "9cc.h"

int main(int argc, char **argv) {
  if(argc > 3) {
    fprintf(stderr, "引数の数が正しくありません\n");
    return 1;
  }

  // ユーザー入力値をグローバル変数として取っておく
  // エラー関数用
  // TODO: ファイルから読み込む場合に対応してない
  user_input = argv[1];
  char input[MAX_LINE][MAX_COLUMN] = {};

  // modeに値があれば、argv1の値はファイルパスとして扱う
  // ファイルから読み込むようにしたけど、テスト実行時にはワンラインで引数としてコードを渡したい
  // 標準入力があるかどうかで判断したいところ
  if(argv[2]) {
    char *filePath = argv[1];
    if(!readFile(filePath, input))
      return -1;
  } else {
    // なければ、argv1はコードそのもの

    // やっぱり、char text[][]の扱いがよくわからない
    // input[0] =  argv[1]は、Arrayにassingnmentするなとコンパイラエラーになる
    // そもそも、左辺に設定できる値はmodifiableだけのものということらしい。つまりどういうことだろう。
    // https://stackoverflow.com/questions/37225244/error-assignment-to-expression-with-array-type-error-when-i-assign-a-struct-f

    // input[0][0] = argv[1]ならコンパイルは通る
    // ただ、argv[1]はcharへのポインタであり、input[0][0]はcharそのものだから、型が違うとwargningがでる

    // srcpyを使えばよさそう。値そのものをコピーする。
    // ポインタじゃなくって値そのもの。
    // man でみてみると第一引数はcharへのポインタになってる。つまり、input[0]はcharへのポインタだって考えていいのかな
    // https://stackoverflow.com/questions/28680557/c-assign-string-from-argv-to-char-array
    strcpy(input[0], argv[1]);
    // ファイル読み込みと同じでEOFを設定しとく
    input[1][0] = EOF;
  }

  // ローカル変数格納用の変数の初期設定
  // これにより、localsの先頭はゴミデータになっちゃうので微妙
  locals = calloc(1, sizeof(Lvar));
  locals->next = NULL;

  // トークナイズする
  tokenize(input);
  program();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");

  for(int i = 0; code[i]; i++) {
    // code[]はstmt()の結果のnodeを格納してる
    gen(code[i]);
  }

  return 1;
}

