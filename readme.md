# vtm (Virtual Terminal Multiplexer)

Vtm is a text-based application that introduces a new class of **Hybrid TUI** (**HTUI**) software, offering a unified experience within **a single executable file**, whether running in a native graphical window or any standard text console. It can wrap any console application and be nested indefinitely, forming a **text-based desktop environment**, bridging the gap between traditional **TUI** and **GUI**.

<a href="https://www.youtube.com/watch?v=kofkoxGjFWQ">
  <img width="400" alt="Demo on YouTube" src="https://user-images.githubusercontent.com/11535558/146906370-c9705579-1bbb-4e9e-8977-47312f551cc8.gif">
</a>

## Key Features & Benefits

| Feature                                                           | Benefit
|-------------------------------------------------------------------|--------
| **[Hybrid TUI (HTUI)](doc/architecture.md#hybrid-tui)**           | Run the **same application** seamlessly in both dedicated **GUI windows** and standard **terminals**. (GUI mode is available on Windows only for now)
| **[Advanced Input](doc/vt-input-mode.md)**                        | **Track all** key events, **high-resolution** mouse movement and window states.
| **[VT2D Technology](doc/character_geometry.md)**                  | **Scaling** and **transformation** of individual characters or their parts **at the cell level**.
| **[DirectVT I/O](doc/architecture.md#io-modes)**                  | Ability to fully binary **serialize/deserialize** user **input** and own **visual state** through duplex channels (sockets, pipes, SSH-tunnels, TCP-connections, etc.).
| **Desktop Mode**                                                  | A **borderless workspace** that allows infinite panning in all directions.
| **Tiling Window Manager**                                         | Vtm in desktop mode includes a built-in **Tiling Window Manager** for organizing the workspace into non-overlapping panels **with Drag & Drop** support.
| **Multi-User Sessions**                                           | **Share** vtm desktop **over a LAN** (using inetd, netcat, or SSH).
| **[Scripting & UI](doc/settings.md#lua-scripting)**               | Use the full power of **[DynamicXML](doc/settings.md#dynamicxml)+Lua** for customize **reactive UI** (similar to WPF or web apps).
| **[Terminal Mode](doc/apps.md#terminal-and-teletype-console)**    | A standalone **terminal emulator** as a wrapper **for any console applications** for seamless integration with the text-based desktop.
| **[Horizontal Scrolling](doc/apps.md#private-control-sequences)** | Support for displaying simultaneously **wrapped** and **non-wrapped** text runs in the terminal **with horizontal scrolling**.
| **Windows Console Server**                                        | **In-process Windows Console Server** own implementation on Windows and **independence** from `conhost.exe`.

## Get Started

### Desktop Mode

Run `vtm` to start the desktop environment.

### Terminal Mode

Run `vtm -r term [<your_shell>]` to use vtm as a full-fledged standalone terminal emulator.

### Try Auto-DirectVT via SSH

Accessing vtm via SSH with auto-DirectVT mode outperforms the classic connection:

```bash
vtm ssh user@host vtm
```

### Demos

Check Out VT2D Power (Windows only for now):

```bash
vtm --run test
```

Hybrid TUI app examples (just concepts):

```bash
vtm --run calc
vtm --run text
vtm --run gems
```

## Supported platforms

- Windows
  - Windows 8.1 and later (including **Windows Server Core** and **Windows PE**)
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
