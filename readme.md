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
 - `cmake` (https://github.com/netxs-group/vtm/issues/172#issuecomment-1340519942)
 - C++20 compiler
 - Minimal requirements to compile
   - Using [`GCC`](https://gcc.gnu.org/projects/cxx-status.html) — `3GB` of RAM
   - Using [`Clang`](https://clang.llvm.org/cxx_status.html) — `8GB` of RAM

```bash
git clone https://github.com/netxs-group/vtm.git && cd ./vtm
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
 - `UTF-8` for worldwide language support, https://github.com/netxs-group/vtm/issues/175#issuecomment-1034734346

Use `Developer Command Prompt` as a build environment

`Visual Studio 2019`:
```cmd
git clone https://github.com/netxs-group/vtm.git
cd ./vtm
cmake ./src -DCMAKE_BUILD_TYPE=Release "-GVisual Studio 16 2019"
cmake --build . --config Release
cd Release
vtm
```
`Visual Studio 2022`:
```cmd
git clone https://github.com/netxs-group/vtm.git
cd ./vtm
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
    <th>≡ Menu</th>
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
    <td colspan="9">Switch between running apps and assign exclusive keyboard focus</td>
  </tr>
  <tr>
    <th>LeftClick</th>
    <td>Run app</td>
    <td>Go to app</td>
    <td></td>
    <td>Maximize/restore</td>
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
    <td colspan="2">Maximize/restore app window</td>
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
    <td colspan="2">Toggle menu height</td>
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
    <td colspan="5">Clear clipboard</td>
    <td></td>
  </tr>
  <tr>
    <th>LeftDrag</th>
    <td colspan="3">Adjust taskbar width</td>
    <td colspan="5">Move window or Select text</td>
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
    <td colspan="4">Move window / Restore maximized window</td>
    <td colspan="2">Panoramic workspace scrolling</td>
  </tr>
  <tr>
    <th>Ctrl+LeftDrag</th>
    <td colspan="3">Adjust folded width</td>
    <td colspan="6">Modify selection</td>
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
    <th>Shift+Wheel or Ctrl+Wheel</th>
    <td colspan="7">Horizontal scrolling</td>
    <td colspan="2"></td>
  </tr>
</tbody>
</table>

# Command line Options `vtm(.exe)`

 `vtm [ -c <config_file> ] [ -l | -d | -s | -r [<app> [<args...>]] ]`

Option         | Description
---------------|-------------------------------------------------------
No arguments   | Run client (auto start server)
` -c <file> `  | Use specified configuration file
` -l `         | Show configuration and exit
` -d `         | Run server in background
` -s `         | Run server in interactive mode
` -r [<app>] ` | Run the specified `<app>` in offline mode<br>`Term` Terminal emulator (default)<br>`Calc` (Demo) Spreadsheet calculator<br>`Text` (Demo) Text editor<br>`Gems` (Demo) Desktopio application manager

Configuration file location precedence (descending priority):<br>
1. Command line options `vtm -c path/to/settings.xml`<br>
2. Environment variable `VTM_CONFIG=path/to/settings.xml`<br>
3. Hardcoded location `~/.config/vtm/settings.xml`<br>
4. Use predefined configuration at apps.hpp(~line:28)

# Settings

vtm can be configured in the `~/.config/vtm/settings.xml` file in xml format. Alternative configuration file location can be specified using command line option ` -c <config_file> ` or using environment variable VTM_CONFIG.

## Configuration file Format (settings.xml)

Configuration file format is a slightly modified XML-format which allows to store hierarchical list of key=value pairs.

### Key differences from the standard XML

 - All stored values are UTF-8 strings:
   - `name=2000` and `name="2000"` has the same meaning.
 - There is no distinction between XML-attribute and XML-subobject, i.e. any attributes are sub-objects:
   - `<name param=value />` and `<name> <param=value /> </name>` has the same meaning.
 - In addition to a set of sub-objects each object can contain its own text value:
   - E.g. `<name=names_value param=params_value />`.
 - Each object can be defined in any way, either using an XML-attribute or an XML-subobject syntax:
   - `<... name=value />`, `<...> <name> "value" </name> </...>`, and `<...> <name=value /> </...>` has the same meaning.
 - The object name that ending in an asterisk indicates that this object is not an object, but it is a template for all subsequent objects with the same name in the same scope. See `Template Example` below.
 - Character escapes
   - `\e`  ASCII 0x1B ESC
   - `\t`  ASCII 0x09 TAB
   - `\a`  ASCII 0x07 BEL
   - `\n`  ASCII 0x0A LF
   - `\\`  ASCII 0x5C Backslash
   - `\"`  ASCII 0x22 Quotes
   - `\'`  ASCII 0x27 Single quote
   - `$0`  Current module full path

Consider the following object hierarchy

- \<document\> - Top-level element
  - \<thing\> - Second level element
    - \<name\> - Third level element

The following forms of element declaration are equivalent

```xml
<document>
    <thing name="a">text1</thing>
    <thing name="b">text2</thing>
</document>
```

```xml
<document>
    <thing="text1" name="a"/>
    <thing="text2" name="b"/>
</document>
```

```xml
<document>
    <thing name="a">
        "text1"
    </thing>
    <thing name="b">
        "text2"
    </thing>
</document>
```

```xml
<document>
    <thing>
        "text1"
        <name="a"/>
    </thing>
    <thing>
        <name="b"/>
        "text2"
    </thing>
</document>
```

```xml
<document>
    <thing="t">
        "ext"
        <name>
            "a"
        </name>
        "1"
    </thing>
    <thing>
        <name>
            "b"
        </name>
        "text"
        "2"
    </thing>
</document>
```

#### Templates

Use asterisk at the end of the element name to set defaults. Using an asterisk with the parameter name of the first element in the list without any other nested arguments indicates the beginning of the list, i.e. the list will replace the existing one when the configuration is merged.

The following declarations are the same

```xml
<document>
    <thing name="text">another_text</thing>
    <thing name="text">another_text</thing>
</document>
```

```xml
<document>
    <thing* name="text"/> <!-- skip this element and set name="text" as default for the following things -->
    <thing>another_text</thing>
    <thing>another_text</thing>
</document>
```

```xml
<document>
    <thing* name="text"/>
    <thing="another_text"/>
    <thing="another_text"/>
</document>
```

```xml
<document>
    <thing*="another_text" name="text"/>  <!-- skip this element and set thing="another_text" and name="text" as default for the following things -->
    <thing/>
    <thing/>
</document>
```

### Configuration Structure

Top-level element `<config>` contains the following base objects
  - Single `<menu>` block - taskbar menu configuration.
    - Single `<selected>` object - the value of this attribute specifies which menu item id will be selected by default at the environment startup.
    - Set of `<item>` objects - a list of menu item definitions.
    - Not implemented: Single `<autorun>` block - a list of menu item to run at the environment startup.
  - Not implemented: Single `<hotkeys>` block - a global hotkeys/shortcuts configuration.

#### Application Configuration

The menu item of DirectVT type `type=DirectVT` can be additionally configured using `<config>` subelement. This is currently only supported by the built-in vtm terminal.

The content of the `<config>` subelement is passed to the application upon startup. This config has the highest priority and is merged with the root of the configuration loaded by this application from a file.

In general, when a DirectVT application starts up, the three configurations are subsequently merged. They are listed below in merged order

- Hardcoded defaults
- Configuration loaded from file
- The configuration received at startup from the launching application (see the `<config>` subelement example)

#### Taskbar menu item attributes

Attribute  | Description                                       | Value type | Mandatory | Default value
-----------|---------------------------------------------------|------------|-----------|---------------
`id`       |  Item textual identifier                          | `string`   | required  |
`alias`    |  Use existing item specified by `id` as template  | `string`   |           |
`hidden`   |  Item visibility                                  | `boolean`  |           | `no`
`label`    |  Item label text                                  | `string`   |           | =`id`
`notes`    |  Item tooltip text                                | `string`   |           | empty
`title`    |  App window title                                 | `string`   |           | empty
`footer`   |  App window footer                                | `string`   |           | empty
`bgc`      |  App window background color                      | `RGBA`     |           |
`fgc`      |  App window foreground color                      | `RGBA`     |           |
`winsize`  |  App window 2D size                               | `x;y`      |           |
`slimmenu` |  App window menu vertical size                    | `boolean`  |           | `no`
`cwd`      |  Current working directory                        | `string`   |           |
`type`     |  App type                                         | `string`   |           | `SHELL`
`param`    |  App constructor arguments                        | `string`   |           | empty
`config`   |  Configuration block for DirectVT apps            | `xml-node` |           | empty

#### Value literals

Type     | Format
---------|-----------------
`RGBA`   |  `#rrggbbaa` \| `0xaabbggrr` \| `rrr,ggg,bbb,aaa` \| 256-color index
`boolean`|  `true` \| `false` \| `yes` \| `no` \| `1` \| `0` \| `on` \| `off`
`index`  |  0 .. N
`string` |  _UTF-8 text string_
`x;y`    |  _integer_ <any_delimeter> _integer_

#### App type

Type              | Parameter        | Description
------------------|------------------|-----------
`DirectVT`        | `_command line_` | Run `_command line_` using DirectVT protocol. Usage example `type=DirectVT param="_command line_"`.
`ANSIVT`          | `_command line_` | Run `_command line_` inside the built-in terminal. Usage example `type=ANSIVT param="_command line_"`. Same as `type=DirectVT param="$0 -r term _command line_"`.
`SHELL` (default) | `_command line_` | Run `_command line_` on top of a system shell that runs inside the built-in terminal. Usage example `type=SHELL param="_command line_"`. Same as `type=DirectVT param="$0 -r term _shell_ -c _command line_"`.
`Group`           | [ v[`n:m:w`] \| h[`n:m:w`] ] ( id_1 \| _nested_block_ , id_2 \| _nested_block_ )] | Run tiling window manager with layout specified in `param`. Usage example `type=Group param="h1:1(Term, Term)"`.
`Region`          | | The `param` attribute is not used, use attribute `title=_view_title_` to set region name.

The following configuration items have the same meaning
```
<item …. param=‘mc’/>
<item …. type=SHELL param=‘mc’/>
<item …. type=ANSIVT param=‘bash -c mc’/>
<item …. type=DirectVT param=‘$0 -r term bash -c mc’/>
```

### Configuration Example

Note: The following configuration sections are not implemented yet
- config/menu/item/hotkeys
- config/menu/autorun
- config/hotkeys

`~/.config/vtm/settings.xml`
```xml
<config>
    <menu>
        <selected=Term /> <!-- Set selected using menu item id. -->
        <item*/>  <!-- Use asterisk at the end of the element name to set defaults.
                       Using an asterisk with the parameter name of the first element in the list without any other nested arguments
                       indicates the beginning of the list, i.e. the list will replace the existing one when the configuration is merged. -->
        <item splitter label="apps">
            <notes>
                " Default applications group                         \n"
                " It can be configured in ~/.config/vtm/settings.xml "
            </notes>
        </item>
        <item* hidden=no slimmenu=false type=SHELL fgc=whitedk bgc=0x00000000 winsize=0,0 wincoor=0,0 />
        <item id=Term label="Term" type=DirectVT title="Terminal Emulator" notes=" run built-in Terminal " param="$0 -r term">
            <config>   <!-- The following config partially overrides the base configuration. It is valid for DirectVT apps only. -->
                <term>
                    <scrollback>
                        <size=35000    />   <!-- Scrollback buffer length. -->
                        <wrap="on"     />   <!-- Lines wrapping mode. -->
                    </scrollback>
                    <color>
                        <color4  = bluedk     /> <!-- See /config/set/* for the color name reference. -->
                        <color15 = whitelt    />
                        <default bgc=0 fgc=15 />  <!-- Initial colors. -->
                    </color>
                    <cursor>
                        <style="underline"/> <!-- block | underline  -->
                    </cursor>
                    <menu>
                        <autohide=off/>  <!--  If true/on, show menu only on hover. -->
                        <enabled="on"/>
                        <slim=off/>
                    </menu>
                </term>
            </config>
        </item>
        <item id=PowerShell label="PowerShell" type=DirectVT title="PowerShell"                  param="$0 -r term powershell" fgc=15 bgc=0xFF562401 notes=" run PowerShell "/>
        <item id=WSL        label="WSL"        type=DirectVT title="Windows Subsystem for Linux" param="$0 -r term wsl"                              notes=" run default WSL profile "/>
   <!-- <item id=Far        label="Far"        type=SHELL    title="Far Manager"                 param="far"                                         notes=" run Far Manager in its own window "/> -->
   <!-- <item id=mc         label="mc"         type=SHELL    title="Midnight Commander"    param="mc"               notes=" run Midnight Commander in its own window "/> -->
        <item id=Tile       label="Tile"       type=Group    title="Tiling Window Manager" param="h1:1(Term, Term)" notes=" run Tiling Window Manager with two terminals attached "/>
        <item id=View       label=View         type=Region   title="\e[11:3pView: Region"                           notes=" set desktop region "/>
        <item id=Settings   label=Settings     type=DirectVT title="Settings"              param="$0 -r settings"   notes=" run Settings " winsize=50,15 />
        <item id=Logs       label=Logs         type=DirectVT title="Logs Title"            param="$0 -r logs"       notes=" run Logs "/>
   <!-- <item splitter label="demo" notes=" Demo apps                    \n Feel the Desktopio Framework "/> -->
   <!-- <item id=Gems       label="Gems"       type=DirectVT title="Gems Title"            param="$0 -r gems"       notes=" App Distribution Hub "/> -->
   <!-- <item id=Text       label="Text"       type=DirectVT title="Text Title"            param="$0 -r text"       notes=" Text Editor "/> -->
   <!-- <item id=Calc       label="Calc"       type=DirectVT title="Calc Title"            param="$0 -r calc"       notes=" Spreadsheet Calculator "/> -->
   <!-- <item id=Test       label="Test"       type=DirectVT title="Test Title"            param="$0 -r test"       notes=" Test Page "/> -->
   <!-- <item id=Truecolor  label="Truecolor"  type=DirectVT title="True Title"            param="$0 -r truecolor"  notes=" Truecolor Test "/> -->
        <autorun>    <!-- not implemented -->
            <item*/>
            <item*=Term winsize=48%,48% /> <!-- item*=_item_id_ - assign the same _item_id_ to each item by default. -->
            <item wincoor=0,0 />
            <item wincoor=52%,0 />
            <item wincoor=0,52% />
            <item=mc wincoor=52%,52% />
        </autorun>
        <width>    <!-- not implemented -->
            <folded=4/>
            <expanded=31/>
        </width>
    </menu>
    <hotkeys>    <!-- not implemented -->
        <key*/>
        <key="Ctrl+PgUp" action=prevWindow />
        <key="Ctrl+PgDn" action=nextWindow />
    </hotkeys>
    <appearance>
        <defaults>
            <fps      = 60   />
            <bordersz = 1,1  />
            <brighter = 60   />
            <kb_focus = 60   />
            <shadower = 180  />
            <shadow   = 180  />
            <lucidity = 0xff /> <!-- not implemented -->
            <selector = 48   />
            <highlight  fgc=purewhite bgc=bluelt      />
            <warning    fgc=whitelt   bgc=yellowdk    />
            <danger     fgc=whitelt   bgc=redlt       />
            <action     fgc=whitelt   bgc=greenlt     />
            <label      fgc=blackdk   bgc=whitedk     />
            <inactive   fgc=blacklt   bgc=transparent />
            <menu_white fgc=whitelt   bgc=0x80404040  />
            <menu_black fgc=blackdk   bgc=0x80404040  />
            <fader duration=0ms fast=0ms />  <!-- Fader animation config. -->
        </defaults>
        <runapp>    <!-- Override defaults. -->
            <brighter=0 />
        </runapp>
    </appearance>
    <set>         <!-- Global namespace - Unresolved literals will be taken from here. -->
        <blackdk   = 0xFF101010 /> <!-- Color reference literals. -->
        <reddk     = 0xFF1f0fc4 />
        <greendk   = 0xFF0ea112 />
        <yellowdk  = 0xFF009cc0 />
        <bluedk    = 0xFFdb3700 />
        <magentadk = 0xFF981787 />
        <cyandk    = 0xFFdd963b />
        <whitedk   = 0xFFbbbbbb />
        <blacklt   = 0xFF757575 />
        <redlt     = 0xFF5648e6 />
        <greenlt   = 0xFF0cc615 />
        <yellowlt  = 0xFFa5f1f8 />
        <bluelt    = 0xFFff783a />
        <magentalt = 0xFF9e00b3 />
        <cyanlt    = 0xFFd6d660 />
        <whitelt   = 0xFFf3f3f3 />
        <pureblack = 0xFF000000 />
        <purewhite = 0xFFffffff />
        <nocolor   = 0x00000000 />
        <transparent = nocolor  />
    </set>
    <client>
        <background fgc=whitedk bgc=0xFF000000 />  <!-- Desktop background color. -->
        <clipboard>
            <preview enabled=true size=80x25 bgc=bluedk fgc=whitelt>
                <alpha=0xFF />  <!-- Preview alpha is applied only to the ansi/rich/html text type -->
                <timeout=3s />  <!-- Preview hiding timeout. Set it to zero to disable hiding. -->
                <shadow=7   />  <!-- Preview shadow strength (0-10). -->
            </preview>
        </clipboard>
        <viewport coor=0,0 />
        <mouse dblclick=500ms />
        <tooltip timeout=500ms enabled=true fgc=pureblack bgc=purewhite />
        <glowfx=true />                      <!-- Show glow effect around selected item. -->
        <debug overlay=faux toggle="🐞" />  <!-- Display console debug info. -->
        <regions enabled=faux />             <!-- Highlight UI objects boundaries. -->
    </client>
    <term>      <!-- Base configuration for the Term app. It can be partially overridden by the menu item's config subarg. -->
        <scrollback>
            <size=20000    />   <!-- Scrollback buffer length. -->
            <growstep=0    />   <!-- Scrollback buffer grow step. The buffer behaves like a ring in case of zero. -->
            <maxline=65535 />   <!-- Max line length. Line splits if it exceeds the limit. -->
            <wrap="on"     />   <!-- Lines wrapping mode. -->
            <reset onkey="on" onoutput="off" />   <!-- Scrollback viewport reset triggers. -->
        </scrollback>
        <color>
            <color0  = blackdk    /> <!-- See /config/set/* for the color name reference. -->
            <color1  = reddk      />
            <color2  = greendk    />
            <color3  = yellowdk   />
            <color4  = bluedk     />
            <color5  = magentadk  />
            <color6  = cyandk     />
            <color7  = whitedk    />
            <color8  = blacklt    />
            <color9  = redlt      />
            <color10 = greenlt    />
            <color11 = yellowlt   />
            <color12 = bluelt     />
            <color13 = magentalt  />
            <color14 = cyanlt     />
            <color15 = whitelt    />
            <default bgc=0 fgc=15 />  <!-- Initial colors. -->
            <match fx=selection bgc="0xFF007F00" fgc=whitelt />  <!-- Color of the selected text occurrences. Set fx to use cell::shaders: xlight | selection | contrast | invert | reverse -->
            <selection>
                <text fx=selection bgc=bluelt fgc=whitelt />  <!-- Highlighting of the selected text in plaintext mode. -->
                <protected fx=selection bgc=bluelt fgc=whitelt />
                <ansi fx=xlight/>
                <rich fx=xlight/>
                <html fx=xlight/>
                <none fx=selection bgc=blacklt fgc=whitedk />  <!-- Inactive selection color. -->
            </selection>
        </color>
        <fields>
            <lucent=0xC0 /> <!-- Fields transparency level. -->
            <size=0      /> <!-- Left/right field size (for hz scrolling UX). -->
        </fields>
        <tablen=8 />   <!-- Tab length. -->
        <cursor>
            <style="underline"/> <!-- block | underline -->
            <blink=400ms/>       <!-- blink period -->
            <show=true/>
        </cursor>
        <menu>
            <autohide=true/>  <!--  If true/on, show menu only on hover. -->
            <enabled="on"/>
            <slim=true />
        </menu>
        <selection>
            <mode="text"/> <!-- text | ansi | rich | html | protected | none -->
        </selection>
        <atexit = auto /> <!-- auto:    Stay open and ask if exit code != 0. (default)
                               ask:     Stay open and ask.
                               close:   Always close.
                               restart: Restart session.
                               retry:   Restart session if exit code != 0. -->
        <hotkeys>    <!-- not implemented -->
            <key*/>
            <key="Alt+RightArrow" action=findNext />
            <key="Alt+LeftArrow"  action=findPrev />
        </hotkeys>
    </term>
    <defapp>
        <menu>
            <autohide=faux />  <!--  If true, show menu only on hover. -->
            <enabled="on"/>
            <slim=faux />
        </menu>
    </defapp>
    <tile>
        <menu>
            <autohide=true />  <!--  If true, show menu only on hover. -->
            <enabled="on"/>
            <slim=1 />
        </menu>
    </tile>
    <text>      <!-- Base configuration for the Text app. It can be overridden by param's subargs. -->
        <!-- not implemented -->
    </text>
    <calc>      <!-- Base configuration for the Calc app. It can be overridden by param's subargs. -->
        <!-- not implemented -->
    </calc>
    <logs>      <!-- Base configuration for the Logs app. It can be overridden by param's subargs. -->
        <!-- not implemented -->
    </logs>
    <settings>      <!-- Base configuration for the Settings app. It can be overridden by param's subargs. -->
        <!-- not implemented -->
    </settings>
</config>
```

Note: `$0` will be expanded to the fully qualified current module filename when the configuration is loaded.

# Built-in Applications

- `▀▄ Term` Terminal emulator
- `▀▄ Logs` Debug output console
- `▀▄ View` Workspace navigation helper
- `▀▄ Tile` Tiling window manager
- `▀▄ Gems` Application manager (Demo)

<details><summary>show details...</summary><p>

 - `▀▄ Term`
   - UTF-8 Everywhere
   - Unicode clustering
   - TrueColor/256-color support
   - [VT-100 terminal emulation](https://invisible-island.net/xterm/ctlseqs/ctlseqs.html) compatible
   - Mouse tracking `DECSET 10060 Extended SGR` mode for mouse reporting outside of the terminal viewport; negative values support (See #62 for details)
   - Text selection by mouse ( See #149 for details)
   - Configurable using VT-sequences

      Name         | Sequence                         | Description
      -------------|----------------------------------|-------------
      `CCC_SBS`    | `CSI` 24 : n : m `p`             | Set scrollback buffer size, `int32_t`<br>`n` Initial buffer size in lines; 0 — grow step is used for initial size; _default (if omitted) is 20.000_<br>`m` Grow step for unlimited buffer; _default (if omitted) is 0_ — for fixed size buffer
      `CCC_SGR`    | `CSI` 28 : Pm `p`                | Set terminal background using SGR parameters (one attribute at once)<br>`Pm` Colon-separated list of attribute parameters, 0 — reset all attributes, _default is 0_
      `CCC_SEL`    | `CSI` 29 : n `p`                 | Set selection mode, _default is 0_<br>`n = 0` Selection is off<br>`n = 1` Select and copy as plaintext<br>`n = 2` Select and copy as ANSI/VT text<br>`n = 3` Select and copy as RTF-document<br>`n = 4` Select and copy as HTML-code<br>`n = 5` Select and copy as protected plaintext (suppressed preview, [details](https://learn.microsoft.com/en-us/windows/win32/dataxchg/clipboard-formats#cloud-clipboard-and-clipboard-history-formats))
      `CCC_PAD`    | `CSI` 30 : n `p`                 | Set scrollbuffer side padding<br>`n` Width in cells, _max = 255, default is 0_
      `CCC_RST`    | `CSI` 1 `p`                      | Reset all parameters to default
      `CCC_TBS`    | `CSI` 5 : n `p`                  | Set tabulation length<br>`n` Length in cells, _max = 256, default is 8_
      `CCC_JET`    | `CSI` 11 : n `p`                 | Set text alignment, _default is Left_<br>`n = 0` default<br>`n = 1` Left<br>`n = 2` Right<br>`n = 3` Center
      `CCC_WRP`    | `CSI` 12 : n `p`                 | Set text autowrap mode, _default is On_<br>`n = 0` default<br>`n = 1` On<br>`n = 2` Off (_enable horizontal scrolling_)
      `CCC_RTL`    | `CSI` 13 : n `p`                 | Set text right-to-left mode, _default is Off_<br>`n = 0` default<br>`n = 1` On<br>`n = 2` Off

      Note: It is possible to combine multiple command into a single sequence using a semicolon. For example, the following sequence disables wrapping, enables text selection, and sets the background to blue: `CSI 12 : 2 ; 29 : 1 ; 28 : 44 p` or `CSI 12 : 2 ; 29 : 1 ; 28 : 48 : 2 : 0 : 0 : 255 p`.

 - `▀▄ Logs`
   - Debug output console.

 - `▀▄ View`
   - Serves for quick navigation through the workspace using cyclic selection (left click on group title) in the `View` group on the taskbar. Right click to set clipboard data as region title (swap clipboard text and title).

 - `▀▄ Tile`
   - Supports Drag and Drop for panes (like tabs in a browser).
   - Use any modifier (`Ctrl` or `Alt`) while pane dragging to disable drag&drop mode.
   - List of panes (outside the right side of the window)
     - `LeftClick` -- Set exclusive focus
     - `Ctrl+LeftClick`/`RightClick` -- Set/Unset group focus
     - `double LeftClick` -- Maxixmize/restore
   - Configurable via settings (See configuration eexample above).

</p></details>

# Related Repositories

[Desktopio Framework Documentation](https://github.com/netxs-group/Desktopio-Docs)

---

[![HitCount](https://views.whatilearened.today/views/github/netxs-group/vtm.svg)](https://github.com/netxs-group/vtm) [![Twitter handle][]][twitter badge]

[//]: # (LINKS)
[twitter handle]: https://img.shields.io/twitter/follow/desktopio.svg?style=social&label=Follow
[twitter badge]: https://twitter.com/desktopio
