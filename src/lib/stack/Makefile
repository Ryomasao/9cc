CFLAGS=-std=c11 -g -static
# カレントディレクト？にある.cのファイルを対象にする
# wildcardはmakeが用意している機能
SRCS=$(wildcard *.c)
# 上記で取得したファイル名の拡張子をoにしたものを用意する
OBJS=$(SRCS:.c=.o)

# 結果的に以下のコマンドを実行することになるみたい
# オブジェクトファイルをつくって
# cc -c -o driver.o driver.c
# オブジェクトファイルをつくって
# cc -c -o stack.o stack.c
# リンクする
# cc -o stack.o driver.o
stack: $(OBJS)
			$(CC) -o stack $(OBJS) $(LDFLAGS)
$(OBJS): stack.h
test: stack
	./stack.sh	
clean:
	rm -f stack *.o *.out
.PHONY: test clean