# K_engine — Esquemas JSON de desarrollo

**Estado:** aprobado el 2026-11-09.
**Versión de esquema:** `1.0`.
**Alcance:** datos editables del editor durante el desarrollo.

Los JSON son el formato de trabajo del editor. El empaquetador los valida, los
convierte a FlatBuffers y los incluye en el `.pak` v1. El runtime no parsea JSON.

## 1. Convenciones generales

- Codificación UTF-8.
- `schema_version` es la cadena `"1.0"` en los tres archivos.
- Los IDs se escriben como cadenas decimales para evitar ambigüedades de
  precisión. En FlatBuffers y en el `.pak` se convierten a `u64`.
- Los IDs de recursos son distintos de cero y únicos en todo el proyecto.
- Las coordenadas y tamaños son enteros; no se usan flotantes en v1.
- Las rutas son relativas al directorio de `project.json`, usan `/` como
  separador, no pueden ser absolutas ni contener `..`, y tienen un máximo de 4096
  bytes.
- En v1 se rechazan los campos desconocidos para detectar errores de escritura.
- Límites generales: `project.json` hasta 4 MiB; `level.json` y `animation.json`
  hasta 64 MiB; cadenas hasta 1 MiB.

### Ámbitos de IDs

- **Globales:** assets PNG, niveles y animaciones. Deben ser únicos entre sí.
- **Locales al nivel:** tilesets, capas y entidades. Deben ser únicos dentro del
  nivel y no compiten con los IDs globales.

## 2. `project.json`

Archivo raíz del proyecto editable. El packer lo usa como punto de entrada.

| Campo | Tipo | Obligatorio | Regla |
|---|---|---:|---|
| `schema_version` | string | Sí | `"1.0"` |
| `project_id` | string | Sí | No vacío; máximo 1 MiB |
| `name` | string | Sí | No vacío; máximo 1 MiB |
| `next_id` | string decimal | Sí | `u64`; mayor que todos los IDs globales |
| `entry_level_id` | string decimal | Sí | Debe existir en `levels` |
| `assets` | array | Sí | Puede estar vacío; cada elemento es un PNG |
| `levels` | array | Sí | Al menos un nivel |
| `animations` | array | No | Puede estar vacío u omitirse |

Elementos de `assets`:

| Campo | Tipo | Obligatorio | Regla |
|---|---|---:|---|
| `id` | string decimal | Sí | ID global único |
| `type` | string | Sí | Solo `"png"` en v1 |
| `path` | string | Sí | Ruta relativa al PNG |

Elementos de `levels`:

| Campo | Tipo | Obligatorio | Regla |
|---|---|---:|---|
| `id` | string decimal | Sí | ID global único; coincide con el `id` del nivel |
| `path` | string | Sí | Ruta relativa al `level.json` |

Elementos de `animations`:

| Campo | Tipo | Obligatorio | Regla |
|---|---|---:|---|
| `id` | string decimal | Sí | ID global único; coincide con el `id` de la animación |
| `path` | string | Sí | Ruta relativa al `animation.json` |

Reglas adicionales:

- `next_id` debe ser mayor que el máximo ID global usado.
- Las rutas de `assets`, `levels` y `animations` deben ser únicas en el proyecto.
- Los archivos referenciados deben existir y ser regulares.
- Los `id` internos de cada nivel y animación deben coincidir con el declarado en
  `project.json`.

Ejemplo:

```json
{
  "schema_version": "1.0",
  "project_id": "k_engine_demo",
  "name": "Demo técnica",
  "next_id": "20",
  "entry_level_id": "2",
  "assets": [
    {
      "id": "1",
      "type": "png",
      "path": "assets/tiles.png"
    }
  ],
  "levels": [
    {
      "id": "2",
      "path": "levels/level01.json"
    }
  ],
  "animations": [
    {
      "id": "3",
      "path": "animations/player_idle.json"
    }
  ]
}
```

## 3. `level.json`

