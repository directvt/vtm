cmake_minimum_required(VERSION 3.22)
include(FetchContent)

project("vtm" CXX)
# project("term")
# project("calc")

set(LUA_MIN_VER "5.4")
set(HB_MIN_VER "12.2.0")
set(FT_MIN_VER "2.13.2")

set(FT_VER "2.14.3")
set(FT_TAG "VER-2-14-3")
set(HB_VER "14.2.1")
set(CMAKE_POLICY_VERSION_MINIMUM "3.5" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the build type to Release if none is specified.
if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

add_executable(vtm "src/vtm.cpp" $ENV{WIN32_RESOURCES})
# add_executable(term "src/netxs/apps/term.cpp")
# add_executable(calc "src/netxs/apps/calc.cpp")

if(NOT WIN32 AND "$ENV{LINUX_CXX_FLAGS}" STREQUAL "") # If CMakeSettings.json is not used (MS Visual Studio).
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -pthread")
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(vtm PRIVATE -fconstexpr-steps=100000000)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(vtm PRIVATE /constexpr:steps10000000 /EHsc /bigobj /utf-8 /Zc:preprocessor)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(vtm PRIVATE -fconstexpr-loop-limit=100000000)
endif()
if(MSVC)
    set(W0_FLAG "/W0")
else()
    set(W0_FLAG "-w")
endif()

set(ALL_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_CXX_FLAGS_RELEASE}")
string(FIND "${ALL_CXX_FLAGS}" "-static" HAVE_UNIX_STATIC)
string(FIND "${ALL_CXX_FLAGS}" "/MT" HAVE_WIN_STATIC)

if(NOT HAVE_UNIX_STATIC EQUAL -1 OR NOT HAVE_WIN_STATIC EQUAL -1)
    message(STATUS "Full-Static Mode: skipping system package search for FreeType/HarfBuzz/Lua/LunaSVG/STB.")
else()
    find_package(Lua ${LUA_MIN_VER}            QUIET)
    find_package(lunasvg                       QUIET)
    find_package(Stb                           QUIET)
    find_package(Freetype ${FT_MIN_VER} CONFIG QUIET)
    find_package(harfbuzz ${HB_MIN_VER} CONFIG QUIET)
    if(NOT Freetype_FOUND)
        find_package(Freetype ${FT_MIN_VER} QUIET)
    endif()
    if(NOT harfbuzz_FOUND)
        find_package(harfbuzz ${HB_MIN_VER} QUIET)
    endif()
endif()

if(Lua_FOUND)
    message(STATUS "Suitable Lua found via find_package: ${LUA_VERSION_STRING}")
    if(NOT TARGET Lua::Lua)
        add_library(Lua::Lua INTERFACE IMPORTED)
        set_target_properties(Lua::Lua PROPERTIES INTERFACE_LINK_LIBRARIES "${LUA_LIBRARIES}")
        set(LUA_INCLUDE_DIR "${LUA_INCLUDE_DIR}")
    endif()
else()
    message(STATUS "Lua ${LUA_MIN_VER} NOT found in the system. Downloading clean source from lua.org...")
    if(POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif()
    FetchContent_Declare(lua_source
                         URL           https://www.lua.org/ftp/lua-5.5.0.tar.gz
                         URL_HASH      SHA256=57ccc32bbbd005cab75bcc52444052535af691789dba2b9016d5c50640d68b3d
                         SOURCE_SUBDIR "DONOTEXIST")
    FetchContent_MakeAvailable(lua_source)
    set(LUA_CUSTOM_CMAKE "${lua_source_BINARY_DIR}/CMakeLists.txt")
    file(WRITE "${LUA_CUSTOM_CMAKE}" [====[
        cmake_minimum_required(VERSION 3.22)
        project(lua C)
        file(GLOB LUA_CORE_SRC "src/*.c")
        list(REMOVE_ITEM LUA_CORE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/lua.c" "${CMAKE_CURRENT_SOURCE_DIR}/src/luac.c")
        add_library(lua STATIC ${LUA_CORE_SRC})
        target_include_directories(lua PUBLIC src)
        target_compile_definitions(lua PRIVATE LUA_USE_C89)
    ]====])
    file(COPY "${LUA_CUSTOM_CMAKE}" DESTINATION "${lua_source_SOURCE_DIR}")
    add_subdirectory("${lua_source_SOURCE_DIR}" "${lua_source_BINARY_DIR}")

    if(NOT TARGET Lua::Lua)
        add_library(Lua::Lua ALIAS lua)
    endif()
    set(LUA_INCLUDE_DIR "${lua_source_SOURCE_DIR}/src")
endif()

if(Freetype_FOUND AND harfbuzz_FOUND)
    message(STATUS "Suitable FreeType found via find_package: ${FREETYPE_VERSION_STRING}")
    message(STATUS "Suitable HarfBuzz found via find_package: ${harfbuzz_VERSION}")
else()
    message(STATUS "FreeType or HarfBuzz not found/outdated (FT ${FT_MIN_VER} and HB ${HB_MIN_VER} required). Downloading both via FetchContent...")

    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

    set(FT_DISABLE_ZLIB     ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_BZIP2    ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_PNG      ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_BROTLI   ON CACHE BOOL "" FORCE)
    FetchContent_Declare(freetype
                         GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
                         GIT_TAG        ${FT_TAG}
                         EXCLUDE_FROM_ALL)
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
                         GIT_TAG        ${HB_VER}
                         EXCLUDE_FROM_ALL)
    FetchContent_MakeAvailable(harfbuzz)
    if(NOT TARGET harfbuzz::harfbuzz)
        add_library(harfbuzz::harfbuzz ALIAS harfbuzz)
    endif()

    set(FREETYPE_INCLUDE_DIRS "${freetype_SOURCE_DIR}/include" "${freetype_BINARY_DIR}/include")
    set(HARFBUZZ_INCLUDE_DIRS "${harfbuzz_SOURCE_DIR}/src"     "${harfbuzz_BINARY_DIR}/src")
    # Suppress warnings.
    target_compile_options(harfbuzz PRIVATE ${W0_FLAG})
endif()

if(lunasvg_FOUND)
    message(STATUS "LunaSVG found via find_package.")
else()
    message(STATUS "LunaSVG not found. Downloading via FetchContent...")
    set(LUNASVG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(LUNASVG_BUILD_TOOLS    OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(lunasvg
                         GIT_REPOSITORY https://github.com/sammycage/lunasvg.git
                         GIT_TAG        master)
    FetchContent_MakeAvailable(lunasvg)
    # Suppress warnings.
    target_compile_options(lunasvg PRIVATE ${W0_FLAG})
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

target_include_directories(vtm PRIVATE ${FREETYPE_INCLUDE_DIRS}
                                       ${HARFBUZZ_INCLUDE_DIRS}
                                       ${LUA_INCLUDE_DIR})

target_link_libraries(vtm PRIVATE Lua::Lua Freetype::Freetype harfbuzz::harfbuzz lunasvg::lunasvg stb::stb)
# target_link_libraries(term PRIVATE Lua::Lua Freetype::Freetype harfbuzz::harfbuzz lunasvg::lunasvg stb::stb)
# target_link_libraries(calc PRIVATE Lua::Lua Freetype::Freetype harfbuzz::harfbuzz lunasvg::lunasvg stb::stb)

if(MSVC)
    target_compile_definitions(vtm PRIVATE _CRT_SECURE_NO_WARNINGS)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(vtm PRIVATE -Wno-deprecated-declarations)
endif()

if(NOT WIN32)
    install(TARGETS vtm DESTINATION bin)
endif()
