# Building from Source

## Unix

Build-time dependencies
 - 64-bit system host
 - `git`, `cmake`,  `C++20 compiler` ([GCC 11](https://gcc.gnu.org/projects/cxx-status.html), [Clang 14](https://clang.llvm.org/cxx_status.html))
 - RAM requirements for compilation:
   - Compiling with GCC — 4GB of RAM
   - Compiling with Clang — 9GB of RAM

Use any terminal as a build environment
```
git clone https://github.com/netxs-group/vtm.git
cd vtm
cmake . -B bin
cmake --build bin
sudo cmake --install bin
vtm
```

Note: A 32-bit binary executable can only be built using cross-compilation on a 64-bit system.

## Windows

Build-time dependencies
 - [git](https://git-scm.com/download/win), [cmake](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-170#installation), [MSVC (Desktop Development with C++)](https://visualstudio.microsoft.com/downloads/)

Use Developer Command Prompt as a build environment

```
git clone https://github.com/netxs-group/vtm.git
cd vtm
cmake . -B bin
cmake --build bin --config Release
bin\Release\vtm.exe
```