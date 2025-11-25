cmake_minimum_required(VERSION 3.22)
include(FetchContent)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Note: Make sure that the path to the sources does not contain symlinks to avoid issues with vcpkg.
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "vcpkg toolchain file")
endif()

project("vtm")
# project("term")
# project("calc")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the build type to Release if none is specified.
if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Disable manifest embedding for the windows builds.
    # Reason: Anti-virus program (Windows Defender) may lock and scan `vtm.exe` file before embedding the manifest.
    #         mt.exe: general error c101008d: Failed to write the updated manifest to the resource of file...
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    # /EHsc             Tells the compiler that exceptions can only occur at a throw statement or at a function call.
    # /bigobj           Our event model spawns a large number of objects. By default, an object file can hold up to 65,279 (almost 2^16) addressable sections. This limit applies no matter which target platform is specified. /bigobj increases that address capacity to 4,294,967,296 (2^32).
    # /utf-8            All literals in our source code are in UTF-8 format.
    # /Zc:preprocessor  Enables a new, modern preprocessor that is fully conformant to C99, C++11, and later standards.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /EHsc /bigobj /utf-8 /Zc:preprocessor")
    set(WIN32_RESOURCES ".resources/images/vtm.rc")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -pthread")
    # Uncomment for static build
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libstdc++")
endif()

add_executable(vtm "src/vtm.cpp" ${WIN32_RESOURCES})
# add_executable(term "src/netxs/apps/term.cpp")
# add_executable(calc "src/netxs/apps/calc.cpp")

find_package(Freetype REQUIRED)
find_package(harfbuzz REQUIRED)
find_package(Lua      REQUIRED)

add_library(Lua::Lua INTERFACE IMPORTED)
set_target_properties(Lua::Lua PROPERTIES INTERFACE_LINK_LIBRARIES ${LUA_LIBRARIES})

target_include_directories(vtm PRIVATE ${FREETYPE_INCLUDE_DIRS}
                                       ${HARFBUZZ_INCLUDE_DIRS}
                                       ${LUA_INCLUDE_DIR})

target_link_libraries(vtm PRIVATE Lua::Lua Freetype::Freetype harfbuzz::harfbuzz)
# target_link_libraries(term PRIVATE Lua::Lua Freetype::Freetype harfbuzz::harfbuzz)
# target_link_libraries(calc PRIVATE Lua::Lua Freetype::Freetype harfbuzz::harfbuzz)

if(NOT WIN32)
    install(TARGETS vtm DESTINATION bin)
endif()
