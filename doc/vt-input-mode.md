status: draft

# VT Input Mode Protocol

The goal of the `vt-input-mode` protocol is to enable cross-platform command-line interactivity.

- No TTY required.
- No OS-level signal tracking required.

## Audience

This protocol is intended for anyone who needs to:

- Operate without a TTY.
- Share applications over a LAN (using inetd, netcat, etc.).
- Track every key press and key release event.
- Track position-dependent keys such as WASD.
- Distinguish between physical Left and Right keyboard keys.
- Get consistent output regardless of terminal window resizing.
- Track mouse movement at a pixel-wise level.
- Track mouse movement outside the terminal window (receiving negative coordinates).
- Take advantage of high-resolution (fine) scrolling.
- Track scrollback text manipulation events.
- Track application closing and system shutdown events.

## Limitations of Existing Approaches

Existing approaches have the following drawbacks:

- There is no uniform way to receive keyboard events across platforms.
- Window size tracking requires platform-specific calls with no way to synchronize the output consistently.
- Mouse tracking modes lack support for negative coordinates and high-resolution scrolling.
- Bracketed paste mode does not support the transfer of binary data or data containing the bracketed paste mode sequences themselves.

## Conventions

- We use the HEX form of a `uint32` integer for representing the 32-bit floating-point value (**IEEE-754 32-bit binary float, Little-Endian**). For example, the floating-point value `3.1415f` is represented as the unsigned integer in hex `40490E56` (decimal `1078529622`).
  - Note: The use of the floating-point format allows for the representation of special states such as "coordinate unavailable" (e.g., when a mouse device is disconnected) using values like `NaN` (Not a Number) or infinity.
- Space characters are not used within sequence payloads; they are included in this description solely for readability.
- All string data transmitted within the protocol is encoded using `UTF-8`.
- [Clipboard/Keyboard Input]: All unescaped symbols outside the scope of this protocol should be treated as data pasted from the clipboard.

## Event tracking activation

Event tracking is activated by sending an APC VT sequence to the terminal containing a script that switches the event tracking mode.

The following APC sequences are used to set, reset, or request the current event tracking mode for specified event sources:

- Set:
  ```
  ESC _ lua: terminal.EventReporting("Source0", ..., "SourceN") ESC \
  ```
- Reset (the event tracking is deactivated if an empty string is specified):
  ```
  ESC _ lua: terminal.EventReporting("") ESC \
  ```
- Get a list of active sources:
  ```
  ESC _ lua: src_list=terminal.EventReporting() ESC \
  ```

Sources      | Events to track
-------------|----------------
`"keyboard"` | Keyboard events.
`"mouse"`    | Mouse events.
`"focus"`    | Focus events.
`"format"`   | Line format changes.
`"clipboard"`| Clipboard events.
`"window"`   | Window size and selection events.
`"system"`   | System signals.
`""`         | Deactivate all event reporting.

//todo: keybd mode only
Note: By enabling `vt-input-mode`, all current terminal modes are automatically saved (to be restored on exit) and switched to a raw input mode. In this mode, input is available character by character, echoing is disabled, and all special processing of terminal input and output characters is deactivated (except for `LF` to `CR+LF` conversion).

### Event format

