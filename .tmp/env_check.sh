#!/usr/bin/env bash
set -euo pipefail
echo "MSYSTEM=${MSYSTEM:-<unset>}"
echo "MSYSTEM_PREFIX=${MSYSTEM_PREFIX:-<unset>}"
echo "PATH head: $(echo "$PATH" | tr ':' '\n' | head -3 | tr '\n' ':' )"
which gcc
which g++
which cmake
which ninja
gcc -dumpmachine
gcc --version | head -1
cmake --version | head -1
ninja --version
