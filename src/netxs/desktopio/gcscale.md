# Unicode Character Size Modifiers

In the terminal world, character width detection is not well defined and is context dependent, which is the cause of the following issues:

- No way to specify a custom width for displayed characters.
- No way to simultaneously display the same characters in both narrow and wide variants.
- No way to use triple and quadruple characters along with narrow and wide.
- Different assumptions about character widths in applications and terminal emulators.
- No way to display wide characters partially.
- No way to display characters higher two cells.
- No way to display subcell sized characters.

By defining that the graphical representation of a character is a cellular matrix (1x1 matrix consists of one fragment), the concept of "wide/narrow" can be completely avoided.

Terminals can annotate each scrollback cell with character matrix metadata and use it to display either the entire character image or a specific fragment within the cell.

Users can explicitly specify the size of the character matrix or select any fragment of it using codepoints from some range, e.g. 0xD0000-0xD02A2.

For character matrices larger than 8x4, pixel graphics should be used.

Encoding matrix fragments up to 8x4 in size requires four integer values, which can be packed into Unicode codepoint space by enumerating "wh_xy" values:

  - w: Character matrix width
  - h: Character matrix height
  - x: Horizontal fragment selector inside the matrix
  - y: Vertical fragment selector inside the matrix

![image](https://github.com/directvt/vtm/assets/11535558/88bf5648-533e-4786-87de-b3dc4103273c)

Placement in the text:

```
<basechar><SM-wh_xy>
```

Examples:

  - 😊+`<SM-11_00>` produce 1x1 character.
  - 😊+`<SM-21_00>` produce 2x1 character.
  - 👨‍👩‍👧‍👦+`<SM-31_00>` produce 3x1 character.
  - 😊+`<SM-42_00>` produce 4x2 character.
  - A+`<SM-44_00>` produce 4x4 character.
  - 😊+`<SM-21_11>` produce the left half of the 2x1 character.
  - 😊+`<SM-21_21>` produce the right half of the 2x1 character.
