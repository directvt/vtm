### AnyPlex Protocol (APP)

The **AnyPlex Protocol** allows vector, bitmap, and extensible markup objects to be embedded directly into the terminal's scrollback buffer.

#### Backend & Frontend Separation

The protocol operates on a decoupled architecture to ensure high performance and display adaptability.

Scope                           | Role
--------------------------------|-----
**The Backend (BE)**            | Acts as the "Source of Truth". It manages the terminal state, handles the scrollback buffer, and maintains the global object cache. It tracks metadata per cell but remains agnostic of physical pixels.
**The Graphical Frontend (FE)** | Acts as the "Painter". It performs the actual rasterization of documents (e.g., SVG to pixels) based on local cell metrics. It handles alpha-blending, Z-order layering, and pixel-wise offsets.

#### Rendering & Interaction

- **Rectangular Area**: The object is hosted within a grid of cells defined by `ceil(width)` and `ceil(height)`.
- **Pixel-wise Precision**: The raster is scaled using floating-point `width` and `height` and positioned with `dx` and `dy` offsets. Offsets are calculated **per-frontend** based on its current cell metrics.
- **Asynchronous Rasterization**: It is recommended to perform rasterization in a parallel thread. Until the raster is ready, the frontend should display the cells without the graphic.
- **Persistence**: Metadata is stored per-cell to survive scrollback and ensure that wrapped cell-runs remain logically linked for a strict rectangular reflow.
- **Cursor Position**: Anchored at the top-left; moves to the cell immediately following the rectangle's bottom-right corner after output.
- **Destructivity**:
  - If the **`gc`** attribute is not empty, the provided grapheme cluster is written to **every cell** in the area (`ceil(width)` by `ceil(height)`), replacing existing text and SGR attributes.
  - If **`gc`** is empty, the output is non-destructive; existing text and SGR attributes remain visually intact under the transparent object.
  - Any text subsequently written over the object's area does not destroy the underlying object. The object metadata remains intact in the cell until explicitly cleared or replaced.
- **Selection & SGR 7**: When selecting text/graphics with the mouse, or when the **SGR 7** attribute is present in a cell, the frontend **halves the alpha channel** of the object's pixels within that cell.
- **Searchability**: Any text contained within the document (e.g., `<text>` in SVG) is treated as part of the graphic and is not required to be indexed by the terminal's text search.
- **Layering & Transparency**: Supports per-pixel alpha transparency. The `ontop` attribute determines Z-order relative to text (0 = background `[Cell BG] -> [Object] -> [Text]`, 1 = foreground `[Cell BG] -> [Text] -> [Object]`; the cell's background color always remains in the background). The terminal cursor is always drawn on top of everything. Alpha blending is performed in **linear color space**.
- **Foreground Color**: The underlying cell **SGR foreground color** maps to `currentColor` (for SVG). All other external references (e.g., `http://...`) are ignored for security.

#### Scroll & Reflow Behavior

- **Normal Buffer**: If the cell-run containing the object does not fit within the viewport width, it triggers a standard wrap with a scroll-up if necessary (without triggering **BCE - Background Color Erase**). This may break the rectangular visual of the raster until the viewport is widened or a non-wrap mode is enabled.
- **Alternate Buffer**: The object's rectangle is strictly clipped (cropped) by the right and bottom edges of the viewport; no scrolling occurs.

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
**gc**        | `string`                               | empty string (`""`)      | Grapheme cluster to write to cells (will be scaled to a 1x1 cell size).
**ontop**     | `0`\|`1`                               | `0`                      | 0 = under text, 1 = over text.
**width**     | `float (0..65535]`                     | Terminal viewport width  | Raster scale width (cells).
**height**    | `float (0..65535]`                     | Terminal viewport height | Raster scale height (cells).
**dx**        | `float`                                | `0.0`                    | Horizontal offset of the raster within the grid (cells).
**dy**        | `float`                                | `0.0`                    | Vertical offset of the raster within the grid (cells).
**column**    | `0`..`ceil(width)`                     | `0`                      | Horizontal 1-based slicing index for partial rendering (0 = full width, 1..n = specific cell/slice).
**row**       | `0`..`ceil(height)`                    | `0`                      | Vertical 1-based slicing index for partial rendering (0 = full height, 1..m = specific cell/slice).
**align**     | \[`left`\|`center`\|`right`\]\[`-`\]\[`top`\|`middle`\|`bottom`\] | `center-middle` | 2D alignment within the rectangle.
**scale**     | `inside`\|`outside`\|`stretch`\|`none` | `inside`                 | Fit logic (none = exact pixels, cropped if larger).
**transform** | `0`..`7`                               | `0`                      | 3-bit compact transformation state `[FlipY][FlipX][SwapXY]`.
**flip**      | `none`\|`v`\|`h`\|`vh`\|`hv`           | `none`                   | Applied in order of appearance in the string.
**rotate**    | `0`\|`90`\|`180`\|`270`                | `0`                      | CCW rotation applied in order of appearance.

> Notes:
> - If `id` is omitted , the empty string `id=""` is used for registration and output.
> - Attribute values `width` and `height` are clamped to the `(0..65535]` range and further limited by the terminal's maximum window size settings.
> - Attribute values `width`, `height`, `dx` and `dy` are multiplied by the cell size and **rounded** to get the exact pixel values on the FE side.
> - The first part of the object reference ID (`id`) references the raw object document in the Backend cache.
> - The second part of the object reference ID (`sub-id`) addresses the specific named element (e.g., an `id="..."` within an SVG document). The Frontend renders only this fragment if it is specified.

#### Lifecycle & Cache Management

Input State             | Action
------------------------|-------
**id** + **doc**        | Store/update the object document in cache and output.
**id** + **empty-doc**  | Unregister `id` and free the index.
**id** + **no-doc**     | Output cached object.
**id/sub-id**           | If a `sub-id` is specified, the FE renders that specific element in its original coordinates (as it would appear in the full document), inheriting all parent styles (CSS, `<g>` groups).
**Errors**              | If a document is invalid, the `id` is unknown, the `sub-id` is missing, or the cache is full, the FE **clears the object metadata** in the target area (rendering nothing) and logs the error.

> Note:
> When the BE deletes an `id`, or upon `reset`/session close, it frees the index and signals the FEs.
> - FEs then traverse their viewport cells; any image in the FE cache no longer referenced by any cell is purged.
> - If an FE encounters an unknown object index in a cell, it must request full metadata/document from the BE.

#### Parsing Rules (Backend)

1. Scan the OSC string for `key=value` pairs.
2. Identify document boundaries via first `<` and last `>`.
3. Apply transformation pipeline (`transform`, `flip`, `rotate`) in the order they appear.

#### Extensibility

The protocol is engine-agnostic. Focused on **SVG**, but designed to support other formats (e.g., `<html>`, `<object>`) via root tag identification.

The protocol can be extended to include non-visual data segments such as `<audio>` or `<wav>`, allowing for synchronized multimedia playback managed by the Frontend.
