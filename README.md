# C コンパイラ作成入門をやってみた

https://www.sigbus.info/compilerbook

## 準備

Linux 環境でやったほがいいとのことなので docker を使う。
docker を久しぶりさわっていろいろ忘れてた。

### alpine を使う

C をコンパイルするだけであれば、docker 直接でよかったかも。
でも docker-compose が便利な気がするので、dokcer-compose を使う。
`docker-compose.yml`を適当に作った。

#### コンテナ起動

とりあえず、alpine で ash を実行するといい。

```sh
$ docker-compose run  --rm [サービス名] ash
```

以降は、全部メモ。

`--rm`オプションをつけると、コンテナ停止後に削除される。

#### 参考:コンテナ削除

`--rm`をつけ忘れて、コンテナを削除する場合のメモ

```
$ docker rm  [コンテナID]
```

不要なものを全部削除

```
$ docker system prune
```

### Dockerfile 作成

```
FROM alpine:3.9.4
RUN apk --update --no-cache add gcc make binutils libc-dev bash gdb
```

説明では、`libc-dev`ではなくって、`libc6-dev`
Alpine で見つからなかったので`libc-dev`にする。

あとでユニットテストを`bash`で書くのと、`gdb`でレジスタの中味とかを見たいので、いれとく。

次に、dokcer-compose.yml を作成する。

```
version: '3'
services:
  main:
    build: .
    volumes:
      - ./:/data
```

```sh
$ docker-compose run  --rm main ash
```

#### 01-first-compile

整数渡すことだけできるコンパイラを作成

```c
  // C言語で、とある言語で書かれたファイルを読み込んで
  // アセンブリ言語に変換するイメージ

  // ここでは単純な整数を引数にとり、それをreturnするだけのもの
  // raxは関数の返り値を格納するレジスタ

  // 直接機械語にコンパイルするかと思ったけれども、アセンブリにするんだね

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  printf("  mov rax, %d\n", atoi(argv[1]));
  printf("  ret \n");


  // gcc main main.c
  // ./main 123 > test.s
  // アセンブリを機械語にする。
  // 面白いのは、このアセンブリが123をreturnするプログラムってところ
  // gcc test test.s
  // ./test
  // echo $?
```

#### 02-test と 02-test-make

テスト用シェルを作成する

<b>思い出す</b>

```sh
# 変数を適当に宣言して、引数をセット
varibable=$1
# 変数を参照するときは`$`をつけてアクセスする
echo $variable
# これもいける
echo "テンプレートリテラル$variable的な？"
# 文字列は変数を展開しないんであれば""をつける必要もない？
echo OK
```

なつかしの`make`

`Makefile`を作成して、これを書くだけで

```
9cc: 9cc.c
```

※ 最初 9cc: main.c としてたんだけどコンパイルの 2 回目以降に`Nothing to be done`ってなっちゃう。
なので、ファイル名を変更した。オブジェクトの名前とソースって合わせる必要があるのかしら。

これと同じ意味になる

```sh
$ gcc -o 9cc 9cc.c
```

#### 03-basic-calc

簡単な計算を行えるようになった！

`strtol`がややこしい、というより C のポインタまわりはややこしい

```c
  char *p = argv[1];

  // pは文字列の先頭アドレスを格納している変数
  // もう一回書く。アドレスを格納している変数だ
  // &p はその変数のアドレス
  // strtolは文字列のアドレスを受け取って、10進数の値を先頭から探す
  // 10進数以外の値が見つかったら、その文字列のアドレスをpに格納して返す
  // アドレスを書き換えたいから、pそのもののアドレスを渡すんだね
  printf(" mov rax, %ld\n", strtol(p, &p, 10));

  // pは何を指しているかdebug
  //printf("remain %s\n", p);

```

#### 03-token

<b>構造体をおもいだす</b>

