# Architecture

## Process Model

- At startup, vtm connects to an existing session or creates a new one.
- The new session is hosted in a forked and detached vtm process.
- The session is tied to an operating system's named pipe coined from the creator's name (if no explicitly specified pipe name).
- Only the session creator can access the session (for non-elevated users).
- The session allows multiple access in real time.
- The user can disconnect from the session and reconnect later.
- Several independent sessions with different names can coexist.
- Console applications are launched/executed/terminated by the user within the current session.
- Non-DirectVT console application runs a pair of operating system processes: terminal process + application process.
- The terminal process is a fork of the vtm session process, running as standalone terminal. Terminating this process will automatically close the application window.
- The session exists until it is explicitly shutted down.

## Inter-Process Communication (client side)

## DirectVT mode

In DirectVT mode, all input event channels and output operations are serialized and sent in binary form as is (with platform endianness correction). The exception is the synchronization of grapheme clusters larger than 7 bytes in UTF-8 format. Large clusters are synchronized between processes by request.

## VT mode (plain text mode)

### Ouput

vtm renders itself at a constant frame rate into internal buffers and outputs to the console only when the console is ready to accept the next frame. This applies to slow connections and consoles.

Rendering is done taking into account the capabilities of the text console used. These capabilities are detected at startup. There are four groups:
- VT Terminal with true colors support
- VT Terminal with 256 colors support (Apple Terminal)
- VT Terminal with 16 colors support (Linux VGA Console, 16-color terminals)
- Win32 Console with 16 colors support (Command Prompt on platforms from Windows 8 upto Windows 2019 Server)

### Input

vtm expects input on multiple sources. The set of input sources varies by platform.

#### Unix

- STDIN
    - Bracketed paste marks `\x1b[200~`/`\x1b[201~` are treated as the boundaries of a binary immutable block pasted from the clipboard. This immutable block is handled independently of keyboard input.
    - SGR mouse reporting sequences `\x1b[<s;x;yM/m` are redirected to the mouse event channel.
    - Terminal window focus reporting sequences `\x1b[I`/`\x1b[O` are redirected to the focus event channel.
    - Line style reporting sequences `\x1b[33:STYLEp` are redirected to the style event channel (current/selected line wrapping on/off, left/right/center alingment).
    - All incoming text flow that does not fall into the above categories is clusterized, tied to the keys pressed, and forwarded to the keyboard event channel.
- Operating system signals
    - SIGWINCH: Event is forwarded to the window size event channel.
    - SIGINT: Event is forwarded to the shutdown event channel (going to graceful shutdown).
    - SIGHUP: Event is forwarded to the shutdown event channel (going to graceful shutdown).
    - SIGTERM: Event is forwarded to the shutdown event channel (going to graceful shutdown).
- PS/2 Mouse device (Linux VGA Console only)
    - `/dev/input/mice`
    - `/dev/input/mice.vtm` (used in case of inaccessibility of `/dev/input/mice`)

#### Windows

- Win32 Console Input (ReadConsoleInput)
    - The KEY_EVENT stream is clusterized, tied to the keys pressed, and forwarded to the keyboard event channel (excluding repeat modifier keys).
    - The MOUSE_EVENT stream is forwarded to the mouse event channel (excluding double clicks and idle events).
    - The FOCUS_EVENT stream is forwarded to the focus event channel.
    - The WINDOW_BUFFER_SIZE_EVENT stream is forwarded to the window size event channel.
    - The MENU_EVENT stream is interpreted using the Event.MenuEvent.dwCommandId value:
        - 0x8000: The subsequent MENU_EVENT record is forwarded to the style event channel.
        - 0x8001: Clipboard immutable block start (INPUT_RECORD begin mark). Subsequent KEY_EVENT records are read until the INPUT_RECORD end mark appears, and then forwarded to the clipboard paste event channel.
        - 0x8002: Clipboard immutable block end (INPUT_RECORD end mark).
- Operating system signals
    - CTRL_C_EVENT: Event is tied to the `Ctrl+C` keys pressed, and forwarded to the keyboard event channel.
    - CTRL_BREAK_EVENT: Event is tied to the `Ctrl+Break` keys pressed, and forwarded to the keyboard event channel.
    - CTRL_CLOSE_EVENT: Event is forwarded to the shutdown event channel (going to graceful shutdown).
    - CTRL_LOGOFF_EVENT: Event is forwarded to the shutdown event channel (going to graceful shutdown).
    - CTRL_SHUTDOWN_EVENT: Event is forwarded to the shutdown event channel (going to graceful shutdown).

# Usage Scenarios

## Tiling Window Manager

Terminal windows can be organized using the built-in tiling window manager. Grouping can be temporary within the current session, or pre-configured using settings. See [Settings/App type `Group`](settings.md#app-type) for details.

## Default Terminal Boost

In addition to the default windowed mode, vtm can run as a standalone terminal emulator on top of the host console, extending its functionality with the following features:

- Unlimited scrollback*
- Unwrapped-text option
- Horizontal scrolling
- Rich text copy

The standalone terminal mode can be run by specifying the `-r` option: `vtm -r term`. See [Command line Options](command-line-options.md) for details.

## VT Logging for Developers

vtm allows developers to visualize standard input/output streams. Launched with the `vtm -m` option, vtm will log the event stream of each terminal window with the `Logs` switch enabled.

Important: Avoid enabling the `Logs` switch in the terminal window with the `vtm -m` process running, this may lead to recursive event logging of event logging with unpredictable results.

Important: Be careful with enabling the `Logs` switch when working with sensitive information, since all IO events, including keypresses, are logged in this mode.