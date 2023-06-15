# Keyboard/Viewport/Mouse VT Protocol

The introduced `kvm-input-mode` protocol is backwards-compatible with the `win32-input-mode` protocol. The goal of this protocol is to raise the interactivity level of the cross-platform command line interface.

## Audience

...

## Initialization
```
Set:   ESC [ ? 9001 h
Reset: ESC [ ? 9001 l
```

## Input Events

- Keyboard
- Viewport
- Mouse
- Selection
- Clipboard

All values used in this protocol are decimal and zero-based.  
The bracketed paste mode is mandatory.

### Keyboard

The sequence fired after every key press and release.

```
ESC [ VirtualKey ; ScanCode ; BaseChar ; KeyDown ; CtrlState ; RepeatCount ; C1 ; … ; Cn _
```

Field            | Description
-----------------|--------------------------
`VirtualKey`     | Virtual key code.
`ScanCode`       | Key scan code.
`BaseChar`       | Base(first) Unicode character codepoint.
`KeyDown`        | Key pressed flag: 1 - Pressed, 0 - Released.
`CtrlState`      | Keyboard modifiers state.
`RepeatCount`    | Key repeat count.
`C1`, …, `Cn`    | Continuing character codepoints.

### Viewport

The following sequence is the first one received by the application after `kvm-input-mode` request (a-la mode request acknowledgment). The application can respond with a copy of this message if it needs to enable viewport and mouse tracking.

```
ESC [ # 1 ; CaretX ; CaretY ; WinSizeX ; WinSizeY ; ScrollTop ; ScrollBottom ; ScrollLeft ; ScrollRight ; SelStartX ; SelStartY ; SelEndX ; SelEndY ; SelMode ; CtrlState _
```

#### Viewport tracking

When the terminal window is resized, the viewport is finally changed only after a handshake between the terminal and the application. Successive multiple resizing of the window initiates the same number of handshakes.

Handshake steps:
1. The terminal requests a new size.
2. Application must reply with the same message.
3. The terminal applies the new size and sends the changes.
4. Application must reply with the same message.

Between 2 and 4 steps, the application must keep radio silence, as their output may be ignored by the terminal.

```
Terminal:    ESC [ # 0 ; WinSizeX ; WinSizeY _
Application: ESC [ # 0 ; WinSizeX ; WinSizeY _
Terminal:    ESC [ # 1 ; CaretX ; CaretY ; WinSizeX ; WinSizeY ; ScrollTop ; ScrollBottom ; ScrollLeft ; ScrollRight ; SelStartX ; SelStartY ; SelEndX ; SelEndY ; SelMode ; CtrlState _
Application: ESC [ # 1 ; CaretX ; CaretY ; WinSizeX ; WinSizeY ; ScrollTop ; ScrollBottom ; ScrollLeft ; ScrollRight ; SelStartX ; SelStartY ; SelEndX ; SelEndY ; SelMode ; CtrlState _
```

Note that the terminal window resizing always reflows the scrollback, so the viewport size, cursor position, scrollable region margins, and selection coordinates are changed at step 3 of handshake.

Field                                        | Description
---------------------------------------------|------------
`CaretX`<br>`CaretY`                         | Current text cursor position.
`WinSizeX`<br>`WinSizeY`                     | Terminal viewport size.
`ScrollTop`/`Bottom`<br>`ScrollLeft`/`Right` | Scrolling region margins.
`SelStartX`/`Y`<br>`SelEndX`/`Y`             | Coordinates of the text selection start/end.
`SelMode`                                    | Text selection mode: 0 - line-based, 1 - rect-based.
`CtrlState`                                  | Keyboard modifiers state.

### Mouse

This event always fires regardless of whether the mouse tracking mode is active or not. An application MUST ignore this event if a selection mode frag is non-zero, unless it intends to participate in the processing of the selection.

```
ESC [ # 2 ; CaretX ; CaretY ; MouseX ; MouseY ; ButtonState ; VwheelDt ; HwheelDt ; CtrlState ; SelActive _
```

Field                    | Description
-------------------------|--------------------------
`CaretX`<br>`CaretY`     | Current text cursor position.
`MouseX`<br>`MouseY`     | Current mouse cursor position.
`ButtonState`            | Pressed mouse button state.
`VwheelDt`<br>`HwheelDt` | Vertical and horizontal wheel delta.
`CtrlState`              | Keyboard modifiers state.
`SelActive`              | Text selection flag: 0 - Disabled; non-zero - Enabled.

### Selection

The sequence fired after every scrollback text selection changed.

```
ESC [ # 3 ; CaretX ; CaretY ; SelStartX ; SelStartY ; SelEndX ; SelEndY ; SelMode _
```

