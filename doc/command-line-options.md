# Text-based desktop environment

## Command-line options

### Syntax

```
vtm [ -i | -u ] | [ -v ] | [ -? ] | [ -c <file> ][ -l ]

vtm [ --script <commands> ] [ -p <name> ] [ -c <file> ]
    [ -q ] [ -m | -d | -s | -r [<type>] ] [<cliapp...>]

<run commands via piped redirection> | vtm [options...]
```

Option                  | Description
------------------------|-------------------------------------------------------
No arguments            | Connect to the desktop (autostart new if not running).
`-h`, `-?`, `--help`    | Print command-line options.
`-v`, `--version`       | Print version.
`-l`, `--listconfig`    | Print configuration.
`-i`, `--install`       | Perform system-wide installation.
`-u`, `--uninstall`     | Perform system-wide deinstallation.
`-c`, `--config <file>` | Specifies the settings file to load.
`-p`, `--pipe <name>`   | Specifies the desktop session connection point (case sensitive).
`-m`, `--monitor`       | Run desktop session log monitor.
`-d`, `--daemon`        | Run desktop server in background.
`-s`, `--server`        | Run desktop server in interactive mode.
`-r`, `--`, `--runapp`  | Run the specified built-in terminal type in standalone mode.
`-q`, `--quiet`         | Disable logging.
`--script <commands>`   | Specifies script commands to be run by the desktop when ready.
`<type>`                | Built-in terminal type to use to run a console application (case insensitive).
`<cliapp...>`           | Console application with arguments to run.

### Settings loading order

  - Initialize hardcoded settings.
  - Merge with explicitly specified settings from `--config <file>`.
  - If the `--config` option is not used or `<file>` cannot be loaded:
      - Merge with system-wide settings from `/etc/vtm/settings.xml` (`%PROGRAMDATA%/vtm/settings.xml` on Windows).
      - Merge with user-wise settings from `~/.config/vtm/settings.xml`.
      - Merge with DirectVT packet received from the parent process (dtvt-mode).

### Built-in terminal types

 Type  | Description                                       | Syntax
-------|---------------------------------------------------|------------------------------------
`Term` | Terminal emulator to run common cli applications. | `vtm [options ...] -r term [cliapp ...]`
`NoUI` | Terminal emulator without UI (scrollback only).   | `vtm [options ...] -r noui [cliapp ...]`
`DTVT` | DirectVT proxy to run dtvt-apps in a generic text console. | `vtm [options ...] -r dtvt [dtvt_app ...]`
`XLVT` | DirectVT proxy with controlling terminal to run dtvt-apps over SSH.<br>`XLVT` stands for Cross-linked VT. | `vtm [options ...] -r xlvt ssh <user@host dtvt_app ...>`

The following commands have a short form:
  - `vtm -r xlvt ssh <user@host dtvt_app ...>` can be shortened to `vtm ssh <user@host dtvt_app ...>`.
  - `vtm -r noui [cliapp ...]` can be shortened to `vtm [cliapp ...]`.

### Scripting

Syntax: `command1([args...])[; command2([args...]); ... commandN([args...])]`

The following characters in the script body will be de-escaped: `\e` `\t` `\r` `\n` `\a` `\"` `\'` `\\`

 Command                                 | Description
-----------------------------------------|-------------------------------------------
`vtm.run([<attr_list>...])`              | Create and run a menu item constructed using a space-separated list of `attribute=<value>` (derived from existing or updated temporary item).<br>Create and run temporary menu item constructed using default attributes if no `<attr_list...>` specified.
`vtm.set(id=<item_id> [<attr_list>...])` | Create or override a menu item using a space-separated list of `attribute=<value>`.
`vtm.del([<item_id>])`                   | Delete the taskbar menu item by `<id>`.<br>Delete all menu items if no `<id>` specified.
`vtm.dtvt(<dtvt_app...>)`                | Create a temporary menu item and run the specified dtvt-executable.
`vtm.selected(<item_id>)`                | Set selected menu item using specified `<id>` (affected to the desktop RightDrag gesture and Tile's `+` button).
`vtm.shutdown()`                         | Terminate the running desktop session.

### Usage Examples

Command                                  | Description
-----------------------------------------|--------------------------------------------
`vtm`                                    | Run vtm desktop inside the current console.
`vtm ssh <user@server> vtm`              | Run remote vtm desktop inside the current console over SSH.
`vtm -r [term]`                          | Run the built-in terminal inside the current console.
`vtm [-r [term]] </path/to/console/app>` | Run an application inside the built-in terminal.
`vtm ssh <user@server> vtm [-r [term]] </path/to/console/app>` | Run an application remotely over SSH.
`vtm --script "vtm.del(); vtm.set(splitter id=Apps); vtm.set(id=Term)"` | Run vtm desktop and reconfigure the taskbar menu.
`echo "vtm.del(); vtm.set(splitter id=Apps); vtm.set(id=Term)" \| vtm`<br><br>`echo "vtm.set(id=user@server type=xlvt cmd='ssh <user@server> vtm')" \| vtm` | Reconfigure the taskbar menu of the running desktop.
`echo "vtm.run()" \| vtm`<br><br>`echo "vtm.run(id=Term)" \| vtm`<br><br>`echo "vtm.dtvt(vtm -r term)" \| vtm` | Run a terminal window on the running desktop.
`echo "vtm.run(title='Console \nApplication' cmd=</path/to/app>)" \| vtm` | Run an application window on the running desktop.
`echo "vtm.run(type=group title=Terminals cmd='v(h(Term,Term),Term)')" \| vtm` | Run tiling window manager with three terminals attached.
`echo "vtm.shutdown()" \| vtm`           | Terminate the running desktop session.