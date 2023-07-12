# vtm

Text-baased desktop inside your console.

<a href="https://www.youtube.com/watch?v=kofkoxGjFWQ">
  <img width="400" alt="Demo on YouTube" src="https://user-images.githubusercontent.com/11535558/146906370-c9705579-1bbb-4e9e-8977-47312f551cc8.gif">
</a>

# Supported Platforms

- Windows
  - Server/Desktop
- Unix
  - Linux
  - Android <sup><sup>Linux kernel</sup></sup>
  - macOS
  - FreeBSD
  - NetBSD
  - OpenBSD
  - [`...`](https://en.wikipedia.org/wiki/POSIX#POSIX-oriented_operating_systems)
- [Tested Terminals](https://github.com/netxs-group/vtm/discussions/72)

# Building from Source

### Unix

Build-time dependencies
 - 64-bit system host
 - `git`, `cmake`,  `C++20 compiler` ([`GCC 11`](https://gcc.gnu.org/projects/cxx-status.html), [`Clang 13`](https://clang.llvm.org/cxx_status.html), [`MSVC 2019`](https://visualstudio.microsoft.com/downloads/))
 - Minimum RAM requirements for compilation:
   - Compiling with GCC — 4GB of RAM
   - Compiling with Clang — 9GB of RAM

Note: A 32-bit binary executable can only be built using cross-compilation on a 64-bit system, since building directly on a 32-bit environment is not possible due to compile time RAM requirements.

Use any terminal as a build environment
```
git clone https://github.com/netxs-group/vtm.git
cd ./vtm
./build64.sh
./install.sh
vtm
```

### Windows

Build-time dependencies
 - [`git`](https://git-scm.com/download/win), [`cmake`](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-170#installation), [`MSVC 2019 (Desktop Development with C++)`](https://visualstudio.microsoft.com/downloads/)

Use `Developer Command Prompt` as a build environment

```
git clone https://github.com/netxs-group/vtm.git
cd vtm
build64.cmd
Release\vtm.exe
```

# Binary Downloads

![](.resources/status/macos.svg)   [![](.resources/status/arch_any.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_macos_any.tar.gz)  
![](.resources/status/linux.svg)   [![](.resources/status/arch_x86_64.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_linux_x86_64.tar.gz) [![](.resources/status/arch_x86.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_linux_x86.tar.gz)   [![](.resources/status/arch_arm64.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_linux_arm64.tar.gz)   [![](.resources/status/arch_arm32.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_linux_arm32.tar.gz)  
![](.resources/status/windows.svg) [![](.resources/status/arch_x86_64.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_windows_x86_64.zip)  [![](.resources/status/arch_x86.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_windows_x86.tar.gz) [![](.resources/status/arch_arm64.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_windows_arm64.tar.gz) [![](.resources/status/arch_arm32.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_windows_arm32.tar.gz)  

---

# Documentation

- [Command line Options](doc/command-line-options.md)
- [User Interface](doc/user-interface.md)
- [Settings](doc/settings.md)
- [Built-in Applications](doc/apps.md)
- Draft: [VT Input Mode](doc/vt-input-mode.md)

# Related Repositories

[Desktopio Framework Documentation](https://github.com/netxs-group/Desktopio-Docs)

---

[![HitCount](https://views.whatilearened.today/views/github/netxs-group/vtm.svg)](https://github.com/netxs-group/vtm) [![Twitter handle][]][twitter badge]

[//]: # (LINKS)
[twitter handle]: https://img.shields.io/twitter/follow/desktopio.svg?style=social&label=Follow
[twitter badge]: https://twitter.com/desktopio
