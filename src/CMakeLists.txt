cmake_minimum_required (VERSION 3.8)

project ("vtmd")
project ("vtm")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable (vtmd "vtmd.cpp")
add_executable (vtm  "vtm.cpp")
