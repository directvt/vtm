cmake_minimum_required (VERSION 3.8)

project ("vtmd")
project ("vtm")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/vtm.conf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable (vtmd "vtmd.cpp" "terminal.h")
add_executable (vtm  "vtm.cpp")

target_include_directories(vtmd PRIVATE "include\\netxs")
target_include_directories(vtm  PRIVATE "include\\netxs")