The event signaling also uses APC `ESC _ <payload> ESC \` vt-sequences with the following payload format:
```
<attr>=<val>,...,<val>;...;<attr>=<val>,...,<val>
```

Field             | Descriprtion
------------------|-------------
`<attr>`          | Attribute name.
`<val>,...,<val>` | Comma-separated value list.

## Events

- Keyboard
  ```
  ESC _ event=keyboard ; id=<ID> ; kbmods=<KeyMods> ; keyid=<KeyId> ; pressed=<KeyDown> ; scancode=<ScanCode> ; id_chord=<HexEncodedData> ; ch_chord=<HexEncodedData> ; sc_chord=<HexEncodedData> ; cluster=<C0>,...,<Cn> ESC \
  ```
- Mouse
  ```
  ESC _ event=mouse ; id=<ID> ; kbmods=<KeyMods> ; coor=<X>,<Y> ; buttons=<ButtonState> ; iscroll=<DeltaX>,<DeltaY> ; fscroll=<DeltaX>,<DeltaY> ESC \
  ```
- Focus
  ```
  ESC _ event=focus ; id=<ID> ; state=<FocusState> ESC \
  ```
- Format
  ```
  ESC _ event=format ; wrapping=<State> ; alignment=<State> ; rtl=<State> ESC \
  ```
//todo Textinput, Text, IME or Input for IME preview etc
- Clipboard
  ```
  ESC _ event=clipboard ; id=<ID> ; format=<ClipFormat> ; security=<SecLevel> ; data=<Data> ESC \
  ```
- Window
  ```
  ESC _ event=window ; size=<Width>,<Height> ; cursor=<X>,<Y> ; region=<Left>,<Top>,<Right>,<Bottom> ; selection=<StartX>,<StartY>,<EndX>,<EndY>,<Mode> ESC \
  ```
- System
  ```
  ESC _ event=system ; signal=<Signal> ESC \
  ```

### Keyboard

```
ESC _ event=keyboard ; id=<ID> ; kbmods=<KeyMods> ; keyid=<KeyId> ; pressed=<KeyDown> ; scancode=<ScanCode> ; id_chord=<HexEncodedData> ; ch_chord=<HexEncodedData> ; sc_chord=<HexEncodedData> ; cluster=<C0>,...,<Cn> ESC \
```

> Q: Do we need to track a scancode chord? `scanchord=<Code0>,...,<CodeN>`?

Attribute                     | Description
------------------------------|------------
`id=<ID>`                     | Device group ID (unsigned integer).
`kbmods=<KeyMods>`            | Keyboard modifiers bit field.
`keyid=<KeyId>`               | Physical key ID.
`pressed=<KeyDown>`           | Key state:<br>\<KeyDown\>=1 - Pressed.<br>\<KeyDown\>=0 - Released.
`scancode=<ScanCode>`         | Scan code.
`id_chord=<HexEncodedData>`   | Simultaneously pressed key IDs in ascending order. //todo define format
`ch_chord=<HexEncodedData>`   | Simultaneously pressed key IDs and grapheme cluster at the last place representing a key press.
`sc_chord=<HexEncodedData>`   | Simultaneously pressed key scancodes in ascending order.
`cluster=<C0>,...,<Cn>`       | Codepoints of the generated string/text cluster (list of decimal integers).

In response to the activation of `keyboard` tracking, the application receives a VT sequence containing the keyboard modifiers state:
```
ESC _ event=keyboard ; id=<ID> ; kbmods=<KeyMods> ESC \
```

The full sequence is fired after every key press and key release. The sequence can contain a string generated by a keystroke as a set of codepoints: `C0` through `Cn`.

//todo revise, define format
The `xx_chord=<HexEncodedData>` attributes contain the set of simultaneously pressed key IDs (e.g., KeyId0,...,KeyIdN) in ascending order and are used to track key combinations (chords). It is possible to track both chord presses (`+`, e.g., `Ctrl+F1`) and chord releases (`-`, e.g., `Ctrl-F1` or `Ctrl-Alt`):
...

#### Keyboard modifiers

The state `kbmods=<KeyMods>` of keyboard modifiers is the binary OR of all currently pressed modifiers and enabled modes.

 Bit | Side   | Modifier Key                 | Value
 ----|--------|------------------------------|--------------
 0   | Left   | <kbd>⌃ Ctrl</kbd>            | `0x0001`
 1   | Right  | <kbd>⌃ Ctrl</kbd>            | `0x0002`
 2   | Left   | <kbd>⎇ Alt</kbd><br><kbd>◆ Meta</kbd><br><kbd>⌥ Option</kbd>     | `0x0004`
 3   | Right  | <kbd>⎇ Alt</kbd><br><kbd>◆ Meta</kbd><br><kbd>⌥ Option</kbd><br> | `0x0008`
 4   | Left   | <kbd>⇧ Shift</kbd>           | `0x0010`
 5   | Right  | <kbd>⇧ Shift</kbd>           | `0x0020`
 6   | Left   | <kbd>⊞ Win</kbd><br><kbd>⌘ Command</kbd><br><kbd>❖ Super</kbd> | `0x0040`
 7   | Right  | <kbd>⊞ Win</kbd><br><kbd>⌘ Command</kbd><br><kbd>❖ Super</kbd> | `0x0080`
 8   | Left   | <kbd>Hyper</kbd>             | `0x0100`
 9   | Right  | <kbd>Hyper</kbd>             | `0x0200`
 10  |        | <kbd>reserved</kbd>          | `0x0400`
 11  |        | <kbd>reserved</kbd>          | `0x0800`
 12  |        | <kbd>⇭ NumLock Mode</kbd>    | `0x1000`
 13  |        | <kbd>⇪ CapsLock Mode</kbd>   | `0x2000`
 14  |        | <kbd>⇳ ScrollLock Mode</kbd> | `0x4000`
 15  |        | <kbd>reserved Mode</kbd>     | `0x8000`

#### Scan codes

//todo scancodes are real, captured on fly (predefined values as a fallback option)
Scan codes are usually useful for applications that need to know which key is pressed, regardless of the current keyboard layout. For example, the WASD (Up, Left, Down, Right) keys for games, which ensure a consistent key formation across QWERTY, AZERTY or Dvorak keyboard layouts.

```
QWERTY: "W" = 11  "A" = 1E  "S" = 1F  "D" = 20
AZERTY: "Z" = 11  "Q" = 1E  "S" = 1F  "D" = 20
Dvorak: "," = 11  "A" = 1E  "O" = 1F  "E" = 20
```

Scan codes for the keys of the abstract 112-key keyboard:

```
                                                                               ┌────┐    ┌────┐
                                                                               | 54 |    |E046| 2 keys (SysReq=Alt+PrintScreen, Break=Ctrl+Pause)
                                                                               └────┘    └────┘
