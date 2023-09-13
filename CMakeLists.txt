cmake_minimum_required (VERSION 3.8)

project ("vtm")
# project ("term")
# project ("calc")

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the build type to Release if none is specified.
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set (CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")    # WIN32 and similar checks are soft-deprecated
    # Disable manifest embedding for the windows builds.
    # Reason: Anti-virus program (Windows Defender) may lock and scan `vtm.exe` file before embedding the manifest.
    #         mt.exe: general error c101008d: Failed to write the updated manifest to the resource of file...
    #set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    # /EHsc    Tells the compiler that exceptions can only occur at a throw statement or at a function call.
    # /bigobj  Our event model spawns a large number of objects. By default, an object file can hold up to 65,279 (almost 2^16) addressable sections. This limit applies no matter which target platform is specified. /bigobj increases that address capacity to 4,294,967,296 (2^32).
    # /utf-8   All literals in our source code are in UTF-8 format.
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /bigobj /utf-8")
elseif (NOT CMAKE_SYSTEM_NAME MATCHES "Android")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    # Static linkage
    #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -pthread -s")
endif ()

add_executable (vtm "src/vtm.cpp")
# add_executable (term "src/netxs/apps/term.cpp")
# add_executable (calc "src/netxs/apps/calc.cpp")

if (NOT WIN32)
    install (TARGETS vtm  DESTINATION bin)
endif ()
