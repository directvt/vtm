# vtm

Text-baased desktop inside your console.

<a href="https://www.youtube.com/watch?v=kofkoxGjFWQ">
  <img width="400" alt="Demo on YouTube" src="https://user-images.githubusercontent.com/11535558/146906370-c9705579-1bbb-4e9e-8977-47312f551cc8.gif">
</a>

# Supported Platforms

- Windows
  - Server/Desktop
- Unix-like
  - Linux
  - Android <sup><sup>Linux kernel</sup></sup>
  - macOS
  - FreeBSD
  - NetBSD
  - OpenBSD
  - [`...`](https://en.wikipedia.org/wiki/POSIX#POSIX-oriented_operating_systems)
- [Tested Terminals](https://github.com/netxs-group/vtm/discussions/72)

# Runtime Requirements

- 100MB of RAM per built-in 40k line-terminal instance [#147](https://github.com/netxs-group/vtm/issues/147)

# Building from Source

### Unix-like

Build-time dependencies
 - `git`, `cmake`,  `C++20 compiler` ([`GCC 11`](https://gcc.gnu.org/projects/cxx-status.html), [`Clang 13`](https://clang.llvm.org/cxx_status.html), [`MSVC 2019`](https://visualstudio.microsoft.com/downloads/))
 - Minimal requirements to compile
   - Compiling with GCC — 4GB of RAM
   - Compiling with Clang — 9GB of RAM

Note: A 32-bit binary executable can only be built using cross-compilation on a 64-bit system, since building directly on a 32-bit environment is not possible due to compile time RAM requirements.

```bash
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

```cmd
git clone https://github.com/netxs-group/vtm.git
cd vtm
build64.bat
Release\vtm.exe
```

# Binary Downloads

[![](.resources/status/macos.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_macos_any.tar.gz)  
[![](.resources/status/linux.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_linux_amd64.tar.gz)  
[![](.resources/status/linux_i386.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_linux_i386.tar.gz)  
[![](.resources/status/windows.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_windows_amd64.zip)  
[![](.resources/status/windows_i386.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_windows_i386.zip)  
[![](.resources/status/freebsd.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_freebsd_amd64.tar.gz)  
[![](.resources/status/netbsd.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_netbsd_amd64.tar.gz)  
[![](.resources/status/openbsd.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_openbsd_amd64.tar.gz)  

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