┌────┐    ┌────╥────╥────╥────┐  ┌────╥────╥────╥────┐  ┌────╥────╥────╥────┐  ┌────╥────╥────┐
| 01 │    | 3B ║ 3C ║ 3D ║ 3E |  | 3F ║ 40 ║ 41 ║ 42 |  | 43 ║ 44 ║ 57 ║ 58 |  |E037║ 46 ║ 45 | 16 keys
└────┘    └────╨────╨────╨────┘  └────╨────╨────╨────┘  └────╨────╨────╨────┘  └────╨────╨────┘
┌────╥────╥────╥────╥────╥────╥────╥────╥────╥────╥────╥────╥────╥────╥─────┐  ┌────╥────╥────┐  ┌────╥────╥────╥────┐
| 29 ║ 02 ║ 03 ║ 04 ║ 05 ║ 06 ║ 07 ║ 08 ║ 09 ║ 0A ║ 0B ║ 0C ║ 0D ║ 7D ║  0E |  |E052║E047║E049|  |E045║E035║ 37 ║ 4A | 22 keys
╞════╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═════╡  ╞════╬════╬════╡  ╞════╬════╬════╬════╡
| 0F   ║ 10 ║ 11 ║ 12 ║ 13 ║ 14 ║ 15 ║ 16 ║ 17 ║ 18 ║ 19 ║ 1A ║ 1B ║     2B |  |E053║E04F║E051|  | 47 ║ 48 ║ 49 ║ 4E | 21 keys
╞══════╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩════════╡  └────╨────╨────┘  ╞════╬════╬════╬════╣
| 3A     ║ 1E ║ 1F ║ 20 ║ 21 ║ 22 ║ 23 ║ 24 ║ 25 ║ 26 ║ 27 ║ 28 ║        1C |                    | 4B ║ 4C ║ 4D ║ 7E | 17 keys
╞═════╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦═════════╡       ┌────┐       ╞════╬════╬════╬════╡
| 2A  ║ 56 ║ 2C ║ 2D ║ 2E ║ 2F ║ 30 ║ 31 ║ 32 ║ 33 ║ 34 ║ 35 ║ 73 ║      36 |       |E048|       | 4F ║ 50 ║ 51 ║    | 19 keys
╞═════╩═╦══╩═╦══╩═══╦╩═══╦╩════╩════╩════╩════╩╦═══╩╦═══╩╦═══╩╦═══╩═╦═══════╡  ┌────┼────┼────┐  ╞════╩════╬════╣E01C|
| 1D    ║ 5B ║ 38   ║ 7B ║            39       ║ 70 ║E038║ 5C ║  5D ║  E01D |  |E04B|E050|E04D|  |      52 ║ 53 ║    | 15 keys
└───────╨────╨──────╨────╨─────────────────────╨────╨────╨────╨─────╨───────┘  └────┴────┴────┘  └─────────╨────╨────┘
```

#### Physical keys

The `<KeyId>` is incremented by 2 for each generic key, providing two `<KeyId>` placeholders for each physical key to distinguish between Left and Right (or Numpad) in the last bit. Ignore the last bit of `<KeyId>` for tracking generic keys.

Key ID | Name               | Generic Name       | Literal Code | Unicode Code | Nodes
-------|--------------------|--------------------|--------------|--------------|------
 0     | undef              | "undef"            | ""           | 0            | Any unknown key
 2     | LeftCtrl           | "Ctrl"             | ""           | 0
  3    | RightCtrl          | "Ctrl"             | ""           | 0
 4     | LeftAlt            | "Alt"              | ""           | 0
  5    | RightAlt           | "Alt"              | ""           | 0
 6     | LeftShift          | "Shift"            | ""           | 0
  7    | RightShift         | "Shift"            | ""           | 0
 8     | LeftSuper          | "Super"            | ""           | 0            | LeftWin on Windows
  9    | RightSuper         | "Super"            | ""           | 0            | RightWin on Windows
 10    | LeftHyper          | "Hyper"            | ""           | 0
  11   | RightHyper         | "Hyper"            | ""           | 0
 12    | LeftMeta           | "Meta"             | ""           | 0
  13   | RightMeta          | "Meta"             | ""           | 0
 14    | NumLock            | "NumLock"          | ""           | 0
 16    | CapsLock           | "CapsLock"         | ""           | 0
 18    | ScrollLock         | "ScrollLock"       | ""           | 0
 20    | AltGr              | "AltGr"            | ""           | 0            | aka IsoLevel3Shift
 22    | Level5Shift        | "Level5Shift"      | ""           | 0            | GroupSelect on Canadian Multilingual Standard layout
 24    | Kana               | "Kana"             | ""           | 0
 26    | Henkan             | "Henkan"           | ""           | 0
 28    | Muhenkan           | "Muhenkan"         | ""           | 0
 30    | Hanja              | "Hanja"            | ""           | 0
 32    | Hanguel            | "Hanguel"          | ""           | 0
 34    | Apps               | "Apps"             | ""           | 0
 36    | Select             | "Select"           | ""           | 0
 38    | Fn                 | "Fn"               | ""           | 0            | macOS/Linux specific
 40    | F1                 | "F1"               | ""           | 0
 42    | F2                 | "F2"               | ""           | 0
 44    | F3                 | "F3"               | ""           | 0
 46    | F4                 | "F4"               | ""           | 0
 48    | F5                 | "F5"               | ""           | 0
 50    | F6                 | "F6"               | ""           | 0
 52    | F7                 | "F7"               | ""           | 0
 54    | F8                 | "F8"               | ""           | 0
 56    | F9                 | "F9"               | ""           | 0
 58    | F10                | "F10"              | ""           | 0
 60    | F11                | "F11"              | ""           | 0
 62    | F12                | "F12"              | ""           | 0
 64    | F13                | "F13"              | ""           | 0
 66    | F14                | "F14"              | ""           | 0
 68    | F15                | "F15"              | ""           | 0
 70    | F16                | "F16"              | ""           | 0
 72    | F17                | "F17"              | ""           | 0
 74    | F18                | "F18"              | ""           | 0
 76    | F19                | "F19"              | ""           | 0
 78    | F20                | "F20"              | ""           | 0
 80    | F21                | "F21"              | ""           | 0
 82    | F22                | "F22"              | ""           | 0
 84    | F23                | "F23"              | ""           | 0
 86    | F24                | "F24"              | ""           | 0
 88    | F25                | "F25"              | ""           | 0
 90    | F26                | "F26"              | ""           | 0
 92    | F27                | "F27"              | ""           | 0
 94    | F28                | "F28"              | ""           | 0
 96    | F29                | "F29"              | ""           | 0
 98    | F30                | "F30"              | ""           | 0
 100   | F31                | "F31"              | ""           | 0
 102   | F32                | "F32"              | ""           | 0
 104   | F33                | "F33"              | ""           | 0
 106   | F34                | "F34"              | ""           | 0
 108   | F35                | "F35"              | ""           | 0
 110   | PrintScreen        | "PrintScreen"      | ""           | 0
 112   | Pause              | "Pause"            | ""           | 0
 114   | Break              | "Break"            | "\x03"       | 0x03
 116   | SysReq             | "SysReq"           | ""           | 0
 118   | Esc                | "Esc"              | "\x1B"       | 0x1b
 120   | Tab                | "Tab"              | "\x09"       | 0x09
 122   | Backspace          | "Backspace"        | "\x08"       | 0x08
 124   | Space              | "Space"            | "\x20"       | 0x20
 126   | KeyEnter           | "Enter"            | "\x0D"       | 0x0d
  127  | NumpadEnter        | "Enter"            | "\x0D"       | 0x0d
 128   | KeyInsert          | "Insert"           | ""           | 0
  129  | NumpadInsert       | "Insert"           | ""           | 0
 130   | KeyDelete          | "Delete"           | ""           | 0
  131  | NumpadDelete       | "Delete"           | ""           | 0
 132   | KeyClear           | "Clear"            | ""           | 0
  133  | NumpadClear        | "Clear"            | ""           | 0
 134   | KeyPageUp          | "PageUp"           | ""           | 0
  135  | NumpadPageUp       | "PageUp"           | ""           | 0
 136   | KeyPageDown        | "PageDown"         | ""           | 0
  137  | NumpadPageDown     | "PageDown"         | ""           | 0
 138   | KeyHome            | "Home"             | ""           | 0
  139  | NumpadHome         | "Home"             | ""           | 0
 140   | KeyEnd             | "End"              | ""           | 0
  141  | NumpadEnd          | "End"              | ""           | 0
 142   | KeyLeftArrow       | "LeftArrow"        | ""           | 0
  143  | NumpadLeftArrow    | "LeftArrow"        | ""           | 0
 144   | KeyRightArrow      | "RightArrow"       | ""           | 0
  145  | NumpadRightArrow   | "RightArrow"       | ""           | 0
 146   | KeyUpArrow         | "UpArrow"          | ""           | 0
  147  | NumpadUpArrow      | "UpArrow"          | ""           | 0
 148   | KeyDownArrow       | "DownArrow"        | ""           | 0
  149  | NumpadDownArrow    | "DownArrow"        | ""           | 0
 150   | Key0               | "0"                | "0"          | 0x30
  151  | Numpad0            | "0"                | "0"          | 0x30
 152   | Key1               | "1"                | "1"          | 0x31
  153  | Numpad1            | "1"                | "1"          | 0x31
 154   | Key2               | "2"                | "2"          | 0x32
  155  | Numpad2            | "2"                | "2"          | 0x32
 156   | Key3               | "3"                | "3"          | 0x33
  157  | Numpad3            | "3"                | "3"          | 0x33
 158   | Key4               | "4"                | "4"          | 0x34
  159  | Numpad4            | "4"                | "4"          | 0x34
 160   | Key5               | "5"                | "5"          | 0x35
  161  | Numpad5            | "5"                | "5"          | 0x35
 162   | Key6               | "6"                | "6"          | 0x36
  163  | Numpad6            | "6"                | "6"          | 0x36
 164   | Key7               | "7"                | "7"          | 0x37
  165  | Numpad7            | "7"                | "7"          | 0x37
 166   | Key8               | "8"                | "8"          | 0x38
  167  | Numpad8            | "8"                | "8"          | 0x38
 168   | Key9               | "9"                | "9"          | 0x39
  169  | Numpad9            | "9"                | "9"          | 0x39
 170   | KeyMultiply        | "*"                | "*"          | 0x2A
  171  | NumpadMultiply     | "*"                | "*"          | 0x2A
 172   | KeySlash           | "/"                | "/"          | 0x2F
  173  | NumpadDivide       | "/"                | "/"          | 0x2F
 174   | KeyPlus            | "Plus"             | "+"          | 0x2B
  175  | NumpadPlus         | "Plus"             | "+"          | 0x2B
 176   | KeyMinus           | "Minus"            | "-"          | 0x2D
  177  | NumpadMinus        | "Minus"            | "-"          | 0x2D
 178   | KeyEqual           | "="                | "="          | 0x3D
  179  | NumpadEqual        | "="                | "="          | 0x3D
 180   | KeyPeriod          | "."                | "."          | 0x2E
  181  | NumpadDecimal      | "."                | "."          | 0x2E
 182   | KeyComma           | ","                | ","          | 0x2C
  183  | NumpadPoint        | ","                | ","          | 0x2C
 184   | Colon              | ":"                | ":"          | 0x3A
 186   | Semicolon          | ";"                | ";"          | 0x3B
 188   | TurnedComma        | "ʻ"                | "ʻ"          | 0x02BB
 190   | OpenSquareBracket  | "["                | "["          | 0x5B
 192   | CloseSquareBracket | "]"                | "]"          | 0x5D
 194   | OpenCurlyBracket   | "{"                | "{"          | 0x7B
 196   | CloseCurlyBracket  | "}"                | "}"          | 0x7D
 198   | CloseRoundBracket  | ")"                | ")"          | 0x29
 200   | LessThan           | "<"                | "<"          | 0x3C
 202   | BackSlash          | "\\"               | "\\"         | 0x5C
 204   | Underscore         | "_"                | "_"          | 0x5F
 206   | VerticalBar        | "\|"               | "\|"         | 0x7C
 208   | DivisionSign       | "÷"                | "÷"          | 0xF7
 210   | OneHalf            | "½"                | "½"          | 0xBD
 212   | SuperscriptTwo     | "²"                | "²"          | 0xB2
 214   | DegreeSign         | "°"                | "°"          | 0xB0
 216   | NumeroSign         | "º"                | "º"          | 0xBA
 218   | Acute              | "´"                | "´"          | 0xB4
 220   | Caron              | "ˇ"                | "ˇ"          | 0x02C7
 222   | Cedilla            | "¸"                | "¸"          | 0xB8
 224   | Circumflex         | "^"                | "^"          | 0x5E
 226   | Ogonek             | "˛"                | "˛"          | 0x02DB
 228   | Cross              | "˟"                | "˟"          | 0x02DF
 230   | Tilde              | "~"                | "~"          | 0x7E
 232   | Tonos              | "΄"                | "΄"          | 0x0384
 234   | Umlaut             | "¨"                | "¨"          | 0xA8
 236   | BackQuote          | "`"                | "`"          | 0x60
 238   | SingleQuote        | "'"                | "'"          | 0x27
 240   | DoubleQuote        | "\""               | "\""         | 0x22
 242   | SingleRightQuote   | "’"                | "’"          | 0x2019
 244   | SingleLowQuote     | "‚"                | "‚"          | 0x201A
 246   | DoubleLowQuote     | "„"                | "„"          | 0x201E
 248   | LeftGuillemet      | "«"                | "«"          | 0xAB
 250   | Hash               | "#"                | "#"          | 0x23
 252   | AtSign             | "@"                | "@"          | 0x40
 254   | Exclamation        | "!"                | "!"          | 0x21
 256   | InvertedExclamation| "¡"                | "¡"          | 0xA1
 258   | QuestionMark       | "?"                | "?"          | 0x3F
 260   | InvertedQuestion   | "¿"                | "¿"          | 0xBF
 262   | Paragraph          | "§"                | "§"          | 0xA7
 264   | Ampersand          | "&"                | "&"          | 0x26
 266   | Dollar             | "$"                | "$"          | 0x24
 268   | Percent            | "%"                | "%"          | 0x25
 270   | Dong               | "₫"                | "₫"          | 0x20AB
 272   | Yen                | "¥"                | "¥"          | 0xA5
 274   | DotlessI           | "ı"                | "ı"          | 0x0131
 276   | MicroSign          | "µ"                | "µ"          | 0xB5
 278   | Eth                | "ð"                | "ð"          | 0xF0
 280   | Thorn              | "þ"                | "þ"          | 0xFE
 282   | Eszett             | "ẞ"                | "ß"          | 0xDF
 284   | KeyA               | "A"                | "a"          | 0x61
 286   | KeyB               | "B"                | "b"          | 0x62
 288   | KeyC               | "C"                | "c"          | 0x63
 290   | KeyD               | "D"                | "d"          | 0x64
 292   | KeyE               | "E"                | "e"          | 0x65
 294   | KeyF               | "F"                | "f"          | 0x66
 296   | KeyG               | "G"                | "g"          | 0x67
 298   | KeyH               | "H"                | "h"          | 0x68
 300   | KeyI               | "I"                | "i"          | 0x69
 302   | KeyJ               | "J"                | "j"          | 0x6A
 304   | KeyK               | "K"                | "k"          | 0x6B
 306   | KeyL               | "L"                | "l"          | 0x6C
 308   | KeyM               | "M"                | "m"          | 0x6D
 310   | KeyN               | "N"                | "n"          | 0x6E
 312   | KeyO               | "O"                | "o"          | 0x6F
 314   | KeyP               | "P"                | "p"          | 0x70
 316   | KeyQ               | "Q"                | "q"          | 0x71
 318   | KeyR               | "R"                | "r"          | 0x72
 320   | KeyS               | "S"                | "s"          | 0x73
 322   | KeyT               | "T"                | "t"          | 0x74
 324   | KeyU               | "U"                | "u"          | 0x75
 326   | KeyV               | "V"                | "v"          | 0x76
 328   | KeyW               | "W"                | "w"          | 0x77
 330   | KeyX               | "X"                | "x"          | 0x78
 332   | KeyY               | "Y"                | "y"          | 0x79
 334   | KeyZ               | "Z"                | "z"          | 0x7A
 336   | AeLigature         | "Æ"                | "æ"          | 0xE6
 338   | AcuteA             | "Á"                | "á"          | 0xE1
 340   | BreveA             | "Ă"                | "ă"          | 0x0103
 342   | CircumflexA        | "Â"                | "â"          | 0xE2
 344   | GraveA             | "À"                | "à"          | 0xE0
 346   | OgonekA            | "Ą"                | "ą"          | 0x0105
 348   | RingA              | "Å"                | "å"          | 0xE5
 350   | TildeA             | "Ã"                | "ã"          | 0xE3
 352   | UmlautA            | "Ä"                | "ä"          | 0xE4
 354   | AcuteC             | "Ć"                | "ć"          | 0x0107
 356   | CaronC             | "Č"                | "č"          | 0x010D
 358   | CedillaC           | "Ç"                | "ç"          | 0xE7
 360   | DotAboveC          | "Ċ"                | "ċ"          | 0x010B
 362   | AcuteE             | "É"                | "é"          | 0xE9
 364   | CircumflexE        | "Ê"                | "ê"          | 0xEA
 366   | GraveE             | "È"                | "è"          | 0xE8
 368   | DotAboveE          | "Ė"                | "ė"          | 0x0117
 370   | OgonekE            | "Ę"                | "ę"          | 0x0119
 372   | UmlautE            | "Ë"                | "ë"          | 0xEB
 374   | CrossedD           | "Đ"                | "đ"          | 0x0111
 376   | BreveG             | "Ğ"                | "ğ"          | 0x011F
 378   | DotAboveG          | "Ġ"                | "ġ"          | 0x0121
 380   | CrossedH           | "Ħ"                | "ħ"          | 0x0127
 382   | AcuteI             | "Í"                | "í"          | 0xED
 384   | CircumflexI        | "Î"                | "î"          | 0xEE
 386   | GraveI             | "Ì"                | "ì"          | 0xEC
 388   | OgonekI            | "Į"                | "į"          | 0x012F
 390   | CrossedL           | "Ł"                | "ł"          | 0x0142
 392   | CaronN             | "Ň"                | "ň"          | 0x0148
 394   | TildeN             | "Ñ"                | "ñ"          | 0xF1
 396   | AcuteO             | "Ó"                | "ó"          | 0xF3
 398   | CircumflexO        | "Ô"                | "ô"          | 0xF4
 400   | DoubleAcuteO       | "Ő"                | "ő"          | 0x0151
 402   | GraveO             | "Ò"                | "ò"          | 0xF2
 404   | HornO              | "Ơ"                | "ơ"          | 0x01A1
 406   | SlashedO           | "Ø"                | "ø"          | 0xF8
 408   | TildeO             | "Õ"                | "õ"          | 0xF5
 410   | UmlautO            | "Ö"                | "ö"          | 0xF6
 412   | AcuteS             | "Ś"                | "ś"          | 0x015B
 414   | CaronS             | "Š"                | "š"          | 0x0161
 416   | CedillaS           | "Ş"                | "ş"          | 0x015F
 418   | CommaS             | "Ș"                | "ș"          | 0x0219
 420   | CedillaT           | "Ţ"                | "ţ"          | 0x0163
 422   | CommaT             | "Ț"                | "ț"          | 0x021B
 424   | AcuteU             | "Ú"                | "ú"          | 0xFA
 426   | DoubleAcuteU       | "Ű"                | "ű"          | 0x0171
 428   | GraveU             | "Ù"                | "ù"          | 0xF9
 430   | HornU              | "Ư"                | "ư"          | 0x01B0
 432   | MacronU            | "Ū"                | "ū"          | 0x016B
 434   | OgonekU            | "Ų"                | "ų"          | 0x0173
 436   | RingU              | "Ů"                | "ů"          | 0x016F
 438   | UmlautU            | "Ü"                | "ü"          | 0xFC
 440   | CaronZ             | "Ž"                | "ž"          | 0x017E
 442   | DotAboveZ          | "Ż"                | "ż"          | 0x017C
 444   | Sleep              | "Sleep"            | ""           | 0
 446   | AppStart1          | "AppStart1"        | ""           | 0
 448   | AppStart2          | "AppStart2"        | ""           | 0
 450   | AppNewWindow       | "AppNewWindow"     | ""           | 0
 452   | AppOpenWindow      | "AppOpenWindow"    | ""           | 0
 454   | AppHelp            | "AppHelp"          | ""           | 0
 456   | AppSave            | "AppSave"          | ""           | 0
 458   | AppFind            | "AppFind"          | ""           | 0
 460   | AppPrint           | "AppPrint"         | ""           | 0
 462   | AppClose           | "AppClose"         | ""           | 0
 464   | AppCut             | "AppCut"           | ""           | 0
 466   | AppCopy            | "AppCopy"          | ""           | 0
 468   | AppPaste           | "AppPaste"         | ""           | 0
 470   | AppUndo            | "AppUndo"          | ""           | 0
 472   | AppRedo            | "AppRedo"          | ""           | 0
 474   | AppSpeechMode      | "AppSpeechMode"    | ""           | 0
 476   | AppSpeechCorrection| "AppSpeechCorrect" | ""           | 0
 478   | AppSpellCheck      | "AppSpellCheck"    | ""           | 0
 480   | Calculator         | "Calculator"       | ""           | 0
 482   | Mail               | "Mail"             | ""           | 0
 484   | MailSend           | "MailSend"         | ""           | 0
 486   | MailForward        | "MailForward"      | ""           | 0
 488   | MailReply          | "MailReply"        | ""           | 0
 490   | MediaBassBoost     | "MediaBassBoost"   | ""           | 0
 492   | MediaBassDown      | "MediaBassDown"    | ""           | 0
 494   | MediaBassUp        | "MediaBassUp"      | ""           | 0
 496   | MediaChanDown      | "MediaChanDown"    | ""           | 0
 498   | MediaChanUp        | "MediaChanUp"      | ""           | 0
 500   | MediaTrebleDown    | "MediaTrebleDown"  | ""           | 0
 502   | MediaTrebleUp      | "MediaTrebleUp"    | ""           | 0
 504   | MediaVolMute       | "MediaVolMute"     | ""           | 0
 506   | MediaVolDown       | "MediaVolDown"     | ""           | 0
 508   | MediaVolUp         | "MediaVolUp"       | ""           | 0
 510   | MediaNext          | "MediaNext"        | ""           | 0
 512   | MediaPrev          | "MediaPrev"        | ""           | 0
 514   | MediaStop          | "MediaStop"        | ""           | 0
 516   | MediaPause         | "MediaPause"       | ""           | 0
 518   | MediaPlayPause     | "MediaPlayPause"   | ""           | 0
 520   | MediaPlay          | "MediaPlay"        | ""           | 0
 522   | MediaSelectMode    | "MediaSelectMode"  | ""           | 0
 524   | MediaReverse       | "MediaReverse"     | ""           | 0
 526   | MediaRecord        | "MediaRecord"      | ""           | 0
 528   | MediaFastForward   | "MediaFastForward" | ""           | 0
 530   | MediaRewind        | "MediaRewind"      | ""           | 0
 532   | MicAirToggle       | "MicAirToggle"     | ""           | 0
 534   | MicMute            | "MicMute"          | ""           | 0
 536   | MicVolDown         | "MicVolDown"       | ""           | 0
 538   | MicVolUp           | "MicVolUp"         | ""           | 0
 540   | BrowserBackward    | "BrowserBackward"  | ""           | 0
 542   | BrowserForward     | "BrowserForward"   | ""           | 0
 544   | BrowserRefresh     | "BrowserRefresh"   | ""           | 0
 546   | BrowserStop        | "BrowserStop"      | ""           | 0
 548   | BrowserSearch      | "BrowserSearch"    | ""           | 0
 550   | BrowserFavorites   | "BrowserFavorites" | ""           | 0
 552   | BrowserHome        | "BrowserHome"      | ""           | 0

### Mouse

```
ESC _ event=mouse ; id=<ID> ; kbmods=<KeyMods> ; coor=<X>,<Y> ; buttons=<ButtonState> ; iscroll=<DeltaX>,<DeltaY> ; fscroll=<DeltaX>,<DeltaY> ESC \
```

Attribute                       | Description
--------------------------------|------------
`id=<ID>`                       | Device group ID (unsigned integer).
`kbmods=<KeyMods>`              | Keyboard modifiers bit field (unsigned integer, the same value as in Keyboard event).
`coor=<X>,<Y>`                  | 32-bit floating point coordinates of the mouse pointer relative to the console's text cell grid. The integer part corresponds to the cell coordinates, and the fractional part corresponds to the normalized position within the cell. The pointer's screen pixel coordinates can be calculated by multiplying these floating point values by the cell size. Receiving a NaN value is a signal that the mouse has left the window or disconnected.
`buttons=<ButtonState>`         | Mouse buttons bit field (unsigned integer).
`iscroll=<DeltaX>,<DeltaY>`     | Horizontal and vertical low-resolution scroll deltas in form of signed integers (one scroll line corresponds to a value of 1). Low-resolution scroll deltas increase as the values of high-resolution deltas accumulate, and are zeroed when the scroll direction changes.
`fscroll=<DeltaX>,<DeltaY>`     | Horizontal and vertical high-resolution scroll deltas in form of 32-bit floating-point values (one scroll line corresponds to a value of 1.0f).

The mouse tracking event fires on any mouse activity, as well as on keyboard modifier changes.

#### Mouse button state

Bit | Active button
----|--------------
0   | Left
1   | Right
2   | Middle
3   | 4th
4   | 5th
... | ...
N-1 | Nth

Note: Mouse tracking will continue outside the terminal window as long as the mouse button pressed inside the window is active. In this case, coordinates with negative values are possible.

### Focus

```
ESC _ event=focus ; id=<ID> ; state=<FocusState> ESC \
```

Attribute            | Description
---------------------|------------
`id=<ID>`            | Device group ID (unsigned integer). Identifies the specific keyboard/mouse pair that receives focus. This is necessary in a multi-user environment where multiple input sessions may exist concurrently.
`state=<FocusState>` | Terminal window focus:<br>\<FocusState\>=1 - Focused.<br>\<FocusState\>=0 - Unfocused.

In response to the activation of `focus` tracking, the application receives a vt-sequence containing current focus state.

### Format

```
ESC _ event=format ; wrapping=<State> ; alignment=<State> ; rtl=<State> ESC \
```

Attribute           | Description
--------------------|------------
`wrapping=<State>`  | Line wrapping:<br>\<State\>=0 - Unwrapped.<br>\<State\>=1 - Wrapped.
`alignment=<State>` | Line alignment:<br>\<State\>=0 - Left.<br>\<State\>=1 - Right.<br>\<State\>=2 - Center.
`rtl=<State>`       | Text flow direction:<br>\<State\>=0 - Left to Right.<br>\<State\>=1 - Right to Left.

In response to the activation of `format` tracking, the application receives a vt-sequence containing current line format.

### Clipboard

```
ESC _ event=clipboard ; id=<ID> ; format=<ClipFormat> ; security=<SecLevel> ; data=<Data> ESC \
```

Attribute             | Description
----------------------|------------
`id=<ID>`             | Device group ID (unsigned integer).
`format=<ClipFormat>` | Clipboard data format.
`security=<SecLevel>` | Security level bit field.
`data=<Data>`         | Base64 encoded data.

#### Clipboard data format

Format | Description
-------|------------
`text` | Plain text.
`rich` | Rich text format.
`html` | HTML format.
`ansi` | ANSI/VT format.

#### Security level

Bit    | Description
-------|------------
`0x01` | Exclude clipboard content from monitor processing.
`0x02` | Can include in clipboard history.
`0x04` | Can upload to cloud clipboard.

### Window

```
ESC _ event=window ; size=<Width>,<Height> ; cursor=<X>,<Y> ; region=<Left>,<Top>,<Right>,<Bottom> ; selection=<StartX>,<StartY>,<EndX>,<EndY>,<Mode> ESC \
```

Attribute                                          | Description
---------------------------------------------------|------------
`size=<Width>,<Height>`                            | Terminal window size in cells.
`cursor=<X>,<Y>`                                   | Current text cursor position.
`region=<Left>,<Top>,<Right>,<Bottom>`             | Scrolling region margins.
`selection=<StartX>,<StartY>,<EndX>,<EndY>,<Mode>` | Coordinates of the text selection start/end (half-open interval) and text selection mode:<br>\<Mode\>=0 - Line-based.<br>\<Mode\>=1 - Rect-based.

In response to the activation of `window` tracking, the application receives a vt-sequence containing current window state.

#### Mandatory synchronization

When the terminal window is resized, the changes are only applied after a handshake between the terminal and the application. Successive multiple resizing of the window initiates the same number of handshakes.

Handshake steps:
1. Terminal requests a new size, omitting all other parameters.
   - Application receives the request and prepares for resizing, e.g. by updating, cropping or deleting visible lines.
2. Application replies with the same message as the request.
3. Terminal applies the new size and sends the changes.
   - Application updates its UI.

```
Terminal:    ESC _ event=window ; size=<Width>,<Height> ESC \
Application: ESC _ event=window ; size=<Width>,<Height> ESC \
Terminal:    ESC _ event=window ; size=<Width>,<Height> ; cursor=<X>,<Y> ; region=<Left>,<Top>,<Right>,<Bottom> ; selection=<StartX>,<StartY>,<EndX>,<EndY>,<Mode> ESC \
```

Note that the terminal window resizing always reflows the scrollback, so the window size, cursor position, scrolling regions, and selection coordinates are subject to change during step 3. Upon receiving the resize request (step 1), a fullscreen application can prepare a scrollback by cropping visible lines to avoid unwanted line wrapping or line extrusion, then send a resize confirmation (step 2). In case the application's output is anchored to the current cursor position or uses scrolling regions, the application should wait after step 2 for the updated values before continuing to output.

Hypothetical case with Far Manager (FM):
- FM saves visible original scrollback.
- FM draws its UI on top of the visible original scrollback.
- Terminal sends a resize request.
- FM receives request.
- FM restores the original scrollback.
- FM replies on resize request.
- Terminal receives reply.
- Terminal resizes the window and reflows the scrollback.
- Terminal sends modified window parameters.
- FM receives modified window parameters.
- FM saves the modified scrollback.
- FM draws its UI on top of the scrollback.

#### Selection tracking

The window size tracking sequence is fired after every scrollback text selection changed. In the case of using the mouse for selection, a single left click is treated as a special case of selection when the start and end are the same (empty selection).

Note that selected text in the scrollback above the top index row will produce negative Y-coordinate values.

### System

```
ESC _ event=system ; signal=<Signal> ESC \
```

Signal | Description
-------|------------
`0`    | Window closing.
`1`    | System shutdown.

The application must respond to the terminal within 5 seconds with the same message confirming that it will close itself without being forced. After a response to Signal=0, the application can continue running and closing the terminal window will be silently aborted. In the absence of confirmation, and also in the case of Signal=1, the application will be forced to close.

//todo
## Usage Examples

### C++20

```c++
#include <iostream>

int main()
{
    std::cout << "test" << '\n';
}
```

### Python

```python
#!/bin/python

while True:
    print('test\n')
```

### PowerShell

```powershell
while ($True)
{
    "test\n";
}
```
