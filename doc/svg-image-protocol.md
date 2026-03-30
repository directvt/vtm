### SVG Image Protocol Specification

Outputting SVG as an image protocol allows vector and bitmap graphics to be displayed directly in the scrollback.

#### Rendering & Attributes

- **Z-order**: Default is background (text on top). **SGR 7 (Reverse)** toggles to foreground (image on top).
- **Coloring**: **SGR fgc** maps to `currentColor`.
- **Persistence**: Metadata is stored per-cell; survives scrollback and reflows.
- **Re-rasterization**: FÉ re-renders the SVG upon cell size changes for pixel-perfection.

| Attribute  | Values                                          | Default         | Description
|:-----------|:------------------------------------------------|:----------------|:-----------
| **width**  | `1..2047`                                       | =viewport width | Width of the charcell rectangle (hosting area).
| **height** | `1..1023`                                       | =vewport height | Height of the charcell rectangle (hosting area).
| **row**    | `0..height`                                     | `0`             | Vertical slice index (0 = full height, 1..n = specific cell).
| **column** | `0..width`                                      | `0`             | Horizontal slice index (0 = full width, 1..n = specific cell).
| **align**  | `[center\|left\|right][-][middle\|top\|bottom]` | `center-middle` | 2D alignment within the charcell rectangle.
| **scale**  | `inside`, `outside`, `stretch`, `none`          | `inside`        | Fitting: touch inside/outside, stretch, or 1:1 pixels.
| **flip**   | `none`, `v`, `h`, `vh`                          | `none`          | Vertical and/or horizontal flipping.
| **mirror** | `none`, `v`, `h`, `vh`                          | `none`          | Vertical and/or horizontal mirroring.
| **rotate** | `0`, `90`, `180`, `270`                         | `0`             | Counter-clockwise rotation in degrees.

#### Sequence Format

`ESC ] image ; <id>[/sub-id] [; <attributes> [; <svg-document>]] ST`

#### Parsing Logic

- **Segment 1 (OSC command)**: Mandatory. `image`.
- **Segment 2 (ID)**: Mandatory. Must not contain `;`.
- **Segment 3 (Attributes)**: Optional. Space-separated `attribute=value`.
- **Segment 4 (SVG Data)**: Optional. Raw text until `ST`.

#### Lifecycle Logic

| Fields                            | Action
|:----------------------------------|:------
| **id** + **svg-doc**              | **Register & Display**: Store in cache, update cells.
| **id** + **empty doc** (with `;`) | **Unregister**: Remove `id` from cache.
| **id** + **empty doc** (no `;`)   | **Display**: Reuse existing `id` from cache.
| **no id** + **svg-doc**           | **Cache Only**: Root `<svg id="...">` is used for future reference.
