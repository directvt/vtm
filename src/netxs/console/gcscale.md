```
[This comment will be updated on every clarification and additional information]
```
## Character Segmentation and Scaling (Fractaling)

### 1. Abstract

This paper provides a general description of the solution to the problem of presenting and processing multi-sized (up to 4x4 cells) characters in a cell-based grid, where each cell one-to-one represents the visilbe part (fraction) of the character.

The following applications are related to multisize characters and character segmentation:
- Terminal Emulators.
- Linux console (when the X Window System is not running).
- Windows Console.
- Monospaced text documents.
- ... your suggestions

The solution allows to manipulate and store individual fractions of the whole character in a single cell for displaying them, as well as displaying  multi-size characters in a cell-based grid and even allow their vertical splitting.
The solution also solves the problem of displaying wide characters in terminals by letting the terminal or the application running in it decide how wide the character will be, rather than relying on external data sources of these values that are subject to regular changes.

### 2. Solution

#### 2.1 Definitions

[Accordingly to the Unicode® Standard Annex #29, "UNICODE TEXT SEGMENTATION"](https://unicode.org/reports/tr29/#Grapheme_Cluster_Boundaries)
> It is important to recognize that what the user thinks of as a “character”—a basic unit of a writing system for a language—may not be just a single Unicode code point. Instead, that basic unit may be made up of multiple Unicode code points. To avoid ambiguity with the computer use of the term character, this is called a user-perceived character. For example, “G” + grave-accent is a user-perceived character: users think of it as a single character, yet is actually represented by two Unicode code points. These user-perceived characters are approximated by what is called a grapheme cluster, which can be determined programmatically.

This paper defines a character as user-perceived character (or grapheme cluster).

#### 2.2 Mathematical Presentation

To correctly display either a whole character of any size (up to 4x4 cells) or any selected character segment, only four numeric parameters `Ps` = `Dx`, `Nx`, `Dy`, `Ny` with range values of each from 1 to 4 are required.

##### Parameters
- `Dx` - count of parts along X-axis
- `Nx` - either width of the whole character or segment selector of the `Dx` available parts from left to right along the X-axis
- `Dy` - count of parts along Y-axis
- `Ny` - either width of the whole character or segment selector of the `Dy` available parts from top to bottom along the Y-axis

##### Interpretation

There are several cases possible (for each axis accordingly)
- `D = 0`
  - turn the scale mode off.
- `N <= D`
  - select part `N` of the character from `D` available parts and use it as a sinle-cell character (along the corresponding axis).
- `N > D AND D = ANY`
  - stretch the character to `N` cells.

#### 2.3 Storing In Memory

##### Screen Buffer / Monospaced Text File

Each multisize character with a size of `n x m` that is greater than `1x1` is stored in the screen buffer (or monospaced text file)`W x H` as a matrix of `n x m`

Example:

3x2 stretched character `"A"` is located at `x=3, y=2` in the screen buffer (of monospaced text file)

|     |  1  |  2  |  3  | ... | ... | ... |  W  |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
|  1  | ... | ... | ... | ... | ... | ... | ... |
|  2  | ... | ... |A+Ps1|A+Ps2|A+Ps3| ... | ... |
| ... | ... | ... |A+Ps4|A+Ps5|A+Ps6| ... | ... |
| ... | ... | ... | ... | ... | ... | ... | ... |
| ... | ... | ... | ... | ... | ... | ... | ... |
|  H  | ... | ... | ... | ... | ... | ... | ... |

```
A = "A"
Ps1 = { Dx=3, Nx=1, Dy=2, Ny=1 }
Ps2 = { Dx=3, Nx=2, Dy=2, Ny=1 }
Ps3 = { Dx=3, Nx=3, Dy=2, Ny=1 }
Ps4 = { Dx=3, Nx=1, Dy=2, Ny=2 }
Ps5 = { Dx=3, Nx=2, Dy=2, Ny=2 }
Ps6 = { Dx=3, Nx=3, Dy=2, Ny=2 }
```



`Ps` can be packed in one byte and overhead of screen buffer is 1 byte per cell:
```
byte = (Nx-1) + (Dx-1) * 4 + (Ny-1) * 16 + (Dy-1) * 64
```

Also there are only 256 variants for the Unicode modifier character value 0 - 255.

Characters with parameters `N > D` are not allowed to be stored in a cell-based grid. When such a character is to be printed to the grid, it must be segmented for each grid cell, and the parameters are recalculated for each filled cell.

#### 2.4 Naming

##### 2.4.1 VT-Sequence

Variants of the name for the VT-sequence
- Grapheme Cluster Scaling
- GCS
- GCSCALE
- GCSC
- ... your suggestions

##### 2.4.2 Unicode Standard

Name of the Unicode modifier letter
  - \<GCSCALE1>..\<GCSCALE256> (like VS1..VS256)
  - ... your suggestions

### 3. Usage

#### 3.1 Unicode Standard

Latest Unicode Standard defines three types of variation sequences:
- Standardized variation sequences.
- Emoji variation sequences.
- Ideographic variation sequences defined in the Ideographic Variation Database.

Only those three types of variation sequences are sanctioned for use by conformant implementations.

[Accorginly to the Standardized variation sequences FAQ](http://unicode.org/faq/vs.html)

> Q: How can I propose a standardized variation sequence?

> A: You can initiate the process of requesting a variation sequence by submitting an inquiry via the contact form. A thorough understanding of how Variation Selectors are used will make a proposal more likely to be accepted by the UTC. Read Section 23.4, Variation Selectors, UTR #25 and UAX #34, as well as the rest of this FAQ for background information. [AF]

[Accodingly to the Section 23.4, Variation Selectors, UTR #25](http://www.unicode.org/versions/Unicode12.1.0/ch23.pdf#G19053)

> A variant form is a different glyph for a character, encoded in Unicode through the mechanism of variation sequences: sequences in Unicode that consist of a base character followed by a variation selector character.

##### Variation Sequence

> In a variation sequence the variation selector affects the appearance of the base character. Such changes in appearance may, in turn, have a visual impact on subsequent characters, particularly combining characters applied to that base character.

> The standardization or support of a particular variation sequence does not limit the set of glyphs that can be used to represent the base character alone. 

##### Placement in the Text
```
<basechar><GCSCALE1..256>
```
> what if there's a pause in the input stream after the base character?

If such a modifier appears the first in the input stream the terminal should be triggered to text reflowing as in the case of window resize.

#### 3.2 VT-Sequence

[XTerm Control Sequences, Functions using CSI](https://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h3-Functions-using-CSI-_-ordered-by-the-final-character_s_)

Assing VT-sequence as a CSI/SGR command, because it define characters rendition state and sets the appearance of the following characters.
```
Human readable format
	ESC[ 110; <n1>;<n2>;<n3>;<n4> m

Sequence with "cooked" parameter
	ESC[ 111; <P> m
```
- `n1`, `n2`, `n3`, `n4` are from 0 to 4.
- `n1 = Dx`, `n2 = Nx`, `n3 = Dy`, `n4 = Ny`.
- `P = (Nx-1) + (Dx-1) * 4 + (Ny-1) * 16 + (Dy-1) * 64` from 0 to 255.
- SGR code `110`: missing numbers are treated as 1.
- SGR code `111`: missing number is treated as 0.
- 0 treated as reset the scaling mode (OFF).
- `ESC[m` (all attributes off) also resets the scaling mode.
- Instead of SGR codes `110`, `111` suggest `...` yours.

### 4. Expected Behavior

It doesn’t matter what size (cwidth) the character has, it allows put a wide character to a single cell if you want.

#### 4.1 Unicode Standard
```
...
```

#### 4.2 VT-Sequence
##### 4.2.1 Printing

Output examples (VT sequence <SCALE;;;>)
```
- cout “a” produce  1x1 in buffer:
  [1/1,1/1]
- cout “😊” produce 2x1 in buffer:
  [1/2,1/1][2/2,1/1]
- cout “👨‍👩‍👧‍👦” produce 3x1 in buffer: 
  [1/3,1/1][2/3,1/1][3/3,1/1]
- cout “<SCALE1;1;1;1>😀” produce 1x1
- cout “<SCALE3;1;3;1>😀” produce 3x3
- cout “<SCALE1;1;1;1>👨‍👩‍👧‍👦” produce 1x1
- cout “<SCALE1;1;1;1>😀X” produce 1x1, 1x1
- cout “<SCALE2;1;1;1>😀X<SCALE0;0;0;0>H” produce 2x1(😀), 2x1(X), 1x1(H)
- cout “<SCALE1;2;1;1>😀🌎XH😀😀” produce 
  [1/2,1/1](left half 😀), [2/2,1/1](right half 🌎), [1/2,1/1](left half X) , [2/2,1/1](right half H), [1/2,1/1](left half 😀) , [2/2,1/1](right half 😀)
```

It is also possible with this technique to print out mathematical expressions and multi-level formulas (*monospace* textual documents with formulas, CJK, wide emoji and so on - are the Unicode problems that outside terminal world).

###### Line Wrap
```
...
```

###### Side effects
```
...
```

##### 4.2.2 Capturing
```
...
```

### 5. Applications
#### 5.1 Cost of Initial Implementation
```
...
```

### 6. Existed Infrastructure Compatibility
```
...
```

### 7. Security Issues

#### 7.1 Unicode Security Considerations

[Unicode Technical Report #36](http://unicode.org/reports/tr36/)

This section describes some of the security considerations that programmers, system analysts, standards developers, and users should take into account.

For example, consider visual spoofing, where a similarity in visual appearance fools a user and causes him or her to take unsafe actions.

```
...
```
