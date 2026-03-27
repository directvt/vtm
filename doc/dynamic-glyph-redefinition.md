# OSC glyph: Dynamic Glyph Redefinition

The `OSC glyph` sequence allows an application to redefine the visual representation of a specific **grapheme cluster** by injecting SVG data directly into the terminal's glyph atlas. This mechanism decouples the text stream from static font files, enabling custom icons, complex widgets, and high-fidelity script rendering.

## Syntax

`OSC` `glyph` `;` `<mappings>` `;` `<metrics>` `;` `<svg_payload>` `ST`

### Parameters

| Parameter | Description |
| :--- | :--- |
| **`mappings`** | A comma-separated list of `cluster=id` assignments. The `cluster` is the UTF-8 sequence (including `STX` and `U+Dxxxx` boundaries) to be intercepted. The `id` refers to the `id` attribute within the SVG document. If no `=` is present, the entire SVG is mapped to the cluster. |
| **`metrics`** | A reference character (e.g., `M`) used to derive the bounding box and alignment. If **empty**, the glyph will occupy the full character matrix area defined by the cluster (VT2D dimensions). |
| **`svg_payload`**| Raw XML/SVG data. The parser consumes all bytes after the third semicolon until the **ST** is encountered. |

### Control Characters
- **`OSC`**: `ESC ]` (hex `1B 5D`)
- **`ST`**: `ESC \` (hex `1B 5C`) or `BEL` (hex `07`)
- **Semicolon (`;`)**: Delimiter for the first three fields. Semicolons inside the `<svg_payload>` do not require escaping. Semicolons inside a `<cluster>` must be escaped as `\;`.

---

## Interaction with VT2D (Character Geometry)

This command integrates natively with the **VT2D manual clustering** protocol:

1.  **Manual Boundaries**: When a cluster is defined using `STX ... U+Dxxxx`, the entire sequence acts as the unique lookup key in the glyph atlas.
2.  **Geometry Inheritance**: The SVG is automatically scaled to fit the matrix dimensions specified by the `U+Dxxxx` codepoint (e.g., `U+D009F` provides a 3-cell wide canvas).
3.  **Fallback**: If the terminal cannot render the SVG, it falls back to the raw text characters contained within the cluster.

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
Define a 3-cell wide area using VT2D manual clustering and fill it with an SVG sparkline, stretching to the full available area (using empty metrics).

```bash
\x1b]glyph;\x02chart\uD009F;;<svg viewBox="...">...</svg>\x1b\\
```

## Implementation Notes

- Persistence: Redefining a glyph affects all current and future occurrences of the cluster within the session.
- State Management:
  - To clear a specific mapping, send the sequence with a valid cluster but an empty <svg_payload>.
  - To reset all custom glyphs, send the command with both empty <mappings> and <svg_payload>.
- Parsing Logic: Once the third semicolon is reached, the parser must switch to a "raw" mode, capturing all subsequent bytes until the ST sequence (\x1b\\ or \x07) is detected.
- Security: Implementations should impose a reasonable byte limit on the <svg_payload> to prevent memory exhaustion from malicious or malformed SVG data.
- Performance: Terminal emulators may cache the rendered SVG as a bitmap in the glyph atlas for high-speed compositing.
