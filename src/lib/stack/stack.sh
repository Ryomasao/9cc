#!/bin/bash
try() {
  expected="$1"

  # テスト対象の実行
  ./stack

  # 返り値を格納する
  actual="$?"

  echo "acutual:$actual expected:$1"
  if [ "$actual" = "$expected" ]; then
    echo "OK!"
  else
    echo "NG!"
    exit 1
  fi
}

try 1