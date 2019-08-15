FROM alpine:3.9.4
RUN apk --update --no-cache add \
# コンパイラ
gcc \ 
# ビルドツール
make \
# リンカとかを含むさまざまなツール
binutils \
# Cのライブラリ？
libc-dev \
# テスト用のシェルスクリプトはbashなので
bash \
# デバッカ
gdb