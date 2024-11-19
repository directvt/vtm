status: draft

# VT Input Mode Protocol

The goal of the `vt-input-mode` protocol is to make command line interactivity cross-platform.

- No TTY required.
- No OS-level signal tracking required.

## Audience

Anyone who wants to:
- Operate without TTY.
- Share applications on LAN (using inetd, netcat, etc).
- Track every key press and key release.
- Track position dependent keys such as WASD.
- Distinguish between Left and Right physical keys.
- Get consistent output regardless of terminal window resize.
- Track mouse on a pixel-wise level.
- Track mouse outside the terminal window (getting negative coordinates).
- Take advantage of high-resolution wheel scrolling.
- Track scrollback text manipulation.
- Track application closing and system shutdown.
- Be independent of operating system and third party libraries.

Existing approaches have the following drawbacks:
- There is no uniform way to receive keyboard events.
- Window size tracking requires platform-specific calls with no way to synchronize the output.
- Mouse tracking modes lack support for negative coordinates, high-resolution wheel scrolling, and have a limited set of buttons.
- Bracketed paste mode does not support the transfer of binary data and data containing sequences of bracketed paste mode itself.

## Conventions

- We use HEX-form of the uint32 (IEEE-754 32-bit binary float, Little-Endian) for the floating point value representation.
- Space characters are not used in sequence payloads and are only used for readability of the description.
- All unescaped symbols outside of this protocol should be treated as clipboard pasted data.

### Format

