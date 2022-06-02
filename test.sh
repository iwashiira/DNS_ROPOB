#!/bin/bash
noassert() {
  expected="$1"

  ./dns_ropob
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$actual"
  else
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

assert() {
  expected="$1"
  input="$2"

  ./dns_ropob "$input"
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}
noassert 1
assert 2 "test"
assert 2 "test.cpp"
assert 3 "aaaaaaaaaaaaaaaaaaaaaaaa.c"
assert 4 "aaaaaaaaaaaaaaaaaaaaaaa.c"

echo OK
