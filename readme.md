# Monotty Desktopio

Text-based desktop environment inside your terminal*

![image](https://dice.netxs.online/cloud/vtm/mde_banner_v1.18.png)

#### * Terminal Requirements

 - [Unicode/UTF-8](https://www.cl.cam.ac.uk/~mgk25/unicode.html)
 - [Grapheme Clustering](https://unicode.org/reports/tr29/#Grapheme_Cluster_Boundaries)
 - [24-bit True Color](https://gist.github.com/XVilka/8346728)
 - [xterm-style Mouse Reporting](https://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h2-Mouse-Tracking)

#### [Tested Terminals](https://github.com/netxs-group/VTM/discussions/72)

# Demo
### Live SSH

 - `ssh vtm@netxs.online`

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
- Windows
  - Windows 10
  - Windows Server 2019

# Building from Source

### POSIX-oriented

Build-time dependencies:
 - `git`
 - `cmake`
 - [`gcc`](https://gcc.gnu.org/projects/cxx-status.html) or [`clang`](https://clang.llvm.org/cxx_status.html) with support for C++20

```bash
git clone https://github.com/netxs-group/VTM.git && cd ./VTM
cmake ./src -DCMAKE_BUILD_TYPE="Release"
cmake --build .
cp -v ./vtm* "/usr/bin/"
```

### Windows

Build-time dependencies: `Visual Studio 2019`

Use `Developer Command Prompt for VS 2019`
```cmd
git clone https://github.com/netxs-group/VTM.git && cd ./VTM
cmake ./src -DCMAKE_BUILD_TYPE=Release "-GVisual Studio 16 2019"
cmake --build . --config Release
cd ./Release
powershell ../src/install/install.ps1
```

# Releases for `amd64`

[![](https://dice.netxs.online/cloud/vtm/status/macos)](https://github.com/netxs-group/VTM/releases/latest/download/vtm_macos_amd64.tar.gz)  
[![](https://dice.netxs.online/cloud/vtm/status/freebsd)](https://github.com/netxs-group/VTM/releases/latest/download/vtm_freebsd_amd64.tar.gz)  
[![](https://dice.netxs.online/cloud/vtm/status/netbsd)](https://github.com/netxs-group/VTM/releases/latest/download/vtm_netbsd_amd64.tar.gz)  
[![](https://dice.netxs.online/cloud/vtm/status/openbsd)](https://github.com/netxs-group/VTM/releases/latest/download/vtm_openbsd_amd64.tar.gz)  
[![](https://dice.netxs.online/cloud/vtm/status/linux)](https://github.com/netxs-group/VTM/releases/latest/download/vtm_linux_amd64.tar.gz)  
[![](https://dice.netxs.online/cloud/vtm/status/windows)](https://github.com/netxs-group/VTM/releases/latest/download/vtm_windows_amd64.zip)  

---

# Command Line Options

Module               | Options
---------------------|--------------------------------------
`vtm(.exe)` client   | No arguments
`vtmd(.exe)` server  | `[ -d ]` run in background

# User Interface Commands

`ButtonClick + Drag` = `ButtonDrag`

Shortcut              | Action
----------------------|--------------------------------------
`Ctrl + PgUp/Dn`      | Switch between windows
`LeftClick`           | Taskbar: Go to window<br>Window: Assign exclusive keyboard focus
`Ctrl + LeftClick`    | Assign/clear group keyboard focus
double `LeftClick`    | Menu: Create new window<br>Window: Maximize/restore window
`RightClick`          | Taskbar: Move window to center of view
`Left + Right`<br>or `MiddleClick` | Window: Close/destroy window
`LeftDrag`            | Desktop: Scroll workspace<br>Window header: Move window
`RightDrag`           | Desktop: Create new window<br>Window: Panoramic scrolling
`Left + RightDrag`    | Scroll workspace
`Ctrl + RightDrag`<br>or `Ctrl + MiddleDrag` | Copy selected area to clipboard `OSC 52`
`Wheel`               | Window: Vertical scrolling
`Shift + Wheel`<br>or `Ctrl + Wheel` | Window: Horizontal scrolling

# Built-in Applications

- `▀▄ Term` Terminal emulator
- `▀▄ Logs` Debug output console
- `▀▄ View` Workspace navigation helper

<details><summary>show details...</summary><p>

 - `▀▄ Term`
   - UTF-8 Everywhere
   - Unicode clustering
   - TrueColor/256-color support
   - Auto-wrap mode `DECAWM` (with horizontal scrolling)
   - Focus tracking `DECSET 1004`
   - Bracketed paste mode `DECSET 2004`
   - SGR attributes: overline, double underline, strikethrough, and others
   - Save/restore terminal window title `XTWINOPS 22/23`
   - Mouse tracking `DECSET 1000/1002/1003/1006 SGR` mode
   - Mouse tracking `DECSET 10060 Extended SGR` mode, mouse reporting outside of the terminal viewport (outside + negative arguments) #62
   - Configurable using VT-sequences

      Name         | Sequence                         | Description
      -------------|----------------------------------|-------------
      `CCC_SBS`    | `CSI` 24 : n : m `p`             | Set scrollback buffer size, `int32_t`<br>`n` Buffer limit in lines, 0 is unlimited, _default is 20.000_<br>`m` Grow step for unlimited buffer, _default is 0_
      `CCC_RST`    | `CSI` 1 `p`                      | Reset all parameters to default
      `CCC_TBS`    | `CSI` 5 : n `p`                  | Set tabulation length<br>`n` Length in chars, _max = 256, default is 8_
      `CCC_JET`    | `CSI` 11 : n `p`                 | Set text alignment, _default is Left_<br>`n = 0` default<br>`n = 1` Left<br>`n = 2` Right<br>`n = 3` Center
      `CCC_WRP`    | `CSI` 12 : n `p`                 | Set text autowrap mode, _default is On_<br>`n = 0` default<br>`n = 1` On<br>`n = 2` Off (_enable horizontal scrolling_)
      `CCC_RTL`    | `CSI` 13 : n `p`                 | Set text right-to-left mode, _default is Off_<br>`n = 0` default<br>`n = 1` On<br>`n = 2` Off

 - `▀▄ Logs`
   - Debug output console. Use double `RightClick` to clear scrollback.

 - `▀▄ View`
   - Serves for quick navigation through the desktop space using cyclic selection (left click on group title) in the `View` group on the taskbar.

</p></details>

# Related Repositories

[Desktopio Framework Documentation](https://github.com/netxs-group/Desktopio-Docs)

---

[![HitCount](https://views.whatilearened.today/views/github/netxs-group/VTM.svg)](https://github.com/netxs-group/VTM) [![Twitter handle][]][twitter badge]

[//]: # (LINKS)
[twitter handle]: https://img.shields.io/twitter/follow/desktopio.svg?style=social&label=Follow
[twitter badge]: https://twitter.com/desktopio