```c

// 基本は構造体タグを定義して
struct member {
  char name[10];
  int age;
}

int main() {
  // 構造体タグ + struct 変数名で使える
  member struct ayane;
  ayane.age = 24;
}

// 面倒なのでこれでいい
typedef struct {
  char name[10];
  int age;
} member

int main() {
  member ayane;
  ayane.age = 24;
}

```

### そういえば while(\*p) って

文字列走査の`while(*p)`って p のどの状態を falsy として扱ってるんだろ。
https://www.grapecity.com/developer/support/powernews/column/clang/028/page02.htm
こちらに終端に NULL(\0)が置かれるって書いてあった。

試しにのぞいてみる。

16 進数で参照してみると、こんな感じに

```c
char *p = argv[1];
printf("char: %x\n", p[0]);
printf("char: %x\n", p[1]);
printf("char: %x\n", p[2]);
```

```sh
$ ./9cc ab
char: 61
char: 62
char: 0
```

### token の内容

こんな感じになってる

```c
  // ex)123+45　
  // 123 token[0]
  // +   token[1]
  // 45  token[2]
```

### トークナイザ

#### 05-足し算・引き算

シンプルな構文木で考える。

```
expr = num ("+" num | "-" num)*
```

これをコードで構文木にしようとするとこんな感じにかける

```c
Node *expr() {
  // numから始まるので、まずはnumをとってくる
  // 関数numをみる
  Node *node = num();

  // 関数numの中でposをカウントアップしてる
  // consumeはposをもとにtokenの値を引数と比較するヘルパ
  // consumeの中でもposをインクリメントするから注意
  // やりたいことは、1+2+3-4を走査していって、
  // こういうノードをつくること
  //      -
  //    +    4
  //   +  3
  //  1 2
  //
  // 1・2・+すべてnode
  //    +  ←ty
  //  1   2
  // ↑lhs  ↑rhs
  // そして↑の構成全体も1nodeになる


  for(;;) {
    if(consume('+'))
      node = new_node('+', node, num());
    else if(consume('-'))
      node = new_node('-', node, num());
    else
      return node;
  }
}

Node *num() {
  // posはグローバル変数、初期値は当然0
  //いろんな関数で値を更新するから半端なくわかりにくい
  // 数字であれば、数字のノードを返却
  if(tokens[pos].ty == TK_NUM)
    // 値を設定してposインクリメント。これもわかりにくい
    return new_node_num(tokens[pos++].val);

  error_at(tokens[pos].input, "数値でも()でもないトークンです");
}
```

#### 06-掛け算・割り算

このシンプルさは神の所業

```
expr = mul ("+" mul | "-" mul)*
mul = num("*" num | "/" num)*
```

1 + 2 \* 3 の場合、以下に構文木になる

```
   +
1    *
    2  3
```

#### 07-括弧による優先順位

強い

```
expr = mul ("+" mul | "-" mul)*
mul = term("*" term | "/" term)*
term = num | "(" expr ")"
```

(1 + 2) \* 3 の場合、以下の構文木になる

```
     *
  +    3
1  2
```

06 の掛け算のときと構文木がかわることに気づく。

1 + 2 \* 3

1. expr→mult→term→ で node 1 を返す
1. 次は+なので expr で 左辺 1 + 右辺 mul の node をつくる
1. mul で 2 を取得するが、_なので、左辺 2 _ 右辺 3 の node を mul がつくり return
1. 結果、左辺 1 + 右辺が ↑ で return された node になる

(1 + 2) \* 3

1. expr1→mult→term→ で(を発見し、再度 expr2
1. expr2 で mult→term で node 1 を返す
1. expr2 で次が+なので, 左辺 1 + 右辺 mul2-1 を実行、結果 node 2 が返る
   ※2 の次が)になっているところがポイント？
1. expr2 は結果、 1+2 の node を返す

##### 08-unary

単項演算子 +3 or -3 とか。
-3 はわかるけど+3 は使わない。-があるから、+を追加したとのこと。

単項演算子を加味すると以下の通りになる

