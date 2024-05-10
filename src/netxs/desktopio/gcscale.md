# Character Fragments and Oversize Characters

This proposal solves the problem of displaying wide characters in a text-based environment allowing to store individual fragments of the character in a cell-based grid.

Accordingly to the [Unicode® Standard Annex #29, "UNICODE TEXT SEGMENTATION"](https://unicode.org/reports/tr29/#Grapheme_Cluster_Boundaries)
> It is important to recognize that what the user thinks of as a “character”—a basic unit of a writing system for a language—may not be just a single Unicode code point. Instead, that basic unit may be made up of multiple Unicode code points. To avoid ambiguity with the computer use of the term character, this is called a user-perceived character. For example, “G” + grave-accent is a user-perceived character: users think of it as a single character, yet is actually represented by two Unicode code points. These user-perceived characters are approximated by what is called a grapheme cluster, which can be determined programmatically.

In this proposal, a character is defined as a user-perceived character (or grapheme cluster).

## Memory Storage

Every grid cell aside the rest of rendition attributes and grapheme cluster reference should contain the following additional numeric values:

- Width of the character matrix in cells.
- Height of the character matrix in cells.
- X-coordinate of the character fragment contained in the cell.
- Y-coordinate of the character fragment contained in the cell.

## Unicode Character Modifier

> A variant form is a different glyph for a character, encoded in Unicode through the mechanism of variation sequences: sequences in Unicode that consist of a base character followed by a variation selector character.

Encoding Format

According to https://www.unicode.org/ivd/data/2022-09-13/ Variation Selectors in range from 0xE0120 up to 0xE01FF are not used. So user can use a sub-range of the [Unicode Variation Selectors](https://en.wikipedia.org/wiki/Variation_Selectors_Supplement) to specify character dimensions or select its fragment. Four integer values are packed into one byte by plain enumeration of "wh_xy" records.
  - w: Matrix width
  - h: Matrix height
  - x: Horizontal fragment selector inside the matrix
  - y: Vertical fragment selector inside the matrix

Unicode block for "Unicode Character Modifiers" `VSwh_xy`:

![image](https://github.com/directvt/vtm/assets/11535558/792a5b87-712f-4313-91bc-9637964fc7fa)

todo Language Tags Unicode block is unused today E0000 - E00FF.

Placement in the text:

```
<basechar><VSwh_xy>
```

Examples:

  - 😊+`<VS11_00>` produce 1x1 character.
  - 😊+`<VS21_00>` produce 2x1 character.
  - 👨‍👩‍👧‍👦+`<VS31_00>` produce 3x1 character.
  - 😊+`<VS42_00>` produce 4x2 character.
  - A+`<VS44_00>` produce 4x4 character.
  - 😊+`<VS21_11>` produce the left half of the 2x1 character.
  - 😊+`<VS21_21>` produce the right half of the 2x1 character.

## VT Sequence: CFA - Character Fragmentation Attribute.

The following sequences allow to set the character 4x4-fragmentation attribute as a cell rendition state:

- Human readable sequence format:
  ```
               ┌──────────────────────────────── matrix width, range: 1..4 cells
               │     ┌────────────────────────── matrix height, range: 1..4 cells
               │     │     ┌──────────────────── column selector, range: 1..W cells, 0 - to select all
               │     │     │     ┌────────────── row selector, range: 1..H cells, 0 - to select all
               │     │     │     │     ┌──────── text cluster length in codepoints, always > 0
               │     │     │     │     │     ┌── text rotation inside the ligature: 0 - 0°, 1 - 90°, 2 - 180°, 3 - 270°
  ESC [ 110 : <W> : <H> : <X> : <Y> : <S> : <R> m
  ```
  - The SGR Reset `\e[m` command resets mode.
  - Missing values are treated as 1.
  - `W = 0` resets the fragmentation mode.
- Sequence with a "cooked" parameter:
  ```
    ESC [ 111 : <P> m
  ```
  - `P = (X - 1) + (W - 1) * 4 + (Y - 1) * 16 + (H - 1) * 64` from 0 to 255.
  - Missing value is treated as 0.
  - `P = 0` resets the fragmentation mode.

`ESC[m` resets all SGR-attributes and the same way resets the character fragmentation attribute.

## Examples

```
- cout "a" produce 1x1 in buffer:
  [1/1,1/1]
- cout "😊" produce 2x1 in buffer:
  [1/2,1/1][2/2,1/1]
- cout "👨‍👩‍👧‍👦" produce 3x1 in buffer:
  [1/3,1/1][2/3,1/1][3/3,1/1]
- cout "<SCALE1;1;1;1>😀" produce 1x1
- cout "<SCALE3;1;3;1>😀" produce 3x3
- cout "<SCALE1;1;1;1>👨‍👩‍👧‍👦" produce 1x1
- cout "<SCALE1;1;1;1>😀X" produce 1x1, 1x1
- cout "<SCALE2;1;1;1>😀X<SCALE0;0;0;0>H" produce 2x1(😀), 2x1(X), 1x1(H)
- cout "<SCALE1;2;1;1>😀🌎XH😀😀" produce
  [1/2,1/1](left half 😀), [2/2,1/1](right half 🌎), [1/2,1/1](left half X) , [2/2,1/1](right half H), [1/2,1/1](left half 😀) , [2/2,1/1](right half 😀)
```