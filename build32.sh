#!/bin/sh

# Make sure you have additional cross-compilation libraries installed.
#   E.g.: sudo apt install g++-multilib
./cleanup.sh
cmake ./src -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-static -pthread -s -m32"
cmake --build .