Signaling uses APC `ESC _ <payload> ESC \` with an event-specific payload syntax.

The payload consists of a list of attributes in the following format:
```
<attr>=<val>,...,<val>; ...; <attr>=<val>,...,<val>
```

Field             | Descriprtion
------------------|-------------
`<attr>`          | Attribute name.
`<val>,...,<val>` | Comma-separated value list.

## Initialization

```
Set:   ESC _ events=<Source0>,...,<SourceN> ESC \
Reset: ESC _ events ESC \
```

Source     | Events to track
-----------|----------------
`keyboard` | Keyboard.
`mouse`    | Mouse.
`focus`    | Focus.
`format`   | Line format.
`clipoard` | Clipboard.
`window`   | Window size and selection.
`system`   | System signals.

This sequence enables `vt-input-mode` and event tracking for the specified event `Source`s. The `vt-input-mode` is deactivated if none of the `Source`s is specified.

Note: By enabling `vt-input-mode`, all current terminal modes are automatically saved (to be restored on exit) and switched to something like "raw" mode, in which input is available character by character, echoing is disabled, and all special processing of terminal input and output characters is disabled (except for `LF` to `CR+LF` conversion).

## Events

- Keyboard
  ```
  ESC _ event=keyboard ; id=0 ; kbmods=<KeyMods> ; keyid=<KeyId> ; pressed=<KeyDown> ; scancode=<ScanCode> ; id_chord=<HexFormString> ; ch_chord=<HexFormString> ; sc_chord=<HexFormString> ; cluster=<C0>,...,<Cn> ESC \
  ```
- Mouse
  ```
  ESC _ event=mouse ; id=0 ; kbmods=<KeyMods> ; coord=<X>,<Y> ; buttons=<ButtonState> ; wheel=<DeltaY>,<DeltaX> ESC \
  ```
- Focus
  ```
  ESC _ event=focus ; id=0 ; state=<FocusState> ESC \
  ```
- Format
  ```
  ESC _ event=format ; wrapping=<State> ; alignment=<State> ; rtl=<State> ESC \
  ```
//todo Textinput, Text, IME or Input for IME preview etc
- Clipboard
  ```
  ESC _ event=clipoard ; id=0 ; format=<ClipFormat> ; security=<SecLevel> ; data=<Data> ESC \
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
ESC _ event=keyboard ; id=0 ; kbmods=<KeyMods> ; keyid=<KeyId> ; pressed=<KeyDown> ; scancode=<ScanCode> ; id_chord=<HexFormString> ; ch_chord=<HexFormString> ; sc_chord=<HexFormString> ; cluster=<C0>,...,<Cn> ESC \
```

> Q: Do we need to track scancode chord? `scanchord=<Code0>,...,<CodeN>`?

Attribute                     | Description
------------------------------|------------
`id=0`                        | Seat id.
`kbmods=<KeyMods>`            | Keyboard modifiers.
`keyid=<KeyId>`               | Physical key ID.
`pressed=<KeyDown>`           | Key state:<br>\<KeyDown\>=1 - Pressed.<br>\<KeyDown\>=0 - Released.
`scancode=<ScanCode>`         | Scan code.
`id_chord=<HexFormString>`    | Simultaneously pressed key id's in ascending order. //todo define format
`ch_chord=<HexFormString>`    | Simultaneously pressed key id's and grapheme cluster at the last place representing a key press.
`sc_chord=<HexFormString>`    | Simultaneously pressed key scancodes in ascending order.
`cluster=<C0>,...,<Cn>`       | Codepoints of the generated string/text cluster.

In response to the activation of `keyboard` tracking, the application receives a vt-sequence containing keyboard modifiers state:
```
ESC _ event=keyboard ; id=0 ; kbmods=<KeyMods> ESC \
```

The full sequence is fired after every key press and key release. The sequence can contain a string generated by a keystroke as a set of codepoints: `C0 + ... + Cn`. ~~The string can be fragmented and delivered by multiple consecutive events.~~

//todo revise, define format
The `xx_chord=<HexFormString>` attribute contains the set of simultaneously pressed key id's (KeyId0,...,KeyIdN) in ascending order and is used to track key combinations. It is possible to track both chord presses `+` (e.g. `Ctrl+F1`) and chord releases `-` (e.g. `Ctrl-F1` or `Ctrl-Alt`):
...

#### Keyboard modifiers

The state `kbmods=<KeyMods>` of keyboard modifiers is the binary OR of all currently pressed modifiers and enabled modes.

 Bit | Side   | Modifier Key                 | Value
 ----|--------|------------------------------|--------------
 0   | Left   | <kbd>⌃ Ctrl</kbd>            | `0x0001`
 1   | Right  | <kbd>⌃ Ctrl</kbd>            | `0x0002`
 2   | Left   | <kbd>⎇ Alt</kbd><br><kbd>⌥ Option</kbd>                       | `0x0004`
 3   | Right  | <kbd>⎇ Alt</kbd><br><kbd>⌥ Option</kbd><br><kbd>⇮ AltGr</kbd> | `0x0008`
 4   | Left   | <kbd>⇧ Shift</kbd>           | `0x0010`
 5   | Right  | <kbd>⇧ Shift</kbd>           | `0x0020`
 6   | Left   | <kbd>⊞ Win</kbd><br><kbd>⌘ Command</kbd><br><kbd>◆ Meta</kbd><br><kbd>❖ Super</kbd> | `0x0040`
 7   | Right  | <kbd>⊞ Win</kbd><br><kbd>⌘ Command</kbd><br><kbd>◆ Meta</kbd><br><kbd>❖ Super</kbd> | `0x0080`
 8   |        | <kbd>reserved</kbd>          | `0x0100`
 9   |        | <kbd>reserved</kbd>          | `0x0200`
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

Scan codes for the keys on a standard 104-key keyboard:

```
┌────┐  ┌────╥────╥────╥────┐  ┌────╥────╥────╥────┐  ┌────╥────╥────╥────┐  ┌────╥────╥────┐
| 01 │  | 3B ║ 3C ║ 3D ║ 3E |  | 3F ║ 40 ║ 41 ║ 42 |  | 43 ║ 44 ║ 57 ║ 58 |  |E037║ 46 ║E045|
└────┘  └────╨────╨────╨────┘  └────╨────╨────╨────┘  └────╨────╨────╨────┘  └────╨────╨────┘
┌────╥────╥────╥────╥────╥────╥────╥────╥────╥────╥────╥────╥────╥────────┐  ┌────╥────╥────┐  ┌────╥────╥────╥────┐
| 29 ║ 02 ║ 03 ║ 04 ║ 05 ║ 06 ║ 07 ║ 08 ║ 09 ║ 0A ║ 0B ║ 0C ║ 0D ║     0E |  |E052║E047║E049|  | 45 ║E035║ 37 ║ 4A |
╞════╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══════╡  ╞════╬════╬════╡  ╞════╬════╬════╬════╡
| 0F   ║ 10 ║ 11 ║ 12 ║ 13 ║ 14 ║ 15 ║ 16 ║ 17 ║ 18 ║ 19 ║ 1A ║ 1B ║   2B |  |E053║E04F║E051|  | 47 ║ 48 ║ 49 ║    |
╞══════╩╦═══╩╦═══╩╦═══╩╦═══╩╦═══╩╦═══╩╦═══╩╦═══╩╦═══╩╦═══╩╦═══╩╦═══╩══════╡  └────╨────╨────┘  ╞════╬════╬════╣ 4E |
| 3A    ║ 1E ║ 1F ║ 20 ║ 21 ║ 22 ║ 23 ║ 24 ║ 25 ║ 26 ║ 27 ║ 28 ║       1C |                    | 4B ║ 4C ║ 4D ║    |
╞═══════╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩═╦══╩══════════╡       ┌────┐       ╞════╬════╬════╬════╡
| 2A      ║ 2C ║ 2D ║ 2E ║ 2F ║ 30 ║ 31 ║ 32 ║ 33 ║ 34 ║ 35 ║          36 |       |E048|       | 4F ║ 50 ║ 51 ║    |
╞══════╦══╩═╦══╩═══╦╩════╩════╩════╩════╩════╩════╩╦═══╩╦═══╩╦═════╦══════╡  ┌────┼────┼────┐  ╞════╩════╬════╣E01C|
| 1D   ║ 5B ║ 38   ║               39              ║E038║ 5C ║  5D ║ E01D |  |E04B|E050|E04D|  |      52 ║ 53 ║    |
└──────╨────╨──────╨───────────────────────────────╨────╨────╨─────╨──────┘  └────┴────┴────┘  └─────────╨────╨────┘
```

#### Physical keys

The `<KeyId>` is incremented by 2 for each generic key, providing two `<KeyId>` placeholders for each physical key to distinguish between Left and Right (or Numpad) in the last bit. Ignore the last bit of `<KeyId>` for tracking generic keys.

Key ID | Name               | Generic Name       | Scan Code | Notes
-------|--------------------|--------------------|-----------|------
0      | `undef`            | `undef`            | `0x0000`  |
1      | `config`           | `config`           | `0x00FF`  |
2      | `LeftCtrl`         | `Ctrl`             | `0x001D`  |
3      | `RightCtrl`        | `Ctrl`             | `0x011D`  |
4      | `LeftAlt`          | `Alt`              | `0x0038`  |
5      | `RightAlt`         | `Alt`              | `0x0138`  |
6      | `LeftShift`        | `Shift`            | `0x002A`  |
7      | `RightShift`       | `Shift`            | `0x0036`  |
8      | `LeftWin`          | `Win`              | `0x015B`  |
9      | `RightWin`         | `Win`              | `0x015C`  |
10     | `Apps`             | `Apps`             | `0x015D`  |
12     | `NumLock`          | `NumLock`          | `0x0045`  |
14     | `CapsLock`         | `CapsLock`         | `0x003A`  |
16     | `ScrollLock`       | `ScrollLock`       | `0x0045`  |
18     | `Esc`              | `Esc`              | `0x0001`  |
20     | `Space`            | `Space`            | `0x0039`  |
22     | `Backspace`        | `Backspace`        | `0x000E`  |
24     | `Tab`              | `Tab`              | `0x000F`  |
26     | `Break`            | `Break`            | `0x0046`  | Ctrl + Pause
28     | `Pause`            | `Pause`            | `0x0045`  |
30     | `Select`           | `Select`           | `0x0000`  |
32     | `SysRq`            | `SysRq`            | `0x0054`  | Alt + PrintScreen
34     | `PrintScreen`      | `PrintScreen`      | `0x0137`  |
36     | `KeyEnter`         | `Enter`            | `0x001C`  |
37     | `NumpadEnter`      | `Enter`            | `0x011C`  |
38     | `KeyPageUp`        | `PageUp`           | `0x0149`  |
39     | `NumpadPageUp`     | `PageUp`           | `0x0049`  |
40     | `KeyPageDown`      | `PageDown`         | `0x0151`  |
41     | `NumpadPageDown`   | `PageDown`         | `0x0051`  |
42     | `KeyEnd`           | `End`              | `0x014F`  |
43     | `NumpadEnd`        | `End`              | `0x004F`  |
44     | `KeyHome`          | `Home`             | `0x0147`  |
45     | `NumpadHome`       | `Home`             | `0x0047`  |
46     | `KeyLeftArrow`     | `LeftArrow`        | `0x014B`  |
47     | `NumpadLeftArrow`  | `LeftArrow`        | `0x004B`  |
48     | `KeyUpArrow`       | `UpArrow`          | `0x0148`  |
49     | `NumpadUpArrow`    | `UpArrow`          | `0x0048`  |
50     | `KeyRightArrow`    | `RightArrow`       | `0x014D`  |
51     | `NumpadRightArrow` | `RightArrow`       | `0x004D`  |
52     | `KeyDownArrow`     | `DownArrow`        | `0x0150`  |
53     | `NumpadDownArrow`  | `DownArrow`        | `0x0050`  |
54     | `Key0`             | `0`                | `0x000B`  |
55     | `Numpad0`          | `0`                | `0x0052`  |
56     | `Key1`             | `1`                | `0x0002`  |
57     | `Numpad1`          | `1`                | `0x004F`  |
58     | `Key2`             | `2`                | `0x0003`  |
59     | `Numpad2`          | `2`                | `0x0050`  |
60     | `Key3`             | `3`                | `0x0004`  |
61     | `Numpad3`          | `3`                | `0x0051`  |
62     | `Key4`             | `4`                | `0x0005`  |
63     | `Numpad4`          | `4`                | `0x004B`  |
64     | `Key5`             | `5`                | `0x0006`  |
65     | `Numpad5`          | `5`                | `0x004C`  |
66     | `Key6`             | `6`                | `0x0007`  |
67     | `Numpad6`          | `6`                | `0x004D`  |
68     | `Key7`             | `7`                | `0x0008`  |
69     | `Numpad7`          | `7`                | `0x0047`  |
70     | `Key8`             | `8`                | `0x0009`  |
71     | `Numpad8`          | `8`                | `0x0048`  |
72     | `Key9`             | `9`                | `0x000A`  |
73     | `Numpad9`          | `9`                | `0x0049`  |
74     | `KeyInsert`        | `Insert`           | `0x0152`  |
75     | `NumpadInsert`     | `Insert`           | `0x0052`  |
76     | `KeyDelete`        | `Delete`           | `0x0153`  |
77     | `NumpadDelete`     | `Delete`           | `0x0055`  |
78     | `KeyClear`         | `Clear`            | `0x014C`  |
79     | `NumpadClear`      | `Clear`            | `0x004C`  | Numpad 5
80     | `KeyMultiply`      | `*`                | `0x0009`  |
81     | `NumpadMultiply`   | `*`                | `0x0037`  |
82     | `KeyPlus`          | `Plus`             | `0x000D`  |
83     | `NumpadPlus`       | `Plus`             | `0x004E`  |
84     | `KeySeparator`     | `Separator`        | `0x0000`  |
85     | `NumpadSeparator`  | `Separator`        | `0x0000`  |
86     | `KeyMinus`         | `Minus`            | `0x000C`  |
87     | `NumpadMinus`      | `Minus`            | `0x004A`  |
88     | `KeyPeriod`        | `.`                | `0x0034`  |
89     | `NumpadDecimal`    | `.`                | `0x0053`  |
90     | `KeySlash`         | `/`                | `0x0035`  |
91     | `NumpadSlash`      | `/`                | `0x0135`  |
92     | `BackSlash`        | `BackSlash`        | `0x002B`  |
94     | `OpenBracket`      | `[`                | `0x001A`  |
96     | `ClosedBracket`    | `]`                | `0x001B`  |
98     | `Equal`            | `=`                | `0x000D`  |
100    | `BackQuote`        | `` ` ``            | `0x0029`  |
102    | `SingleQuote`      | `'`                | `0x0028`  |
104    | `Comma`            | `,`                | `0x0033`  |
106    | `Semicolon`        | `;`                | `0x0027`  |
108    | `F1`               | `F1`               | `0x003B`  |
110    | `F2`               | `F2`               | `0x003C`  |
112    | `F3`               | `F3`               | `0x003D`  |
114    | `F4`               | `F4`               | `0x003E`  |
116    | `F5`               | `F5`               | `0x003F`  |
118    | `F6`               | `F6`               | `0x0040`  |
120    | `F7`               | `F7`               | `0x0041`  |
122    | `F8`               | `F8`               | `0x0042`  |
124    | `F9`               | `F9`               | `0x0043`  |
126    | `F10`              | `F10`              | `0x0044`  |
128    | `F11`              | `F11`              | `0x0057`  |
130    | `F12`              | `F12`              | `0x005B`  |
132    | `F13`              | `F13`              | `0x0000`  |
134    | `F14`              | `F14`              | `0x0000`  |
136    | `F15`              | `F15`              | `0x0000`  |
138    | `F16`              | `F16`              | `0x0000`  |
140    | `F17`              | `F17`              | `0x0000`  |
142    | `F18`              | `F18`              | `0x0000`  |
144    | `F19`              | `F19`              | `0x0000`  |
146    | `F20`              | `F20`              | `0x0000`  |
148    | `F21`              | `F21`              | `0x0000`  |
150    | `F22`              | `F22`              | `0x0000`  |
152    | `F23`              | `F23`              | `0x0000`  |
154    | `F24`              | `F24`              | `0x0000`  |
156    | `KeyA`             | `A`                | `0x0000`  |
158    | `KeyB`             | `B`                | `0x0000`  |
160    | `KeyC`             | `C`                | `0x0000`  |
162    | `KeyD`             | `D`                | `0x0000`  |
164    | `KeyE`             | `E`                | `0x0000`  |
166    | `KeyF`             | `F`                | `0x0000`  |
168    | `KeyG`             | `G`                | `0x0000`  |
170    | `KeyH`             | `H`                | `0x0000`  |
172    | `KeyI`             | `I`                | `0x0000`  |
174    | `KeyJ`             | `J`                | `0x0000`  |
176    | `KeyK`             | `K`                | `0x0000`  |
178    | `KeyL`             | `L`                | `0x0000`  |
180    | `KeyM`             | `M`                | `0x0000`  |
182    | `KeyN`             | `N`                | `0x0000`  |
184    | `KeyO`             | `O`                | `0x0000`  |
186    | `KeyP`             | `P`                | `0x0000`  |
188    | `KeyQ`             | `Q`                | `0x0000`  |
190    | `KeyR`             | `R`                | `0x0000`  |
192    | `KeyS`             | `S`                | `0x0000`  |
194    | `KeyT`             | `T`                | `0x0000`  |
196    | `KeyU`             | `U`                | `0x0000`  |
198    | `KeyV`             | `V`                | `0x0000`  |
200    | `KeyW`             | `W`                | `0x0000`  |
202    | `KeyX`             | `X`                | `0x0000`  |
204    | `KeyY`             | `Y`                | `0x0000`  |
206    | `KeyZ`             | `Z`                | `0x0000`  |
208    | `Sleep`            | `Sleep`            | `0x0000`  |
210    | `Calculator`       | `Calculator`       | `0x0000`  |
212    | `Mail`             | `Mail`             | `0x0000`  |
214    | `MediaVolMute`     | `MediaVolMute`     | `0x0000`  |
216    | `MediaVolDown`     | `MediaVolDown`     | `0x0000`  |
218    | `MediaVolUp`       | `MediaVolUp`       | `0x0000`  |
220    | `MediaNext`        | `MediaNext`        | `0x0000`  |
222    | `MediaPrev`        | `MediaPrev`        | `0x0000`  |
224    | `MediaStop`        | `MediaStop`        | `0x0000`  |
226    | `MediaPlayPause`   | `MediaPlayPause`   | `0x0000`  |
228    | `MediaSelect`      | `MediaSelect`      | `0x0000`  |
230    | `BrowserBack`      | `BrowserBack`      | `0x0000`  |
232    | `BrowserForward`   | `BrowserForward`   | `0x0000`  |
234    | `BrowserRefresh`   | `BrowserRefresh`   | `0x0000`  |
236    | `BrowserStop`      | `BrowserStop`      | `0x0000`  |
238    | `BrowserSearch`    | `BrowserSearch`    | `0x0000`  |
240    | `BrowserFavorites` | `BrowserFavorites` | `0x0000`  |
242    | `BrowserHome`      | `BrowserHome`      | `0x0000`  |

