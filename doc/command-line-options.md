# Text-based desktop environment

## Command-line options

### Syntax

```
vtm [ -i | -u ] | [ -v ] | [ -? ]  |  [ -c <file>][ -l ]

vtm [ --script <commands>][ -p <name>][ -c <file>][ -q ]
    [ -m | -d | -s | [ -r [<console>]][<arguments ...>]]

<run commands via piped redirection> | vtm [options ...]
```

> By default, the full-screen Desktop Client console will run and the Desktop Server daemon will launched if it is not running.

Option                  | Description
------------------------|-------------------------------------------------------
`-h`, `-?`, `--help`    | Print command-line options.
`-v`, `--version`       | Print version.
`-l`, `--listconfig`    | Print configuration.
`-i`, `--install`       | Perform system-wide installation.
`-u`, `--uninstall`     | Perform system-wide deinstallation.
`-c`, `--config <file>` | Specifies the settings file to load.
`-p`, `--pipe <name>`   | Specifies the desktop session connection point.
`-m`, `--monitor`       | Run Desktop Session Monitor.
`-d`, `--daemon`        | Run Desktop Server daemon.
`-s`, `--server`        | Run Desktop Server.
`-r`, `--`, `--run`     | Run full-screen console.
`-q`, `--quiet`         | Disable logging.
`--script <commands>`   | Specifies script commands to be run by the desktop when ready.
`<console>`             | Full-screen console to run.
`<arguments ...>`       | Full-screen console arguments.

### Settings loading order

  - Initialize hardcoded settings.
  - Merge with explicitly specified settings from `--config <file>`.
  - If the `--config` option is not used or `<file>` cannot be loaded:
      - Merge with system-wide settings from `/etc/vtm/settings.xml` (`%PROGRAMDATA%/vtm/settings.xml` on Windows).
      - Merge with user-wise settings from `~/.config/vtm/settings.xml`.
      - Merge with DirectVT packet received from the parent process (dtvt-mode).

### Full-screen consoles

`<console>` | `<aruments>`     | Object type to run detached        | Description
------------|------------------|------------------------------------|----------------------
`vtty`      | `<cui_app ...>`  | `teletype`/`Teletype console`      | Used to run CUI applications.
`term`      | `<cui_app ...>`  | `terminal`/`Terminal console`      | Used to run CUI applications.
`dtvt`      | `<dtvt_app ...>` | `dtvt`/`DirectVT console`          | Used to run DirectVT aware applications.
`dtty`      | `<dtvt_src ...>` | `dtty`/`DirectVT console with TTY` | Used to run CUI applications that redirect DirectVT traffic to standard output and require user input via platform's TTY.
|           |                  | `desk`/`Desktop Client console`    | Used by default to run Desktop Client console.

The following commands have a short form:
  - `vtm -r vtty [cui_app ...]` can be shortened to `vtm [cui_app ...]`.
  - `vtm -r dtty ssh <user@host dtvt_app ...>` can be shortened to `vtm ssh <user@host dtvt_app ...>`.

### Scripting

Syntax: `command1([args...])[; command2([args...]); ... commandN([args...])]`

The following characters in the script body will be de-escaped: `\e` `\t` `\r` `\n` `\a` `\"` `\'` `\\`

 Command                                 | Description
-----------------------------------------|-------------------------------------------
`vtm.run([<attr_list>...])`              | Create and run a menu item constructed using a space-separated list of `attribute=<value>` (derived from existing or updated temporary item).<br>Create and run temporary menu item constructed using default attributes if no `<attr_list...>` specified.<br>See [Settings/Taskbar menu item attributes](settings.md#Taskbar-menu-item-attributes) for details.
`vtm.set(id=<item_id> [<attr_list>...])` | Create or override a menu item using a space-separated list of `attribute=<value>`.
`vtm.del([<item_id>])`                   | Delete the taskbar menu item by `<id>`.<br>Delete all menu items if no `<id>` specified.
`vtm.dtvt(<dtvt_app...>)`                | Create a temporary menu item and run the specified dtvt-executable.
`vtm.selected(<item_id>)`                | Set selected menu item using specified `<id>` (affected to the desktop RightDrag gesture and Tile's `+` button).
`vtm.shutdown()`                         | Terminate the running desktop session.

### Usage Examples

Command                                            | Description
---------------------------------------------------|--------------------------------------------
`vtm`                                              | Run Desktop Client console.
`vtm ssh <user@server> vtm`                        | Run Desktop Client console remotely over SSH.
`vtm -r term`                                      | Run Terminal console.
`vtm -r term </path/to/console/app>`               | Run Terminal console with a CUI application inside.
`vtm ssh <user@server> vtm </path/to/console/app>` | Run a CUI application remotely over SSH.
`vtm --script "vtm.del(); vtm.set(splitter id=Apps); vtm.set(id=Term)"` | Run Desktop Client console and reconfigure the taskbar menu.
`echo "vtm.del(); vtm.set(splitter id=Apps); vtm.set(id=Term)" \| vtm`<br><br>`echo "vtm.set(id=user@server type=dtty cmd='ssh <user@server> vtm')" \| vtm` | Reconfigure the taskbar menu of the running desktop.
`echo "vtm.run()" \| vtm`<br><br>`echo "vtm.run(id=Term)" \| vtm`<br><br>`echo "vtm.dtvt(vtm -r term)" \| vtm` | Run Terminal console on the running desktop.
`echo "vtm.run(title='Console \nApplication' cmd=</path/to/app>)" \| vtm` | Run Teletype console with a CUI application inside on the running desktop.
`echo "vtm.run(type=tile title=Terminals cmd='v(h(Term,Term),Term)')" \| vtm` | Run Tiling Window Manager with three terminals attached.
`echo "vtm.shutdown()" \| vtm`                     | Terminate the running desktop session.