```
expr = mul ("+" mul | "-" mul)*
mul = unary("*" unary | "/" unary)*
unary = ("+" | "-") ? term
term = num | "(" expr ")"
```

これ以降は、コードの方に書いてた。

##### すごいうろ覚えな構造体

構造体をコピーしたい場合、単純にこうしてた

```
  Node *copiedNode = targetNode
```

これは、構造体のアドレスをコピーしているので、参照している構造体は`copied`も`target`も一緒。
immutable なものしたいぜ！ってときは、メモリを確保する必要がある。
immutable じゃなくて単純にアドレスを取っておきたいのであればこれでもいいはず。
その場合、`memcpy`を使うよりも以下のようにするといいとのこと。

```
  Node copiedNode = *targetNode
```

とはいえ shallowCopy とのことなので、構造体にポインタがある場合は個別に deepCopy するために関数を作る必要があるとかないとか。

##### ちょっと休憩、ローカル変数の場合のノード

このケースを考えてみると

```
a = 1 + 2
```

構文木はこうなるね

```
 =
a  +
  1  2
```

変数割り当てのアセンブリを、`lvar.s`に書いた

##### gdb メモ

https://sites.google.com/site/isutbe2018/zi-liaoasm/gdb
http://higepon.hatenablog.com/entry/20091026/1256561135

```
# gdb ./lvar
(gdb) b main   # brackpointをmainにセット
(gdb) run      # 実行
(gdb) s        # step実行
(gdb) i r rax  # レジスタraxの値を参照
(gdb) quit     # 終了
```

rsp が 16 の倍数になるように調整する途中の例

```
.intel_syntax noprefix
.global main

main:
  mov rax, rsp
  mov rdi, 16
  cqo
  div rdi
  mov rax, rdx
  # raxの値が0以外であれば、 sub rsp, 8 or pushとか
  ret
```

##### 14 &と\*によるアドレス演算子

ちょっと時間が空いたけど、アドレスのことが気になったので再開。

これができるようになる。

```c
main()
{
  a = 10;
  b = 11;
  c = 12;
  d = &b - 8;
  // 変数cの値12が返却される
  return *d;
}
```

`*`は乗算で使ってるから、なにか考える必要があるかなと思ったけど、`unary`項に書くだけでいいんだなぁ。
BNF で構文規則を考えるのがやっぱり天才的な気がする。

#### vsocode setting

```json
{
  "java.errors.incompleteClasspath.severity": "ignore",
  "emmet.includeLanguages": {
    "blade": "html",
    "javascript": "javascriptreact"
  },
  "extensions.ignoreRecommendations": false,
  "emmet.syntaxProfiles": {
    "vue-html": "html",
    "vue": "html"
  },
  "workbench.startupEditor": "newUntitledFile",
  "workbench.editor.showTabs": false,
  "editor.tabSize": 2,
  "window.zoomLevel": 0,
  "editor.formatOnSave": true,
  // JavaScriptでのみ保存時フォーマットを有効化
  "[javascript]": {
    "editor.formatOnSave": true
  },
  "[typescript]": {
    "editor.formatOnSave": true
  },
  "[typescriptreact]": {
    "editor.formatOnSave": true
  },
  // prettierのシングルクオーテーションオプションをon
  "prettier.semi": false,
  "prettier.singleQuote": true,
  "prettier.trailingComma": "es5",
  "terminal.integrated.shell.osx": "/usr/local/bin/bash",
  "vetur.validation.template": false,
  "prettier.eslintIntegration": true,
  //es-lint
  // https://qiita.com/madono/items/a134e904e891c5cb1d20
  "eslint.validate": [
    "javascript",
    "javascriptreact",
    { "language": "typescript", "autoFix": true },
    { "language": "typescriptreact", "autoFix": true }
  ],
  "C_Cpp.updateChannel": "Insiders",
  "vsicons.dontShowNewVersionMessage": true,
  "[markdown]": {
    "files.trimTrailingWhitespace": false
  }
}
```
