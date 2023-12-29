# Text Mode Desktop

### Command-line options

 `vtm [ -c <file> ] [ -p <pipe> ] [ -i | -u ] [ -q ] [ -l | -m | -d | -s | -r [<app> [<args...>]] ]`

Option                       | Description
-----------------------------|-------------------------------------------------------
No arguments                 | Connect to the desktop (autostart new if not running).
` -c `, ` --config <file> `  | Load the specified settings file.
` -p `, ` --pipe <pipe> `    | Specify the desktop session connection point."
` -q `, ` --quiet `          | Disable logging.
` -l `, ` --listconfig `     | Print configuration.
` -m `, ` --monitor `        | Desktop session log.
` -d `, ` --daemon `         | Run desktop server in background.
` -s `, ` --server `         | Run desktop server in interactive mode.
` -r `, ` --runapp [<app>] ` | Run the specified application in standalone mode.
` -i `, ` --install `        | System-wide installation.
` -u `, ` --uninstall `      | System-wide deinstallation.
` -v `, ` --version `        | Print version.
` -? `, ` -h `, ` --help `   | Print command-line options.
` --onlylog  `               | Disable interactive user input for desktop server.

### Built-in applications

Application | Description
------------|------------------------------------------
`Term`      | Terminal emulator (default)
`Headless`  | Terminal emulator without UI
`DTVT`      | DirectVT Proxy Console
`XLVT`      | DirectVT Proxy Console with controlling terminal (Cross-linked VT)