#!/bin/bash
# 変数割り当ての確認をするためのアセンブリ確認用
gcc -o lvar lvar.s
./lvar
echo $?