| Campo | Tipo | Obligatorio | Regla |
|---|---|---:|---|
| `schema_version` | string | Sí | `"1.0"` |
| `id` | string decimal | Sí | ID global; coincide con `project.json` |
| `name` | string | Sí | No vacío |
| `grid` | objeto | Sí | Ver abajo |
| `tilesets` | array | Sí | 1–64 elementos |
| `layers` | array | Sí | 1–16 elementos |
| `entities` | array | No | 0–65 536 elementos |

### `grid`

| Campo | Tipo | Obligatorio | Regla |
|---|---|---:|---|
| `tile_width` | entero | Sí | 1–4096 |
| `tile_height` | entero | Sí | 1–4096 |
| `columns` | entero | Sí | 1–4096 |
| `rows` | entero | Sí | 1–4096 |

`columns * rows` no puede superar 1 048 576.

### `tilesets`

| Campo | Tipo | Obligatorio | Regla |
|---|---|---:|---|
| `id` | string decimal | Sí | ID local único |
| `image_id` | string decimal | Sí | Referencia a un asset PNG global |
| `tile_width` | entero | Sí | Igual a `grid.tile_width` |
| `tile_height` | entero | Sí | Igual a `grid.tile_height` |
| `columns` | entero | Sí | 1–4096 |
| `rows` | entero | Sí | 1–4096 |

### `layers`

| Campo | Tipo | Obligatorio | Regla |
|---|---|---:|---|
| `id` | string decimal | Sí | ID local único |
| `name` | string | Sí | No vacío |
| `kind` | string | Sí | `"tile"` o `"collision"` |
| `visible` | booleano | No | Por defecto `true` |
| `tileset_id` | string decimal | Sí | Referencia a un tileset del nivel |
| `tiles` | array de enteros | Sí | Longitud exactamente `columns * rows` |

Convención de tiles:

- `0` = celda vacía.
- `1..N` = índice de tile dentro del tileset de la capa.
- `N` no puede superar `tileset.columns * tileset.rows`.
- La capa `collision` usa la misma codificación; puede tener `visible: false`.

### `entities`

| Campo | Tipo | Obligatorio | Regla |
|---|---|---:|---|
| `id` | string decimal | Sí | ID local único |
| `image_id` | string decimal | Condicional | ID global de PNG |
| `animation_id` | string decimal | Condicional | ID global de animación |
| `x` | entero | Sí | `int32` |
| `y` | entero | Sí | `int32` |
| `layer_id` | string decimal | Sí | Referencia a una capa local |

Debe existir exactamente uno de `image_id` o `animation_id`. En FlatBuffers, el
campo ausente se representa con `0`.

### Ejemplo

```json
{
  "schema_version": "1.0",
  "id": "2",
  "name": "Nivel técnico",
  "grid": {
    "tile_width": 32,
    "tile_height": 32,
    "columns": 4,
    "rows": 3
  },
  "tilesets": [
    {
      "id": "10",
      "image_id": "1",
      "tile_width": 32,
      "tile_height": 32,
      "columns": 16,
      "rows": 16
    }
  ],
  "layers": [
    {
      "id": "11",
      "name": "fondo",
      "kind": "tile",
      "visible": true,
      "tileset_id": "10",
      "tiles": [1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1]
    },
    {
      "id": "12",
      "name": "paredes",
      "kind": "collision",
      "visible": false,
      "tileset_id": "10",
      "tiles": [0, 0, 0, 0, 0, 2, 2, 0, 0, 2, 2, 0]
    }
  ],
  "entities": [
    {
      "id": "13",
      "animation_id": "3",
      "x": 64,
      "y": 128,
      "layer_id": "11"
    }
  ]
}
```

## 4. `animation.json`

| Campo | Tipo | Obligatorio | Regla |
|---|---|---:|---|
| `schema_version` | string | Sí | `"1.0"` |
| `id` | string decimal | Sí | ID global; coincide con `project.json` |
| `name` | string | Sí | No vacío |
| `image_id` | string decimal | Sí | Referencia a un asset PNG global |
| `frame_width` | entero | Sí | 1–4096 |
| `frame_height` | entero | Sí | 1–4096 |
| `fps` | entero | Sí | 1–240 |
| `loop` | booleano | No | Por defecto `true` |
| `pivot` | objeto | Sí | `x`, `y` enteros |
| `hitbox` | objeto | Sí | `x`, `y`, `w`, `h` enteros |
| `frames` | array | Sí | 1–4096 elementos |

