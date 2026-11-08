#!/bin/sh
# Builds and runs every program of snippets/, then reference_kernels.cpp for SSE4.2, AVX2, and AVX-512,
# with and without -fcx-limited-range. Expects kyosu/include, eve/include, and kumi/include beside this script.
set -e
cd "$(dirname "$0")"

CXX=${CXX:-g++}
CORE=${CORE:-3}

echo "load $(cut -d' ' -f1-3 /proc/loadavg)"
grep -m1 "model name" /proc/cpuinfo
$CXX --version | head -1

for f in snippets/*.cpp; do
  name=$(basename "$f" .cpp)
  case "$name" in
    kumi_*) includes="-Ikumi/include" ;;
    *)      includes="-Ikyosu/include -Ieve/include" ;;
  esac
  $CXX -std=c++20 -O2 -march=x86-64-v3 $includes "$f" -o "bin_$name"
  echo
  echo "== $name"
  "./bin_$name"
done

for arch in x86-64-v2 x86-64-v3 x86-64-v4; do
  for flags in "" "-fcx-limited-range"; do
    $CXX -std=c++20 -O3 -DNDEBUG -march=$arch $flags -Ikyosu/include -Ieve/include reference_kernels.cpp -o reference_kernels
    # In cache, a run lasts microseconds: 1001 runs let the clock of a powersave governor settle.
    for size in 4096:1001 1048576:101; do
      echo
      echo "== reference kernels, -march=$arch $flags"
      taskset -c "$CORE" ./reference_kernels "${size%%:*}" "${size##*:}"
    done
  done
done
