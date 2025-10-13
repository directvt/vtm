# Text-based Desktop Environment

## Command-line options

### Syntax

```
vtm [ -c <file> ][ -q ][ -p <id> ][ -s | -d | -m ][ -x <cmds> ]
vtm [ -c <file> ][ -q ][ -t | -g ][ -r [ <type> ]][ <args...> ]
vtm [ -c <file> ]  -l
vtm -i | -u | -a [mode] | -v | -?

<script relay via piped redirection> | vtm [ -p <id> ]
```

> Without options, vtm runs Desktop Client, running an additional instance with Desktop Server in background if it is not running.

Option                  | Description
------------------------|-------------------------------------------------------
`-h`, `-?`, `--help`    | Print command-line options.
`-v`, `--version`       | Print version.
`-l`, `--listconfig`    | Print configuration.
`-t`, `--tui`           | Force TUI mode.
`-g`, `--gui`           | Force GUI mode.
`-i`, `--install`       | Perform system-wide installation. Allow Desktop Server to run in user context in Session 0 on Windows.<br>Placing Desktop Server in Session 0 allows console applications to run independently of the user's GUI login session. Note: This prevents GUI applications from running from the vtm desktop environment. See "Session 0 Isolation" on the Web for details.<br>Elevated privileges required.
`-u`, `--uninstall`     | Perform system-wide deinstallation.<br>Elevated privileges required.
`-0`, `--session0`      | Use Session 0 to run Desktop Server in background. For Windows only.
`-a`, `--mouse [mode]`  | Set/reset persistent access to mouse devices for all users on Linux platform (excluding Android).<br>Run `sudo vtm --mouse 0` to reset access.<br>Elevated privileges required.
`-q`, `--quiet`         | Disable logging.
`-x`, `--script <cmds>` | Specifies script commands to be run by the desktop when ready.
`-c`, `--config <file>` | Specifies a settings file to load or plain xml-data to merge.
`-p`, `--pin <id>`      | Specifies the desktop id it will be pinned to.
`-s`, `--server`        | Run Desktop Server.
`-d`, `--daemon`        | Run Desktop Server in background.
`-m`, `--monitor`       | Run Log Monitor.
`-r`, `--`, `--run`     | Run desktop applet standalone.
`<type>`                | Desktop applet to run.
`<args...>`             | Desktop applet arguments.
`--env <var=val>`       | Set environment variable.
`--cwd <path>`          | Set current working directory.

#### Inline configuration

The plain xml-data could be specified in place of `<file>` in `--config <file>` option:
- `command-line`:
  ```cmd
  vtm -c "<config><terminal><scrollback size=1000000/></terminal></config>" -r term
  ```
  or (using compact syntax)
- `command-line`:
  ```cmd
  vtm -c "<config/terminal/scrollback size=1000000/>" -r term
  ```

#### Linux VGA Console (in-kernel console)

In order to use a mouse or touchpad in Linux VGA Console, you must grant the user access to mouse/pointing devices. By default, only privileged users and users of the `input` group have access. To grant all users permanent access to all pointing devices, use the command:

- ```
  sudo vtm --mouse
  ```
To reset permanent access, use the command:
- ```
  sudo vtm --mouse 0
  ```

### Desktop Applets

Applet                     | Type | Arguments
---------------------------|------|------------------------------------------
Teletype Console (default) | vtty | CUI application with arguments to run
Terminal Console           | term | CUI application with arguments to run
DirectVT Gateway           | dtvt | DirectVT-aware application to run
DirectVT Gateway with TTY  | dtty | CUI application to run, forwarding DirectVT I/O

The following commands have a short form:
  - `vtm -r vtty <cui_app...>` can be shortened to `vtm <cui_app...>`.
  - `vtm -r dtty ssh <user@host dtvt_app...>` can be shortened to `vtm ssh <user@host dtvt_app...>`.

### Scripting

#### 28 Feb 2025: This functionality is under development.

Script Command                                          | Description
--------------------------------------------------------|-------------------------------------------
`vtm.desktop.Run([{ <attr_list...> }])`                 | Create and run a menu item constructed using a comma-separated list of `<attribute>=<value>` (derived from existing or updated temporary item).<br>Create and run temporary menu item constructed using default attributes if no `<attr_list...>` specified.<br>See [Settings/Taskbar menu item attributes](settings.md#Taskbar-menu-item-attributes) for details.
`vtm.taskbar.Set({ id='<item_id>', [<attr_list...>] })` | Create or override a menu item using a comma-separated list of `<attribute>=<value>`.
`vtm.taskbar.Del(['<item_id>'])`                        | Delete the taskbar menu item by `<item_id>`.<br>Delete all menu items if no `<id>` specified.
`vtm.taskbar.dtvt(<dtvt_app...>)`                       | Create a temporary menu item and run the specified dtvt-executable.
`vtm.taskbar.Selected('<item_id>')`                     | Set selected menu item using specified `<item_id>` (affected to the desktop RightDrag gesture and Tile's `+` button).
`vtm.desktop.Shutdown(['try'])`                         | Terminate the running desktop session. If `try` is specified, the server will only shut down if there are no running windows.

### Character escaping

The following escaped characters have special meaning:

Characters | Expanded to
-----------|-------------------------
`\a`       | ASCII 0x07 BEL
`\t`       | ASCII 0x09 TAB
`\n`       | ASCII 0x0A LF
`\r`       | ASCII 0x0D CR
`\e`       | ASCII 0x1B ESC
`\\`       | ASCII 0x5C Backslash
`\u`       | A Unicode escape sequence in the form `\u{XXX}` or `\uXXX`, where `XXX` is the hexadecimal codepoint value.
`$0`       | Current module full path

### Usage Examples

> 28 Feb 2025: Scripting functionality is under development.

|                                                     | Description
------------------------------------------------------|--------------------------------------------
`vtm`                                                 | Run Desktop Client.
`vtm ssh <user@server> vtm`                           | Run Desktop Client remotely over SSH.
`vtm -r term`                                         | Run Terminal Console.
`vtm -r term </path/to/console/app...>`               | Run Terminal Console with a CUI application inside.
`vtm ssh <user@server> vtm </path/to/console/app...>` | Run a CUI application remotely over SSH.
`vtm -x "vtm.taskbar.Del(); vtm.taskbar.Set({ splitter, id='Apps' }); vtm.taskbar.Set({ id='Term' })"` | Run Desktop Client and reconfigure the taskbar menu.
`echo "vtm.taskbar.Del(); vtm.taskbar.Set({ splitter, id='Apps' }); vtm.taskbar.Set({ id='Term' })" \| vtm`<br><br>`echo "vtm.taskbar.Set({ id='user@server', type='dtty', cmd='ssh <user@server> vtm' })" \| vtm` | Reconfigure the taskbar menu of the running desktop.
`echo "vtm.desktop.Run()" \| vtm`<br><br>`echo "vtm.desktop.Run({ id='Term' })" \| vtm`<br><br>`echo "vtm.desktop.dtvt('vtm -r term')" \| vtm` | Run Terminal Console on the running desktop.
`echo "vtm.desktop.Run({ title='Console \nApplication', cmd='</path/to/app...>' })" \| vtm` | Run Teletype Console with a CUI application inside on the running desktop.
`echo "vtm.desktop.Run({ type='tile', title='Terminals', cmd='v(h(Term,Term),Term)' })" \| vtm` | Run Tiling Window Manager with three terminals attached.
`echo "vtm.desktop.Shutdown()" \| vtm`                | Shutdown desktop server.