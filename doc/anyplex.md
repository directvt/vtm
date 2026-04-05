### AnyPlex Protocol

The **AnyPlex Protocol (APP)** allows vector, bitmap, and extensible markup objects to be embedded directly into the terminal's scrollback buffer.

#### Backend & Frontend Separation

The protocol operates on a decoupled architecture to ensure high performance and display adaptability.

Scope                           | Role
--------------------------------|-----
**The Backend (BE)**            | Acts as the "Source of Truth". It manages the terminal state, handles the scrollback buffer, and maintains the global object cache. It tracks metadata per cell but remains agnostic of physical pixels.
**The Graphical Frontend (FÉ)** | Acts as the "Painter". It performs the actual rasterization of documents (e.g., SVG to pixels) based on local cell metrics, font size, and DPI. It handles alpha-blending, Z-order layering, and pixel-wise offsets.

#### Rendering

- **Rectangular Area**: The object is hosted within a grid of cells defined by `ceil(width)` and `ceil(height)`.
- **Pixel-wise Precision**: The raster is scaled using floating-point `width` and `height` and positioned with `dx` and `dy` offsets. This allows for exact pixel-wise alignment and movement within the grid. Offsets are calculated **per-frontend** based on its current cell metrics.
- **Persistence**: Metadata is stored per-cell to survive scrollback and ensure that wrapped cell-runs remain logically linked for a strict rectangular reflow.
- **Cursor Position**: Anchored at the top-left; moves to the cell immediately following the rectangle's bottom-right corner after output.
- **Destructivity**: 
  - If the **`gc`** attribute is not empty, the provided grapheme cluster is written to the cells with current SGR attributes, replacing existing text and SGR attributes.
  - If **`gc`** is empty, the output is non-destructive to existing text and SGR attributes.
  - Any text subsequently written over the object's area does not modify or destroy the underlying object. The object remains intact in the cell metadata until explicitly cleared or replaced.
  - Dynamic SGR attributes (such as `blink`) applied to the text within the object's area do not affect the rendered object. The object remains static and persistent regardless of the text's visual state.
- **Scroll Behavior (Normal Buffer)**: Outputting an object does not trigger **BCE (Background Color Erase)**. If the object does not fit at the bottom, it triggers a standard scroll-up.
- **Viewport Clipping (Alt Buffer)**: The object's rectangle is strictly clipped by the right and bottom edges of the terminal viewport; no scrolling occurs.
- **Layering**: The `ontop` attribute switches the layering, placing the object on top of the text instead of behind it.
- **Alpha Transparency**: The rendered object supports per-pixel alpha transparency (at least an 8-bit alpha channel).
- **Foreground Color**: The underlying cell **SGR foreground color** maps to `currentColor` (for SVG).
- **Re-rasterization**: The Graphical Frontend (FÉ) re-renders the object upon cell size changes to maintain pixel-perfection.
- **Gamma-Correct Blending**: Alpha blending is performed in **linear color space** to ensure visual fidelity.

#### Sequence Format

```
ESC ] app ; [<attributes>] [<document>] ST
```

Field             | Description
------------------|------------
**OSC command**   | Mandatory. `app`.
**attributes**    | Optional. Space-separated `key=value` pairs. Values can be quoted (`"` or `'`) or unquoted. All keys and values are **case-sensitive**.
**document**      | Optional. UTF-8 data starting with `<` (the first character of the openning tag, e.g. `<svg>`) and ending with `>` (the last character of the closing tag, e.g. `</svg>`). The specified document is considered to be an `empty-doc` if it has the form `<tag></tag>`, where `tag` is any string.

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
> - Attribute values `width`, `height`, `dx` and `dy` are multiplied by the cell size and **rounded** to get the exact pixel values on the FÉ side.
> - The first part of the object reference ID (`id`) references the raw object document in the Backend cache.
> - The second part of the object reference ID (`sub-id`) addresses the specific named element (e.g., an `id="..."` within an SVG document). The Frontend renders only this fragment if it is specified.

#### Lifecycle Logic

Input State             | Action
------------------------|-------
**id** + **doc**        | **Register & Output**: Store/update the object document in cache and output it.
**id** + **empty-doc**  | **Unregister**: Remove the object document referenced by `id` from cache.
**id** + **no-doc**     | **Output**: Output the cached document. If the `id` is unknown OR the `sub-id` is not found within the document, the Frontend **clears the object metadata** in the target area (rendering nothing).

#### Parsing Rules (Backend)

1. Scan the OSC string for `key=value` pairs.
2. Locate the document boundaries by finding the first `<` (of `<tag1>`) and the last `>` (of `</tag2>`).
3. Extract the document body and resume parsing attributes from the remaining OSC string.
4. The transformation pipeline (`transform`, `flip`, `rotate`) is execution-order dependent.
5. Bitwise Transformation Logic (3-bit state):
   ```cpp
   Rotate:          state = (state & 0b100) | ((state + rotationCCW90_steps) & 0b011)
   Horizontal Flip: state = (state ^ 0b100) | ((state + (state & 1 ? 2 : 0)) & 0b011)
   Vertical Flip:   state = (state ^ 0b100) | ((state + (state & 1 ? 0 : 2)) & 0b011)
   ```

#### Extensibility

The protocol is engine-agnostic. Focused on **SVG**, but designed to support other formats (e.g., `<html>`, `<object>`) via root tag identification.

The protocol can be extended to include non-visual data segments such as `<audio>` or `<wav>`, allowing for synchronized multimedia playback managed by the Frontend.
