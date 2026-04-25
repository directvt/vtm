### AnyPlex Protocol (APP)

The **AnyPlex Protocol** allows vector, bitmap, and extensible markup objects to be embedded directly into the terminal's scrollback buffer.

#### Backend & Frontend Separation

The protocol operates on a decoupled architecture to ensure high performance and display adaptability.

Scope                           | Role
--------------------------------|-----
**The Backend (BE)**            | Acts as the "Source of Truth". It manages the terminal state, handles the scrollback buffer, and maintains the global object cache. It tracks metadata per cell but remains agnostic of physical pixels.
**The Graphical Frontend (FE)** | Acts as the "Painter". It performs the actual rasterization of documents (e.g., SVG to pixels) based on local cell metrics. It handles alpha-blending, Z-order layering, and pixel-wise offsets.

#### Rendering & Interaction

- **Normalized Source Viewport**: The source document is first projected onto a virtual canvas of size `1.0` by `1.0`. A rectangular fragment (crop) is then extracted from this canvas using normalized coordinates `u`, `v` (top-left) and `uw`, `vh` (size). Negative values for `uw` or `vh` cause the extracted fragment to be flipped along the respective axis.
- **Target Rectangular Area**: The grid footprint is explicitly defined by the unique attributes **W** and **H**. The range of affected cells starts at the current cursor position and spans `[0 .. W-1]` horizontally and `[0 .. H-1]` vertically. If `W` or `H` is `0`, no cells are modified (registration only).
- **Pixel-wise Precision**:
  - **Content Positioning**: The image fragment is scaled according to `fit` and aligned within the **Clipping Box** (`w x h`) using the `a` attribute.
  - **Box Positioning**: This Clipping Box is then aligned within the **Grid Container** (`W x H`) using the same `a` attribute.
  - **Offsets**: Shared `x` and `y` values are applied as final pixel offsets from the resulting position.
  - **Final Cut**: The Grid Container acts as a strict physical mask; any pixels falling outside the `W x H` cell area are not rendered.
- **Persistence**: Metadata is stored per-cell to survive scrollback and ensure logical linking.
- **Cursor Position**: Anchored at the top-left; moves to the cell immediately following the bottom-right corner of the **Grid Container** (`W x H`) after output.
- **Destructivity**:
  - If the `gc` attribute is not empty, the provided grapheme cluster is written to every cell in the target area, replacing existing text and SGR attributes.
  - If `gc` is empty, the output is non-destructive; existing text and SGR attributes remain visually intact.
  - Subsequent text written over the area does not destroy the underlying object. Metadata remains in the cell until explicitly replaced.
- **Selection & Highlighting**: When a cell is selected by mouse or highlighted using **SGR 7**, the frontend **applies a 0.5 opacity mask** to the object's pixels within that specific cell to ensure the selection remains visible.
- **Searchability**: Any text contained within the document (e.g., `<text>` in SVG) is rendered as part of the graphic and is not indexed for terminal text search.
- **Layering & Composition**:
  - Supports per-pixel alpha blending in linear color space.
  - **Recursive Rendering**: If a layer (`l`) references an object that itself has layers, the entire tree is rendered. Circular references are silently ignored.
  - **Z-Order**: Layers are rendered in the order they are defined (first is bottom). The "main" document of the current `id` is rendered as the final layer on top.
  - **Ontop (`o`)**: Determines if the *entire composite* is drawn under (`0`) or over (`1`) the cell text:
    - `0`: `[Cell BG] -> [Object] -> [Text]`
    - `1`: `[Cell BG] -> [Text] -> [Object]`
  - The cell's background color always remains at the bottom. The terminal cursor is always drawn on top. Alpha blending should be performed in **linear color space**.
- **Foreground Color & Security**: The underlying cell **SGR foreground color** maps to `currentColor` (for SVG). All external references (e.g., `http://...`) are ignored for security purposes.

#### Scroll & Reflow Behavior

- **Normal Buffer**: If the cell-run containing the object does not fit within the viewport width, it triggers a standard wrap with a scroll-up if necessary (without triggering **BCE - Background Color Erase**). This may break the rectangular visual of the raster until the viewport is widened or a non-wrap mode is enabled.
- **Alternate Buffer**: The object's rectangle is strictly clipped (cropped) by the right and bottom edges of the viewport; no scrolling occurs.

#### Sequence Format

```
OSC app [ ; [<attributes>] [<document>] [<attributes>] ] ST
```

Field           | Description
----------------|------------
**OSC command** | Mandatory. `app`.
**attributes**  | Optional. Space-separated `key=value` pairs. If an attribute is specified both before and after the document, the **last occurrence** takes precedence. Values can be quoted (`"` or `'`) or unquoted. All keys and values are **case-sensitive**.
**document**    | Optional. UTF-8 data starting with `<` (the first character of the opening tag, e.g. `<svg>`) and ending with `>` (the last character of the closing tag, e.g. `</svg>`). The specified document is considered to be an `empty-doc` if it has the form `<tag></tag>`, where `tag` is any string.

#### Attribute Scoping

Attributes are divided into **Shared** (object-wide), **Layer** (layer specific) and **Unique** (cell specific).

