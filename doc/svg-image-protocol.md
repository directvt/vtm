### SVG Image Protocol

Outputting SVG as an image protocol allows vector and bitmap graphics to be displayed directly in the scrollback.

#### Rendering & Blending

- **Persistence**: Metadata is stored per-cell; survives scrollback and reflows.
- **Coloring**: The underlying cell **SGR fgc** maps to `currentColor`.
- **Z-order**: Default is background (text on top). **SGR 7 (Reverse)** toggles to foreground (image on top).
- **Re-rasterization**: FÉ (Graphical Frontend) re-renders the SVG upon cell size changes for pixel-perfection.

#### Sequence Format

```
ESC ] image ; [<attributes>] [<svg-document>] ST
```

Field             | Description
------------------|------------
**OSC command**   | Mandatory. `image`.
**attributes**    | Optional. Space-separated `key=value` pairs. Values can be quoted (`"` or `'`) or unquoted.
**svg-document**  | Optional. UTF-8 SVG data starting with `<svg` and ending with `</svg>`.

#### Attributes

Attribute  | Values                                          | Default           | Description
-----------|-------------------------------------------------|-------------------|------------
**id**     | `<id>[/sub-id]`                                 | `""`              | Image reference ID. If omitted, the ID from the SVG root is used.
**width**  | `1..2047`                                       | Terminal viewport | Width of the charcell rectangle (hosting area) in cells.
**height** | `1..1023`                                       | Terminal viewport | Height of the charcell rectangle (hosting area) in cells.
**row**    | `0..height`                                     | `0`               | Vertical slice index (0 = full height, 1..n = specific cell).
**column** | `0..width`                                      | `0`               | Horizontal slice index (0 = full width, 1..n = specific cell).
**align**  | `[left\|center\|right][-][top\|middle\|bottom]` | `center-middle`   | 2D alignment within the charcell rectangle.
**scale**  | `inside`, `outside`, `stretch`, `none`          | `inside`          | Fit logic (none = exact SVG pixels, cropped if larger).
**flip**   | `none`, `v`, `h`, `vh`                          | `none`            | Transformation applied in the order specified in the string.
**mirror** | `none`, `v`, `h`, `vh`                          | `none`            | Transformation applied in the order specified in the string.
**rotate** | `0`, `90`, `180`, `270`                         | `0`               | CCW rotation applied in the order specified in the string.

#### Lifecycle Logic

Input State             | Action
------------------------|-------
**id** + **svg-doc**    | **Register & Display**: Store/update document in cache and output to the current cursor position.
**id** + **empty-doc**  | **Unregister**: Remove the document from cache (triggered by `<svg></svg>`).
**id** + **no svg-doc** | **Display**: Output the existing cached document using provided or default attributes.
**no id** + **svg-doc** | **Anonymous Display**: Use the internal SVG root `<svg id="..."` for the session.

#### Parsing Rules (Backend)

1. Scan the OSC string for `key=value` pairs.
2. Locate the document boundaries by finding the first `<svg` and the last `</svg>`.
3. Extract the document body and resume parsing attributes from the remaining string segments.
4. The transformation pipeline (`flip`, `mirror`, `rotate`) is execution-order dependent based on their sequence in the attributes string.
