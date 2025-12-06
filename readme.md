# vtm

### TL;DR

It is a text-based application where the entire user interface is represented by a mosaic of text cells forming a TUI matrix. The resulting TUI matrix is just rendered either into its own GUI window or into a compatible text console.

There are two modes - desktop mode and terminal mode.
- In terminal mode, it can work as a full-fledged stanalone terminal emulator.  
  Just run `vtm -r term [<your_shell>]` or `vtm -r [<your_shell>]`.
- In desktop mode it can wrap any console application and be nested indefinitely, forming a text-based desktop environment.  
  Just run `vtm`.  
  <a href="https://www.youtube.com/watch?v=kofkoxGjFWQ">
    <img width="400" alt="Demo on YouTube" src="https://user-images.githubusercontent.com/11535558/146906370-c9705579-1bbb-4e9e-8977-47312f551cc8.gif">
  </a>

Note: Accessing via ssh with auto-DirectVT mode outperforms the classic connection. Just run `vtm ssh user@host vtm`.

### Details

- Vtm is a text-based application that comes with a single executable and has a number of runtime modes for running multiple instances in parallel to form the desktop environment.
- A vtm process running in `Desktop Server` mode creates a desktop session.
- Desktop users connect to an existing desktop session through an additional vtm process running in `Desktop Client` mode. The desktop is presented to the user as a **borderless workspace that allows panning** in all **directions** (infinite desktop).
- The desktop session has a unique ID, coined from the platform-specific creator UID, unless explicitly specified otherwise.
- Sessions with different IDs can coexist independently.
- Only the session creator or an elevated user can access the session.
- The "regular" user and the "elevated" user are different independent users despite having the same username.
- The session allows multiple access **in real time**.
- Multiple connected users can share a focused application, while each user can have multiple applications focused.
- Users can disconnect from the session and reconnect later.
- To maximize rendering efficiency and minimize cross-platform issues, along with the character-oriented xterm-compatible I/O mode called `Classic VT`, vtm supports an additional message-based binary I/O mode called `DirectVT`.
- Using `DirectVT` mode (when vtm is running as a `Desktop Client` or `DirectVT Gateway`), vtm has the ability to fully binary deserialize/serialize its state through arbitrary channels (like socat over SSH reverse tunnel) and does not require a running SSH server on the remote side.
- Vtm employs a hybrid TUI/GUI approach: it can render itself into both GUI windows and terminals (`vtm --gui` and `vtm --tui` flags). Currently, rendering into a native GUI window is only available on the Windows platform.
- In GUI mode, vtm replicates its unique TUI-mode style and windowing mechanics, including keyboard multifocus (activated by `Ctrl+LeftClick`).
- On Windows, any user can launch an **SSH-accessible desktop** session **in Session 0**, running under their own security context and is independent of any active graphical session (requires the vtm service installed via `vtm --install` from an elevated console).
- When running in the **Linux in-kernel VGA Console** or **KMSCON** environment, vtm can directly use any kernel pointer devices (`/dev/input/eventX`) (requires persistent access configured using `sudo vtm --mouse 1`).
- A typical console application integrates into the desktop using the `DirectVT Gateway` window as the DirectVT connection endpoint.
  - A DirectVT-aware application directly connected to the environment can seamlessly send and receive the entire set of desktop events, as well as render itself in a binary form, avoiding expensive `Classic VT` parsing.
  - To run a non-DirectVT application, an additional vtm host process is launched in `Desktop Applet` mode with the `Teletype Console` or `Terminal Console` applet as a DirectVT bridge to the desktop environment.