Coordenadas:

- `pivot` y `hitbox` son relativos a la esquina superior izquierda del
  fotograma.
- `0 <= pivot.x <= frame_width` y `0 <= pivot.y <= frame_height`.
- `hitbox.w > 0`, `hitbox.h > 0`, y la caja debe caber dentro del fotograma.

Elementos de `frames`:

| Campo | Tipo | Obligatorio | Regla |
|---|---|---:|---|
| `x` | entero | Sí | ≥ 0; dentro de la imagen |
| `y` | entero | Sí | ≥ 0; dentro de la imagen |
| `w` | entero | Sí | 1–4096; dentro de la imagen |
| `h` | entero | Sí | 1–4096; dentro de la imagen |
| `duration_ms` | entero | No | 0–60 000; `0` u omitido usa `fps` |

Todas las coordenadas de fotograma deben quedar dentro de las dimensiones reales
del PNG referenciado; el packer las comprueba.

Ejemplo:

```json
{
  "schema_version": "1.0",
  "id": "3",
  "name": "player_idle",
  "image_id": "1",
  "frame_width": 32,
  "frame_height": 32,
  "fps": 8,
  "loop": true,
  "pivot": {
    "x": 16,
    "y": 32
  },
  "hitbox": {
    "x": 4,
    "y": 8,
    "w": 24,
    "h": 24
  },
  "frames": [
    { "x": 0, "y": 0, "w": 32, "h": 32, "duration_ms": 0 },
    { "x": 32, "y": 0, "w": 32, "h": 32, "duration_ms": 0 },
    { "x": 64, "y": 0, "w": 32, "h": 32, "duration_ms": 0 },
    { "x": 96, "y": 0, "w": 32, "h": 32, "duration_ms": 0 }
  ]
}
```

## 5. Validaciones cruzadas del packer

Antes de convertir a FlatBuffers o escribir el `.pak`, el packer comprueba:

1. Los tres `schema_version` son `"1.0"`.
2. Los IDs globales son únicos y distintos de cero.
3. `next_id` es mayor que todos los IDs globales.
4. `entry_level_id` existe.
5. Cada nivel y animación declarado existe y su `id` interno coincide.
6. Cada `image_id`, `animation_id`, `tileset_id` y `layer_id` referenciado existe.
7. Cada ruta es relativa, segura, única y apunta a un archivo regular.
8. Cada PNG es válido, respeta dimensiones y límites, y puede decodificarse.
9. Cada `tiles` tiene la longitud exacta y cada índice es válido.
10. Cada fotograma cabe dentro de la imagen referenciada.
11. Cada entidad tiene exactamente un `image_id` o un `animation_id`.
12. No hay campos JSON desconocidos.

Si falla una comprobación, el packer no escribe el archivo final y devuelve un
error con archivo, campo e ID afectado.

## 6. Correspondencia con FlatBuffers

| JSON | FlatBuffers |
|---|---|
| `schema_version: "1.0"` | `schema_version: uint = 1` |
| ID decimal | `uint64` |
| `tiles` | `[uint]` |
| `kind: "tile"` | `LayerKind.tile` |
| `kind: "collision"` | `LayerKind.collision` |
| `visible` | `bool` |
| `pivot` | struct `Vec2i` |
| `hitbox` | struct `Recti` |
| `frames` | `[Frame]` |
| `duration_ms` ausente o `0` | `duration_ms = 0` |
| `image_id` o `animation_id` ausente | `0` |

Los esquemas están en:

- `engine/schema/level.fbs` → `file_identifier "KLVL"`.
- `engine/schema/animation.fbs` → `file_identifier "KANI"`.

El runtime verifica cada buffer con `flatbuffers::Verifier` antes de leerlo. Los
FlatBuffers se empaquetan como entradas de tipo `1` y `2` en el `.pak` v1.
