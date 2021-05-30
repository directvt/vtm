cmake_minimum_required (VERSION 3.8)

project ("vtmd")
project ("vtm")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable manifest embedding for the windows builds.
# Reason: Anti-virus program (Windows Defender) may lock and scan `vtm(d).exe` file before embedding the manifest.
#         mt.exe: general error c101008d: Failed to write the updated manifest to the resource of file...
IF (WIN32)
#    set(CMAKE_EXE_LINKER_FLAGS /MANIFEST:NO)
ENDIF (WIN32)

add_executable (vtmd "vtmd.cpp")
add_executable (vtm  "vtm.cpp")
