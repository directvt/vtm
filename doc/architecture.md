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

## Adaptive Rendering

vtm renders itself at a constant frame rate into internal buffers and outputs to the console only when the console is ready to accept the next frame. This applies to slow connections and consoles.

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