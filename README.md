
これをちょっとやってみたい！リポジトリ。

https://www.sigbus.info/compilerbook


## 準備
Linux環境でやったほがいいとのことなのでdockerを使う。
dockerを久しぶりさわっていろいろ忘れてた。

### alpineを使う
Cをコンパイルするだけであれば、docker直接でよかったかな。でもdocker-composeが便利な気がするので、dokcer-composeに。
`docker-compose.yml`を適当に作った。


#### コンテナ起動

とりあえず、alpineでashを実行するといい。
```sh
$ docker-compose run  --rm [サービス名] ash
```

`--rm`オプションをつけると、コンテナ停止後に削除される。

#### コンテナ削除

```
$ docker rm  [コンテナID]
```

```
$ docker system prune
```


### Dockerfile作成
```
FROM alpine:3.9.4
RUN apk --update --no-cache add gcc make binutils libc-dev bash
```

それぞれ何かがあんまわかってない。

説明では、`libc-dev`ではなくって、`libc6-dev`
Alpineで見つからなかったので。

あとでユニットテストを`bash`で書くので、これもいれとく。


なんとなく、上で書いてみて、お試しににビルド。
```sh
$ docker build .
```

次に、dokcer-composeで使用するようにする。

`build`で`Dockerfile`の場所を指定すればよかったんだよね
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

#### 02-testと02-test-make
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

※ 最初 9cc: main.c としてたんだけどコンパイルの2回目以降に`Nothing to be done`ってなっちゃう。
なので、ファイル名を変更した。オブジェクトの名前とソースって合わせる必要があるのかしら。

これと同じ意味になる
```sh
$ gcc -o 9cc 9cc.c
```

#### 03-basic-calc
簡単な計算を行えるようになった！

`strtol`がややこしい、というよりCのポインタまわりはややこしい

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

### そういえば while(*p) って

文字列走査の`while(*p)`ってpのどの状態をfalsyとして扱ってるんだろ。
https://www.grapecity.com/developer/support/powernews/column/clang/028/page02.htm
こちらに終端にNULL(\0)が置かれるって書いてあった。

試しにのぞいてみる。

16進数で参照してみると、こんな感じに
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


### tokenの内容  
こんな感じになってる
```c
  // ex)123+45　
  // 123 token[0]
  // +   token[1]
  // 45  token[2]
```

### トークナイザ

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















