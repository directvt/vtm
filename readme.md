# vtm

TL;DR: Text-based desktop inside your console.

<a href="https://www.youtube.com/watch?v=kofkoxGjFWQ">
  <img width="400" alt="Demo on YouTube" src="https://user-images.githubusercontent.com/11535558/146906370-c9705579-1bbb-4e9e-8977-47312f551cc8.gif">
</a>

## Concept

vtm is a windowed multi-user environment for unlimited number of terminals. In other words this is an infinite 2-D space of terminal windows.

To render its interface, vtm needs a text console -- be it a terminal emulator, Windows Command Prompt, or a Linux VGA Console. See [Tested Terminals](https://github.com/netxs-group/vtm/discussions/72) for details.

### Portability

vtm is just a single executable file without any third party dependencies.

### Adaptive Rendering

vtm renders itself at 60 frames per second into its own internal buffers. Output to the text console occurs only when the console is ready to receive the next frame. All pending frames are merged for smooth running even on non-accelerated text consoles.

### Multiplayer

vtm's multi-user architecture allows any number of participants to directly connect to the environment for collaboration. Each environment session is identified by an operating system's named pipe that serves as a gateway for users. To connect, the user just need to run vtm in their text console, either locally or remotely via SSH. See [Command line Options](doc/command-line-options.md) for details.

### Infinite Terminal Count

The number of terminal windows is unlimited*. Each terminal window runs a pair of operating system processes: terminal process + shell process. The terminal process is a fork of the main vtm environment process, running as standalone terminal. Terminating this process will automatically close the terminal window.

### Tiling Window Manager

Users can organize terminal windows by freely moving terminal windows and nesting them into the built-in tiling window manager. Grouping of terminal windows can be done either manually on the fly, or can be pre-configured using a configuration file. See [Settings](doc/settings.md) for details.

### Default Terminal Boost

Besides windowed mode, vtm can operate as a standalone terminal emulator inside the default terminal of your operating system, extenging its functionality with the following features:

- Unlimited scrollback*
- Unwrapped-text option
- Horizontal scrolling
- Selected-text copy encoding:
  - RTF file format
  - HTML code
  - ANSI escape sequences 

Basicly, it allows users to use a huge scrollback buffer with text wrapping disabled, taking advantage of horizontal scrolling within whatever text console they happen to use.

It is noteworthy that vtm allows users to get a full-fledged terminal on those platforms where there are no terminals a priori, but there is a text console - good examples are Windows 8 or Windows Server Core like platforms with only a Command Prompt onboard.

The standalone terminal mode can be run by specifying the following option: `vtm -r term`. See [Command line Options](doc/command-line-options.md) for details.

### VT Logging for Developers

vtm allows developers to visualize the standard input/output stream of running console applications. Launched with the `vtm -m` switch, vtm will log the event stream of each terminal window with the `Logs` switch enabled.

Important: Avoid enabling the `Logs` switch in the terminal window with the `vtm -m` process running, this may lead to recursive event logging of event logging with unpredictable results.

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

# Binary Downloads

![Linux](.resources/status/linux.svg)     [![Intel 64-bit](.resources/status/arch_x86_64.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_linux_x86_64.zip) [![Intel 32-bit](.resources/status/arch_x86.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_linux_x86.zip) [![ARM 64-bit](.resources/status/arch_arm64.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_linux_arm64.zip) [![ARM 32-bit](.resources/status/arch_arm32.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_linux_arm32.zip)  
![Windows](.resources/status/windows.svg) [![Intel 64-bit](.resources/status/arch_x86_64.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_windows_x86_64.zip)  [![Intel 32-bit](.resources/status/arch_x86.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_windows_x86.zip)  [![ARM 64-bit](.resources/status/arch_arm64.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_windows_arm64.zip)  [![ARM 32-bit](.resources/status/arch_arm32.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_windows_arm32.zip)  
![macOS](.resources/status/macos.svg)     [![Universal](.resources/status/arch_any.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_macos_any.zip)  

---
Note: You can use [Github Actions](../../actions) to build statically linked binaries for the big three OS platforms: Linux, Windows, and macOS.

# Documentation

- [Building from Source](doc/build.md)
- [Command line Options](doc/command-line-options.md)
- [User Interface](doc/user-interface.md)
- [Settings](doc/settings.md)
- [Desktop Live Panel](doc/panel.md)
- [Built-in Applications](doc/apps.md)
- Draft: [VT Input Mode](doc/vt-input-mode.md)

# Related Repositories

[Desktopio Framework Documentation](https://github.com/netxs-group/Desktopio-Docs)

---

[![HitCount](https://views.whatilearened.today/views/github/netxs-group/vtm.svg)](https://github.com/netxs-group/vtm) [![Twitter handle][]][twitter badge]

[//]: # (LINKS)
[twitter handle]: https://img.shields.io/twitter/follow/desktopio.svg?style=social&label=Follow
[twitter badge]: https://twitter.com/desktopio
