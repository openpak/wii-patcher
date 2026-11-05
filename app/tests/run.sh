#!/bin/sh
# Host test of the network-settings logic (no console needed).
set -e
cd "$(dirname "$0")"
out=${TMPDIR:-.}/patcher_test
g++ -std=c++17 -Wall -o "$out" patcher_test.cpp ../source/patcher.cpp ../source/netconfig.cpp
"$out"
rm -f "$out"
