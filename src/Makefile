# c11:コンパイラの最適化のオプション？
# -g: デバックオプション　デバッグするときにつけるといいみたい
# -static: コンパイル時に、ライブラリをバイナリに含めるかどうか、staticは含める
# 含めないパターンをダイナミックリンクと呼ぶみたい。その場合、実行環境に依存するね
# CFLAGSの前後でコンパイルした後のサイズを比較するとだいぶ違うことがわかる
# root        200112 Jun 30 07:41 9cc
# root         19504 Jun 30 07:42 9cc
# 
CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
9cc: $(OBJS)
			$(CC) -o 9cc $(OBJS) $(LDFLAGS)
$(OBJS): 9cc.h
test: 9cc
	./test.sh
clean:
	rm -f 9cc *.o *~ tmp*