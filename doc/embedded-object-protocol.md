### Embedded Object Protocol

The **Embedded Object Protocol (EOP)** allows vector, bitmap, and extensible markup objects to be embedded directly into the terminal's scrollback buffer.

#### Rendering & Alpha Blending

- **Rectangular Area**: The object is hosted within a defined rectangular grid of cells.
- **Persistence**: Metadata is stored per-cell to survive scrollback and ensure that wrapped cell-runs remain logically linked for a strict rectangular reflow.
- **Cursor Position**: Anchored at the top-left; moves to the cell immediately following the rectangle's bottom-right corner.
- **Non-destructive & Color State**: The object's rectangular area is filled with the **current SGR background color** without destroying existing text.
- **Scroll Behavior**: Outputting an object does not trigger **BCE (Background Color Erase)**; the background color is applied strictly to the object's cells.
- **Layering**: The `ontop` attribute switches the layering, placing the object on top of the text instead of behind it.
- **Per-pixel Transparency**: The rendered object supports full alpha-channel transparency.
- **Foreground Color**: The underlying cell **SGR foreground color** maps to `currentColor` (for SVG).
- **Re-rasterization**: The Graphical Frontend (FÉ) re-renders the object upon cell size changes to maintain pixel-perfection.
- **Gamma-Correct Blending**: Alpha blending is performed in **linear color space** to ensure visual fidelity.

#### Sequence Format

```
ESC ] object ; [<attributes>] [<document>] ST
```

Field             | Description
------------------|------------
**OSC command**   | Mandatory. `object`.
**attributes**    | Optional. Space-separated `key=value` pairs. Values can be quoted (`"` or `'`) or unquoted. All keys and values are **case-sensitive**.
**document**      | Optional. UTF-8 data starting with a format tag (e.g., `<svg`) and ending with a closing tag (e.g., `</svg>`).

#### Attributes

Attribute     | Values                                 | Default                  | Description
--------------|----------------------------------------|--------------------------|------------
**id**        | `<id>[/sub-id]`                        | empty string (`""`)      | Object reference ID. If omitted, the ID from the root tag is used.
**ontop**     | `0`|`1`                                | `0`                      | Layering: 0 = background (under text), 1 = foreground (over text).
**width**     | `1`..`2047`                            | Terminal viewport width  | Width of the rectangle in cells.
**height**    | `1`..`1023`                            | Terminal viewport height | Height of the rectangle in cells.
**row**       | `0`..`<height>`                        | `0`                      | Vertical slice index (0 = full height, 1..n = specific cell).
**column**    | `0`..`<width>`                         | `0`                      | Horizontal slice index (0 = full width, 1..n = specific cell).
**align**     | \[`left`\|`center`\|`right`\]\[`-`\]\[`top`\|`middle`\|`bottom`\] | `center-middle` | 2D alignment within the rectangle.
**scale**     | `inside`\|`outside`\|`stretch`\|`none` | `inside`                 | Fit logic (none = exact pixels, cropped if larger).
**transform** | `0`..`7`                               | `0`                      | 3-bit compact transformation state (flip+rotate).
**flip**      | `none`\|`v`\|`h`\|`vh`|`hv`            | `none`                   | Applied in order of appearance in the string.
**rotate**    | `0`\|`90`\|`180`\|`270`                | `0`                      | CCW rotation applied in order of appearance.

#### Lifecycle Logic

Input State             | Action
------------------------|-------
**id** + **doc**        | **Register & Display**: Store/update document in cache and output to current position.
**id** + **empty-doc**  | **Unregister**: Remove the object referenced by `id` from cache (e.g., `<svg></svg>`).
**id** + **no doc**     | **Display**: Output existing cached object using provided or default attributes.
**no id** + **doc**     | **Anonymous Display**: Use internal root tag `id="..."` for the session.

#### Parsing Rules (Backend)

1. Scan the OSC string for `key=value` pairs.
2. Locate the document boundaries by finding the first `<tag` and the last `</tag>`.
3. Extract the document body and resume parsing attributes from the remaining segments.
4. The transformation pipeline (`transform`, `flip`, `rotate`) is execution-order dependent.
5. Bitwise Transformation Logic (3-bit state):
   ```cpp
   Rotate:          state = (state & 0b100) | ((state + rotationCCW90_steps) & 0b011)
   Horizontal Flip: state = (state ^ 0b100) | ((state + (state & 1 ? 2 : 0)) & 0b011)
   Vertical Flip:   state = (state ^ 0b100) | ((state + (state & 1 ? 0 : 2)) & 0b011)
   ```

#### Extensibility

The protocol is engine-agnostic. Focused on **SVG**, but designed to support other formats (e.g., `<html>`, `<object>`) via root tag identification.