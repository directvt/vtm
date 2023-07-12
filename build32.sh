#!/bin/sh

# Make sure you have additional cross-compilation libraries installed.
#   E.g.: sudo apt install gcc-i686-linux-gnu g++-i686-linux-gnu
#         sudo apt install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf

./cleanup.sh

# Compile the x86 target binary:
#cmake ./src -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER="/bin/i686-linux-gnu-g++" -DCMAKE_CXX_FLAGS="-static -pthread -s" && cmake --build .

# Compile the ARM32 target binary
cmake ./src -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER="/bin/arm-linux-gnueabihf-g++" -DCMAKE_CXX_FLAGS="-static -pthread -s" && cmake --build .
