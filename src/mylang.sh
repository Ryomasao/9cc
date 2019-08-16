#!/bin/bash
# mylang.cをコンパイルして実行するだけ
make
./9cc "../code/mylang.c" true > mylang.s
gcc -g -o mylang mylang.s
./mylang
echo $?