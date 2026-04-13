### AnyPlex Protocol (APP)

The **AnyPlex Protocol** allows vector, bitmap, and extensible markup objects to be embedded directly into the terminal's scrollback buffer.

#### Backend & Frontend Separation

The protocol operates on a decoupled architecture to ensure high performance and display adaptability.

Scope                           | Role
--------------------------------|-----
**The Backend (BE)**            | Acts as the "Source of Truth". It manages the terminal state, handles the scrollback buffer, and maintains the global object cache. It tracks metadata per cell but remains agnostic of physical pixels.
**The Graphical Frontend (FE)** | Acts as the "Painter". It performs the actual rasterization of documents (e.g., SVG to pixels) based on local cell metrics. It handles alpha-blending, Z-order layering, and pixel-wise offsets.

#### Rendering & Interaction

- **Normalized Source Viewport**: The source document is first projected onto a virtual canvas of size `1.0` by `1.0`. A rectangular fragment (crop) is then extracted from this canvas using normalized coordinates `u`, `v` (top-left) and `uw`, `vh` (size), where `1.0` equals the full canvas dimension. Negative values for `uw` or `vh` cause the extracted fragment to be flipped along the respective axis.
- **Target Rectangular Area**: The resulting fragment is hosted within a grid of cells starting at `x, y`. The range of affected cell indices is defined as `[floor(x) .. ceil(x + w) - 1]` horizontally and `[floor(y) .. ceil(y + h) - 1]` vertically, where `w > 0` and `h > 0`.
- **Pixel-wise Precision**: The extracted fragment is scaled and aligned within the bounding box calculated **per-frontend** based on its current cell metrics: `pixel_pos = round(x_or_y * cell_size)` and `pixel_dim = round(w_or_h * cell_size)`.
- **Persistence**: Metadata is stored per-cell to survive scrollback and ensure logical linking for rectangular reflow, using only an implementation-defined minimum of data (e.g., a lightweight object reference) to minimize memory overhead.
- **Cursor Position**: Anchored at the top-left; moves to the cell immediately following the bottom-right corner of the target area after output.
- **Destructivity**:
  - If the **`gc`** attribute is not empty, the provided grapheme cluster is written to **every cell** in the target area, replacing existing text and SGR attributes.
  - If **`gc`** is empty, the output is non-destructive; existing text and SGR attributes remain visually intact.
  - Subsequent text written over the area does not destroy the underlying object. Metadata remains in the cell until explicitly replaced.
- **Selection & Highlighting**: When a cell is selected (e.g., mouse selection or **SGR 7**), the frontend **applies a 0.5 opacity mask** to the object's pixels within that specific cell to ensure the selection remains visible.
- **Searchability**: Any text contained within the document (e.g., `<text>` in SVG) is rendered as part of the graphic and is not indexed for terminal text search.
- **Layering & Transparency**: Supports per-pixel alpha. The `ontop` attribute determines Z-order: 
  - `0`: `[Cell BG] -> [Object] -> [Text]`
  - `1`: `[Cell BG] -> [Text] -> [Object]`
  - The cell's background color always remains at the bottom. The terminal cursor is always drawn on top. Alpha blending should be performed in **linear color space**.
- **Foreground Color & Security**: The underlying cell **SGR foreground color** maps to `currentColor` (for SVG). All external references (e.g., `http://...`) are ignored for security purposes.

#### Scroll & Reflow Behavior

- **Normal Buffer**: If the cell-run containing the object does not fit within the viewport width, it triggers a standard wrap with a scroll-up if necessary (without triggering **BCE - Background Color Erase**). This may break the rectangular visual of the raster until the viewport is widened or a non-wrap mode is enabled.
- **Alternate Buffer**: The object's rectangle is strictly clipped (cropped) by the right and bottom edges of the viewport; no scrolling occurs.

#### Sequence Format

```
ESC ] app ; [<attributes>] [<document>] [<attributes>] ST
```

Field             | Description
------------------|------------
**OSC command**   | Mandatory. `app`.
**attributes**    | Optional. Space-separated `key=value` pairs. If an attribute is specified both before and after the document, the **last occurrence** takes precedence. Values can be quoted (`"` or `'`) or unquoted. All keys and values are **case-sensitive**.
**document**      | Optional. UTF-8 data starting with `<` (the first character of the opening tag, e.g. `<svg>`) and ending with `>` (the last character of the closing tag, e.g. `</svg>`). The specified document is considered to be an `empty-doc` if it has the form `<tag></tag>`, where `tag` is any string.

#### Attributes

Attribute     | Value/Range                            | Default                  | Description
--------------|----------------------------------------|--------------------------|------------
**id**        | `<id>[/sub-id]`                        | empty string (`""`)      | Object reference ID.
**gc**        | `string`                               | ASCII Space (0x20) `" "` | Grapheme cluster to write to cells (will be scaled to a 1x1 cell size).
**ontop**     | `0`\|`1`                               | `0`                      | 0 = under text, 1 = over text.
**u, v**      | `float`                                | `0.0`                    | Top-left of the source crop (0.0 to 1.0 relative to object size).
**uw, vh**    | `float`                                | `1.0`                    | Size of the source crop (0.0 to 1.0 relative to object size). Negative values flip the raster along the corresponding axis.
**x, y**      | `float`                                | `0.0`                    | Target position on the terminal grid (cells).
**w, h**      | `float (0.0-65535.0]`                  | Terminal viewport        | Target size on the terminal grid (cells).
**r, c**      | `index 0 .. ceil(h/w)`                 | `0`                      | Vertical/Horizontal (row, column) 1-based slicing index for partial rendering of target cells (0 = full height/width, 1..n = specific cell/slice).
**align**     | \[`left`\|`center`\|`right`\]\[`-`\]\[`top`\|`middle`\|`bottom`\] | `center-middle` | 2D alignment: How the crop fits into the target width/height.
**fit**       | `inside`\|`outside`\|`stretch`\|`none` | `inside`                 | Fit logic: How the crop fits into the target width/height (none = exact pixels, cropped if larger).
**transform** | `0`..`7`                               | `0`                      | 3-bit compact transformation state `[FlipY][FlipX][SwapXY]`.
**flip**      | `none`\|`v`\|`h`\|`vh`\|`hv`           | `none`                   | Applied in order of appearance in the string.
**rotate**    | `0`\|`90`\|`180`\|`270`                | `0`                      | CCW rotation applied in order of appearance.

> Notes:
> - If `id` is omitted , the empty string `id=""` is used for registration and output.
> - Attribute values `w` and `h` are clamped to the `(0.0-65535.0]` range and further limited by the terminal's maximum window size settings.
> - Attribute values that depend on the cell size are multiplied by the cell size and **rounded** to get the exact pixel values on the FE side.
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
