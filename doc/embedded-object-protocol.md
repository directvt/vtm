### Embedded Object Protocol

The **Embedded Object Protocol (EOP)** allows vector, bitmap, and extensible markup objects to be embedded directly into the terminal's scrollback buffer.

#### Rendering & Alpha Blending

- **Persistence**: Metadata is stored per-cell; survives scrollback, window resizing, and reflows.
- **Coloring**: The underlying cell **SGR fgc** maps to `currentColor` (for SVG).
- **Z-order**: Default is **background** (text on top). **SGR 7 (Reverse Video)** toggles the cell to **foreground** (object on top of text).
- **Non-destructive**: Outputting an object does not destroy existing text in the cells. Only the **SGR bgc** (background color) is replaced by the object's visual data.
- **Rectangular Area**: The object is hosted within a defined rectangular grid of cells ($width \times height$).
- **Cursor Position**: After outputting the object, the cursor moves to the cell immediately following the **bottom-right** corner of the object's rectangle.
- **Re-rasterization**: The Graphical Frontend (FÉ) re-renders the object upon cell size changes to maintain pixel-perfection.

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

Attribute  | Values                                 | Default                  | Description
-----------|----------------------------------------|--------------------------|------------
**id**     | `<id>[/sub-id]`                        | empty string (`""`)      | Object reference ID. If omitted, the ID from the root tag is used.
**width**  | `1`..`2047`                            | Terminal viewport width  | Width of the charcell rectangle (hosting area) in cells.
**height** | `1`..`1023`                            | Terminal viewport height | Height of the charcell rectangle (hosting area) in cells.
**row**    | `0`..`<height>`                        | `0`                      | Vertical slice index (0 = full height, 1..n = specific cell).
**column** | `0`..`<width>`                         | `0`                      | Horizontal slice index (0 = full width, 1..n = specific cell).
**align**  | \[`left`\|`center`\|`right`\]\[`-`\]\[`top`\|`middle`\|`bottom`\] | `center-middle` | 2D alignment within the charcell rectangle.
**scale**  | `inside`\|`outside`\|`stretch`\|`none` | `inside`                 | Fit logic (none = exact pixels, cropped if larger).
**flip**   | `none`\|`v`\|`h`\|`vh`                 | `none`                   | Transformation applied in the order specified in the string.
**mirror** | `none`\|`v`\|`h`\|`vh`                 | `none`                   | Transformation applied in the order specified in the string.
**rotate** | `0`\|`90`\|`180`\|`270`                | `0`                      | CCW rotation applied in the order specified in the string.

#### Lifecycle Logic

Input State             | Action
------------------------|-------
**id** + **doc**        | **Register & Display**: Store/update document in cache and output to the current cursor position.
**id** + **empty-doc**  | **Unregister**: Remove the object referenced by `id` from cache (e.g., `<svg></svg>`).
**id** + **no doc**     | **Display**: Output the existing cached object using provided or default attributes.
**no id** + **doc**     | **Anonymous Display**: Use the internal root tag `id="..."` (e.g., `<svg id="..."></svg>`) for the session.

#### Parsing Rules (Backend)

1. Scan the OSC string for `key=value` pairs.
2. Locate the document boundaries by finding the first `<tag` and the last `</tag>`.
3. Extract the document body and resume parsing attributes from the remaining string segments.
4. The top-left corner of the object's rectangle is anchored to the current cursor position.
5. The transformation pipeline (`flip`, `mirror`, `rotate`) is execution-order dependent based on their sequence in the attributes string.

#### Extensibility

The protocol is engine-agnostic. While currently focused on **SVG**, the data segment is designed to support other resteriable formats - such as `<html>...</html>` or `<object>...</object>` - by identifying the root tag of the document body.