Field                            | Description
---------------------------------|--------------------------
`CaretX`<br>`CaretY`             | Current text cursor position.
`SelStartX`/`Y`<br>`SelEndX`/`Y` | Coordinates of the text selection start/end.
`SelMode`                        | Text selection mode: 0 - line-based, 1 - rect-based.

### Clipboard

The sequence is used to input a block of unmodified text, mainly for pasted from the clipboard.

```
ESC [ 200 ~   to signify the beginning of pasted text and
ESC [ 201 ~   to signify the end.
```

## Constants

### Scan Codes

...

### Keyboard Modifiers

The state `CtrlState` of keyboard modifiers is the sum of all currently pressed modifiers and enabled modes.

Modifier                                | kvm CtrlState   | win32 CtrlState
----------------------------------------|-----------------|-----------------
<kbd>RightAlt</kbd> or <kbd>AltGr</kbd> | `   1` `0x0001` | `   1`  `0x0001`
<kbd>LeftAlt</kbd>                      | `   2` `0x0002` | `   2`  `0x0002`
<kbd>RightCtrl</kbd>                    | `   4` `0x0004` | `   4`  `0x0004`
<kbd>LeftCtrl</kbd>                     | `   8` `0x0008` | `   8`  `0x0008`
<kbd>RightShift</kbd>                   | `2064` `0x0810` | `  16`  `0x0010`
<kbd>LeftShift</kbd>                    | `  16` `0x0010` | `  16`  `0x0010`
NumLock on                              | `  32` `0x0020` | `  32`  `0X0020`
ScrollLock mode on                      | `  64` `0x0040` | `  64`  `0X0040`
CapsLock mode on                        | ` 128` `0x0080` | ` 128`  `0x0080`
Extended key flag                       | ` 256` `0x0100` | ` 256`  `0x0100`
<kbd>RightWin</kbd>                     | ` 512` `0x0200` |
<kbd>LeftWin</kbd>                      | `1024` `0x0400` |

### Virtual Key Codes

The following codes is based on the Win32 Virtual Keys repertoire to be compatible with win32-input-mode.