- The desktop has a built-in Tiling Window Manager for organizing desktop space into non-overlapping panels with Drag and Drop support for moving panels (like in web browsers).
- The user interface supports Lua scripting, allowing scripts to be bound to various internal events via configuration settings, as well as executed directly from child processes via APC sequences.
- The desktop server can receive and execute Lua scripts relayed from other vtm processes (running on behalf of the session creator) via a redirected standard input, or interactively executed from the attached log monitor (`vtm --monitor`).
- In terminal emulator mode (`Teletype Console` or `Terminal Console` launched via `vtm --run term` or `vtm --run vtty`), vtm also supports the following features:
  - Simultaneous output of wrapped and non-wrapped text lines of arbitrary length with horizontal scrolling.
  - An **in-process Win32 Console Server implementation**, which is independent of the standard system `conhost.exe` and compatible with **Windows 8.1** and **Windows Server 2012 Core** (including GUI mode with true-color Unicode rendering).
  - Mouse reports with floating point coordinates, where the cursor position inside a cell is normalized from 0 to 1.
  - Special (Exclusive) keyboard mode for the terminal window to transfer all keyboard events to the terminal as is.
  - A configurable scrollback buffer size (**100k lines by default**, limited by max_int32 and system RAM).
  - Text lookup in the scrollback buffer.
  - Unicode character Geometry Modifiers VT2D with the ability to output text characters of arbitrary size and in parts (up to 16x4 cells).
  - Stdin/stdout logging.
- Vtm supports the creation of advanced keyboard bindings (generic: `Ctrl+Enter`, literal: `Ctrl+'\n'`, specific: `LeftCtrl+KeyEnter`, scancodes: `0x1D+0x1C`), allowing for the configuration of complex behavior, like a tmux-style prefix key for modality (e.g., toggling window movement with arrow keys).
- The entire user interface can be localized to any language, including those with complex scripts, via a configuration file (rendering is powered by VT2D in GUI mode).
- Vtm has a built-in logging subsystem; the log output is available via the `vtm --monitor` command.
- Used non-standard technologies:
  - DirectVT (binary input and UI rendering)
  - VT2D (Unicode character Geometry Modifiers)
  - DynamicXML (settings configuration)
  - Lua scripting (dynamic UI)
  - TUI Shadows (SGR attribute)
  - VT Input Mode (floating point mouse reporting)
  - Hybrid UI (TUI/GUI)
  - In-process Windows Console Server (Windows 8.1 and later compatibility)
  - Terminal with horizontal scrolling support (wrapped and un-wrapped text lines simultaneously)

# Supported platforms

- Windows
  - Windows 8.1 and later
- [Unix-like](https://en.wikipedia.org/wiki/Unix-like)
  - Linux
  - macOS
  - FreeBSD
  - NetBSD
  - OpenBSD
  - [`...`](https://en.wikipedia.org/wiki/POSIX#POSIX-oriented_operating_systems)

[Tested Terminals](https://github.com/directvt/vtm/discussions/72)

<sup>Currently, rendering into a native GUI window is only available on the Windows platform; on Unix-like platforms, a terminal emulator is required.</sup>

# Binary downloads

![Linux](.resources/status/linux.svg)     [![Intel 64-bit](.resources/status/arch_x86_64.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_linux_x86_64.tar.7z) [![ARM 64-bit](.resources/status/arch_arm64.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_linux_arm64.tar.7z) [![Intel 32-bit](.resources/status/arch_x86.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_linux_x86.tar.7z) [![ARM 32-bit](.resources/status/arch_arm32.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_linux_arm32.tar.7z)  
![Windows](.resources/status/windows.svg) [![Intel 64-bit](.resources/status/arch_x86_64.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_windows_x86_64.7z)   [![ARM 64-bit](.resources/status/arch_arm64.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_windows_arm64.7z)   [![Intel 32-bit](.resources/status/arch_x86.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_windows_x86.7z)  
![macOS](.resources/status/macos.svg)     [![Intel 64-bit](.resources/status/arch_x86_64.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_macos_x86_64.tar.7z) [![ARM 64-bit](.resources/status/arch_arm64.svg)](https://github.com/directvt/vtm/releases/latest/download/vtm_macos_arm64.tar.7z)  

Linux platform notes:
- Linux binaries are statically built using gcc-12/g++-12 on Ubuntu 22.04, provided by Github Actions.
- Runtime dependencies on Linux:
  - ```
    GLIBC 2.34
    ```

# Documentation

- [Quickstart](doc/architecture.md#quickstart)
- [Architecture](doc/architecture.md)
- [Building from source](doc/build.md)
- [Command-line options](doc/command-line-options.md)
- [User interface](doc/user-interface.md)
- [Settings](doc/settings.md)
- [VT2D](doc/character_geometry.md)
