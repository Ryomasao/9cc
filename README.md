
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
9cc: main.c
```

これと同じ意味になる
```sh
$ gcc -o 9cc main.c
```









