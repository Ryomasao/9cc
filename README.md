
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











