cmake_minimum_required(VERSION 3.22)
include(FetchContent)

project("vtm" CXX)
# project("term")
# project("calc")

set(CMAKE_POLICY_VERSION_MINIMUM "3.5" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the build type to Release if none is specified.
if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

if(NOT WIN32 AND "$ENV{LINUX_CXX_FLAGS}" STREQUAL "") # If CMakeSettings.json is not used (MS Visual Studio).
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -pthread")
endif()

add_executable(vtm "src/vtm.cpp" $ENV{WIN32_RESOURCES})
# add_executable(term "src/netxs/apps/term.cpp")
# add_executable(calc "src/netxs/apps/calc.cpp")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(vtm PRIVATE -fconstexpr-steps=100000000)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(vtm PRIVATE /constexpr:steps10000000)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(vtm PRIVATE -fconstexpr-loop-limit=100000000)
endif()

find_package(Lua 5.4         REQUIRED)
find_package(Freetype 2.13.2 QUIET)
find_package(harfbuzz 12.2.0 QUIET)
find_package(lunasvg         QUIET)
find_package(Stb             QUIET)

if(Freetype_FOUND AND harfbuzz_FOUND)
    message(STATUS "Suitable FreeType found via find_package: ${FREETYPE_VERSION_STRING}")
    message(STATUS "Suitable HarfBuzz found via find_package: ${harfbuzz_VERSION}")
else()
    message(STATUS "FreeType or HarfBuzz not found/outdated (FT 2.13.2 and HB 12.2.0 required). Downloading both via FetchContent...")

    unset(Freetype_FOUND CACHE)
    unset(harfbuzz_FOUND CACHE)

    set(FT_WITH_ZLIB     OFF CACHE BOOL "" FORCE)
    set(FT_WITH_BZIP2    OFF CACHE BOOL "" FORCE)
    set(FT_WITH_PNG      OFF CACHE BOOL "" FORCE)
    set(FT_WITH_HARFBUZZ OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(freetype
                         GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
                         GIT_TAG        VER-2-13-2)
    FetchContent_MakeAvailable(freetype)
    if(NOT TARGET Freetype::Freetype)
        add_library(Freetype::Freetype ALIAS freetype)
    endif()

    set(HB_HAVE_FREETYPE      ON  CACHE BOOL "" FORCE)
    set(HB_BUILD_TESTS        OFF CACHE BOOL "" FORCE)
    set(HB_BUILD_UTILS        OFF CACHE BOOL "" FORCE)
    set(HB_HAVE_GOBJECT       OFF CACHE BOOL "" FORCE)
    set(HB_HAVE_INTROSPECTION OFF CACHE BOOL "" FORCE)
    set(HB_BUILD_DOCS         OFF CACHE BOOL "" FORCE)
    set(HB_BUILD_SUBSET       OFF CACHE BOOL "" FORCE)
    set(Freetype_DIR                   "${freetype_BINARY_DIR}"         CACHE PATH "" FORCE)
    set(FREETYPE_INCLUDE_DIR_ft2build  "${freetype_SOURCE_DIR}/include" CACHE PATH "" FORCE)
    set(FREETYPE_INCLUDE_DIR_freetype2 "${freetype_SOURCE_DIR}/include" CACHE PATH "" FORCE)
    FetchContent_Declare(harfbuzz
                         GIT_REPOSITORY https://github.com/harfbuzz/harfbuzz.git
                         GIT_TAG        12.2.0)
    FetchContent_MakeAvailable(harfbuzz)
    if(NOT TARGET harfbuzz::harfbuzz)
        add_library(harfbuzz::harfbuzz ALIAS harfbuzz)
    endif()

    set(FREETYPE_INCLUDE_DIRS "${freetype_SOURCE_DIR}/include" "${freetype_BINARY_DIR}/include")
    set(HARFBUZZ_INCLUDE_DIRS "${harfbuzz_SOURCE_DIR}/src"     "${harfbuzz_BINARY_DIR}/src")
endif()

if(lunasvg_FOUND)
    message(STATUS "LunaSVG found via find_package.")
else()
    message(STATUS "LunaSVG not found. Downloading via FetchContent...")
    set(LUNASVG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(LUNASVG_BUILD_TOOLS    OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(lunasvg
                         GIT_REPOSITORY https://github.com/sammycage/lunasvg.git
                         GIT_TAG        v3.5.0)
    FetchContent_MakeAvailable(lunasvg)
endif()

if(NOT TARGET lunasvg::lunasvg)
    add_library(lunasvg::lunasvg ALIAS lunasvg)
endif()

if(Stb_FOUND)
    message(STATUS "stb found via find_package.")
    if(NOT TARGET stb::stb)
        add_library(stb_interface INTERFACE)
        target_include_directories(stb_interface INTERFACE ${Stb_INCLUDE_DIR})
        add_library(stb::stb ALIAS stb_interface)
    endif()
else()
    message(STATUS "stb not found. Downloading via FetchContent...")
    FetchContent_Declare(stb
                         GIT_REPOSITORY https://github.com/nothings/stb
                         GIT_TAG        master)
    FetchContent_MakeAvailable(stb)
    add_library(stb_interface INTERFACE)
    target_include_directories(stb_interface INTERFACE ${stb_SOURCE_DIR})
    add_library(stb::stb ALIAS stb_interface)
endif()

add_library(Lua::Lua INTERFACE IMPORTED)
set_target_properties(Lua::Lua PROPERTIES INTERFACE_LINK_LIBRARIES "${LUA_LIBRARIES}")

target_include_directories(vtm PRIVATE ${FREETYPE_INCLUDE_DIRS}
                                       ${HARFBUZZ_INCLUDE_DIRS}
                                       ${LUA_INCLUDE_DIR})

target_link_libraries(vtm PRIVATE Lua::Lua Freetype::Freetype harfbuzz::harfbuzz lunasvg::lunasvg stb::stb)
# target_link_libraries(term PRIVATE Lua::Lua Freetype::Freetype harfbuzz::harfbuzz lunasvg::lunasvg stb::stb)
# target_link_libraries(calc PRIVATE Lua::Lua Freetype::Freetype harfbuzz::harfbuzz lunasvg::lunasvg stb::stb)

if(NOT WIN32)
    install(TARGETS vtm DESTINATION bin)
endif()
