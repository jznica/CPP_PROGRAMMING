#!/usr/bin/env bash
# Task B3 - cost of virtual vs switch vs variant (+dynamic_cast, templates) and of exceptions
cd "$(dirname "$0")" && source ../common.sh
fresh B3_dispatch
cap B3_dispatch "$CXX -O2 bench_dispatch.cpp -o bench_dispatch"
cap B3_dispatch "lscpu | grep -E 'Model name|^CPU\(s\)'; g++ --version | head -1"
for i in 1 2 3; do cap B3_dispatch "./bench_dispatch"; done
fresh B3_exceptions
cap B3_exceptions "$CXX -O2 bench_exceptions.cpp -o bench_exceptions"
for i in 1 2 3; do cap B3_exceptions "./bench_exceptions"; done
