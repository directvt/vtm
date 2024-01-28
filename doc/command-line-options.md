# Text-based desktop environment

### Command-line options

 `vtm [ -c <file> ] [ -p <pipe> ] [ -i | -u ] [ -q ] [ -l | -m | -d | -s | -r [<app> [<args...>]] ]`

Option                       | Description
-----------------------------|-------------------------------------------------------
No arguments                 | Connect to the desktop (autostart new if not running).
` -c `, ` --config <file> `  | Load the specified settings file.
` -p `, ` --pipe <name> `    | Specify the desktop session connection point."
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
` --onlylog `                | Disable interactive user input for desktop server.
` --script <body> `          | Run the specified script on ready.

### Built-in applications

Application | Description                                                 | Usage
------------|-------------------------------------------------------------|------------------------------------
`Term`      | Terminal emulator to run cli applications.                  | `vtm -r term [cli_application]`
`noui`      | Terminal emulator without extra UI.                         | `vtm -r noui [cli_application]`
`DTVT`      | DirectVT proxy to run dtvt-apps in text console.            | `vtm -r dtvt [dtvt_application]`
`XLVT`      | DirectVT proxy with controlling terminal (Cross-linked VT), | `vtm -r xlvt ssh <user@host dtvt_application>`

The following commands have a short form:"
  - `vtm -r xlvt ssh <user@host dtvt_application>` can be shortened to `vtm ssh <user@host dtvt_application>`.
  - `vtm -r noui [cli_application]` can be shortened to `vtm -r [cli_application]`.