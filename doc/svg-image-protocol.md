### SVG Image Protocol Specification

Outputting SVG as an image protocol allows vector and bitmap graphics to be displayed directly in the scrollback.

#### Rendering & Attributes

- **Z-order**: Default is background (text on top). **SGR 7 (Reverse)** toggles to foreground (image on top).
- **Coloring**: **SGR fgc** maps to `currentColor`.
- **Persistence**: Metadata is stored per-cell; survives scrollback and reflows.
- **Re-rasterization**: FÉ re-renders the SVG upon cell size changes for pixel-perfection.

#### Sequence Format

`ESC ] image ; [<attributes>] [<svg-document>] ST`

Field             | Description
------------------|------------
**OSC command**   | Mandatory. `image`.
**attributes**    | Optional. Space-separated `attribute=value`.
**svg-document**  | Optional. SVG-document in UTF-8 format beginning with `<svg` and ending with `</svg>`.

#### Attributes

| Attribute  | Values                                          | Default         | Description
|:-----------|:------------------------------------------------|:----------------|:-----------
| **id**     | \<id\>[/sub-id]                                 | empty string    | An image ID for referencing. If not specified, the value or an empty string will be taken from the SVG document.
| **width**  | `1..2047`                                       | =viewport width | Width of the charcell rectangle (hosting area).
| **height** | `1..1023`                                       | =vewport height | Height of the charcell rectangle (hosting area).
| **row**    | `0..height`                                     | `0`             | Vertical slice index (0 = full height, 1..n = specific cell).
| **column** | `0..width`                                      | `0`             | Horizontal slice index (0 = full width, 1..n = specific cell).
| **align**  | `[center\|left\|right][-][middle\|top\|bottom]` | `center-middle` | 2D alignment within the charcell rectangle.
| **scale**  | `inside`, `outside`, `stretch`, `none`          | `inside`        | Fitting: touch inside/outside, stretch, or 1:1 pixels.
| **flip**   | `none`, `v`, `h`, `vh`                          | `none`          | Vertical and/or horizontal flipping.
| **mirror** | `none`, `v`, `h`, `vh`                          | `none`          | Vertical and/or horizontal mirroring.
| **rotate** | `0`, `90`, `180`, `270`                         | `0`             | Counter-clockwise rotation in degrees.

- Attribute values can be either quoted or unquoted, using either double `"` or single `'` quotes.
- The order of the `flip`, `mirror`, and `rotate` attributes is significant. These transformations will be applied in the specified order.
- Specifying attributes before or after the SVG document does not matter.

#### Lifecycle Logic

| Fields                     | Action
|:---------------------------|:------
| **id** + **svg-doc**       | Store/update SVG-document in cache, output specified element/sub-element.
| **id** + **empty-svg-doc** | Remove SVG-document referenced by `id` from cache. The `empty-svg-doc` is literally `<svg></svg>`.
| **id** + **no svg-doc**    | Output existing SVG-document.
| **no id** + **svg-doc**    | Root `<svg id="...">` is used to specify outputting element.
