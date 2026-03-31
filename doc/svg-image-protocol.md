### Universal Object Protocol

Outputting an **object** allows vector, bitmap, and extensible markup graphics to be displayed directly in the scrollback.

#### Rendering & Alpha Blending

- **Persistence**: Metadata is stored per-cell; survives scrollback and reflows.
- **Coloring**: The underlying cell **SGR fgc** maps to `currentColor` (for SVG).
- **Z-order**: Default is background (text on top). **SGR 7 (Reverse)** toggles to foreground (object on top).
- **Re-rasterization**: FÉ (Graphical Frontend) re-renders the object upon cell size changes for pixel-perfection.

#### Sequence Format

```
ESC ] object ; [<attributes>] [<document>] ST
```

Field             | Description
------------------|------------
**OSC command**   | Mandatory. `object`.
**attributes**    | Optional. Space-separated `key=value` pairs. Values can be quoted (`"` or `'`) or unquoted.
**document**      | Optional. UTF-8 data starting with a format tag (e.g., `<svg`) and ending with a closing tag (e.g., `</svg>`).

#### Attributes

Attribute  | Values                                          | Default                  | Description
-----------|-------------------------------------------------|--------------------------|------------
**id**     | `<id>[/sub-id]`                                 | empty string (`""`)      | Object reference ID. If omitted, the ID from the root tag is used.
**width**  | `1..2047`                                       | Terminal viewport width  | Width of the charcell rectangle (hosting area) in cells.
**height** | `1..1023`                                       | Terminal viewport height | Height of the charcell rectangle (hosting area) in cells.
**row**    | `0..height`                                     | `0`                      | Vertical slice index (0 = full height, 1..n = specific cell).
**column** | `0..width`                                      | `0`                      | Horizontal slice index (0 = full width, 1..n = specific cell).
**align**  | `[left\|center\|right][-][top\|middle\|bottom]` | `center-middle`          | 2D alignment within the charcell rectangle.
**scale**  | `inside`, `outside`, `stretch`, `none`          | `inside`                 | Fit logic (none = exact pixels, cropped if larger).
**flip**   | `none`, `v`, `h`, `vh`                          | `none`                   | Transformation applied in the order specified in the string.
**mirror** | `none`, `v`, `h`, `vh`                          | `none`                   | Transformation applied in the order specified in the string.
**rotate** | `0`, `90`, `180`, `270`                         | `0`                      | CCW rotation applied in the order specified in the string.

#### Lifecycle Logic

Input State             | Action
------------------------|-------
**id** + **doc**        | **Register & Display**: Store/update document in cache and output to the current cursor position.
**id** + **empty-doc**  | **Unregister**: Remove the object referenced by `id` from cache (e.g., `<svg></svg>`).
**id** + **no doc**     | **Display**: Output the existing cached object using provided or default attributes.
**no id** + **doc**     | **Anonymous Display**: Use the internal root tag `id="..."` (e.g., `<svg id="..."></svg>`) for the session.

#### Extensibility

The protocol is engine-agnostic. While currently focused on **SVG**, the data segment can be extended to support other resteriable formats - such as `<html>...</html>` or `<object>...</object>` - by identifying the root tag of the document body.