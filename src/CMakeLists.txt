cmake_minimum_required (VERSION 3.8)

project ("vtmd")
project ("vtm")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    #add_compile_options(
    #    $<$<CONFIG:>:/MT> #---------|
    #    $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
    #    $<$<CONFIG:Release>:/MT> #--|
    #    )
    #set(CompilerFlags 
    #    "/MD"
    #    )

    # By default, an object file can hold up to 65,279 (almost 2^16)
    # addressable sections. This limit applies no matter which target
    # platform is specified. /bigobj increases that address
    # capacity to 4,294,967,296 (2^32).

    # /EHsc - exception handling for STL

    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /bigobj /MT") # exception handling for STL
    
    #set(CMAKE_EXE_LINKER_FLAGS
    #  "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:libvcruntime.lib")

    #add_compile_options("/EHsc")
else()
    #set(CMAKE_CXX_FLAGS "-pthread")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")
    #set(CMAKE_CXX_FLAGS "-O0 -pthread")
    #add_compile_options("-pthread")
endif()

# todo revise
#project(vtm)
#add_executable (vtm "vtm.cpp")
#set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT vtm)

#add_library(DateTime "include/termxs/datetime/date_time.cpp")

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/vtm.conf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Add source to this project's executable.
add_executable (vtmd "vtmd.cpp")
add_executable (vtm  "vtm.cpp")

#why
#set_property(TARGET vtmd PROPERTY
#  MSVC_RUNTIME_LIBRARY "MultiThreaded")
#set_property(TARGET vtm PROPERTY
#  MSVC_RUNTIME_LIBRARY "MultiThreaded")

#target_link_libraries(vtmd PUBLIC DateTime)

#target_include_directories(vtmd PRIVATE "include\\termxs")
#target_include_directories(vtm  PRIVATE "include\\termxs")

# TODO: Add tests and install targets if needed.
=======
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/vtm.conf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable (vtmd "vtmd.cpp" "terminal.h")
add_executable (vtm  "vtm.cpp")

target_include_directories(vtmd PRIVATE "include\\netxs")
target_include_directories(vtm  PRIVATE "include\\netxs")