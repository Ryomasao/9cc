# c11:Cの最新規格であるC11で書かれたソースコードということみたい
# -g: デバックオプション　デバッグするときにつけるといいみたい
# -static: コンパイル時に、ライブラリをバイナリに含めるかどうか、staticは含める
# 含めないパターンをダイナミックリンクと呼ぶみたい。その場合、実行環境に依存するね
# CFLAGSの前後でコンパイルした後のサイズを比較するとだいぶ違うことがわかる
# root        200112 Jun 30 07:41 9cc
# root         19504 Jun 30 07:42 9cc
# 

# ひとまず直接書く
# /lib/README.mdを参照
LIBHEADER=./lib/stack
LIBOBJECT=./lib/stack/stack.o 

CFLAGS=-std=c11 -g -static -I$(LIBHEADER)
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
9cc: $(OBJS) $(LIBOBJECT)
			$(CC) -o 9cc $(OBJS) $(LIBOBJECT) $(LDFLAGS) 
$(OBJS): 9cc.h
test: 9cc
	./test.sh
clean:
	rm -f 9cc *.o *~ tmp* $(LIBOBJECT)

.PHONY: test clean