- **Shared Attributes**: `l`, `u`, `v`, `uw`, `vh`, `x`, `y`, `w`, `h`, `fit`, `rt`, `a`, `f`. Global to all instances of the `id`.
- **Layer Attributes**: All Shared attributes can be overridden per layer inside `l=id(key=val)`.
- **Unique Attributes**: `W`, `H`, `r`, `c`, `o`, `gc`. Specific to the cell.

#### Attributes

Attribute  | Scope  | Value/Range                      | Default                  | Description
-----------|--------|----------------------------------|--------------------------|------------
**id**     | -      | `<id>[/sub-id]`                  | empty string (`""`)      | Object reference ID.
**l**      | Shared | `<id>[/sub-id][(<layer attrs>)]` | empty string (`""`)      | Adds a layer. Can be specified multiple times.
**u, v**   | Shared | `float`                          | `0.0`                    | Top-left of the source crop (0.0-1.0).
**uw, vh** | Shared | `float`                          | `1.0`                    | Size of the source crop (0.0-1.0). Negative flips.
**x, y**   | Shared | `float`                          | `0.0`                    | Offset of the Clipping Box **inside** the `W x H` area (cells).
**w, h**   | Shared | `float (0.0-65535.0]`            | `W, H`                   | Dimensions of the Clipping Box **inside** the `W x H` area (cells).
**fit**    | Shared | `inside\|outside\|stretch\|none` | `inside`                 | Fit logic: How the crop fits into the `w x h` Clipping Box. (`none` = exact pixels).
**tr**     | Shared | `0..7`                           | `0`                      | **Tr**ansform: Decimal value of the 3-bit state `[FlipY][FlipX][SwapXY]`.
**rt**     | Shared | `0\|90\|180\|270`                | `0`                      | **R**ota**t**e: CCW rotation of the source (degrees).
**f**      | Shared | `n\|v\|h\|vh\|hv`                | `n`                      | **F**lip: `n` = none, `v` = vertical, `h` = horizontal.
**a**      | Shared | `[l\|c\|r][t\|m\|b]`             | `cm`                     | **A**lign: 2D alignment within the Clipping Box and the `W x H` area. (`l` = left, `c` = center, `r` = right, `t` = top, `m` = middle, `b` = bottom).
**gc**     | Unique | `string`                         | ASCII Space (0x20) `" "` | **G**rapheme **c**luster to write to cells (will be scaled to a 1x1 cell size).
**W, H**   | Unique | `0..65535`                       | `0`                      | Grid Container: Physical footprint in cells. If `0`, nothing is rendered (registration only).
**r, c**   | Unique | `0..65535`                       | `0`                      | **R**ow, **c**olumn 1-based slicing index for target cell slice. (0 = full height/width, 1..n = specific cell/slice).
**o**      | Unique | `0\|1`                           | `0`                      | **O**ntop: 0 = under text, 1 = over text.

> Notes:
> - If `id` is omitted , the empty string `id=""` is used for registration and output.
> - Attribute values `w` and `h` are clamped to the `(0.0-65535.0]` range and further limited by the terminal's maximum window size settings.
> - Attribute values that depend on the cell size are multiplied by the cell size and **rounded** to get the exact pixel values on the FE side.
> - The first part of the object reference ID (`id`) references the raw object document in the Backend cache.
> - The second part of the object reference ID (`sub-id`) addresses the specific named element (e.g., an `id="..."` within an SVG document). The Frontend renders only this fragment if it is specified.

#### Lifecycle & Cache Management

Input State                | Action
---------------------------|-------
**id** + **doc**           | Register/update.
**id** + **doc** + **W,H** | Register/update and output to the specified area.
**id** + **W,H**           | Output to the specified area.
**id** + **empty-doc**     | Unregister.
**id/sub-id**              | If a `sub-id` is specified, the FE renders only that specific element at its original coordinates (as it would appear in the full document), inheriting all parent styles (CSS, `<g>` groups).
**Errors**                 | If a document is invalid, the `id` is unknown, the `sub-id` is missing, or the cache is full, the FE **clears the object metadata** in the target area (rendering nothing) and logs the error.

> Note:
> When the BE deletes an `id`, or upon reset/session close, it frees the index and signals the FEs.
> - FEs then traverse their viewport cells; any image in the FE cache no longer referenced by any cell is purged.
> - If an FE encounters an unknown object index in a cell, it must request full metadata/document from the BE.

#### Parsing Rules (Backend)

- Scan the OSC string for `key=value` pairs.
- Parse layer attributes inside `l=id(...)` with the same logic as primary attributes.
- Identify document boundaries via the first `<` and the last `>`.
- Accumulate orientation attributes (`tr`, `f`, `rt`) in the order of appearance to calculate the final state.
- Scan the OSC string for remaining `key=value` pairs.

#### Extensibility

The protocol is engine-agnostic. Focused on **SVG**, but designed to support other formats (e.g., `<html>`, `<object>`) via root tag identification.

The protocol can be extended to include non-visual data segments such as `<audio>` or `<wav>`, allowing for synchronized multimedia playback managed by the Frontend.
