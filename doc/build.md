# Text-based Desktop Environment

## Building from source

### Unix

Build-time dependencies
 - `C++20 compiler`: [GCC 12](https://gcc.gnu.org/projects/cxx-status.html) or [Clang 14](https://clang.llvm.org/cxx_status.html)
 - `cmake` (minimum version v3.22)
 - `git`
 - `FreeType`
 - `HarfBuzz`
 - `Lua`

Examples of installing dependencies:

OS      | Dependency installation command | Notes
--------|---------------------------------|------
Linux   | `sudo apt install libfreetype-dev libharfbuzz-dev liblua5.4-dev git cmake`
FreeBSD | `pkg install freetype2 harfbuzz lua54 cmake` | Best results with GCC compiler and 6GB of RAM.
MacOS   | `brew install freetype harfbuzz lua cmake`

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
 - [MSVC](https://visualstudio.microsoft.com/downloads/)
   - Desktop Development with C++
   - C++ CMake tools for Windows
   - Git for Windows
   - vcpkg package manager
 - Dependencies managed by vcpkg:
   - `FreeType`
   - `HarfBuzz`
   - `Lua`

To manually compile vtm, launch Visual Studio and clone the repository https://github.com/directvt/vtm.git, after cloning is complete, double-click on the Folder View in Solution Explorer, wait for the dependencies to initialize (may take several minutes), select the required configuration on the top menu toolbar, and click the `Build All` menu button.