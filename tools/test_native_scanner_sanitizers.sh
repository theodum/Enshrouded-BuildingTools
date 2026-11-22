#!/bin/sh
# Host-only tests of the production reader/preflight and original native bridge.
# No game process, game directory or save files are accessed.
set -eu
cd "$(dirname "$0")/.."
mkdir -p build
compiler=${HOSTCXX:-clang++}
for suite in native_snapshot native_calibration native_props native_prop_checks classic_composite classic_reader; do
    "$compiler" -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
        "tests/test_$suite.cpp" -o "build/scanner_asan_$suite"
    ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 "./build/scanner_asan_$suite"
done
