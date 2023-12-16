# Architecture

## Process Model

```mermaid
graph TB
    subgraph IE10[Text Console 1]
        subgraph IE1[Input]
            direction LR
            C1[keybd, mouse, focus\nwinsize, clipboard,\nos signals]
        end
        subgraph OU1[Output]
            TC1[scrollback\nbuffer]
        end
        subgraph CS1[Client 1]
            VTM1[vtm\nprocess 1]
        end
        C1 --> CS1
        TC1 --- CS1
    end

    subgraph IE20[Text Console 2]
        subgraph IE2[Input]
            direction LR
            C2[keybd, mouse, focus\nwinsize, clipboard,\nos signals]
        end
        subgraph OU2[Output]
            TC2[scrollback\nbuffer]
        end
        subgraph CS2[Client 2]
            VTM2[vtm\nprocess 2]
        end
        C2 --> CS2
        TC2 --- CS2
    end

    subgraph SS[Server Session]
        VTMs[vtm\nprocess 0]
    end

    CS1 <-->|DirectVT I/O\nsend: Events\nrecv: Render| SS
    CS2 <-->|DirectVT I/O\nsend: Events\nrecv: Render| SS
```

- At startup, vtm connects to an existing server session or creates a new one.
- The new session is hosted in a forked and detached vtm process.
- The session is tied to an operating system's named pipe coined from the creator's name (if no explicitly specified pipe name).
- Only the session creator can access the session (for non-elevated users).
- The session allows multiple access in real time.
- The user can disconnect from the session and reconnect later.
- Several independent sessions with different names can coexist.
- Console applications are launched/executed/terminated by the user within the current server session.
- Non-DirectVT console application runs a pair of operating system processes: terminal process + application process.
- The terminal process is a fork of the vtm server session process, running as standalone terminal. Terminating this process will automatically close the application window.
- The session exists until it is explicitly shutted down.

## Inter-Process Communication

Interprocess communication relies on the DirectVT binary protocol, multiplexing the following primary channels:
- Keyboard event channel
- Mouse event channel
- Focus event channel
- Window size event channel
- Clipboard event channel
- Render output channel
- Shutdown event channel

The vtm client side can operate in two modes, either in ANSI/VT mode (common terminal environment with plain text I/O), or in DirectVT/dtvt mode (vtm environment with binary I/O).

The vtm server side is always operate in DirectVT mode.

## DirectVT mode

In DirectVT mode, the client side receives the event stream and renders directly in binary form (with platform endianness correction), avoiding any parsing and cross-platform issues. The exception is the synchronization of grapheme clusters larger than 7 bytes in UTF-8 format. Large clusters are synchronized between processes by request.

## ANSI/VT mode

### Input

In ANSI/VT mode, the client side parses input from multiple standard sources, and forwards it through appropriate channels to the server side using the DirectVT protocol. The set of input sources varies by platform.

#### Unix-like platform input sources

- STDIN
    - Bracketed paste marks `\x1b[200~`/`\x1b[201~` are treated as the boundaries of a binary immutable block pasted from the clipboard. This immutable block is handled independently of keyboard input.
    - SGR mouse reporting sequences `\x1b[<s;x;yM/m` are redirected to the mouse event channel.
    - Terminal window focus reporting sequences `\x1b[I`/`\x1b[O` are redirected to the focus event channel.
    - Line style reporting sequences `\x1b[33:STYLEp` are redirected to the style event channel (current/selected line wrapping on/off, left/right/center alignment).
    - All incoming text flow that does not fall into the above categories is clusterized, tied to the keys pressed, and forwarded to the keyboard event channel.
- Operating system signals
    - SIGWINCH: Event is forwarded to the window size event channel.
    - SIGINT: Event is forwarded to the shutdown event channel (going to graceful shutdown).
    - SIGHUP: Event is forwarded to the shutdown event channel (going to graceful shutdown).
    - SIGTERM: Event is forwarded to the shutdown event channel (going to graceful shutdown).
- PS/2 Mouse device (Linux VGA Console only)
    - `/dev/input/mice`: Interpreted ImPS/2 mouse protocol events are forwarded to the mouse event channel.
    - `/dev/input/mice.vtm` (used in case of inaccessibility of `/dev/input/mice`)

#### MS Windows platform input sources

- ReadConsoleInput events (Win32 Console API)
    - The KEY_EVENT stream is clusterized, tied to the keys pressed, and forwarded to the keyboard event channel (excluding repeat modifier keys).
    - The MOUSE_EVENT stream is forwarded to the mouse event channel (excluding double clicks and idle events).
    - The FOCUS_EVENT stream is forwarded to the focus event channel.
    - The WINDOW_BUFFER_SIZE_EVENT stream is forwarded to the window size event channel.
    - The MENU_EVENT stream is interpreted using the Event.MenuEvent.dwCommandId value:
        - 0x8000: The subsequent MENU_EVENT record is forwarded to the style event channel.
        - 0x8001: Clipboard immutable block start (INPUT_RECORD begin mark). Subsequent KEY_EVENT records are read until the INPUT_RECORD end mark appears, and then forwarded to the clipboard paste event channel.
        - 0x8002: Clipboard immutable block end (INPUT_RECORD end mark).
- Windows system-defined messages
    - WM_CREATE: Event is forwarded to the clipboard event channel.
    - WM_CLIPBOARDUPDATE: Event is forwarded to the clipboard event channel.
    - WM_ENDSESSION
        - ENDSESSION_CLOSEAPP: Register CTRL_CLOSE_EVENT signal.
        - ENDSESSION_LOGOFF: Register CTRL_LOGOFF_EVENT signal.
        - any other non-zero: Register CTRL_SHUTDOWN_EVENT signal.
