# OSC glyph: Dynamic Glyph Redefinition

The `OSC glyph` sequence allows an application to redefine the visual representation of a specific **grapheme cluster** by injecting SVG data directly into the terminal's glyph atlas. This mechanism enables custom icons, complex widgets, and high-fidelity script rendering within the standard text stream.

## Syntax

`OSC` `glyph` `;` `<mappings>` `;` `<metrics>` `;` `<svg_payload>` `ST`

### Parameters

| Parameter | Description |
| :--- | :--- |
| **`mappings`** | A comma-separated list of `cluster=id` assignments. The `cluster` is the UTF-8 sequence (including `STX` and `U+Dxxxx` boundaries). The `id` refers to the `id` attribute within the SVG. If no `=` is present, the entire SVG is mapped. |
| **`metrics`** | A reference character (e.g., `M`) used for bounding box/alignment. If **empty**, the glyph fills the character matrix area defined by the cluster (VT2D dimensions). |
| **`svg_payload`**| Raw XML/SVG data. The parser consumes all bytes after the third semicolon until the **ST** is encountered. |

---

## Visual Inheritance & Layering

The terminal follows a layered rendering model, treating custom glyphs as dynamic font characters:

1.  **Background Layer**: The cell matrix is pre-filled with the current **Background Color** (SGR).
2.  **Glyph Layer**: The SVG is rendered on top.
    - **Transparency**: Uncovered areas reveal the pre-filled background.
    - **Dynamic Color (`currentColor`)**: The `currentColor` keyword can be used in any attribute (fill, stroke, stop-color, etc.) to inherit the current **Foreground Color** (SGR).
    - **Raster Integration**: The SVG may contain `<image>` tags with **Base64-encoded** raster data (Data URIs). External URLs are prohibited.

---

## Interaction with VT2D (Character Geometry)

1.  **Manual Boundaries**: When using `STX ... U+Dxxxx`, the entire sequence acts as the unique lookup key in the atlas.
2.  **Geometry Inheritance**: The SVG scales to the matrix dimensions specified by the `U+Dxxxx` codepoint.
3.  **Fallback**: If rendering fails, the terminal displays the raw text characters within the cluster.

---

## Examples

### 1. Simple Emoji Replacement

Override the standard 🚀 emoji using the proportions of an 'M' character.
```bash
\x1b]glyph;🚀;M;<svg>...</svg>\x1b\\
```

### 2. Multi-Glyph Atlas (OT-SVG Style)

Map different IDs from a single SVG document to specific manual clusters. This is highly efficient for loading entire icon sets or complex UI kits in a single atomic operation.
```bash
# Mapping 'cpu' to #icon1 and 'ram' to #icon2
\x1b]glyph;\x02cpu\uD009C=icon1,\x02ram\uD009C=icon2;;<svg><g id='icon1'>...</g><g id='icon2'>...</g></svg>\x1b\\
```

### 3. Full-Cell Widget

Define a 3-cell wide area using VT2D manual clustering and fill it with an SVG sparkline. By leaving the `<metrics>` field empty, the SVG is instructed to stretch and fill the entire available character matrix area.

```bash
\x1b]glyph;\x02chart\uD009F;;<svg viewBox="...">...</svg>\x1b\\
```

### 4. Advanced Styling Example (Theming & Gradients)

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

---

## Implementation Notes

- **Caching**: The terminal caches the **SVG DOM** for efficiency. If the cell size (font size) changes, the terminal re-rasterizes the existing DOM without re-parsing the XML.
- **Persistence**: Mappings affect all current and future occurrences of the cluster in the session.
- **Clearing**: To remove a mapping, send an empty `<svg_payload>`. To reset the atlas, send empty `<mappings>`.
- **Resource Management**: Implementations should support a payload limit of **10MB to 40MB** to accommodate large icon sets or embedded rasters.
- **Parser Logic**: All data after the third semicolon is treated as a raw byte stream until the **ST** sequence (`ESC \` or `BEL`).
