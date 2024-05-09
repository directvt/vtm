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

## VT Sequence: CFA - Character Fragmentation Attribute.

The following sequences allow to set the character 4x4-fragmentation attribute as a cell rendition state:
- Human readable sequence format:
  ```
    ESC[ 110 ; <n1> ; <n2> ; <n3> ; <n4> m
  ```
  - `n1`, `n2`, `n3`, `n4` are from 0 to 4.
  - `n1 = Dx`, `n2 = Nx`, `n3 = Dy`, `n4 = Ny`.
  - Missing values are treated as 1.
  - 0 treated as reset the fragmentation mode and disable it.
- Sequence with a "cooked" parameter:
  ```
    ESC[ 111 ; <P> m
  ```
  - `P = (Nx-1) + (Dx-1) * 4 + (Ny-1) * 16 + (Dy-1) * 64` from 0 to 255.
  - Missing value is treated as 0.
  - 0 treated as reset the fragmentation mode and disable it.

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