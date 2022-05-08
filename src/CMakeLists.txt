cmake_minimum_required (VERSION 3.8)

project ("vtm")

OPTION(DEMO "Demo mode" OFF)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")    # WIN32 and similar checks are soft-deprecated
    # Disable manifest embedding for the windows builds.
    # Reason: Anti-virus program (Windows Defender) may lock and scan `vtm(d).exe` file before embedding the manifest.
    #         mt.exe: general error c101008d: Failed to write the updated manifest to the resource of file...
    #set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /D_WINDOWS /W3 /GR /EHsc /bigobj")
elseif (NOT CMAKE_SYSTEM_NAME MATCHES "Android")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        #static linkage
        #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc -pthread -s")
        #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -s")
    endif ()
endif ()

if (DEMO)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEMO")
endif ()

add_executable (vtm  "vtm.cpp")

if (NOT WIN32)
    install (TARGETS vtm  DESTINATION bin)
endif ()
