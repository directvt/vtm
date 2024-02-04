# Text-based desktop environment

## Command-line options

### Syntax

```
vtm [ -i | -u ] | [ -v ] | [ -? ]  |  [ -c <file>][ -l ]

vtm [ --script <commands>][ -p <name>][ -c <file>][ -q ]
    [ -m | -d | -s | [ -b [ <root>]][ <arguments ...>] ]

<run commands via piped redirection> | vtm [options ...]
```

Option                  | Description
------------------------|-------------------------------------------------------
                        | By default, run Detached Visual Branch with Desktop Client console as a root and autorun Desktop Server daemon if it is not running. Detached Visual Branch can be seamlesly attached to the desktop via dtvt-gate.
`-h`, `-?`, `--help`    | Print command-line options.
`-v`, `--version`       | Print version.
`-l`, `--listconfig`    | Print configuration.
`-i`, `--install`       | Perform system-wide installation.
`-u`, `--uninstall`     | Perform system-wide deinstallation.
`-c`, `--config <file>` | Specifies the settings file to load.
`-p`, `--pipe <name>`   | Specifies the desktop session connection point.
`-m`, `--monitor`       | Run desktop session monitor.
`-d`, `--daemon`        | Run desktop server as daemon.
`-s`, `--server`        | Run desktop server.
`-r`, `--`, `--branch   | Run detached visual branch.
`-q`, `--quiet`         | Disable logging.
`--script <commands>`   | Specifies script commands to be run by the desktop when ready.
`<console>`             | Detached visual branch root.
`<cui_app ...>`         | Detached visual branch root arguments.

### Settings loading order

  - Initialize hardcoded settings.
  - Merge with explicitly specified settings from `--config <file>`.
  - If the `--config` option is not used or `<file>` cannot be loaded:
      - Merge with system-wide settings from `/etc/vtm/settings.xml` (`%PROGRAMDATA%/vtm/settings.xml` on Windows).
      - Merge with user-wise settings from `~/.config/vtm/settings.xml`.
      - Merge with DirectVT packet received from the parent process (dtvt-mode).

### Available detached visual branch roots

`<console>` value                | Object type to run detached        | Description
---------------------------------|------------------------------------|----------------------
`vtm`                            | `desk`/`Desktop Client`            | Used to run Desktop Client.
`vtm cui_app ...`                | `teletype`/`Teletype Console`      | Used to run CUI applications.
`vtm -r cui_app ...`             | `teletype`/`Teletype Console`      | Used to run CUI applications.
`vtm -r dtvt dtvt_app ...`       | `dtvt`/`DirectVT Console`          | Used to run DirectVT aware applications.
`vtm -r vtty cui_app ...`        | `teletype`/`Teletype Console`      | Used to run CUI applications.
`vtm -r term cui_app ...`        | `terminal`/`Desktop Terminal`      | Used to run CUI applications.
`vtm -r xlvt cui_dtvt_proxy ...` | `xlvt`/`DirectVT Console with TTY` | Used to run CUI applications that redirect DirectVT traffic to standard output and require user input via platform's TTY.

The following commands have a short form:
  - `vtm -r xlvt ssh <user@host dtvt_app ...>` can be shortened to `vtm ssh <user@host dtvt_app ...>`.
  - `vtm -r vtty [cui_app ...]` can be shortened to `vtm [cui_app ...]`.

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
`vtm`                                              | Run vtm desktop inside the current console.
`vtm ssh <user@server> vtm`                        | Run remote vtm desktop inside the current console over SSH.
`vtm -r term`                                      | Run Desktop Terminal inside the current console.
`vtm -r term </path/to/console/app>`               | Run a CUI application inside the Desktop Terminal.
`vtm ssh <user@server> vtm </path/to/console/app>` | Run a CUI application remotely over SSH.
`vtm --script "vtm.del(); vtm.set(splitter id=Apps); vtm.set(id=Term)"` | Run vtm desktop and reconfigure the taskbar menu.
`echo "vtm.del(); vtm.set(splitter id=Apps); vtm.set(id=Term)" \| vtm`<br><br>`echo "vtm.set(id=user@server type=xlvt cmd='ssh <user@server> vtm')" \| vtm` | Reconfigure the taskbar menu of the running desktop.
`echo "vtm.run()" \| vtm`<br><br>`echo "vtm.run(id=Term)" \| vtm`<br><br>`echo "vtm.dtvt(vtm -r term)" \| vtm` | Run a Desktop Terminal window on the running desktop.
`echo "vtm.run(title='Console \nApplication' cmd=</path/to/app>)" \| vtm` | Run a CUI application window on the running desktop.
`echo "vtm.run(type=tile title=Terminals cmd='v(h(Term,Term),Term)')" \| vtm` | Run tiling window manager with three terminals attached.
`echo "vtm.shutdown()" \| vtm`                     | Terminate the running desktop session.