Dec  | Hex  | Name                  | Key
-----|------|-----------------------|------------
0    | 0x00 |                       |
1    | 0x01 |                       |
2    | 0x02 |                       |
3    | 0x03 | VK_CANCEL             | <kbd>Break</kbd>
4    | 0x04 |                       |
5    | 0x05 |                       |
6    | 0x06 |                       |
7    | 0x07 |                       |
8    | 0x08 | VK_BACK               | <kbd>Backspace</kbd>
9    | 0x09 | VK_TAB                | <kbd>Tab</kbd>
10   | 0x0A |                       |
11   | 0x0B |                       |
12   | 0x0C | VK_CLEAR              | <kbd>Clear</kbd> or Numpad <kbd>5</kbd>
13   | 0x0D | VK_RETURN             | <kbd>Enter</kbd>
14   | 0x0E |                       |
15   | 0x0F |                       |
16   | 0x10 | VK_SHIFT              | <kbd>Shift</kbd>
17   | 0x11 | VK_CONTROL            | <kbd>Ctrl</kbd>
18   | 0x12 | VK_MENU               | <kbd>Alt</kbd>
19   | 0x13 | VK_PAUSE              | <kbd>Pause</kbd>
20   | 0x14 | VK_CAPITAL            | <kbd>CapsLock</kbd>
21   | 0x15 | VK_KANA               | IME <kbd>Kana</kbd>
22   | 0x16 | VK_IME_ON             | IME <kbd>On</kbd>
23   | 0x17 | VK_JUNJA              | IME <kbd>Junja</kbd>
24   | 0x18 | VK_FINAL              | IME <kbd>Final</kbd>
25   | 0x19 | VK_KANJI              | IME <kbd>Kanji</kbd>
26   | 0x1A | VK_IME_OFF            | IME <kbd>Off</kbd>
27   | 0x1B | VK_ESCAPE             | <kbd>Esc</kbd>
28   | 0x1C | VK_CONVERT            | IME <kbd>Convert</kbd>
29   | 0x1D | VK_NONCONVERT         | IME <kbd>Nonconvert</kbd>
30   | 0x1E | VK_ACCEPT             | IME <kbd>Accept</kbd>
31   | 0x1F | VK_MODECHANGE         | IME <kbd>Mode</kbd>
32   | 0x20 | VK_SPACE              | <kbd>Space</kbd>
33   | 0x21 | VK_PRIOR              | <kbd>PageUp</kbd>
34   | 0x22 | VK_NEXT               | <kbd>PageDown</kbd>
35   | 0x23 | VK_END                | <kbd>End</kbd>
36   | 0x24 | VK_HOME               | <kbd>Home</kbd>
37   | 0x25 | VK_LEFT               | <kbd>Left Arrow</kbd>
38   | 0x26 | VK_UP                 | <kbd>Up Arrow</kbd>
39   | 0x27 | VK_RIGHT              | <kbd>Right Arrow</kbd>
40   | 0x28 | VK_DOWN               | <kbd>Down Arrow</kbd>
41   | 0x29 | VK_SELECT             | <kbd>Select</kbd>
42   | 0x2A |                       |
43   | 0x2B |                       |
44   | 0x2C | VK_SNAPSHOT           | <kbd>PrintScreen</kbd>
45   | 0x2D | VK_INSERT             | <kbd>Insert</kbd>
46   | 0x2E | VK_DELETE             | <kbd>Delete</kbd>
47   | 0x2F |                       |
48   | 0x30 | VK_0                  | <kbd>0</kbd>
49   | 0x31 | VK_1                  | <kbd>1</kbd>
50   | 0x32 | VK_2                  | <kbd>2</kbd>
51   | 0x33 | VK_3                  | <kbd>3</kbd>
52   | 0x34 | VK_4                  | <kbd>4</kbd>
53   | 0x35 | VK_5                  | <kbd>5</kbd>
54   | 0x36 | VK_6                  | <kbd>6</kbd>
55   | 0x37 | VK_7                  | <kbd>7</kbd>
56   | 0x38 | VK_8                  | <kbd>8</kbd>
57   | 0x39 | VK_9                  | <kbd>9</kbd>
58   | 0x3A |                       |
59   | 0x3B |                       |
60   | 0x3C |                       |
61   | 0x3D |                       |
62   | 0x3E |                       |
63   | 0x3F |                       |
64   | 0x40 |                       |
65   | 0x41 | VK_A                  | <kbd>A</kbd>
66   | 0x42 | VK_B                  | <kbd>B</kbd>
67   | 0x43 | VK_C                  | <kbd>C</kbd>
68   | 0x44 | VK_D                  | <kbd>D</kbd>
69   | 0x45 | VK_E                  | <kbd>E</kbd>
70   | 0x46 | VK_F                  | <kbd>F</kbd>
71   | 0x47 | VK_G                  | <kbd>G</kbd>
72   | 0x48 | VK_H                  | <kbd>H</kbd>
73   | 0x49 | VK_I                  | <kbd>I</kbd>
74   | 0x4A | VK_J                  | <kbd>J</kbd>
75   | 0x4B | VK_K                  | <kbd>K</kbd>
76   | 0x4C | VK_L                  | <kbd>L</kbd>
77   | 0x4D | VK_M                  | <kbd>M</kbd>
78   | 0x4E | VK_N                  | <kbd>N</kbd>
79   | 0x4F | VK_O                  | <kbd>O</kbd>
80   | 0x50 | VK_P                  | <kbd>P</kbd>
81   | 0x51 | VK_Q                  | <kbd>Q</kbd>
82   | 0x52 | VK_R                  | <kbd>R</kbd>
83   | 0x53 | VK_S                  | <kbd>S</kbd>
84   | 0x54 | VK_T                  | <kbd>T</kbd>
85   | 0x55 | VK_U                  | <kbd>U</kbd>
86   | 0x56 | VK_V                  | <kbd>V</kbd>
87   | 0x57 | VK_W                  | <kbd>W</kbd>
88   | 0x58 | VK_X                  | <kbd>X</kbd>
89   | 0x59 | VK_Y                  | <kbd>Y</kbd>
90   | 0x5A | VK_Z                  | <kbd>Z</kbd>
91   | 0x5B | VK_LWIN               | <kbd>LeftWin</kbd>
92   | 0x5C | VK_RWIN               | <kbd>RightWin</kbd>
93   | 0x5D | VK_APPS               | <kbd>Application</kbd>
94   | 0x5E |                       |
95   | 0x5F | VK_SLEEP              | <kbd>Sleep</kbd>
96   | 0x60 | VK_NUMPAD0            | Numpad <kbd>0</kbd>
97   | 0x61 | VK_NUMPAD1            | Numpad <kbd>1</kbd>
98   | 0x62 | VK_NUMPAD2            | Numpad <kbd>2</kbd>
99   | 0x63 | VK_NUMPAD3            | Numpad <kbd>3</kbd>
100  | 0x64 | VK_NUMPAD4            | Numpad <kbd>4</kbd>
101  | 0x65 | VK_NUMPAD5            | Numpad <kbd>5</kbd>
102  | 0x66 | VK_NUMPAD6            | Numpad <kbd>6</kbd>
103  | 0x67 | VK_NUMPAD7            | Numpad <kbd>7</kbd>
104  | 0x68 | VK_NUMPAD8            | Numpad <kbd>8</kbd>
105  | 0x69 | VK_NUMPAD9            | Numpad <kbd>9</kbd>
106  | 0x6A | VK_MULTIPLY           | Numpad <kbd>*</kbd>
107  | 0x6B | VK_ADD                | Numpad <kbd>+</kbd>
108  | 0x6C | VK_SEPARATOR          | Numpad <kbd>Separator</kbd>
109  | 0x6D | VK_SUBTRACT           | Numpad <kbd>-</kbd>
110  | 0x6E | VK_DECIMAL            | Numpad <kbd>.</kbd>
111  | 0x6F | VK_DIVIDE             | Numpad <kbd>/</kbd>
112  | 0x70 | VK_F1                 | <kbd>F1</kbd>
113  | 0x71 | VK_F2                 | <kbd>F2</kbd>
114  | 0x72 | VK_F3                 | <kbd>F3</kbd>
115  | 0x73 | VK_F4                 | <kbd>F4</kbd>
116  | 0x74 | VK_F5                 | <kbd>F5</kbd>
117  | 0x75 | VK_F6                 | <kbd>F6</kbd>
118  | 0x76 | VK_F7                 | <kbd>F7</kbd>
119  | 0x77 | VK_F8                 | <kbd>F8</kbd>
120  | 0x78 | VK_F9                 | <kbd>F9</kbd>
121  | 0x79 | VK_F10                | <kbd>F10</kbd>
122  | 0x7A | VK_F11                | <kbd>F11</kbd>
123  | 0x7B | VK_F12                | <kbd>F12</kbd>
124  | 0x7C | VK_F13                | <kbd>F13</kbd>
125  | 0x7D | VK_F14                | <kbd>F14</kbd>
126  | 0x7E | VK_F15                | <kbd>F15</kbd>
127  | 0x7F | VK_F16                | <kbd>F16</kbd>
128  | 0x80 | VK_F17                | <kbd>F17</kbd>
129  | 0x81 | VK_F18                | <kbd>F18</kbd>
130  | 0x82 | VK_F19                | <kbd>F19</kbd>
131  | 0x83 | VK_F20                | <kbd>F20</kbd>
132  | 0x84 | VK_F21                | <kbd>F21</kbd>
133  | 0x85 | VK_F22                | <kbd>F22</kbd>
134  | 0x86 | VK_F23                | <kbd>F23</kbd>
135  | 0x87 | VK_F24                | <kbd>F24</kbd>
136  | 0x88 |                       |
137  | 0x89 |                       |
138  | 0x8A |                       |
139  | 0x8B |                       |
140  | 0x8C |                       |
141  | 0x8D |                       |
142  | 0x8E |                       |
143  | 0x8F |                       |
144  | 0x90 | VK_NUMLOCK            | <kbd>NumLock</kbd>
145  | 0x91 | VK_SCROLL             | <kbd>ScrollLock</kbd>
146  | 0x92 |                       |
147  | 0x93 |                       |
148  | 0x94 |                       |
149  | 0x95 |                       |
150  | 0x96 |                       |
151  | 0x97 |                       |
152  | 0x98 |                       |
153  | 0x99 |                       |
154  | 0x9A |                       |
155  | 0x9B |                       |
156  | 0x9C |                       |
157  | 0x9D |                       |
158  | 0x9E |                       |
159  | 0x9F |                       |
160  | 0xA0 |                       |
161  | 0xA1 |                       |
162  | 0xA2 |                       |
163  | 0xA3 |                       |
164  | 0xA4 |                       |
165  | 0xA5 |                       |
166  | 0xA6 | VK_BROWSER_BACK       | Browser <kbd>Back</kbd>
167  | 0xA7 | VK_BROWSER_FORWARD    | Browser <kbd>Forward</kbd>
168  | 0xA8 | VK_BROWSER_REFRESH    | Browser <kbd>Refresh</kbd>
169  | 0xA9 | VK_BROWSER_STOP       | Browser <kbd>Stop</kbd>
170  | 0xAA | VK_BROWSER_SEARCH     | Browser <kbd>Search</kbd>
171  | 0xAB | VK_BROWSER_FAVORITES  | Browser <kbd>Favorites</kbd>
172  | 0xAC | VK_BROWSER_HOME       | Browser <kbd>Home</kbd>
173  | 0xAD | VK_VOLUME_MUTE        | Media <kbd>Mute</kbd>
174  | 0xAE | VK_VOLUME_DOWN        | Media <kbd>Volume Down</kbd>
175  | 0xAF | VK_VOLUME_UP          | Media <kbd>Volume Up</kbd>
176  | 0xB0 | VK_MEDIA_NEXT_TRACK   | Media <kbd>Next</kbd>
177  | 0xB1 | VK_MEDIA_PREV_TRACK   | Media <kbd>Prev</kbd>
178  | 0xB2 | VK_MEDIA_STOP         | Media <kbd>Stop</kbd>
179  | 0xB3 | VK_MEDIA_PLAY_PAUSE   | Media <kbd>Play/Pause</kbd>
180  | 0xB4 | VK_LAUNCH_MAIL        | <kbd>Mail</kbd>
181  | 0xB5 | VK_LAUNCH_MEDIA_SELECT| <kbd>Media</kbd>
182  | 0xB6 | VK_LAUNCH_APP1        | <kbd>App1</kbd>
183  | 0xB7 | VK_LAUNCH_APP2        | <kbd>App2</kbd>
184  | 0xB8 |                       |
185  | 0xB9 |                       |
186  | 0xBA | VK_OEM_1              | OEM <kbd>;</kbd> or <kbd>:</kbd>
187  | 0xBB | VK_OEM_PLUS           | OEM <kbd>+</kbd>
188  | 0xBC | VK_OEM_COMMA          | OEM <kbd>,</kbd>
189  | 0xBD | VK_OEM_MINUS          | OEM <kbd>-</kbd>
190  | 0xBE | VK_OEM_PERIOD         | OEM <kbd>.</kbd>
191  | 0xBF | VK_OEM_2              | OEM <kbd>/</kbd> or <kbd>?</kbd>
192  | 0xC0 | VK_OEM_3              | OEM <kbd>`</kbd> or <kbd>~</kbd>
193  | 0xC1 |                       |
194  | 0xC2 |                       |
195  | 0xC3 |                       |
196  | 0xC4 |                       |
197  | 0xC5 |                       |
198  | 0xC6 |                       |
199  | 0xC7 |                       |
200  | 0xC8 |                       |
201  | 0xC9 |                       |
202  | 0xCA |                       |
203  | 0xCB |                       |
204  | 0xCC |                       |
205  | 0xCD |                       |
206  | 0xCE |                       |
207  | 0xCF |                       |
208  | 0xD0 |                       |
209  | 0xD1 |                       |
210  | 0xD2 |                       |
211  | 0xD3 |                       |
212  | 0xD4 |                       |
213  | 0xD5 |                       |
214  | 0xD6 |                       |
215  | 0xD7 |                       |
216  | 0xD8 |                       |
217  | 0xD9 |                       |
218  | 0xDA |                       |
219  | 0xDB | VK_OEM_4              | OEM <kbd>[</kbd> or <kbd>{</kbd>
220  | 0xDC | VK_OEM_5              | OEM <kbd>\\</kbd> or <kbd>\|</kbd>
221  | 0xDD | VK_OEM_6              | OEM <kbd>]</kbd> or <kbd>}</kbd>
222  | 0xDE | VK_OEM_7              | OEM <kbd>'</kbd> or <kbd>"</kbd>
223  | 0xDF | VK_OEM_8              | OEM <kbd>;</kbd>
224  | 0xE0 |                       |
225  | 0xE1 | VK_OEM_AX             | OEM <kbd>AX</kbd>
226  | 0xE2 | VK_OEM_102            | OEM <kbd>\<</kbd>, <kbd>\></kbd>, <kbd>\\</kbd> or <kbd>\|</kbd>
227  | 0xE3 | VK_ICO_HELP           | ICO <kbd>Help</kbd>
228  | 0xE4 | VK_ICO_00             | ICO <kbd>00</kbd>
229  | 0xE5 | VK_PROCESSKEY         | IME <kbd>Process</kbd>
230  | 0xE6 | VK_ICO_CLEAR          | ICO <kbd>Clear</kbd>
231  | 0xE7 | VK_PACKET             | non-keyboard input
232  | 0xE8 |                       |
233  | 0xE9 |                       |
234  | 0xEA |                       |
235  | 0xEB |                       |
236  | 0xEC |                       |
237  | 0xED |                       |
238  | 0xEE |                       |
239  | 0xEF |                       |
240  | 0xF0 |                       |
241  | 0xF1 |                       |
242  | 0xF2 |                       |
243  | 0xF3 |                       |
244  | 0xF4 |                       |
245  | 0xF5 |                       |
246  | 0xF6 | VK_ATTN               | <kbd>Attn</kbd>
247  | 0xF7 | VK_CRSEL              | <kbd>CrSel</kbd>
248  | 0xF8 | VK_EXSEL              | <kbd>ExSel</kbd>
249  | 0xF9 | VK_EREOF              | <kbd>Erase EOF</kbd>
250  | 0xFA | VK_PLAY               | <kbd>Play</kbd>
251  | 0xFB | VK_ZOOM               | <kbd>Zoom</kbd>
252  | 0xFC |                       |
253  | 0xFD | VK_PA1                | <kbd>PA1</kbd>
254  | 0xFE | VK_OEM_CLEAR          | OEM <kbd>Clear</kbd>
255  | 0xFF |                       |

### Physical Key Detection

Key                           | VirtCode     | ScanCode     | CtrlState  | Note
------------------------------|--------------|--------------|------------|------
<kbd>Esc</kbd>                | ` 27` `0x1B` | `  1` `0x01` |            |
<kbd>Pause</kbd>              | ` 19` `0x13` | ` 69` `0x45` |            |
<kbd>Break</kbd>              | `  3` `0x03` | ` 69` `0x45` |            | Ctrl + Break
<kbd>PrintScreen</kbd>        | ` 44` `0x2C` | ` 55` `0x37` | & `0x0100` |
<kbd>CapsLock</kbd>           | ` 20` `0x14` | ` 58` `0x3A` |            |
<kbd>NumLock</kbd>            | `144` `0x90` | ` 69` `0x45` |            |
<kbd>ScrollLock</kbd>         | `145` `0x91` | ` 69` `0x45` |            |
<kbd>LeftShift</kbd>          | ` 16` `0x10` | ` 42` `0x2A` |            |
<kbd>RightShift</kbd>         | ` 16` `0x10` | ` 54` `0x36` |            |
<kbd>LeftCtrl</kbd>           | ` 17` `0x11` | ` 29` `0x1D` |            |
<kbd>RightCtrl</kbd>          | ` 17` `0x11` | ` 29` `0x1D` | & `0x0100` |
<kbd>LeftAlt</kbd>            | ` 18` `0x12` | ` 56` `0x38` |            |
<kbd>RightAlt</kbd>           | ` 18` `0x12` | ` 56` `0x38` | & `0x0100` |
<kbd>LeftWin</kbd>            | ` 91` `0x5B` | ` 91` `0x5B` | & `0x0100` |
<kbd>RightWin</kbd>           | ` 92` `0x5C` | ` 92` `0x5C` | & `0x0100` |
<kbd>Apps</kbd>               | ` 93` `0x5D` | ` 93` `0x5D` | & `0x0100` |
<kbd>Backspace</kbd>          | `  8` `0x08` | ` 14` `0x0E` |            |
<kbd>Space</kbd>              | ` 32` `0x20` | ` 57` `0x39` |            |
<kbd>Tab</kbd>                | `  9` `0x09` | ` 15` `0x0F` |            |
<kbd>'</kbd>                  | `192` `0xC0` | ` 41` `0x29` |            | Back qoute
<kbd>Backslash</kbd>          | `220` `0xDC` | ` 43` `0x2B` |            |
<kbd>0</kbd>                  | ` 48` `0x30` | ` 11` `0x0B` |            |
<kbd>1</kbd>                  | ` 49` `0x31` | `  2` `0x02` |            |
<kbd>2</kbd>                  | ` 50` `0x32` | `  3` `0x03` |            |
<kbd>3</kbd>                  | ` 51` `0x33` | `  4` `0x04` |            |
<kbd>4</kbd>                  | ` 52` `0x34` | `  5` `0x05` |            |
<kbd>5</kbd>                  | ` 53` `0x35` | `  6` `0x06` |            |
<kbd>6</kbd>                  | ` 54` `0x36` | `  7` `0x07` |            |
<kbd>7</kbd>                  | ` 55` `0x37` | `  8` `0x08` |            |
<kbd>8</kbd>                  | ` 56` `0x38` | `  9` `0x09` |            |
<kbd>9</kbd>                  | ` 57` `0x39` | ` 10` `0x0A` |            |
<kbd>-</kbd>                  | `189` `0xBD` | ` 12` `0x0C` |            |
<kbd>+</kbd>                  | `187` `0xBB` | ` 13` `0x0D` |            |
<kbd>Enter</kbd>              | ` 13` `0x0D` | ` 28` `0x1C` |            |
<kbd>Numpad Enter</kbd>       | ` 13` `0x0D` | ` 28` `0x1C` | & `0x0100` |
<kbd>Insert</kbd>             | ` 45` `0x2D` | ` 82` `0x52` | & `0x0100` |
<kbd>Numpad Insert</kbd>      | ` 45` `0x2D` | ` 82` `0x52` |            |
<kbd>Delete</kbd>             | ` 46` `0x2E` | ` 83` `0x53` | & `0x0100` |
<kbd>Numpad Delete</kbd>      | ` 46` `0x2E` | ` 83` `0x55` |            |
<kbd>Home</kbd>               | ` 36` `0x24` | ` 71` `0x47` | & `0x0100` |
<kbd>Numpad Home</kbd>        | ` 36` `0x24` | ` 71` `0x47` |            |
<kbd>End</kbd>                | ` 35` `0x23` | ` 79` `0x4F` | & `0x0100` |
<kbd>Numpad End</kbd>         | ` 35` `0x23` | ` 79` `0x4F` |            |
<kbd>PageUp</kbd>             | ` 33` `0x21` | ` 73` `0x49` | & `0x0100` |
<kbd>Numpad PageUp</kbd>      | ` 33` `0x21` | ` 73` `0x49` |            |
<kbd>PageDown</kbd>           | ` 34` `0x22` | ` 81` `0x51` | & `0x0100` |
<kbd>Numpad PageDown</kbd>    | ` 34` `0x22` | ` 81` `0x51` |            |
<kbd>Left Arrow</kbd>         | ` 37` `0x25` | ` 75` `0x4B` | & `0x0100` |
<kbd>Numpad Left Arrow</kbd>  | ` 37` `0x25` | ` 75` `0x4B` |            |
<kbd>Up Arrow</kbd>           | ` 38` `0x26` | ` 72` `0x48` | & `0x0100` |
<kbd>Numpad Up Arrow</kbd>    | ` 38` `0x26` | ` 72` `0x48` |            |
<kbd>Right Arrow</kbd>        | ` 39` `0x27` | ` 77` `0x4D` | & `0x0100` |
<kbd>Numpad Right Arrow</kbd> | ` 39` `0x27` | ` 77` `0x4D` |            |
<kbd>Down Arrow</kbd>         | ` 40` `0x28` | ` 80` `0x50` | & `0x0100` |
<kbd>Numpad Down Arrow</kbd>  | ` 40` `0x28` | ` 80` `0x50` |            |
<kbd>Numpad 0</kbd>           | ` 96` `0x60` | ` 82` `0x52` | & `0x0020` |
<kbd>Numpad 1</kbd>           | ` 97` `0x61` | ` 79` `0x4F` | & `0x0020` |
<kbd>Numpad 2</kbd>           | ` 98` `0x62` | ` 80` `0x50` | & `0x0020` |
<kbd>Numpad 3</kbd>           | ` 99` `0x63` | ` 81` `0x51` | & `0x0020` |
<kbd>Numpad 4</kbd>           | `100` `0x64` | ` 75` `0x4B` | & `0x0020` |
<kbd>Numpad 5</kbd>           | `101` `0x65` | ` 76` `0x4C` | & `0x0020` |
<kbd>Numpad 6</kbd>           | `102` `0x66` | ` 77` `0x4D` | & `0x0020` |
<kbd>Numpad 7</kbd>           | `103` `0x67` | ` 71` `0x47` | & `0x0020` |
<kbd>Numpad 8</kbd>           | `104` `0x68` | ` 72` `0x48` | & `0x0020` |
<kbd>Numpad 9</kbd>           | `105` `0x69` | ` 73` `0x49` | & `0x0020` |
<kbd>Numpad Clear</kbd>       | ` 12` `0x0C` | ` 76` `0x4C` |            |
<kbd>Numpad *</kbd>           | `106` `0x6A` |              |            |
<kbd>Numpad +</kbd>           | `107` `0x6B` |              |            |
<kbd>Numpad Separator</kbd>   | `108` `0x6C` |              |            |
<kbd>Numpad -</kbd>           | `109` `0x6D` |              |            |
<kbd>Numpad .</kbd>           | `110` `0x6E` |              | & `0x0020` |
<kbd>Numpad /</kbd>           | `111` `0x6F` |              | & `0x0100` |
<kbd>/</kbd>                  | `191` `0xBF` |              |            |
<kbd>,</kbd>                  | `188` `0xBC` |              |            | Comma
<kbd>.</kbd>                  | `190` `0xBE` |              |            | Period
<kbd>[</kbd>                  | `219` `0xDB` |              |            |
<kbd>]</kbd>                  | `221` `0xDD` |              |            |
<kbd>;</kbd>                  | `186` `0xBA` |              |            |
<kbd>'</kbd>                  | `222` `0xDE` |              |            | Single qoute
<kbd>F1</kbd>                 | `112` `0x70` | ` 59` `0x3B` |            |
<kbd>F2</kbd>                 | `113` `0x71` | ` 61` `0x3C` |            |
<kbd>F3</kbd>                 | `114` `0x72` | ` 62` `0x3D` |            |
<kbd>F4</kbd>                 | `115` `0x73` | ` 63` `0x3E` |            |
<kbd>F5</kbd>                 | `116` `0x74` | ` 64` `0x3F` |            |
<kbd>F6</kbd>                 | `117` `0x75` | ` 65` `0x40` |            |
<kbd>F7</kbd>                 | `118` `0x76` | ` 66` `0x41` |            |
<kbd>F8</kbd>                 | `119` `0x77` | ` 67` `0x42` |            |
<kbd>F9</kbd>                 | `120` `0x78` | ` 68` `0x43` |            |
<kbd>F10</kbd>                | `121` `0x79` | ` 69` `0x44` |            |
<kbd>F11</kbd>                | `122` `0x7A` | ` 87` `0x57` |            |
<kbd>F12</kbd>                | `123` `0x7B` | ` 88` `0x5B` |            |
<kbd>F13</kbd>                | `124` `0x7C` |              |            |
<kbd>F14</kbd>                | `125` `0x7D` |              |            |
<kbd>F15</kbd>                | `126` `0x7E` |              |            |
<kbd>F16</kbd>                | `127` `0x7F` |              |            |
<kbd>F17</kbd>                | `128` `0x80` |              |            |
<kbd>F18</kbd>                | `129` `0x81` |              |            |
<kbd>F19</kbd>                | `130` `0x82` |              |            |
<kbd>F20</kbd>                | `131` `0x83` |              |            |
<kbd>F21</kbd>                | `132` `0x84` |              |            |
<kbd>F22</kbd>                | `133` `0x85` |              |            |
<kbd>F23</kbd>                | `134` `0x86` |              |            |
<kbd>F24</kbd>                | `135` `0x87` |              |            |
<kbd>A</kbd>                  | ` 65` `0x41` |              |            |
<kbd>B</kbd>                  | ` 66` `0x42` |              |            |
<kbd>C</kbd>                  | ` 67` `0x43` |              |            |
<kbd>D</kbd>                  | ` 68` `0x44` |              |            |
<kbd>E</kbd>                  | ` 69` `0x45` |              |            |
<kbd>F</kbd>                  | ` 70` `0x46` |              |            |
<kbd>G</kbd>                  | ` 71` `0x47` |              |            |
<kbd>H</kbd>                  | ` 72` `0x48` |              |            |
<kbd>I</kbd>                  | ` 73` `0x49` |              |            |
<kbd>J</kbd>                  | ` 74` `0x4A` |              |            |
<kbd>K</kbd>                  | ` 75` `0x4B` |              |            |
<kbd>L</kbd>                  | ` 76` `0x4C` |              |            |
<kbd>M</kbd>                  | ` 77` `0x4D` |              |            |
<kbd>N</kbd>                  | ` 78` `0x4E` |              |            |
<kbd>O</kbd>                  | ` 79` `0x4F` |              |            |
<kbd>P</kbd>                  | ` 80` `0x50` |              |            |
<kbd>Q</kbd>                  | ` 81` `0x51` |              |            |
<kbd>R</kbd>                  | ` 82` `0x52` |              |            |
<kbd>S</kbd>                  | ` 83` `0x53` |              |            |
<kbd>T</kbd>                  | ` 84` `0x54` |              |            |
<kbd>U</kbd>                  | ` 85` `0x55` |              |            |
<kbd>V</kbd>                  | ` 86` `0x56` |              |            |
<kbd>W</kbd>                  | ` 87` `0x57` |              |            |
<kbd>X</kbd>                  | ` 88` `0x58` |              |            |
<kbd>Y</kbd>                  | ` 89` `0x59` |              |            |
<kbd>Z</kbd>                  | ` 90` `0x5A` |              |            |
<kbd>Calculator</kbd>         | `183` `0xB7` |              | & `0x0100` | App2
<kbd>WWW</kbd>                | `172` `0xAC` |              | & `0x0100` |
<kbd>Mail</kbd>               | `184` `0x48` |              | & `0x0100` |
<kbd>Media</kbd>              | `181` `0xB5` |              | & `0x0100` |
<kbd>Mute</kbd>               | `173` `0xAD` |              | & `0x0100` |
<kbd>Volume Down</kbd>        | `174` `0xAE` |              | & `0x0100` |
<kbd>Volume Up</kbd>          | `175` `0xAF` |              | & `0x0100` |

...

### Mouse Buttons

...