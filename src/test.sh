#!/bin/bash
try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
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
try 3 "../code/test/basic_calc.c"
#echo "### <の確認"
#try 1 "0 < 1;"
#try 0 "1 < 1;"
#echo "### <=の確認"
#try 1 "1 <= 1;"
#try 1 "1 <= 2;"
#try 0 "1 <= 0;"
#echo "### >の確認"
#try 1 "1 > 0;"
#try 0 "1 > 1;"
#echo "### >=の確認"
#try 1 "1 >= 1;"
#try 1 "2 >= 1;"
#try 0 "0 >= 1;"
#echo "### == !=の確認"
#try 1 "1 == 1;"
#try 1 "1 != 2;"
#echo "変数割り当て"
#try 5 "amazing=2;number=amazing;1+2*number;"
echo OK