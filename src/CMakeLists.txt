cmake_minimum_required (VERSION 3.8)

project ("vtmd")
project ("vtm")

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    # Disable manifest embedding for the windows builds.
    # Reason: Anti-virus program (Windows Defender) may lock and scan `vtm(d).exe` file before embedding the manifest.
    #         mt.exe: general error c101008d: Failed to write the updated manifest to the resource of file...
    #set (CMAKE_EXE_LINKER_FLAGS "/MANIFEST:NO")

    set (CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /W3 /GR /EHsc /bigobj")
else ()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        #static linkage
        #set (CMAKE_CXX_FLAGS "-static-libstdc++ -static-libgcc -pthread -s")
        set (CMAKE_CXX_FLAGS "-pthread -s")
    else ()
        set (CMAKE_CXX_FLAGS "-pthread")
    endif ()
endif ()

add_executable (vtmd "vtmd.cpp")
add_executable (vtm  "vtm.cpp")

if (NOT WIN32)
    install (TARGETS vtmd DESTINATION bin)
    install (TARGETS vtm  DESTINATION bin)
endif ()