- Operating system signals
    - CTRL_C_EVENT: Event is tied to the `Ctrl+C` keys pressed, and forwarded to the keyboard event channel.
    - CTRL_BREAK_EVENT: Event is tied to the `Ctrl+Break` keys pressed, and forwarded to the keyboard event channel.
    - CTRL_CLOSE_EVENT: Event is forwarded to the shutdown event channel (going to graceful shutdown).
    - CTRL_LOGOFF_EVENT: Event is forwarded to the shutdown event channel (going to graceful shutdown).
    - CTRL_SHUTDOWN_EVENT: Event is forwarded to the shutdown event channel (going to graceful shutdown).

### Output

The binary render received for output from the server side is converted by the client side into a format suitable for the type of console being used. The console type is detected at startup and can be one of the following:
- VT Terminal with truecolor support
- VT Terminal with 256-color support (Apple Terminal)
- VT Terminal with 16-color support (Linux VGA Console, 16-color terminals)
- Win32 Console with 16 colors support (Command Prompt on platforms from Windows 8 upto Windows 2019 Server)

vtm renders itself at a constant frame rate into internal buffers and outputs to the console only when the console is ready to accept the next frame.

# Usage Scenarios

## Remote Access

The following examples assume that the vtm executable is available on both the server and client side, and the path to the vtm executable is added to the PATH environment variable.

In general, the server and client host platforms may be different.

### Run any standalone console application remotely via SSH

- Server:
    - Install SSH-server
- Client:
    - run command
    ```bash
    vtm -r xlvt ssh user@server vtm -r term /path/to/console/app
    # `vtm -r xlvt` to run the next statement in DirectVT/XLVT mode.
    # `ssh user@server vtm` to connect via ssh and run vtm on the remote host.
    ```
    or
    ```bash
    vtm ssh user@server vtm -r /path/to/console/app
    ```
    The `-r xlvt` option is auto added if the first command line argument starts with `ssh ...`.
    The `vtm -r ...` option is auto converted to the `vtm -r term ...`.

### Run vtm remotely via SSH in DirectVT mode

- Server:
    - Install SSH-server
- Client:
    - run command
    ```bash
    vtm -r xlvt ssh user@server vtm
    # `vtm -r xlvt` to run the next statement in DirectVT/XLVT mode.
    # `ssh user@server vtm` to connect via ssh and run vtm on the remote host.
    ```
    or
    ```bash
    vtm ssh user@server vtm
    ```
    The `-r xlvt` option is auto added if the first command line argument starts with `ssh ...`.

### Run vtm remotely via SSH in ANSI/VT mode

- Server:
    - Install SSH-server.
- Client:
    - run commands
    ```bash
    ssh user@server
    vtm
    ```
    or
    ```bash
    ssh user@server vtm
    ```

### Run vtm over `netcat` in DirectVT mode (POSIX only, unencrypted, for private use only)

- Server:
    - run command
    ```bash
    ncat -l server_port -k -e vtm
    # `-l server_port` to specify tcp port to listen.
    # `-k` to keep open for multiple clients.
    # `-e` to run vtm for every connected client.
    ```
- Client:
    - run command
    ```bash
    vtm -r dtvt ncat server_ip server_port
    # `vtm -r dtvt` to run DirectVT proxy (not required inside vtm environment).
    # Note: Make sure `ncat` is installed.
    ```

### Run vtm remotely using `inetd` in DirectVT mode (POSIX only, unencrypted, for private use only)

- Server:
    - Install `inetd`
    - Add the following line to the `/etc/inetd.conf`:
        ```bash
        server_port stream tcp nowait user_name /server/side/path/to/vtm  vtm
        # `server_port` to specify tcp port to listen.
        # `user_name` to specify user login name.
        ```
    - Launch `inetd`
        ```
        inetd
        ```
- Client
    - run command
    ```bash
    vtm -r dtvt ncat server_ip server_port
    # `vtm -r dtvt` to run DirectVT proxy (not required inside vtm environment).
    # Note: Make sure `ncat` is installed.
    ```

## Standard I/O Redirection (POSIX only)

- Server
    - run commands
    ```bash
    mkfifo in && mkfifo out
    vtm >out <in
    ```
- Client:
    - run command
    ```bash
    vtm -r dtvt socat open:out\!\!open:in stdin\!\!stdout
    # `vtm -r dtvt` to run DirectVT proxy (not required inside vtm environment).
    # Note: Make sure `socat` is installed.
    ```

## More Tips

### Tiling Window Manager

Terminal windows can be organized using the built-in tiling window manager. Grouping can be temporary within the current session, or pre-configured using settings. See [Settings/App type `Group`](settings.md#app-type) for details.

### Default Terminal Boost

In addition to the default windowed mode, vtm can run as a standalone terminal emulator on top of the host console, extending its functionality with the following features:

- Unlimited scrollback*
- Unwrapped-text option
- Horizontal scrolling
- Rich text copy

The standalone terminal mode can be run by specifying the `-r` option: `vtm -r term`. See [Command line Options](command-line-options.md) for details.

### VT Logging for Developers

vtm allows developers to visualize standard input/output streams. Launched with the `vtm -m` option, vtm will log the event stream of each terminal window with the `Logs` switch enabled.

Important: Avoid enabling the `Logs` switch in the terminal window with the `vtm -m` process running, this may lead to recursive event logging of event logging with unpredictable results.

Important: Be careful with enabling the `Logs` switch when working with sensitive information, since all IO events, including keypresses, are logged in this mode.