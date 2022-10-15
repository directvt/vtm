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
  - Windows Desktop
  - Windows Server

# Building from Source

### POSIX-oriented

Build-time dependencies
 - `git`
 - `cmake`
 - C++20 compiler
 - Minimal requirements to compile
   - Using [`GCC`](https://gcc.gnu.org/projects/cxx-status.html) — `3GB` of RAM
   - Using [`Clang`](https://clang.llvm.org/cxx_status.html) — `8GB` of RAM

```bash
git clone https://github.com/netxs-group/vtm.git && cd ./vtm
cmake ./src -DCMAKE_BUILD_TYPE=Release
cmake --build .
cmake --install .
```

### Windows

Build-time dependencies
 - `git`
 - `cmake`
 - `Visual Studio 2019` or later (Desktop development with C++)
 - `UTF-8` for worldwide language support, https://github.com/netxs-group/vtm/issues/175#issuecomment-1034734346

Use `Developer Command Prompt` as a build environment
```cmd
git clone https://github.com/netxs-group/vtm.git && cd ./vtm
cmake ./src -DCMAKE_BUILD_TYPE=Release "-GVisual Studio 16 2019"
cmake --build . --config Release
cd ./Release
powershell ../src/install/install.ps1
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
    <td colspan="9">Modify selection</td>
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

 `vtm [ -d [<config_file>] | -s [<config_file>] | -r [<app> [<args...>]] ]`

Option        | Description
--------------|-------------------------------------------------------
No arguments  | Run client (auto start server)
` -d [<file>]`| Run server in background (use off-base configuration file if specified)
` -s [<file>]`| Run server in interactive mode (use off-base configuration file if specified)
` -r [<app>]` | Run the specified `<app>` in offline mode<br>`Term` Terminal emulator (default)<br>`Calc` (Demo) Spreadsheet calculator<br>`Text` (Demo) Text editor<br>`Gems` (Demo) Desktopio application manager

Configuration file location precedence (descending priority):<br>
1. Command line options `vtm -s path/to/settings.xml`<br>
2. Environment variable `VTM_CONFIG=path/to/settings.xml`<br>
3. Hardcoded location `~/.config/vtm/settings.xml`<br>
4. Predefined (hardcoded) configuration at apps.hpp(~line:28)

# Settings

vtm can be configured in the `~/.config/vtm/settings.xml` file in xml format. Alternative configuration file location can be specified using command line option `-s` / `-d` or using environment variable VTM_CONFIG.

## Configuration file Format (settings.xml)

Configuration file format is a slightly modified XML-format that allows to store hierarchical list of key=value pairs.

### Key differences from the standard XML

 - All values are UTF-8 string literals and can be specified without quotes if there are no spaces.
 - There is no distinction between XML-attribute and XML-subobject notations, i.e. any specified XML-attribute is the XML-subobject.
 - In addition to a set of sub-objects, every object has its own textual value.
 - Each object can be defined in any way, either using an XML-attribute or an XML-subobject syntax.
 - An object name ending in an asterisk indicates that this object is not an object, but is a template for all subsequent objects with the same name in this scope (see Templates section below).
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

Use asterisk at the end of the element name to set defaults.

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

Top-level element `<config>` contains the following objects
  - Single `<menu>` block - taskbar menu configuration.
    - Single `<selected>` object - the value of this attribute specifies which menu item id will be selected by default at the environment startup.
    - Set of `<item>` objects - list of menu item definitions.
    - Single `<autorun>` block - list of menu item to run at the environment startup.
  - Single `<hotkeys>` block - global hotkeys/shortcuts configuration.

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
`bgcolor`  |  App window background color                      | `RGBA`     |           |
`fgcolor`  |  App window foreground color                      | `RGBA`     |           |
`winsize`  |  App window 2D size                               | `x;y`      |           |
`slimmenu` |  App window menu vertical size                    | `boolean`  |           | `no`
`cwd`      |  Current working directory                        | `string`   |           |
`type`     |  App type                                         | `string`   |           | `SHELL`
`param`    |  App constructor arguments                        | `string`   |           | empty

#### Value literals

Type     | Format
---------|-----------------
`RGBA`   |  `#rrggbbaa` \| `0xaabbggrr` \| `rrr,ggg,bbb,aaa` \| 256-color index
`boolean`|  `true` \| `false` \| `yes` \| `no` \| `1` \| `0` \| `on` \| `off`
`index`  |  0 .. N
`string` |  _UTF-8 text string_
`x;y`    |  _integer_ <any_delimeter> _integer_

#### App type

Type              | Parameter
------------------|-----------------
`DirectVT`        | `_command line_`
`SHELL` (default) | `_command line_`
`ANSIVT`          | `_command line_`
`Group`           | [ v[`n:m:w`] \| h[`n:m:w`] ] ( id_1 \| _nested_block_ , id_2 \| _nested_block_ )]
`Region`          | `param` attribute is not used, use attribute `title=_view_title_` to set region name

### Configuration Example

Note: The following configuration sections are not implemented yet
- config/menu/item/param/*
- config/menu/item/hotkeys
- config/menu/autorun
- config/hotkeys

`~/.config/vtm/settings.xml`
```xml
<config>
    <menu>
        <selected=Term /> <!-- set selected using menu item id -->
        <item splitter label="apps">
            <notes> 
                " Default applications group                         \n"
                " It can be configured in ~/.config/vtm/settings.xml "
            </notes>
        </item>
        <item* />    <!-- use asterisk at the end of the element name to set defaults -->
        <item* hidden=no slimmenu=false type=SHELL fgcolor=#00000000 bgcolor=#00000000 winsize=0,0 wincoor=0,0 />
        <item id=Term label="Term" type=DirectVT title="Terminal Emulator" notes=" Run built-in terminal emulator ">
            <hotkeys>    <!-- not implemented -->
                <action=start key="Ctrl+'t'"/>
                <action=close key="Ctrl+'z'"/>
            </hotkeys>
            <param="vtm -r term">    <!-- not implemented -->
                <scrollback>
                    <size=20000 />
                    <growstep=0 />
                </scrollback>
                <colors>
                    <palette>
                        <color=0xFF101010 index=0 />  <!-- 0  blackdk   -->
                        <color=0xFF1F0FC4 />          <!-- 1  reddk     -->
                        <color=0xFF0EA112 />          <!-- 2  greendk   -->
                        <color=0xFF009CC0 />          <!-- 3  yellowdk  -->
                        <color=0xFFDB3700 />          <!-- 4  bluedk    -->
                        <color=0xFF981787 />          <!-- 5  magentadk -->
                        <color=0xFFDD963B />          <!-- 6  cyandk    -->
                        <color=0xFFBBBBBB />          <!-- 7  whitedk   -->
                        <color=0xFF757575 />          <!-- 8  blacklt   -->
                        <color=0xFF5648E6 />          <!-- 9  redlt     -->
                        <color=0xFF0CC615 />          <!-- 10 greenlt   -->
                        <color=0xFFA5F1F8 />          <!-- 11 yellowlt  -->
                        <color=0xFFFF783A />          <!-- 12 bluelt    -->
                        <color=0xFF9E00B3 />          <!-- 13 magentalt -->
                        <color=0xFFD6D660 />          <!-- 14 cyanlt    -->
                        <color=0xFFF3F3F3 index=15 /> <!-- 15 whitelt   -->
                    </palette>
                    <default>
                        <fg=15 /> <!-- 256-color index is allowed -->
                        <bg=0 />
                    </default>
                    <match fx=selection bg="0xFF007F00" fg=15 />  <!-- set fx to use cell::shaders: xlight | selection |contrast | invert | reverse -->
                    <selection>
                        <text fx=selection bg=12 fg=15 />
                        <ansi fx=xlight/>
                        <rich fx=xlight/>
                        <html fx=xlight/>
                        <none fx=selection bg=8 fg=7 />
                    </selection>
                </colors>
                <tablen=8 />      <!-- Tab length. -->
                <maxline=65535 /> <!-- Max line length. Line splits if it exceeds the limit. -->
                <cursor>
                    <style="underline"/> <!-- block | underline  -->
                    <blink="400"/>       <!-- blink period in ms -->
                </cursor>
                <menu>
                    <enabled="on"/>
                    <slim="off"/>
                </menu>
                <wrap="on"/>
                <selection>
                    <mode="text"/> <!-- text | ansi | rich | html | none -->
                </selection>
                <hotkeys>
                    <action=findNext key="Alt+RightArrow"/>
                    <action=findPrev key="Alt+LeftArrow"/>
                </hotkeys>
            </param>
        </item>
        <item id=mc        label="mc"        type=SHELL    title="Midnight Commander"    param="mc"               notes=" Run Midnight Commander in its own window (if it is installed) "/>
        <item id=Tile      label="Tile"      type=Group    title="Tiling Window Manager" param="h1:1(Term, Term)" notes=" Run Tiling Window Manager with two terminals attached "/>
        <item id=View      label=View        type=Region   title="\e[11:3pView: Region"                           notes=" Set desktop region "/>
        <item id=Settings  label=Settings    type=DirectVT title="Settings"              param="$0 -r settings"   notes=" Configure frame rate " winsize=50,15 />
        <item id=Logs      label=Logs        type=DirectVT title="Logs Title"            param="$0 -r logs"       notes=" Run Logs application "/>
        <item splitter label="demo" notes=" Demo apps                    \n Feel the Desktopio Framework "/>
        <item id=Gems      label="Gems"      type=DirectVT title="Gems Title"            param="$0 -r gems"       notes=" App Distribution Hub "/>
        <item id=Text      label="Text"      type=DirectVT title="Text Title"            param="$0 -r text"       notes=" Text Editor "/>
        <item id=Calc      label="Calc"      type=DirectVT title="Calc Title"            param="$0 -r calc"       notes=" Spreadsheet Calculator "/>
        <item id=Test      label="Test"      type=DirectVT title="Test Title"            param="$0 -r test"       notes=" Test Page "/>
        <item id=Truecolor label="Truecolor" type=DirectVT title="True Title"            param="$0 -r truecolor"  notes=" Truecolor Test "/>
        <autorun>    <!-- not implemented -->
            <item*=Term winsize=48%,48% /> <!-- item*=_item_id_ - assign the same _item_id_ to each item by default -->
            <item wincoor=0,0 />
            <item wincoor=52%,0 />
            <item wincoor=0,52% />
            <item=mc wincoor=52%,52% />
        </autorun>
    </menu>
    <hotkeys>    <!-- not implemented -->
        <action=prevWindow key="Ctrl+PgUp"/>
        <action=nextWindow key="Ctrl+PgDn"/>
    </hotkeys>
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
   - Auto-wrap mode `DECAWM` (with horizontal scrolling)
   - Focus tracking `DECSET 1004`
   - Bracketed paste mode `DECSET 2004`
   - SGR attributes: overline, double underline, strikethrough, and others
   - Save/restore terminal window title `XTWINOPS 22/23`
   - Mouse tracking `DECSET 1000/1002/1003/1006 SGR` mode
   - Mouse tracking `DECSET 10060 Extended SGR` mode, mouse reporting outside of the terminal viewport (outside + negative arguments) #62
   - Text selection by mouse #149  
   - Configurable using VT-sequences

      Name         | Sequence                         | Description
      -------------|----------------------------------|-------------
      `CCC_SBS`    | `CSI` 24 : n : m `p`             | Set scrollback buffer size, `int32_t`<br>`n` Initial buffer size in lines; 0 — grow step is used for initial size; _default (if omitted) is 20.000_<br>`m` Grow step for unlimited buffer; _default (if omitted) is 0_ — for fixed size buffer
      `CCC_SGR`    | `CSI` 28 : Pm `p`                | Set terminal background using SGR parameters (one attribute at once)<br>`Pm` Colon-separated list of attribute parameters, 0 — reset all attributes, _default is 0_
      `CCC_SEL`    | `CSI` 29 : n `p`                 | Set selection mode, _default is 0_<br>`n = 0` Selection is off<br>`n = 1` Select and copy as plaintext<br>`n = 2` Select and copy as ANSI/VT text<br>`n = 3` Select and copy as RTF-document<br>`n = 4` Select and copy as HTML-code
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
