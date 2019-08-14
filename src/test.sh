#!/bin/bash
try() {
  expected="$1"
  input="$2"
  mode="$3"

  # 引数の数$# が3なら、mode付きで実行
  if [ $# -eq 3 ]; then
    ./9cc "$input" "$mode" > tmp.s
  else
    ./9cc "$input" > tmp.s
  fi

  gcc -o tmp tmp.s
  ./tmp
  actual="$?"
  echo $actual
  echo $expected

  if [ "$actual" = "$expected" ]; then
    echo "$input ====> $actual"
  else
    echo "$expected exptected, but got $actual"
    exit 1
  fi
}

echo "### 加算・減算・乗算・徐算・()の優先順位・単項演算子"
try 3 "(1 + 2) * 2 / 2 + 1 - 1 + -1 + +1;"
echo "### <の確認"
try 1 "0 < 1;"
try 0 "1 < 1;"
echo "### <=の確認"
try 1 "1 <= 1;"
try 1 "1 <= 2;"
try 0 "1 <= 0;"
echo "### >の確認"
try 1 "1 > 0;"
try 0 "1 > 1;"
echo "### >=の確認"
try 1 "1 >= 1;"
try 1 "2 >= 1;"
try 0 "0 >= 1;"
echo "### == !=の確認"
try 1 "1 == 1;"
try 1 "1 != 2;"
echo "変数割り当て"
try 5 "amazing=2;number=amazing;1+2*number;"
echo "if文"
try 3 "../code/test/if.c" "true"
echo "ifelse文"
try 8 "../code/test/ifelse.c" "true"
echo "while文"
try 3 "../code/test/while.c" "true"
echo "for文"
try 3 "../code/test/for.c" "true"
echo "block文"
try 4 "../code/test/block.c" "true"
echo OK