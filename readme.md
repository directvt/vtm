# [Monotty Desktopio Preview](https://github.com/netxs-group/VTM/releases/tag/latest)
A text-based multi-user desktop environment, [/mʌˈnɒti ˈdesktɑːpioʊ/](http://ipa-reader.xyz/?text=m%CA%8C%CB%88n%C9%92ti%20%CB%88deskt%C9%91%CB%90pio%CA%8A&voice=Joanna)

[![HitCount](https://views.whatilearened.today/views/github/netxs-group/VTM.svg)](https://github.com/netxs-group/VTM) [![Gitter](https://badges.gitter.im/netxs-group/VTM.svg)](https://gitter.im/netxs-group/VTM?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge) [![Twitter handle][]][twitter badge]

![image](https://dice.netxs.online/cloud/vtm/mde_banner_v1.07.png)

# Demo

- Live SSH Demo  

     `ssh vtm@netxs.online`  

- Video
  - [Desktop environment](https://youtu.be/fLumnSctakY)
  - [Collaborative interaction](https://youtu.be/0zU4e5Vam8c)
  - [Recursive connection](https://youtu.be/Fm5X75sO62c)

# Supported Platforms

 - GNU/Linux amd64
 - Windows
    - Windows 10 (32/64)
    - Windows Server 2019 (32/64)
 - macOS
    - Catalina 10.15

# Installation
[![](https://dice.netxs.online/cloud/vtm/status/macos)](https://github.com/netxs-group/VTM/releases)  [![](https://dice.netxs.online/cloud/vtm/status/linux)](https://github.com/netxs-group/VTM/releases)
```bash
if   [[ "$OSTYPE" == "linux-gnu"* ]]; then release=vtm_linux_amd64
elif [[ "$OSTYPE" == "darwin"*    ]]; then release=vtm_macos
else exit 1; fi
tmpdir=$(mktemp -d); cd $tmpdir
wget "https://github.com/netxs-group/VTM/releases/download/latest/${release}.tar.gz"
tar -zxvf "${release}.tar.gz"; cd "./${release}"
cat ./install.sh
echo "tmpdir=${tmpdir}"
sudo ./install.sh; rm -rf $tmpdir; cd ~
```

[![](https://dice.netxs.online/cloud/vtm/status/windows)](https://github.com/netxs-group/VTM/releases)
```cmd
cd %TEMP%
set release=vtm_windows_64
curl -LJO https://github.com/netxs-group/VTM/releases/download/latest/%release%.zip
tar -zxvf %release%.zip & cd ./%release%
notepad ./install.ps1
powershell ./install.ps1
```

# Command Line Options

Module                         | Options
-------------------------------|--------------------------------------
`vtm(.exe)` desktopio client   | No arguments
`vtmd(.exe)` desktopio server  | `[ -d ]` run in background

# User Interface Commands

`ButtonClick + Drag` = `ButtonDrag`

Shortcut              | Action
----------------------|--------------------------------------
`Ctrl + PgUp/Dn`      | Switch between windows
`LeftClick`           | Assign exclusive keyboard focus
`Ctrl + LeftClick`    | Assign/clear group keyboard focus
double `LeftClick`    | Window: Maximize/restore
`RightClick`          | Desktop: Call menu
`Left + Right`<br>or `MiddleClick` | Window: Close/destroy
`LeftDrag`            | Desktop: Move visible windows<br>Window: Resize/move
`RightDrag`           | Desktop: Create new window<br>Window: Scroll content
`Left + RightDrag`    | Scroll workspace
`Ctrl + RightDrag`<br>or `Ctrl + MiddleDrag` | Copy selected area to clipboard `OSC 52`

# Built-in Applications

- `▀▄ Term` Terminal emulator
- `▀▄ Logs` Debugging tool for monitoring applications' output in real time
- `▀▄ Info` Software documentation browser _(not ready)_
- `▀▄ Shop` A digital distribution platform _(not ready)_
- `▀▄ Text` A simple text editor _(not ready)_
- `▀▄ Calc` A simple spreadsheet program like VisiCalc or Lotus 1-2-3 _(not ready)_
- `▀▄ Clip` Clipboard manager _(not ready)_
- `▀▄ Draw` ANSI-artworks application _(not ready)_
- `▀▄ Task` Task manager (desktopio) _(not ready)_
- `▀▄ Char` Unicode code points browser _(not ready)_
- `▀▄ File` An orthodox file manager _(not ready)_
- `▀▄ Time` A calendar application _(not ready)_
- `▀▄ Goto` Internet/SSH browser _(not ready)_
- `▀▄ Game` Doom-II _(not ready)_

<details><summary>show details...</summary><p>

- `▀▄ Term`
   - TrueColor/256-color support
   - Auto-wrap mode `DECAWM`
   - Focus tracking `DECSET 1004`
   - Bracketed paste mode `DECSET 2004`
   - SGR attributes: overline, double underline, strikethrough
   - Save/restore terminal window title `XTWINOPS 22/23`
   - Mouse Tracking `DECSET 1000/1002/1003/1006 SGR` mode
   - Mouse tracking `DECSET 10060 Extended SGR` mode, mouse reporting outside of the terminal viewport (outside + negative arguments) #62
   - Scrollback buffer size is limited to 20000 lines (temporarily hardcoded value)

- `▀▄ Logs`
  - Reset by double `RightClick`

- `▀▄ Info`
  - ...

- `▀▄ Shop`
  - Just a sketch

- `▀▄ Text`
  - Just a sketch

- `▀▄ Calc`
  - Just a sketch

- `▀▄ Clip`
  - ...

- `▀▄ Draw`
  - ...

- `▀▄ Task`
  - ...

- `▀▄ Char`
  - ...

- `▀▄ File`
  - ...

- `▀▄ Time`
  - ...

- `▀▄ Goto`
  - ...

- `▀▄ Game`
  - ...

</p></details>

# Related Repositories

[Desktopio Framework Documentation](https://github.com/netxs-group/Desktopio-Docs)

[//]: # (LINKS)
[twitter handle]: https://img.shields.io/twitter/follow/desktopio.svg?style=social&label=Follow
[twitter badge]: https://twitter.com/desktopio