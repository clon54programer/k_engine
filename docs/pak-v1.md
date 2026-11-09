# K_engine — Contrato del contenedor `.pak` v1

**Estado:** aprobado el 2026-11-09.
**Versión del formato:** `1.0`.
**Alcance:** MVP Linux. Sin cifrado ni compresión adicional.

Este documento fija la estructura binaria, los tipos, los límites y las reglas
de validación que deben compartir el lector C++ del runtime y el escritor Python
del empaquetador. Cualquier cambio incompatible requiere una versión nueva.

## 1. Convenciones

- Todos los enteros son little-endian.
- La versión aceptada por el MVP es exactamente `1.0`.
- La firma es ASCII `KPAK`.
- Los offsets son absolutos desde el inicio del archivo.
- Cada payload empieza en un offset múltiplo de 16 bytes.
- El padding se rellena con bytes cero; su contenido no forma parte del recurso.
- No se aplica cifrado ni compresión adicional.
- El lector no carga el `.pak` completo en memoria: abre el archivo, lee la
  cabecera y la tabla, y lee cada recurso bajo demanda.
- No existe un límite global de tamaño del `.pak`. Sí existen límites para la
  tabla y para cada recurso.

## 2. Cabecera

La cabecera ocupa 16 bytes fijos:

| Offset | Tamaño | Campo | Tipo | Regla |
|---:|---:|---|---|---|
| 0 | 4 | Firma | bytes | `KPAK` |
| 4 | 2 | Versión mayor | `u16` | `1` |
| 6 | 2 | Versión menor | `u16` | `0` |
| 8 | 4 | Cantidad de archivos | `u32` | máximo 104 857 |
| 12 | 4 | CRC-32 de la tabla | `u32` | cubre solo las entradas de la TOC |

## 3. Tabla de contenidos

La tabla empieza inmediatamente después de la cabecera, en el offset 16. Cada
registro ocupa 40 bytes fijos:

| Offset relativo | Tamaño | Campo | Tipo | Regla |
|---:|---:|---|---|---|
| 0 | 8 | ID estable | `u64` | distinto de cero |
| 8 | 4 | Tipo de recurso | `u32` | ver enum |
| 12 | 4 | Flags | `u32` | cero en v1 |
| 16 | 8 | Offset absoluto | `u64` | múltiplo de 16 |
| 24 | 8 | Tamaño del recurso | `u64` | excluye padding |
| 32 | 4 | CRC-32 del recurso | `u32` | cubre solo el payload |
| 36 | 4 | Reservado | `u32` | cero en v1 |

Los registros se ordenan por ID de forma estrictamente creciente. Los IDs
duplicados o desordenados son inválidos.

## 4. Distribución del archivo

```text
[ cabecera 16 bytes ][ tabla: count × 40 bytes ][ padding ][ payloads ... ]
```

El inicio del área de datos se calcula con aritmética comprobada:

```text
toc_size   = count * 40
data_start = align_up(16 + toc_size, 16)
```

Reglas:

- `count = 0` es válido a nivel de formato; el packer del MVP exige al menos un
  recurso.
- La tabla debe caber completa en el archivo.
- El primer payload puede empezar en `data_start` o después; los huecos son
  padding.
- Los payloads pueden aparecer en cualquier orden físico. El escritor los emite
  en orden de ID para que el resultado sea determinista.
- El final del archivo puede contener bytes de padding; el lector los ignora.
- Los rangos `[offset, offset + size)` no pueden solaparse.

Ejemplo con un único recurso:

```text
offset   0: 4B 50 41 4B  01 00  00 00  01 00 00 00  CRC_TOC
offset  16: ID (8)  TYPE=3  FLAGS=0  OFFSET=64  SIZE=N  CRC_PNG  RESERVED=0
offset  56: padding de 8 bytes hasta el offset 64
offset  64: payload PNG de N bytes
```

## 5. Tipos de recurso

| Valor | Tipo | Uso | Emitido por el packer v1.0 |
|---:|---|---|---|
| `0` | Inválido | Rechazar | No |
| `1` | Level FlatBuffer | Nivel | Sí |
| `2` | Animation FlatBuffer | Animación | Sí |
| `3` | PNG | Tileset, spritesheet o sprite | Sí |
| `4` | WAV | Reservado para audio futuro | No |
| `5` | Fuente | Reservado para fuentes futuras | No |
| `6` | SPIR-V | Reservado para shaders futuros | No |
| `7–255` | Reservado | Rechazar en v1.0 | No |
| `>255` | Desconocido | Rechazar | No |

## 6. Límites

