# vtm

Vtm is a virtual terminal multiplexer delivered as a single executable. It runs identically in native windows or standard consoles, wraps any CLI app, and supports infinite nesting to create a text-based desktop that bridges the gap between TUI and GUI.

<a href="https://www.youtube.com/watch?v=kofkoxGjFWQ">
  <img width="400" alt="Demo on YouTube" src="https://user-images.githubusercontent.com/11535558/146906370-c9705579-1bbb-4e9e-8977-47312f551cc8.gif">
</a>

## Get started

### Desktop mode

Run `vtm` to start the desktop environment.

### Terminal mode

Run `vtm -r term [<your_shell>]` to use vtm as a full-fledged standalone terminal emulator.

### Binary connection via SSH

Accessing vtm via SSH using the DirectVT protocol outperforms the classic connection:

```bash
vtm ssh user@host vtm
```

### Demos

Check out VT2D power (Windows only for now):

```bash
vtm --run test
```

HybridTUI (HTUI) app examples (just concepts):

```bash
vtm --run calc
vtm --run text
vtm --run gems
```

## Supported platforms

- Windows
  - Windows 8.1 and later (including Windows Server Core and Windows PE)
- [Unix-like](https://en.wikipedia.org/wiki/Unix-like)
  - Linux
  - macOS
  - FreeBSD
  - NetBSD
  - OpenBSD
  - [`...`](https://en.wikipedia.org/wiki/POSIX#POSIX-oriented_operating_systems)

[Tested Terminals](https://github.com/directvt/vtm/discussions/72)

<sup>Currently, rendering into a native GUI window is only available on the Windows platform; on Unix-like platforms, a terminal emulator is required.</sup>

## Binary downloads

![Linux](.resources/status/linux.svg)     [![Intel 64-bit](.resources/status/arch_x86_64.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_linux_x86_64.tar.7z) [![ARM 64-bit](.resources/status/arch_arm64.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_linux_arm64.tar.7z) [![Intel 32-bit](.resources/status/arch_x86.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_linux_x86.tar.7z) [![ARM 32-bit](.resources/status/arch_arm32.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_linux_arm32.tar.7z)  
![Windows](.resources/status/windows.svg) [![Intel 64-bit](.resources/status/arch_x86_64.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_windows_x86_64.7z)   [![ARM 64-bit](.resources/status/arch_arm64.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_windows_arm64.7z)   [![Intel 32-bit](.resources/status/arch_x86.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_windows_x86.7z)  
![macOS](.resources/status/macos.svg)     [![Intel 64-bit](.resources/status/arch_x86_64.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_macos_x86_64.tar.7z) [![ARM 64-bit](.resources/status/arch_arm64.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_macos_arm64.tar.7z)  

## Documentation

- [Quickstart](doc/architecture.md#quickstart)
- [Architecture](doc/architecture.md)
- [Building from source](doc/build.md)
- [Command-line options](doc/command-line-options.md)
- [User interface](doc/user-interface.md)
- [Settings](doc/settings.md)
- [VT2D](doc/character_geometry.md)
