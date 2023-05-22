# vtm

Text-based desktop environment inside your terminal*

![image](.resources/images/mde_banner_v1.18.png)

#### * Terminal Requirements

 - [Unicode/UTF-8](https://www.cl.cam.ac.uk/~mgk25/unicode.html)
 - [Grapheme Clustering](https://unicode.org/reports/tr29/#Grapheme_Cluster_Boundaries)
 - [24-bit True Color](https://github.com//termstandard/colors)
 - [xterm-style Mouse Reporting](https://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h2-Mouse-Tracking)

#### [Tested Terminals](https://github.com/netxs-group/vtm/discussions/72)

# Demo

<a href="https://www.youtube.com/watch?v=kofkoxGjFWQ">
  <img align="right" width="400" alt="Demo on YouTube" src="https://user-images.githubusercontent.com/11535558/146906370-c9705579-1bbb-4e9e-8977-47312f551cc8.gif">
</a>

### Video

 - [Desktop Environment](https://youtu.be/fLumnSctakY)
 - [Collaborative Interaction](https://youtu.be/0zU4e5Vam8c)
 - [Recursive Connection](https://youtu.be/Fm5X75sO62c)

# Supported Platforms

- POSIX-oriented
  - Linux
  - macOS
  - FreeBSD
  - NetBSD
  - OpenBSD
  - [`...`](https://en.wikipedia.org/wiki/POSIX#POSIX-oriented_operating_systems)
- MS Windows
  - Windows Server/Desktop/PE ([approx since Win8/2012](https://en.wikipedia.org/wiki/List_of_Microsoft_Windows_versions))

# Building from Source

### POSIX-oriented

Build-time dependencies
 - `git`
 - `cmake`
 - C++20 compiler (GCC 11, Clang 13, MSVC 2019)
 - Minimal requirements to compile
   - Using [`GCC`](https://gcc.gnu.org/projects/cxx-status.html) — `4GB` of RAM
   - Using [`Clang`](https://clang.llvm.org/cxx_status.html) — `9GB` of RAM

```bash
git clone https://github.com/netxs-group/vtm.git
cd ./vtm
cmake ./src -DCMAKE_BUILD_TYPE=Release
cmake --build .
cmake --install .
vtm
```

### Windows

Build-time dependencies
 - [`git`](https://git-scm.com/download/win)
 - [`cmake`](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-170#installation)
 - [`Visual Studio 2019`](https://visualstudio.microsoft.com/downloads/) or later (Desktop development with C++)

Use `Developer Command Prompt` as a build environment

`Visual Studio 2019`:
```cmd
git clone https://github.com/netxs-group/vtm.git
cd ./vtm
chcp 65001
cmake ./src -DCMAKE_BUILD_TYPE=Release "-GVisual Studio 16 2019"
cmake --build . --config Release
cd Release
vtm
```
`Visual Studio 2022`:
```cmd
git clone https://github.com/netxs-group/vtm.git
cd ./vtm
chcp 65001
cmake ./src -DCMAKE_BUILD_TYPE=Release "-GVisual Studio 17 2022"
cmake --build . --config Release
cd Release
vtm
```

# Binaries

[![](.resources/status/macos.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_macos_any.tar.gz)  
[![](.resources/status/freebsd.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_freebsd_amd64.tar.gz)  
[![](.resources/status/netbsd.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_netbsd_amd64.tar.gz)  
[![](.resources/status/openbsd.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_openbsd_amd64.tar.gz)  
[![](.resources/status/linux.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_linux_amd64.tar.gz)  
[![](.resources/status/windows.svg)](https://github.com/netxs-group/vtm/releases/latest/download/vtm_windows_amd64.zip)  

---

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

# User Interface

<table>
<thead>
  <tr>
    <th rowspan="2"></th>
    <th colspan="3">Taskbar</th>
    <th colspan="4">App window</th>
    <th colspan="2">Desktop</th>
  </tr>
  <tr>
    <th>App group</th>
    <th>Running app</th>
    <th>User list</th>
    <th>≡ Menu button</th>
    <th>Menu bar</th>
    <th>Interior</th>
    <th>Resize grips</th>
    <th>Navigation strings</th>
    <th>Free space</th>
  </tr>
</thead>
<tbody>
  <tr>
    <th>Ctrl + PgUp/Dn</th>
    <td colspan="9">Switch focus between running apps</td>
  </tr>
  <tr>
    <th>LeftClick</th>
    <td>Run app</td>
    <td>Go to app</td>
    <td></td>
    <td>Toggle fullscreen</td>
    <td colspan="3">Assign exclusive keyboard focus</td>
    <td>Go to app</td>
    <td>Clear keyboard focus</td>
  </tr>
  <tr>
    <th>Ctrl + LeftClick</th>
    <td colspan="3"></td>
    <td colspan="5">Assign/clear group keyboard focus</td>
    <td></td>
  </tr>
  <tr>
    <th>double LeftClick</th>
    <td colspan="3"></td>
    <td></td>
    <td colspan="2">Toggle fullscreen</td>
    <td colspan="3"></td>
  </tr>
  <tr>
    <th>triple Left+RightClick</th>
    <td colspan="3">Show/hide sysstat overlay</td>
    <td colspan="6"></td>
  </tr>
  <tr>
    <th>RightClick</th>
    <td>Set default app</td>
    <td>Center app window</td>
    <td colspan="1"></td>
    <td colspan="2">Minimize/restore</td>
    <td colspan="1"></td>
    <td colspan="2">Center app window</td>
    <td></td>
  </tr>
  <tr>
    <th>MiddleClick</th>
    <td colspan="5"></td>
    <td colspan="1">Selection paste</td>
    <td colspan="3"></td>
  </tr>
  <tr>
    <th>Left+RightClick</th>
    <td colspan="3"></td>
    <td colspan="5">Reset clipboard</td>
    <td></td>
  </tr>
  <tr>
    <th>LeftDrag</th>
    <td colspan="3">Adjust taskbar width</td>
    <td colspan="3">Move window or Select text</td>
    <td colspan="1">Resize window</td>
    <td colspan="1">Move window</td>
    <td>Panoramic workspace scrolling</td>
  </tr>
  <tr>
    <th>RightDrag</th>
    <td colspan="5"></td>
    <td>Panoramic content scrolling</td>
    <td colspan="2"></td>
    <td>Run default app</td>
  </tr>
  <tr>
    <th>MiddleDrag</th>
    <td colspan="9">Run default app</td>
  </tr>
  <tr>
    <th>Left+RightDrag</th>
    <td colspan="3"></td>
    <td colspan="4">Move window / Restore fullscreen</td>
    <td colspan="2">Panoramic workspace scrolling</td>
  </tr>
  <tr>
    <th>Ctrl+LeftDrag</th>
    <td colspan="3">Adjust folded width</td>
    <td colspan="3">Modify selection</td>
    <td colspan="1">Zoom window</td>
    <td colspan="2"></td>
  </tr>
  <tr>
  <tr>
    <th>Alt+LeftDrag</th>
    <td colspan="9">Switch boxed/linear selection mode</td>
  </tr>
  <tr>
    <th>Ctrl+RightDrag or Ctrl+MiddleDrag</th>
    <td colspan="9">Copy selected area to clipboard, OSC 52</td>
  </tr>
  <tr>
    <th>Wheel</th>
    <td colspan="7">Vertical scrolling</td>
    <td colspan="2"></td>
  </tr>
  <tr>
    <th>Shift+Wheel or Alt+Wheel</th>
    <td colspan="7">Horizontal scrolling</td>
    <td colspan="2"></td>
  </tr>
  <tr>
    <th>Ctrl+Wheel</th>
    <td colspan="7">Zoom window</td>
    <td colspan="2"></td>
  </tr>
</tbody>
</table>

# Documentation

- [Settings](doc/settings.md)
- [Built-in applications](doc/apps.md)

# Related Repositories

[Desktopio Framework Documentation](https://github.com/netxs-group/Desktopio-Docs)

---

[![HitCount](https://views.whatilearened.today/views/github/netxs-group/vtm.svg)](https://github.com/netxs-group/vtm) [![Twitter handle][]][twitter badge]

[//]: # (LINKS)
[twitter handle]: https://img.shields.io/twitter/follow/desktopio.svg?style=social&label=Follow
[twitter badge]: https://twitter.com/desktopio
