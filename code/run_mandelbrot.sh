#!/bin/sh
# Builds mandelbrot_bench.cpp for SSE4.2, AVX2, and AVX-512, each with and without -fcx-limited-range,
# and runs every binary pinned on one core. Expects kyosu/include and eve/include beside this script.
set -e
cd "$(dirname "$0")"

CXX=${CXX:-g++}
CORE=${CORE:-3}

echo "$(nproc) threads, load $(cut -d' ' -f1-3 /proc/loadavg)"
grep -m1 "model name" /proc/cpuinfo
$CXX --version | head -1

for arch in x86-64-v2 x86-64-v3 x86-64-v4; do
  for flags in "" "-fcx-limited-range"; do
    out="bench_${arch}${flags:+_limited}"
    $CXX -std=c++20 -O3 -DNDEBUG -march=$arch $flags -Ikyosu/include -Ieve/include mandelbrot_bench.cpp -o "$out"
  done
done

for arch in x86-64-v2 x86-64-v3 x86-64-v4; do
  for suffix in "" "_limited"; do
    echo
    echo "== -march=$arch${suffix:+ -fcx-limited-range}"
    taskset -c "$CORE" "./bench_${arch}${suffix}" "${RUNS:-9}"
  done
done
