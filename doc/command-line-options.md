# Text-based desktop environment

### Command-line options

```
 vtm [ -i | -u ] | [ -v ] | [ -? ] | [ -c <file> ][ -l ]

 vtm [ --script <body> ][ -p <name> ][ -c <file> ][ -q ]
     [ -m | -d | -s | -r [<app> [<args...>]] ]
```

Option                       | Description
-----------------------------|-------------------------------------------------------
No arguments                 | Connect to the desktop (autostart new if not running).
` -h `, ` -? `, ` --help `   | Print command-line options.
` -v `, ` --version `        | Print version.
` -l `, ` --listconfig `     | Print configuration.
` -i `, ` --install `        | Perform system-wide installation.
` -u `, ` --uninstall `      | Perform system-wide deinstallation.
` -c `, ` --config <file> `  | Specifies the settings file to load.
` -p `, ` --pipe <name> `    | Specifies the desktop session connection point.
` -m `, ` --monitor `        | Run desktop session log monitor.
` -d `, ` --daemon `         | Run desktop server in background.
` -s `, ` --server `         | Run desktop server in interactive mode.
` -r `, ` --runapp [args] `  | Run the specified application in standalone mode.
` -q `, ` --quiet `          | Disable logging.
` --script <body> `          | Specifies a script to run when ready.

### Settings loading order

  - Initialize hardcoded settings.
  - Merge with explicitly specified settings from `--config <file>`.
  - If the `--config` option is not used or `<file>` cannot be loaded:
      - Merge with system-wide settings from `/etc/vtm/settings.xml` (`%PROGRAMDATA%/vtm/settings.xml` on Windows).
      - Merge with user-wise settings from `~/.config/vtm/settings.xml`.
      - Merge with DirectVT packet received from the parent process (dtvt-mode).

### Built-in applications

Application | Description                                                                           | Syntax
------------|---------------------------------------------------------------------------------------|------------------------------------
`Term`      | Terminal emulator to run cli applications.                                            | `vtm [options ...] -r term [cli_app ...]`
`NoUI`      | Terminal emulator without extra UI.                                                   | `vtm [options ...] -r noui [cli_app ...]`
`DTVT`      | DirectVT proxy to run dtvt-apps in text console.                                      | `vtm [options ...] -r dtvt [dtvt_app ...]`
`XLVT`      | DirectVT proxy with controlling terminal to run dtvt-apps over SSH.<br>`XLVT` stands for Cross-linked VT. | `vtm [options ...] -r xlvt ssh <user@host dtvt_app ...>`

The following commands have a short form:
  - `vtm -r xlvt ssh <user@host dtvt_app ...>` can be shortened to `vtm ssh <user@host dtvt_app ...>`.
  - `vtm -r noui [cli_app ...]` can be shortened to `vtm -r [cli_app ...]`.

### Usage Examples

Command                                  | Description
-----------------------------------------|--------------------------------------------
`vtm`                                    | Run vtm desktop inside the current console.
`vtm ssh <user@server> vtm`              | Run remote vtm desktop inside the current console over SSH.
`vtm -r [term]`                          | Run the built-in terminal inside the current console.
`vtm -r [term] </path/to/console/app>`   | Run an application inside the built-in terminal.
`vtm ssh <user@server> vtm -r [term] </path/to/console/app>` | Run an application remotely over SSH.
`vtm --script "vtm.del(); vtm.set(splitter id=Apps); vtm.set(id=Term)"` | Run vtm desktop and reconfigure the taskbar menu.
`echo "vtm.del(); vtm.set(splitter id=Apps); vtm.set(id=Term)" \| vtm`<br><br>`echo "vtm.set(id=user@server type=xlvt cmd='ssh <user@server> vtm')" \| vtm` | Reconfigure the taskbar menu of the running desktop.
`echo "vtm.run()" \| vtm`<br><br>`echo "vtm.run(id=Term)" \| vtm`<br><br>`echo "vtm.dtvt(vtm -r term)" \| vtm` | Run a terminal window on the running desktop.
`echo "vtm.run(title='Console \nApplication' cmd=</path/to/app>)" \| vtm` | Run an application window on the running desktop.
`echo "vtm.run(type=group title=Terminals cmd='v(h(Term,Term),Term)')" \| vtm` | Run tiling window manager with three terminals attached.
`echo "vtm.shutdown()" \| vtm`           | Terminate the running desktop session.