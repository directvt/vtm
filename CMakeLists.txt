cmake_minimum_required(VERSION 3.22)

project("vtm")
# project("term")
# project("calc")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the build type to Release if none is specified.
if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")    # WIN32 and similar checks are soft-deprecated
    # Disable manifest embedding for the windows builds.
    # Reason: Anti-virus program (Windows Defender) may lock and scan `vtm.exe` file before embedding the manifest.
    #         mt.exe: general error c101008d: Failed to write the updated manifest to the resource of file...
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    # /EHsc    Tells the compiler that exceptions can only occur at a throw statement or at a function call.
    # /bigobj  Our event model spawns a large number of objects. By default, an object file can hold up to 65,279 (almost 2^16) addressable sections. This limit applies no matter which target platform is specified. /bigobj increases that address capacity to 4,294,967,296 (2^32).
    # /utf-8   All literals in our source code are in UTF-8 format.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /EHsc /bigobj /utf-8 /Zc:preprocessor")
    set(WIN32_RESOURCES ".resources/images/vtm.rc")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -pthread")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -DLUA_USE_POSIX")
    # Static linkage
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -O2 -pthread")
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -O2 -DLUA_USE_POSIX")
endif()

# Lua dependency
include(FetchContent)
FetchContent_Declare(lua
                     URL      https://www.lua.org/ftp/lua-5.4.7.tar.gz
                     URL_HASH SHA256=9fbf5e28ef86c69858f6d3d34eccc32e911c1a28b4120ff3e84aaa70cfbf1e30)
FetchContent_MakeAvailable(lua)
file(GLOB lua_src CONFIGURE_DEPENDS ${lua_SOURCE_DIR}/src/*.c)
list(REMOVE_ITEM lua_src ${lua_SOURCE_DIR}/src/lua.c ${lua_SOURCE_DIR}/src/luac.c)
add_library(lua ${lua_src})
target_include_directories(lua PUBLIC ${lua_SOURCE_DIR}/src)
target_sources(lua PRIVATE ${lua_src})

add_executable(vtm "src/vtm.cpp" ${WIN32_RESOURCES})
# add_executable(term "src/netxs/apps/term.cpp")
# add_executable(calc "src/netxs/apps/calc.cpp")

target_link_libraries(vtm PRIVATE lua)
# target_link_libraries(term PRIVATE lua)
# target_link_libraries(calc PRIVATE lua)

if(NOT WIN32)
    install(TARGETS vtm DESTINATION bin)
endif()
