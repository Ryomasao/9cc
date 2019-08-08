#!/bin/bash
# 変数割り当ての確認をするためのアセンブリ確認用
target="$1"

if [ -n "$target" ]; then
echo "exec $target"
gcc -o "$target" "$target".s 
./$target
else
gcc -o lvar lvar.s
./lvar
fi

echo $?