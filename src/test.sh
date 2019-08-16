#!/bin/bash
try() {
  expected="$1"
  input="$2"
  mode="$3"
  func="$4"

  # modeがある場合、ファイルから読み込めるようにオプションを指定
  if [ -n "$3" ]; then
    ./9cc "$input" "$mode" > tmp.s
  else
    ./9cc "$input" > tmp.s
  fi

  # funcがある場合、mock用のfuncをリンクする
  if [ -n "$4" ]; then
    # make -f でsubdirのMakefileを指定したかったんだけど
    # カレントディレクトリと衝突？する
    # なので、ディレクトリを移動
    cd ./lib/mock
    make
    cd ../../
    # リンク
    gcc -g -o tmp tmp.s ./lib/mock/mock.o
  else
    gcc -g -o tmp tmp.s
  fi

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

echo "### main定義 + return文"
try 255 "../code/test/01_basic.c" true
echo "### 変数割り当て"
try 5 "../code/test/02_var.c" true
echo "### 加算・減算・乗算・徐算・()の優先順位・単項演算子"
try 3 "../code/test/03_calc.c" true
echo "### 比較演算子"
try 4 "../code/test/04_comparison.c" true
echo "### == !=="
try 2 "../code/test/05_equall.c" true
echo "if文"
try 3 "../code/test/06_if.c" "true"
echo "ifelse文"
try 8 "../code/test/07_ifelse.c" "true"
echo "while文"
try 3 "../code/test/08_while.c" "true"
echo "for文"
try 3 "../code/test/09_for.c" "true"
echo "block文"
try 4 "../code/test/10_block.c" "true"
echo "function文"
try 11 "../code/test/11_func.c" "true" "true"
echo "function定義"
try 12 "../code/test/12_func_dif.c" "true"
echo OK