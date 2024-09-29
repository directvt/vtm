# Unicode Character Geometry Modifiers

In the terminal world, character width detection is not well defined and is context dependent, which is the cause of the following issues:

- No way to specify a custom width for displayed characters.
- No way to simultaneously display the same characters in both narrow and wide variants.
- No way to use triple and quadruple characters along with narrow and wide.
- Different assumptions about character widths in applications and terminal emulators.
- No way to display wide characters partially.
- No way to display characters higher two cells.
- No way to display subcell sized characters.
- No way to rotate or mirror characters.

## Character Matrix

By defining that the graphical representation of a character is a cellular matrix (1x1 matrix consists of one fragment), the concept of "wide/narrow" can be completely avoided.

1x1 | 2x2 | 3x1
----|-----|-----
![SGR-CFA-A](https://github.com/directvt/vtm/assets/11535558/9eba4601-6bab-4498-8f89-2aee70b59b38) | ![SGR-CFA-E](https://github.com/directvt/vtm/assets/11535558/77c73079-8ea2-43f3-a5a8-2c2ec3adc5ec) | ![SGR-CFA-Indic](https://github.com/directvt/vtm/assets/11535558/f646cc6d-7d01-409a-a8eb-53c42536fdab)

Each character is a sequence of codepoints (one or more) - this is the so-called grapheme cluster. Using a font, this sequence is translated into a glyph run. The final scaling and rasterization of the glyph run is done into a rectangular terminal cell matrix, defined either implicitly based on the Unicode properties of the cluster codepoints, or explicitly using a modifier codepoint from the Unicode codepoint range 0xD0000-0xD02A2.

Matrix fragments up to 8x4 cells require at least four associated integer values, which can be packed into Unicode codepoint space by enumerating "wh_xy" values:
  - w: Character matrix width.
  - h: Character matrix height.
  - x: Horizontal fragment selector inside the matrix.
  - y: Vertical fragment selector inside the matrix.
  - For character matrices larger than 8x4, pixel graphics should be used.

![image](https://github.com/directvt/vtm/assets/11535558/88bf5648-533e-4786-87de-b3dc4103273c)

Terminals can annotate each scrollback cell with character matrix metadata and use it to display either the entire character image or a specific fragment within the cell.

Users can explicitly specify the size of the character matrix (by zeroing `_xy`) or select any fragment of it (non-zero `_xy`) by placing a specific modifier character after the grapheme cluster.

- Example 1. Output a 3x1 character:
  - `pwsh`
    ```pwsh
    "👩‍👩‍👧‍👧`u{D0033}"
    ```
  - `wsl/bash`
    ```bash
    printf "👩‍👩‍👧‍👧\UD0033\n"
    ```
- Example 2. Output a 6x2 character (by stacking two 6x1 fragments on top of each other due to the linear nature of the terminal):
  - `pwsh`
    ```pwsh
    "👩‍👩‍👧‍👧`u{D00C9}`n👩‍👩‍👧‍👧`u{D00F6}"
    ```
  - `wsl/bash`
    ```bash
    printf "👩‍👩‍👧‍👧\UD00C9\n👩‍👩‍👧‍👧\UD00F6\n"
    ```
- Screenshot:  
  ![image](https://github.com/user-attachments/assets/5c6d0e5c-ba36-4602-a626-95f64042c67f)

## Grapheme cluster boundaries

By default, grapheme clustering occurs according to `Unicode UAX #29` https://www.unicode.org/reports/tr29/#Grapheme_Cluster_Boundary_Rules.

To set arbitrary boundaries, the C0 control character `ASCII 0x02 STX` is used, signaling the beginning of a grapheme cluster. The closing character of a grapheme cluster in that case is always a codepoint from the range 0xD0000-0xDFFFF, which sets the dimension of the character matrix. All codepoints between STX and the closing codepoint that sets the matrix size will be included in the grapheme cluster.

## Another brick in the wall

> At present only standardized variation sequences with VS1, VS2, VS3, VS15 and VS16 have been defined; VS15 and VS16 are reserved to request that a character should be displayed as text or as an emoji) respectively.

> VS4–VS14 (U+FE03–U+FE0D) are not used for any variation sequences

- https://en.wikipedia.org/wiki/Variation_Selectors_(Unicode_block)
- https://www.unicode.org/Public/UNIDATA/StandardizedVariants.txt
- https://www.unicode.org/reports/tr51/tr51-16.html#Direction

So, let's try to play this way:

### Glyph run alignment inside the matrix

VS  | Codepoint | Axis       | Alignment
----|-----------|------------|--------------
VS4 | 0xFE03    | Horizontal | Left
VS5 | 0xFE04    | Horizontal | Center
VS6 | 0xFE05    | Horizontal | Right
VS7 | 0xFE06    | Vertical   | Top
VS8 | 0xFE07    | Vertical   | Middle
VS9 | 0xFE08    | Vertical   | Bottom

Notes:
- We are not operating at a low enough level to support justified alignment.
- By default, glyphs are aligned on the baseline at the writing origin.

### Matrix rotation and flips

VS   | Codepoint | Fx
-----|-----------|-----------
VS10 | 0xFE09    | Rotate 90° CCW
VS11 | 0xFE0A    | Rotate 180° CCW
VS12 | 0xFE0B    | Rotate 270° CCW
VS13 | 0xFE0C    | Horizontal flip
VS14 | 0xFE0D    | Vertical flip

Example functions for applying a rotation operation to the current three bits integer `state`:
```c++
void VS10(int& state) { state = (state & 0b100) | ((state + 0b001) & 0b011); }
void VS11(int& state) { state = (state & 0b100) | ((state + 0b010) & 0b011); }
void VS12(int& state) { state = (state & 0b100) | ((state + 0b011) & 0b011); }
void VS13(int& state) { state = (state ^ 0b100) | ((state + (state & 1 ? 0 : 0b010)) & 0b011); }
void VS14(int& state) { state = (state ^ 0b100) | ((state + (state & 1 ? 0b010 : 0)) & 0b011); }

int get_angle(int state) { int angle = 90 * (state & 0b011); return angle; }
int get_hflip(int state) { int hflip = state >> 2; return hflip; }
```

# Summary

![image](https://github.com/user-attachments/assets/4f9f7450-a49c-43db-8001-e8be4530450e)
