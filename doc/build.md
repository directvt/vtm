# Text-based Desktop Environment

## Building from source

### Unix

Build-time dependencies
 - `FreeType` (Linux: `sudo apt install libfreetype-dev`)
 - `HarfBuzz` (Linux: `sudo apt install libharfbuzz-dev`)
 - `Lua` (Linux: `sudo apt install liblua5.4-dev`)
 - `git`
 - `cmake v3.22`
 - `C++20 compiler` ([GCC 12](https://gcc.gnu.org/projects/cxx-status.html), [Clang 14](https://clang.llvm.org/cxx_status.html))

Use any terminal as a build environment

```
git clone https://github.com/directvt/vtm.git
cd vtm
cmake . -B bin
cmake --build bin
sudo cmake --install bin
vtm
```

### Windows

Build-time dependencies
 - `FreeType` (managed by vcpkg)
 - `HarfBuzz` (managed by vcpkg)
 - `Lua` (managed by vcpkg)
 - [git](https://git-scm.com/download/win)
 - [cmake](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-170#installation)
 - [MSVC (Desktop Development with C++; vcpkg package manager)](https://visualstudio.microsoft.com/downloads/)

Use Developer Command Prompt as a build environment

```
git clone https://github.com/directvt/vtm.git
cd vtm
cmake . -B bin
cmake --build bin --config Release
bin\Release\vtm.exe --install
vtm
```