# Dynamic Glyph Redefinition

The `OSC glyph` sequence allows an application to override the visual representation of a particular **grapheme cluster** with an AnyPlex image. This mechanism enables custom icons, complex widgets, and high-fidelity script rendering within the standard text stream.

## Syntax

```
OSC glyph [ ; <glyph-definition> ] ... [ ; <glyph-definition> ] ST

# Where <glyph-definition> is:
# [ ; <mappings> [ matrix=<image-cell-list> ] [ id=<anyplex-id>[/<sub-id>] ]]
```

Field                 | Description
----------------------|------------
**mappings**          | A space-separated list of `cluster="..."` assignments. Values can be quoted (`"` or `'`) or unquoted. The `cluster` value is the UTF-8 sequence (may include `STX` and `U+Dxxxx` boundaries).
**image-cell-list**   | A list of the image cells used for building resultant glyph. If **empty**, a whole image is used to represent the glyph.
**anyplex-id/sub-id** | A reference to the registered AnyPlex object.

> Notes:
> 
> - When using `STX ... U+Dxxxx`, the entire sequence acts as the unique lookup key in the atlas.
> - The glyph image scales to the matrix dimensions specified by the `U+Dxxxx` codepoint.
> - If rendering fails, the terminal displays the raw text characters within the cluster.

## Examples

### 1. Simple Emoji Replacement

Override the standard 🚀 emoji using the proportions of an '😀' character.
```bash
# Define AnyPlex image:
\x1b]app;id="chr_Rocket" <svg>...rocket image...</svg>\x1b\\

# Map cluster to image:
\x1b]glyph;cluster=🚀 matrix=w2h1 id="chr_Rocket"\x1b\\
```

### 2. Multi-Glyph Atlas (OT-SVG Style)

Map different IDs from a single SVG document to specific manual clusters. This is highly efficient for loading entire icon sets or complex UI kits in a single atomic operation.
```bash
# Mapping 'cpu' to #icon1 and 'ram' to #icon2
# Define AnyPlex image-pack:
\x1b]app;id=icons <svg><g id='icon1'>...</g><g id='icon2'>...</g></svg> \x1b\\

# Map clusters to image sub-elements:
\x1b]glyph;cluster="\x02cpu\uD009C" id=icons/icon1 \x1b\\
\x1b]glyph;cluster="\x02ram\uD009C" id=icons/icon2 \x1b\\
```

### 3. Advanced Styling Example (Theming & Gradients)

This example uses `currentColor` to match the terminal's text color and an overlay gradient for a "shimmer" or "fade" effect.

```svg
<svg viewBox="0 0 1000 1000" xmlns="http://www.w3.org">
  <defs>
    <linearGradient id="fade" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#00FF00" stop-opacity="1" />
      <stop offset="100%" stop-color="#00FF00" stop-opacity="0" />
    </linearGradient>
  </defs>
  <!-- This path will change color if the TUI app sends \x1b[31m (Red) -->
  <path d="..." fill="currentColor" />
  <!-- This rectangle provides a static color overlay -->
  <rect x="0" y="400" width="1000" height="200" fill="url(#fade)" />
</svg>
```