| Límite | Valor |
|---|---:|
| Tamaño máximo de la TOC | 4 MiB |
| Entradas máximas | 104 857 |
| Level FlatBuffer | 64 MiB |
| Animation FlatBuffer | 64 MiB |
| PNG codificado | 64 MiB |
| PNG decodificado | 64 MiB |
| Dimensiones PNG | 1 × 1 hasta 4096 × 4096 |
| Celdas de nivel | 1 048 576 |
| Dimensiones de cuadrícula | 4096 × 4096 máximo |
| Capas por nivel | 16 |
| Tilesets por nivel | 64 |
| Entidades por nivel | 65 536 |
| Fotogramas por animación | 4096 |
| `fps` | 1–240 |
| `duration_ms` | 1–60 000; `0` significa usar `fps` |
| Cadenas | 1 MiB |
| Rutas | 4096 bytes |
| ID máximo | `2^64 − 1` |

El tamaño de la TOC se comprueba antes de reservar memoria:

```text
count <= 104857
count * 40 <= 4 MiB
16 + count * 40 <= file_size
```

## 7. CRC-32/ISO-HDLC

Se usa CRC-32/ISO-HDLC:

- Polinomio reflejado: `0xEDB88320`.
- Valor inicial: `0xFFFFFFFF`.
- XOR final: `0xFFFFFFFF`.

El CRC de la tabla cubre únicamente los bytes de las entradas de la TOC, no la
cabecera. El CRC de cada recurso cubre únicamente sus bytes de payload, sin
padding. La verificación se hace por bloques de 64 KiB.

## 8. Validación del lector

El lector valida, en este orden, antes de devolver cualquier recurso:

1. El archivo tiene al menos 16 bytes.
2. La firma es `KPAK`.
3. La versión es exactamente `1.0`.
4. `count` no supera 104 857.
5. `16 + count * 40` no desborda y cabe en el archivo.
6. El tamaño de la TOC no supera 4 MiB.
7. El CRC de la TOC coincide.
8. Cada entrada tiene `flags = 0` y `reservado = 0`.
9. Cada ID es distinto de cero y estrictamente mayor que el anterior.
10. El tipo está reconocido y no es reservado para el MVP.
11. `offset % 16 == 0`.
12. `offset >= data_start`.
13. `size > 0` y no supera el límite de su tipo.
14. `offset + size` no desborda y no sale del archivo.
15. Los rangos de payload no se solapan.
16. Al leer un recurso, su CRC coincide.

Si falla cualquier comprobación, se rechaza el `.pak` completo o el recurso
solicitado, según corresponda, con un mensaje que incluya archivo, ID, offset,
tamaño y CRC esperado/calculado cuando aplique. No se devuelven datos parciales.

## 9. Escritura

El empaquetador:

- Ordena las entradas por ID.
- Calcula offsets absolutos y añade padding cero hasta el siguiente múltiplo de
  16.
- Escribe la cabecera, la TOC y los payloads en little-endian.
- Escribe a un archivo temporal en el mismo directorio y solo publica el destino
  final si todo el proceso termina correctamente.
- No emite tipos reservados ni desconocidos.

## 10. Compatibilidad

- `1.0` es la única versión aceptada por el MVP.
- Un cambio incompatible requiere una versión mayor nueva.
- Los valores de tipo `4–255` quedan reservados para extensiones futuras; el
  lector v1.0 los rechaza.
- Los flags y el campo reservado deben ser cero en v1.0.
- Los campos internos de los FlatBuffers se validan con su propio
  `schema_version = 1` y con `flatbuffers::Verifier`.

## 11. Casos inválidos obligatorios

El lector y el packer deben cubrir, como mínimo:

- Firma incorrecta.
- Versión mayor o menor no soportada.
- Tabla truncada.
- `count * 40` desbordado.
- TOC mayor que 4 MiB.
- CRC de tabla incorrecto.
- ID duplicado.
- IDs desordenados.
- Tipo inválido, reservado o desconocido.
- Flags o reservado distintos de cero.
- Offset no alineado a 16 bytes.
- Offset dentro de la cabecera o la TOC.
- Rango fuera del archivo.
- Solapamiento de payloads.
- Tamaño cero o mayor que el límite del tipo.
- CRC de recurso incorrecto.
- Recurso truncado.
- Apertura sin lectura de payloads.
- Lectura de un recurso limitada a su rango.
- CRC por bloques de 64 KiB.

## 12. Relación con los datos de desarrollo

Los JSON del editor se convierten a FlatBuffers antes de empaquetarse. El
runtime no parsea JSON. Los esquemas JSON y su correspondencia con FlatBuffers se
documentan en `docs/json-schemas.md`.
