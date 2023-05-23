# Command line Options `vtm(.exe)`

 `vtm [ -c <file> ] [ -p <pipe> ] [ -q ] [ -l | -m | -d | -s | -r [<app> [<args...>]] ]`

Option                     | Description
---------------------------|-------------------------------------------------------
No arguments               | Run client (auto start server)
` -c \| --config <file> `  | Use specified configuration file
` -p \| --pipe <pipe> `    | Set the pipe to connect to
` -q \| --quiet `          | Disable logging
` -l \| --listconfig `     | Show configuration and exit
` -m \| --monitor `        | Monitor server log
` -d \| --daemon `         | Run server in background
` -s \| --server `         | Run server in interactive mode
` -r \| --runapp [<app>] ` | Run the specified `<app>` in offline mode<br>`Term` Terminal emulator (default)<br>`Calc` (Demo) Spreadsheet calculator<br>`Text` (Demo) Text editor<br>`Gems` (Demo) Desktopio application manager
` -v \| --version `        | Show version and exit
` -? \| -h \| --help `     | Show usage message