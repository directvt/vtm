# vtm

Desktop inside text console.

<a href="https://www.youtube.com/watch?v=kofkoxGjFWQ">
  <img width="400" alt="Demo on YouTube" src="https://user-images.githubusercontent.com/11535558/146906370-c9705579-1bbb-4e9e-8977-47312f551cc8.gif">
</a>

# Supported Platforms

- Windows
  - Server/Workstation
- POSIX-oriented
  - Linux
  - macOS
  - FreeBSD
  - NetBSD
  - OpenBSD
  - [`...`](https://en.wikipedia.org/wiki/POSIX#POSIX-oriented_operating_systems)
- [Tested Terminals](https://github.com/netxs-group/vtm/discussions/72)

# Building from Source

### POSIX-oriented

Build-time dependencies
 - `git`
 - `cmake`
 - C++20 compiler (GCC 11, Clang 13, MSVC 2019)
 - Minimal requirements to compile
   - Using [`GCC`](https://gcc.gnu.org/projects/cxx-status.html) — `4GB` of RAM
   - Using [`Clang`](https://clang.llvm.org/cxx_status.html) — `9GB` of RAM

```bash
git clone https://github.com/netxs-group/vtm.git
cd ./vtm
cmake ./src -DCMAKE_BUILD_TYPE=Release
cmake --build .
cmake --install .
vtm
```

### Windows

Build-time dependencies
 - [`git`](https://git-scm.com/download/win)
 - [`cmake`](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-170#installation)
 - [`Visual Studio 2019`](https://visualstudio.microsoft.com/downloads/) or later (Desktop development with C++)

Use `Developer Command Prompt` as a build environment

`Visual Studio 2019`:
```cmd
git clone https://github.com/netxs-group/vtm.git
cd ./vtm
chcp 65001
cmake ./src -DCMAKE_BUILD_TYPE=Release "-GVisual Studio 16 2019"
cmake --build . --config Release
cd Release
vtm
```
`Visual Studio 2022`:
```cmd
git clone https://github.com/netxs-group/vtm.git
cd ./vtm
chcp 65001
cmake ./src -DCMAKE_BUILD_TYPE=Release "-GVisual Studio 17 2022"
cmake --build . --config Release
cd Release
vtm
```

# Binary Downloads

[![](.resources/status/macos.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_macos_any.tar.gz)  
[![](.resources/status/freebsd.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_freebsd_amd64.tar.gz)  
[![](.resources/status/netbsd.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_netbsd_amd64.tar.gz)  
[![](.resources/status/openbsd.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_openbsd_amd64.tar.gz)  
[![](.resources/status/linux.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_linux_amd64.tar.gz)  
[![](.resources/status/windows.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_windows_amd64.zip)  

---

# Documentation

- [Command line Options](doc/command-line-options.md)
- [User Interface](doc/user-interface.md)
- [Settings](doc/settings.md)
- [Built-in Applications](doc/apps.md)

# Related Repositories

[Desktopio Framework Documentation](https://github.com/netxs-group/Desktopio-Docs)

---

[![HitCount](https://views.whatilearened.today/views/github/netxs-group/vtm.svg)](https://github.com/netxs-group/vtm) [![Twitter handle][]][twitter badge]

[//]: # (LINKS)
[twitter handle]: https://img.shields.io/twitter/follow/desktopio.svg?style=social&label=Follow
[twitter badge]: https://twitter.com/desktopio
