## Built-in Applications
- `▀▄ Term`     Terminal Emulator
- `▀▄ DirectVT` DirectVT Proxy Console
- `▀▄ XLinkVT`  DirectVT Proxy Console with controlling terminal onboard
- `▀▄ View`     Workspace Navigation Helper
- `▀▄ Tile`     Tiling Window anager

# Terminal Emulator

## Features

- UTF-8 Everywhere
- TrueColor aware
- Horizontal scrolling
- Infinite* scrollback (40k lines by default, * `< max_int32`)
- Scrollback buffer searching and matching
- Line-based/rect-block text selection (See #149 for details)
- Widely used clipboard formats support
  - Plain text
  - RTF
  - HTML
  - ANSI/VT
  - Protected (Windows only: `ExcludeClipboardContentFromMonitorProcessing`, `CanIncludeInClipboardHistory`, `CanUploadToCloudClipboard`)
- [VT-100 terminal emulation](https://invisible-island.net/xterm/ctlseqs/ctlseqs.html) compatible (pass vttest 1 and 2 sections)
- Built-in Windows Console API server
  - Legacy Win32 Console API support
  - No Windows Console Host (conhost.exe) dependency
  - Fullduplex pass-through VT input/output
  - OEM/National, UTF-8 and UTF-16 encoding, even in cmd.exe
  - Enforced ENABLE_WINDOW_INPUT mode  
    Note: In fact it is a viewport resize event reporting. Viewport dimensions is always equal to the win32 console buffer dimensions.
  - Enforced ENABLE_PROCESSED_OUTPUT and ENABLE_VIRTUAL_TERMINAL_PROCESSING modes
  - Disabled ENABLE_QUICK_EDIT_MODE mode
  - Per process instance (not per process name) cmd.exe input history, aka "line input"/"cooked read"
  - Disabled DOSKEY functionality (cmd.exe's F7 input history popups too)  
    Note: Sharing the input history as well as a bunch of command aliases among processes (which could have different elevation levels) is a huge security threat. So DOSKEY functionality is absolutely incompatible with any sort of sudo-like commands/applications.
- Outside terminal viewport mouse tracking (See #62 for details)
- Stdin/stdout parser log on demand
- Configurable at startup via `settings.xml`
- Configurable in runtime using VT-sequences

## VT Runtime configuraion sequences

Name         | Sequence                         | Description
-------------|----------------------------------|-----------
`CCC_SBS`    | `CSI` 24 : n : m : q `p`         | Scrollback buffer configuration<br>`n` Initial buffer size<br>`m` Grow step<br>`q` Grow limit
`CCC_SGR`    | `CSI` 28 : Pm `p`                | Set terminal background using SGR attributes (one attribute at once)<br>`Pm` Colon-separated list of attributes, 0 — reset all attributes, _default is 0_
`CCC_SEL`    | `CSI` 29 : n `p`                 | Set selection mode<br>`n = 0` Selection is off (default)<br>`n = 1` Select and copy as plaintext<br>`n = 2` Select and copy as ANSI/VT text<br>`n = 3` Select and copy as RTF-document<br>`n = 4` Select and copy as HTML-code<br>`n = 5` Select and copy as protected plaintext (suppressed preview, [details](https://learn.microsoft.com/en-us/windows/win32/dataxchg/clipboard-formats#cloud-clipboard-and-clipboard-history-formats))
`CCC_PAD`    | `CSI` 30 : n `p`                 | Set scrollbuffer side padding<br>`n` Width in cells, _max = 255, default is 0_
`CCC_RST`    | `CSI` 1 `p`                      | Reset all parameters to default
`CCC_TBS`    | `CSI` 5 : n `p`                  | Set tab length<br>`n` Length in cells, _max = 256, default is 8_
`CCC_JET`    | `CSI` 11 : n `p`                 | Set text alignment, _default is Left_<br>`n = 0` default<br>`n = 1` Left<br>`n = 2` Right<br>`n = 3` Center
`CCC_WRP`    | `CSI` 12 : n `p`                 | Set text autowrap mode, _default is On_<br>`n = 0` default<br>`n = 1` On<br>`n = 2` Off (_enable horizontal scrolling_)
`CCC_RTL`    | `CSI` 13 : n `p`                 | Set text right-to-left mode, _default is Off_<br>`n = 0` default<br>`n = 1` On<br>`n = 2` Off

Note: It is possible to combine multiple command into a single sequence using a semicolon. For example, the following sequence disables wrapping, enables text selection, and sets background to blue: `CSI 12 : 2 ; 29 : 1 ; 28 : 44 p` or `CSI 12 : 2 ; 29 : 1 ; 28 : 48 : 2 : 0 : 0 : 255 p`.

## Custom menu configuration
      
Terminal window menu can be composed from scratch by specifying a list of menu items in the `<config/term/menu/>` configuration file section.

## Attributes for the `<config/term/menu/item>` object

Attribute  | Description
-----------|------------
type       | Menu item type. `type=Command` is used by default.
label      | Menu item label list. One or more textual representations selected by `data=` value.
notes      | Tooltip.
action     | The function name which called on item activation. Inherited by the label attribute.
data       | Textual parameter for function call. Inherited by the label attribute.
hotkey     | Keyboard shortcut for this menu item. Inherited by the label attribute (not implemented).

## Attributes for the `<config/term/menu/item/label>` sub-object

Attribute        | Description
-----------------|------------
_internal_value_ | Label display variation `label="_internal_value_"`.
notes            | Tooltip. Inherited from item if not specified.
action           | The function name which called on item activation. Inherited from item if not specified.
data             | Textual parameter for function call. Inherited from item if not specified.
hotkey           | Keyboard shortcut for this menu item. Inherited from item if not specified (not implemented).

### Attribute `type=`

Value     | Description
----------|------------
Option    | Cyclically selects the next label in the list and exec the function specified by the `action=` with `data=` as its parameter.
Command   | Exec the function specified by the `action=` with `data=` as its parameter.
Repeat    | Selects the next label and exec the function specified by the `action=` with `data=` as its parameter repeatedly from the time it is pressed until it is released.

### Attribute `action=`

`*` - Not implemented.

Value                        | Description
-----------------------------|------------
TerminalSelectionMode        | Set terminal text selection mode. The `data=` attribute can have the following values `none`, `text`, `ansi`, `rich`, `html`, `protected`.
TerminalWrapMode             | Set terminal scrollback lines wrapping mode. Applied to the active selection if it is. The `data=` attribute can have the following values `on`, `off`.
TerminalAlignMode            | Set terminal scrollback lines aligning mode. Applied to the active selection if it is. The `data=` attribute can have the following values `left`, `right`, `center`.
TerminalFindNext             | Highlight next match of selected text fragment. Clipboard content is used if no active selection.
TerminalFindPrev             | Highlight previous match of selected text fragment. Clipboard content is used if no active selection.
TerminalOutput               | Direct output the `data=` value to the terminal scrollback.
TerminalSendKey              | Simulating keypresses using the `data=` string.
TerminalQuit                 | Terminate runnning console apps and close terminal.
TerminalRestart              | Terminate runnning console apps and restart current session.
TerminalFullscreen           | Toggle fullscreen mode.
TerminalUndo                 | (Win32 Cooked/ENABLE_LINE_INPUT mode only) Discard the last input.
TerminalRedo                 | (Win32 Cooked/ENABLE_LINE_INPUT mode only) Discard the last Undo command.
TerminalClipboardPaste       | Paste from clipboard.
TerminalClipboardWipe        | Reset clipboard.
TerminalSelectionCopy        | Сopy selection to clipboard.
TerminalSelectionRect        | Set linear(false) or rectangular(true) selection form using boolean value.
TerminalSelectionClear       | Deselect a selection.
TerminalViewportCopy         | Сopy viewport to clipboard.
TerminalViewportPageUp       | Scroll one page up.
TerminalViewportPageDown     | Scroll one page down.
TerminalViewportLineUp       | Scroll N lines up.
TerminalViewportLineDown     | Scroll N lines down.
TerminalViewportPageLeft     | Scroll one page to the left.
TerminalViewportPageRight    | Scroll one page to the right.
TerminalViewportColumnLeft   | Scroll N cells to the left.
TerminalViewportColumnRight  | Scroll N cells to the right.
TerminalViewportTop          | Scroll to the scrollback top.
TerminalViewportEnd          | Scroll to the scrollback bottom (reset viewport position).
TerminalStdioLog             | Stdin/stdout log toggle.
*TerminalLogStart            | Start logging to file.
*TerminalLogPause            | Pause logging.
*TerminalLogStop             | Stop logging.
*TerminalLogAbort            | Abort logging.
*TerminalLogRestart          | Restart logging to file.
*TerminalVideoRecStart       | Start DirectVT(DTVT) video recording to file.
*TerminalVideoRecStop        | Stop DTVT-video recording.
*TerminalVideoRecPause       | Pause DTVT-video recording.
*TerminalVideoRecAbort       | Abort DTVT-video recording.
*TerminalVideoRecRestart     | Restart DTVT-video recording to file.
*TerminalVideoPlay           | Play DTVT-video from file.
*TerminalVideoPause          | Pause DTVT-video.
*TerminalVideoStop           | Stop DTVT-video.
*TerminalVideoForward        | Fast forward DTVT-video by N ms.
*TerminalVideoBackward       | Rewind DTVT-video by N ms.
*TerminalVideoHome           | Rewind DTVT-video to the beginning.
*TerminalVideoEnd            | Rewind DTVT-video to the end.

### Terminal configuration example
```xml
<config>
 <term>
  <menu item*>
      <autohide=true />  <!-- If true, show menu only on hover. -->
      <enabled=1 />
      <slim=1 />
      <item label="<" action=TerminalFindPrev>  <!-- type=Command is a default item's attribute. -->
          <label="\e[38:2:0:255:0m<\e[m"/>
          <notes>
              " Previous match                                \n"
              " - Clipboard data will be used if no selection \n"
              " - Scroll one page up if clipboard is empty    "
          </notes>
      </item>
      <item label=">" action=TerminalFindNext>
          <label="\e[38:2:0:255:0m>\e[m"/>
          <notes>
              " Next match                                    \n"
              " - Clipboard data will be used if no selection \n"
              " - Scroll one page down if clipboard is empty  "
          </notes>
      </item>
      <item label="Wrap" type=Option action=TerminalWrapMode data="off">
          <label="\e[38:2:0:255:0mWrap\e[m" data="on"/>
          <notes>
              " Wrapping text lines on/off      \n"
              " - applied to selection if it is "
          </notes>
      </item>
      <item label="Selection" notes=" Text selection mode " type=Option action=TerminalSelectionMode data="none">  <!-- type=Option means that the тext label will be selected when clicked.  -->
          <label="\e[38:2:0:255:0mPlaintext\e[m" data="text"/>
          <label="\e[38:2:255:255:0mANSI-text\e[m" data="ansi"/>
          <label data="rich">
              "\e[38:2:109:231:237m""R"
              "\e[38:2:109:237:186m""T"
              "\e[38:2:60:255:60m"  "F"
              "\e[38:2:189:255:53m" "-"
              "\e[38:2:255:255:49m" "s"
              "\e[38:2:255:189:79m" "t"
              "\e[38:2:255:114:94m" "y"
              "\e[38:2:255:60:157m" "l"
              "\e[38:2:255:49:214m" "e" "\e[m"
          </label>
          <label="\e[38:2:0:255:255mHTML-code\e[m" data="html"/>
          <label="\e[38:2:0:255:255mProtected\e[m" data="protected"/>
      </item>
      <item label="Log" notes=" Stdin/out logging is off " type=Option action=TerminalStdioLog data="off">
          <label="\e[38:2:0:255:0mLog\e[m" notes=" Stdin/out logging is on \n Run Logs to see output  " data="on"/>
      </item>
      <item label="  "    notes=" ...empty menu block/splitter for safety "/>
      <item label="Clear" notes=" Clear TTY viewport "                  action=TerminalOutput data="\e[2J"/>
      <item label="Reset" notes=" Clear scrollback and SGR-attributes " action=TerminalOutput data="\e[!p"/>
      <item label="Restart" type=Command action=TerminalRestart/>
      <item label="Top" action=TerminalViewportTop/>
      <item label="End" action=TerminalViewportEnd/>

      <item label="PgLeft"    type=Repeat action=TerminalViewportPageLeft/>
      <item label="PgRight"   type=Repeat action=TerminalViewportPageRight/>
      <item label="CharLeft"  type=Repeat action=TerminalViewportCharLeft/>
      <item label="CharRight" type=Repeat action=TerminalViewportCharRight/>

      <item label="PgUp"   type=Repeat action=TerminalViewportPageUp/>
      <item label="PgDn"   type=Repeat action=TerminalViewportPageDown/>
      <item label="LineUp" type=Repeat action=TerminalViewportLineUp/>
      <item label="LineDn" type=Repeat action=TerminalViewportLineDown/>

      <item label="PrnScr" action=TerminalViewportCopy/>
      <item label="Deselect" action=TerminalSelectionClear/>
      
      <item label="Line" type=Option action=TerminalSelectionRect data="false">
          <label="Rect" data="true"/>
      </item>
      <item label="Copy" type=Repeat action=TerminalSelectionCopy/>
      <item label="Paste" type=Repeat action=TerminalClipboardPaste/>
      <item label="Undo" type=Command action=TerminalUndo/>
      <item label="Redo" type=Command action=TerminalRedo/>
      <item label="Quit" type=Command action=TerminalQuit/>
      <item label="Fullscreen" type=Command action=TerminalFullscreen/>

      <item label="Hello, World!" notes=" Simulating keypresses "       action=TerminalSendKey data="Hello World!"/>
      <item label="Push Me" notes=" test " type=Repeat action=TerminalOutput data="pressed ">
          <label="\e[37mPush Me\e[m"/>
      </item>
  </menu>
 </term>
</config>
```

# DirectVT Proxy Console

...

This console mode is activated by the `-r dtvt` option.

Example:
```
vtm -r dtvt vtm -r truecolor
```

# DirectVT Proxy Console with controlling terminal onboard

This console mode is used when there is a need for interactive interaction with the user through the controlling terminal. For example, this is required when connecting via SSH with keyboard-interactive authentication or requesting a private key passphrase.

This mode is enabled automatically if the first command line argument begins with `ssh` literal.

The following commands are identical:
```
vtm -r xlvt ssh user@host vtm
```
```
vtm ssh user@host vtm
```

# Workspace Navigation Helper

## Features

- Serves for quick navigation through the workspace using cyclic selection (left click on group title) in the `View` group on the taskbar. Right click to set clipboard data as region title (swap clipboard text and title).

# Tiling Window Manager

## Features

- Supports Drag and Drop for panes (like tabs in a browser).
- Use any modifier (`Ctrl` or `Alt`) while pane dragging to disable drag&drop mode.
- List of panes (outside the right side of the window)
  - `LeftClick` -- Set exclusive focus
  - `Ctrl+LeftClick` -- Set/Unset group focus
  - `double LeftClick` -- Maxixmize/restore
- Configurable via settings (See configuration example in doc\settings.md`).