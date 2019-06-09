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
    echo "$input => $actual"
  else
    echo "$expected exptected, but got $actual"
    exit 1
  fi
}

try 41 " 12 + 34 - 5 "
echo OK