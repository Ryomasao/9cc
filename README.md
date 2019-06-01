
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