### Mouse

```
ESC _ event=mouse ; id=0 ; kbmods=<KeyMods> ; coord=<X>,<Y> ; buttons=<ButtonState> ; wheel=<DeltaY>,<DeltaX> ESC \
```

Attribute                 | Description
--------------------------|------------
`id=0`                    | Seat id.
`kbmods=<KeyMods>`        | Keyboard modifiers (see Keyboard event).
`coord=<X>,<Y>`           | Pixel-wise coordinates of the mouse pointer. Each coordinate is represented in the form of a floating point value of the sum of the integer coordinate of the cell in the terminal window grid and the relative offset within the cell in the range `[0.0f, 1.0f)`.
`buttons=<ButtonState>`   | Mouse button state.
`wheel=<DeltaY>,<DeltaX>` | Vertical and horizontal wheel high-resolution delta represented as floating point value.

In response to the activation of `mouse` tracking, the application receives a vt-sequence containing current mouse state:
```
ESC _ event=mouse ; kbmods=<KeyMods> ; coord=<X>,<Y> ; buttons=<ButtonState> ESC \
```

The mouse tracking event fires on any mouse activity, as well as on keyboard modifier changes.

#### Mouse button state

Bit | Active button
----|--------------
0   | Left
1   | Right
2   | Middle
3   | 4th
4   | 5th

Note: Mouse tracking will continue outside the terminal window as long as the mouse button pressed inside the window is active. In this case, coordinates with negative values are possible.

### Focus

```
ESC _ event=focus ; id=0 ; state=<FocusState> ESC \
```

Attribute            | Description
---------------------|------------
`id=0`               | Seat id.
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
ESC _ event=clipoard ; id=0 ; format=<ClipFormat> ; security=<SecLevel> ; data=<Data> ESC \
```

Attribute             | Description
----------------------|------------
`id=0`                | Seat id.
`format=<ClipFormat>` | Clipboard data format.
`security=<SecLevel>` | Security level.
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

Note that the terminal window resizing always reflows the scrollback, so the window size, cursor position, scrolling regions, and selection coordinates are subject to change during step 3. Upon receiving the resize request (step 1), a fullscreen application can prepare a scrollback by cropping visible lines to avoid unwanted line wrapping or line extrusion, then send a resize confirmation (step 2). In case the aplication's output is anchored to the current cursor position or uses scrolling regions, the application should wait after step 2 for the updated values before continuing to output. 

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