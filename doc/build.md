# Text-based Desktop Environment

## Building from source

### Unix

#### Required System Dependencies

- `C++20 compiler`: [GCC 12+](https://gcc.gnu.org/projects/cxx-status.html) or [Clang 16+](https://clang.llvm.org/cxx_status.html)
- `CMake` (minimum version v3.22)
- `Git`
- `Lua` (minimum version v5.4)

#### Optional Dependencies

The following libraries are required but will be automatically downloaded and built via CMake if they are missing or outdated in your system:
- `FreeType` (minimum version v2.13.2)
- `HarfBuzz` (minimum version v12.2.0)
- `LunaSVG` (minimum version v3.5.0)
- `stb`

#### Examples of installing system dependencies:

OS                     | Dependency installation command | Notes
-----------------------|---------------------------------|---
 Linux (Ubuntu/Debian) | `sudo apt install liblua5.4-dev git cmake build-essential` | Uses FetchContent for FreeType/HarfBuzz on older distros.
 FreeBSD               | `pkg install lua54 cmake git`   | Best results with GCC compiler and 6GB of RAM.
 macOS                 | `brew install lua cmake git`

#### Build Steps

Run the following commands in your terminal:

```bash
git clone https://github.com/directvt/vtm.git
cd vtm

# Specify the required compiler if needed:
# export CXX=/usr/bin/g++-12

cmake . -B bin
cmake --build bin
sudo cmake --install bin
vtm
```

> Installation Note:
> - By default, files are installed to system directories (e.g., `/usr/local/bin/`). If you want to install `vtm` to a user directory without using `sudo` (e.g., `$HOME/.local/bin/`), specify the `CMAKE_INSTALL_PREFIX` flag during the configuration step. Ensure that this directory is added to your `$PATH`:
> ```bash
> cmake . -B bin -DCMAKE_INSTALL_PREFIX=\$HOME/.local
> cmake --build bin
> cmake --install bin
> ```

### Windows

#### Build-time Dependencies

 - [`Visual Studio 2022`](https://visualstudio.microsoft.com/downloads/) with the following workloads enabled:
   - Desktop Development with C++
   - C++ CMake tools for Windows
   - Git for Windows
   - vcpkg package manager
 - Dependencies managed by vcpkg:
   - `FreeType`
   - `HarfBuzz`
   - `Lua`
   - `LunaSVG`
   - `stb`

#### Build Steps

- Launch Visual Studio and clone the repository: `https://github.com/directvt/vtm`.
- Wait for the CMake generation and dependencies initialization to complete (this may take several minutes).
- Select your target build configuration (e.g., `1.Win-x64-Debug` or `PROD-Win-x64`) from the top toolbar configuration dropdown.
- Open the `Build` menu and click `Build All`.
