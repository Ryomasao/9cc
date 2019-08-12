# 目的
9ccのカレントディレクトリが肥大化するので、汎用的な機能は外に切り出す。  
ライブラリとして作成したいんだけど、運用方法がよくわかってない。 

## 運用方法
ひとまず、以下の方針としている。

*** ex) stack機能の場合 ***

* 9ccに提供する機能は`/lib/stack`配下に`stack.h`を作成する
* 9ccのMakefileでincludeするヘッダに上記ディレクトリを追加
* 9ccのMakefileでsourceにするヘッダに`stack.o`を追加

```
# ヘッダも
LIBHEADER=./lib/stack
# オブジェクトも直接書いてる
LIBOBJECT=./lib/stack/stack.o

CFLAGS=-std=c11 -g -static -I$(LIBHEADER)

# 省略・・・
9cc: $(OBJS) $(LIBOBJECT)
			$(CC) -o 9cc $(OBJS) $(LIBOBJECT) $(LDFLAGS) 
clean:
	rm -f 9cc *.o *~ tmp* ./lib/stack/*.o
```

## テスト  
ライブラリ単体だとテストできないので、`driver.c`をつくってライブラリのテストを行うようにしとく。


