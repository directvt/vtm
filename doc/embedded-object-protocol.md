### Embedded Object Protocol

The **Embedded Object Protocol (EOP)** allows vector, bitmap, and extensible markup objects to be embedded directly into the terminal's scrollback buffer.

#### Rendering & Alpha Blending

- **Rectangular Area**: The object is hosted within a grid of cells defined by `ceil(width)` and `ceil(height)`.
- **Sub-cell Precision**: The raster is scaled using floating-point `width` and `height` and positioned with `dx` and `dy` offsets. This allows for sub-pixel alignment and smooth movement across the cell grid.
- **Persistence**: Metadata is stored per-cell to survive scrollback and ensure that wrapped cell-runs remain logically linked for a strict rectangular reflow.
- **Cursor Position**: Anchored at the top-left; moves to the cell immediately following the rectangle's bottom-right corner after output.
- **Non-destructive & Color State**: 
  - If the **`gc`** attribute is not empty, the provided grapheme cluster is written to the cells with current SGR attributes, replacing existing text. 
  - If **`gc`** is empty, the output is non-destructive to existing text and SGR attributes.
- **Scroll Behavior (Normal Buffer)**: Outputting an object does not trigger **BCE (Background Color Erase)**. If the object does not fit at the bottom, it triggers a standard scroll-up.
- **Viewport Clipping (Alt Buffer)**: The object's rectangle is strictly clipped by the right and bottom edges of the terminal viewport; no scrolling occurs.
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
**id**        | `<id>[/sub-id]`                        | empty string (`""`)      | Object reference ID.
**gc**        | `string`                               | empty string (`""`)      | Grapheme cluster to write to the cells (replaces existing text and SGR attributes).
**ontop**     | `0`\|`1`                               | `0`                      | Layering: 0 = background (under text), 1 = foreground (over text).
**width**     | `float (0..65535]`                     | Terminal viewport width  | Raster scale width (cells). Grid area = `ceil(width)`.
**height**    | `float (0..65535]`                     | Terminal viewport height | Height of the rectangle in cells. Grid area = `ceil(height)`.
**dx**        | `float`                                | `0.0`                    | Horizontal offset of the raster within the grid (cells).
**dy**        | `float`                                | `0.0`                    | Vertical offset of the raster within the grid (cells).
**row**       | `0`..`ceil(height)`                    | `0`                      | Vertical slice index (0 = full height, 1..n = specific cell).
**column**    | `0`..`ceil(width)`                     | `0`                      | Horizontal slice index (0 = full width, 1..n = specific cell).
**align**     | \[`left`\|`center`\|`right`\]\[`-`\]\[`top`\|`middle`\|`bottom`\] | `center-middle` | 2D alignment within the rectangle.
**scale**     | `inside`\|`outside`\|`stretch`\|`none` | `inside`                 | Fit logic (none = exact pixels, cropped if larger).
**transform** | `0`..`7`                               | `0`                      | 3-bit compact transformation state (flip+rotate).
**flip**      | `none`\|`v`\|`h`\|`vh`\|`hv`           | `none`                   | Applied in order of appearance in the string.
**rotate**    | `0`\|`90`\|`180`\|`270`                | `0`                      | CCW rotation applied in order of appearance.

> Notes:
> - If `id` is omitted , the empty string `id=""` is used for registration and output.
> - Attribute values `width` and `height` are clamped to the `(0..65535]` range and further limited by the terminal's maximum window size settings.

#### Lifecycle Logic

Input State             | Action
------------------------|-------
**id** + **doc**        | **Register & Output**: Store/update document in cache and output to current position.
**id** + **empty-doc**  | **Unregister**: Remove the object referenced by `id` from cache (e.g., `<svg></svg>`).
**id** + **no doc**     | **Output**: If `id` is registered, output the cached object. If `id` is unknown/unregistered, clear object metadata in the specified rectangular area.

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

The protocol can be extended to include non-visual data segments such as `<audio>`, allowing for synchronized multimedia playback managed by the Frontend.