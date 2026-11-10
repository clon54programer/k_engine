#!/usr/bin/env python3
"""Genera el fixture .pak de T2 y sus variantes inválidas.

Este script es solo una herramienta de pruebas del engine. No forma parte de la
CLI empaquetadora de T4 y no debe reutilizarse como código de producción.
"""

from __future__ import annotations

import random
import struct
import sys
import zlib
from pathlib import Path

import flatbuffers

from k_engine.schema import Animation, Entity, Grid, Layer, Level, Recti, Tileset, Vec2i
from k_engine.schema.Frame import CreateFrame
from k_engine.schema.LayerKind import LayerKind

HEADER_SIZE = 16
TOC_ENTRY_SIZE = 40
PAYLOAD_ALIGNMENT = 16
ENTRY_FORMAT = "<QIIQQII"

LEVEL_ID = 2
SPRITE_ID = 100
LARGE_PNG_ID = 101
ANIMATION_ID = 200
TILESET_ID = 10
LAYER_ID = 11
ENTITY_ID = 12


def align_up(value: int, alignment: int) -> int:
    return (value + alignment - 1) & ~(alignment - 1)


def png_chunk(tag: bytes, payload: bytes) -> bytes:
    return (
        struct.pack(">I", len(payload))
        + tag
        + payload
        + struct.pack(">I", zlib.crc32(tag + payload) & 0xFFFFFFFF)
    )


def make_png(width: int, height: int, pixels: bytes) -> bytes:
    """Codifica píxeles RGBA de 8 bits como PNG sin dependencias externas."""
    raw = bytearray()
    stride = width * 4
    for row in range(height):
        raw.append(0)  # filtro None
        start = row * stride
        raw.extend(pixels[start : start + stride])

    return (
        b"\x89PNG\r\n\x1a\n"
        + png_chunk(
            b"IHDR",
            struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0),
        )
        + png_chunk(b"IDAT", zlib.compress(bytes(raw), 9))
        + png_chunk(b"IEND", b"")
    )


def make_sprite_png() -> bytes:
    width = 64
    height = 64
    pixels = bytearray(width * height * 4)
    for y in range(height):
        for x in range(width):
            index = (y * width + x) * 4
            if x < width // 2 and y < height // 2:
                color = (220, 40, 40, 255)
            elif x >= width // 2 and y < height // 2:
                color = (40, 200, 80, 255)
            elif x < width // 2 and y >= height // 2:
                color = (40, 80, 220, 255)
            else:
                color = (240, 240, 240, 255)
            pixels[index : index + 4] = bytes(color)
    return make_png(width, height, bytes(pixels))


def make_large_png() -> bytes:
    width = 256
    height = 256
    rng = random.Random(20261109)
    pixels = bytes(rng.randrange(256) for _ in range(width * height * 4))
    return make_png(width, height, pixels)


def build_level_buffer() -> bytes:
    builder = flatbuffers.Builder(4096)
    level_name = builder.CreateString("runtime_level")
    layer_name = builder.CreateString("background")

    tiles = [1] * 12
    Layer.StartTilesVector(builder, len(tiles))
    for tile in reversed(tiles):
        builder.PrependUint32(tile)
    tiles_vector = builder.EndVector()

    Layer.Start(builder)
    Layer.AddId(builder, LAYER_ID)
    Layer.AddName(builder, layer_name)
    Layer.AddKind(builder, LayerKind.tile)
    Layer.AddVisible(builder, True)
    Layer.AddTilesetId(builder, TILESET_ID)
    Layer.AddTiles(builder, tiles_vector)
    layer = Layer.End(builder)

    Tileset.Start(builder)
    Tileset.AddId(builder, TILESET_ID)
    Tileset.AddImageId(builder, SPRITE_ID)
    Tileset.AddTileWidth(builder, 32)
    Tileset.AddTileHeight(builder, 32)
    Tileset.AddColumns(builder, 2)
    Tileset.AddRows(builder, 2)
    tileset = Tileset.End(builder)

    Level.StartTilesetsVector(builder, 1)
    builder.PrependUOffsetTRelative(tileset)
    tilesets = builder.EndVector()

    Level.StartLayersVector(builder, 1)
    builder.PrependUOffsetTRelative(layer)
    layers = builder.EndVector()

    Entity.Start(builder)
    Entity.AddId(builder, ENTITY_ID)
    Entity.AddImageId(builder, SPRITE_ID)
    Entity.AddX(builder, 64)
    Entity.AddY(builder, 64)
    Entity.AddLayerId(builder, LAYER_ID)
    entity = Entity.End(builder)

    Level.StartEntitiesVector(builder, 1)
    builder.PrependUOffsetTRelative(entity)
    entities = builder.EndVector()

    Level.Start(builder)
    Level.AddSchemaVersion(builder, 1)
    Level.AddId(builder, LEVEL_ID)
    Level.AddName(builder, level_name)
    Level.AddTilesets(builder, tilesets)
    Level.AddLayers(builder, layers)
    Level.AddEntities(builder, entities)
    grid = Grid.CreateGrid(builder, 32, 32, 4, 3)
    Level.AddGrid(builder, grid)
    level = Level.End(builder)

    builder.Finish(level, file_identifier=b"KLVL")
    return bytes(builder.Output())


def build_animation_buffer() -> bytes:
    builder = flatbuffers.Builder(1024)
    name = builder.CreateString("runtime_idle")

    Animation.StartFramesVector(builder, 1)
    CreateFrame(builder, 0, 0, 64, 64, 0)
    frames = builder.EndVector()

    Animation.Start(builder)
    Animation.AddSchemaVersion(builder, 1)
    Animation.AddId(builder, ANIMATION_ID)
    Animation.AddName(builder, name)
    Animation.AddImageId(builder, SPRITE_ID)
    Animation.AddFrameWidth(builder, 64)
    Animation.AddFrameHeight(builder, 64)
    Animation.AddFps(builder, 8)
    Animation.AddLoop(builder, True)
    Animation.AddFrames(builder, frames)
    pivot = Vec2i.CreateVec2i(builder, 32, 64)
    Animation.AddPivot(builder, pivot)
    hitbox = Recti.CreateRecti(builder, 8, 16, 48, 48)
    Animation.AddHitbox(builder, hitbox)
    animation = Animation.End(builder)

    builder.Finish(animation, file_identifier=b"KANI")
    return bytes(builder.Output())


def build_pak(entries: list[tuple[int, int, bytes]]) -> bytes:
    ordered = sorted(entries, key=lambda item: item[0])
    count = len(ordered)
    toc_size = count * TOC_ENTRY_SIZE
    data_start = align_up(HEADER_SIZE + toc_size, PAYLOAD_ALIGNMENT)

    toc = bytearray()
    payloads = bytearray()
    for resource_id, resource_type, payload in ordered:
        current = data_start + len(payloads)
        padding = align_up(current, PAYLOAD_ALIGNMENT) - current
        payloads.extend(b"\x00" * padding)
        offset = current + padding
        crc = zlib.crc32(payload) & 0xFFFFFFFF
        toc.extend(
            struct.pack(
                ENTRY_FORMAT,
                resource_id,
                resource_type,
                0,
                offset,
                len(payload),
                crc,
                0,
            )
        )
        payloads.extend(payload)

    toc_crc = zlib.crc32(bytes(toc)) & 0xFFFFFFFF
    header = struct.pack("<4sHHII", b"KPAK", 1, 0, count, toc_crc)
    padding_to_data = b"\x00" * (data_start - HEADER_SIZE - toc_size)
    return header + bytes(toc) + padding_to_data + bytes(payloads)


def entry_fields(data: bytearray, index: int) -> tuple[int, ...]:
    return struct.unpack_from(ENTRY_FORMAT, data, HEADER_SIZE + index * TOC_ENTRY_SIZE)


def set_entry_field(data: bytearray, index: int, field: str, value: int) -> None:
    offsets = {
        "id": (0, "<Q"),
        "type": (8, "<I"),
        "flags": (12, "<I"),
        "offset": (16, "<Q"),
        "size": (24, "<Q"),
        "crc": (32, "<I"),
        "reserved": (36, "<I"),
    }
    relative, fmt = offsets[field]
    struct.pack_into(fmt, data, HEADER_SIZE + index * TOC_ENTRY_SIZE + relative, value)


def recompute_toc_crc(data: bytearray) -> None:
    count = struct.unpack_from("<I", data, 8)[0]
    toc = bytes(data[HEADER_SIZE : HEADER_SIZE + count * TOC_ENTRY_SIZE])
    struct.pack_into("<I", data, 12, zlib.crc32(toc) & 0xFFFFFFFF)


def write_variant(output_dir: Path, name: str, data: bytes) -> None:
    (output_dir / name).write_bytes(data)


def overwrite_payload(data: bytearray, index: int, replacement: bytes) -> None:
    fields = entry_fields(data, index)
    offset = fields[3]
    size = fields[4]
    if len(replacement) != size:
        raise ValueError("El reemplazo debe conservar el tamaño del payload")
    data[offset : offset + size] = replacement
    set_entry_field(data, index, "crc", zlib.crc32(replacement) & 0xFFFFFFFF)


def make_variants(output_dir: Path, valid: bytes, sprite_png: bytes) -> None:
    write_variant(output_dir, "empty.pak", struct.pack("<4sHHII", b"KPAK", 1, 0, 0, 0))

    data = bytearray(valid)
    data[0] = ord("X")
    write_variant(output_dir, "bad_signature.pak", data)

    data = bytearray(valid)
    struct.pack_into("<H", data, 4, 2)
    write_variant(output_dir, "bad_version.pak", data)

    data = bytearray(valid)
    struct.pack_into("<I", data, 8, 104858)
    write_variant(output_dir, "bad_count.pak", data)

    data = bytearray(valid)
    data[12] ^= 0xFF
    write_variant(output_dir, "bad_toc_crc.pak", data)

    write_variant(output_dir, "truncated.pak", valid[: HEADER_SIZE + TOC_ENTRY_SIZE])
    write_variant(output_dir, "truncated_payload.pak", valid[:-1])

    data = bytearray(valid)
    set_entry_field(data, 0, "id", 0)
    recompute_toc_crc(data)
    write_variant(output_dir, "bad_id_zero.pak", data)

    data = bytearray(valid)
    first_id = entry_fields(data, 0)[0]
    set_entry_field(data, 1, "id", first_id)
    recompute_toc_crc(data)
    write_variant(output_dir, "duplicate_id.pak", data)

    data = bytearray(valid)
    set_entry_field(data, 1, "id", 1)
    recompute_toc_crc(data)
    write_variant(output_dir, "unsorted_id.pak", data)

    data = bytearray(valid)
    set_entry_field(data, 0, "type", 4)
    recompute_toc_crc(data)
    write_variant(output_dir, "bad_type.pak", data)

    data = bytearray(valid)
    set_entry_field(data, 0, "flags", 1)
    recompute_toc_crc(data)
    write_variant(output_dir, "bad_flags.pak", data)

    data = bytearray(valid)
    set_entry_field(data, 0, "reserved", 1)
    recompute_toc_crc(data)
    write_variant(output_dir, "bad_reserved.pak", data)

    data = bytearray(valid)
    offset = entry_fields(data, 0)[3]
    set_entry_field(data, 0, "offset", offset + 1)
    recompute_toc_crc(data)
    write_variant(output_dir, "unaligned_offset.pak", data)

    data = bytearray(valid)
    set_entry_field(data, 0, "offset", HEADER_SIZE)
    recompute_toc_crc(data)
    write_variant(output_dir, "offset_in_toc.pak", data)

    data = bytearray(valid)
    set_entry_field(data, 0, "offset", align_up(len(valid), PAYLOAD_ALIGNMENT))
    set_entry_field(data, 0, "size", 1)
    recompute_toc_crc(data)
    write_variant(output_dir, "range_outside.pak", data)

    data = bytearray(valid)
    first_offset = entry_fields(data, 0)[3]
    set_entry_field(data, 1, "offset", first_offset)
    recompute_toc_crc(data)
    write_variant(output_dir, "overlap.pak", data)

    data = bytearray(valid)
    set_entry_field(data, 0, "size", 0)
    recompute_toc_crc(data)
    write_variant(output_dir, "zero_size.pak", data)

    data = bytearray(valid)
    set_entry_field(data, 0, "size", 64 * 1024 * 1024 + 1)
    recompute_toc_crc(data)
    write_variant(output_dir, "size_over_limit.pak", data)

    data = bytearray(valid)
    crc = entry_fields(data, 0)[5]
    set_entry_field(data, 0, "crc", crc ^ 1)
    recompute_toc_crc(data)
    write_variant(output_dir, "bad_resource_crc.pak", data)

    data = bytearray(valid)
    overwrite_payload(data, 0, b"\x00" * entry_fields(data, 0)[4])
    recompute_toc_crc(data)
    write_variant(output_dir, "bad_level_buffer.pak", data)

    data = bytearray(valid)
    overwrite_payload(data, 1, b"\x00" * entry_fields(data, 1)[4])
    recompute_toc_crc(data)
    write_variant(output_dir, "bad_png.pak", data)

    data = bytearray(valid)
    overwrite_payload(data, 3, b"\x00" * entry_fields(data, 3)[4])
    recompute_toc_crc(data)
    write_variant(output_dir, "bad_animation_buffer.pak", data)

    # Conserva el PNG original para comparaciones directas en el test C++.
    (output_dir / "sprite.png").write_bytes(sprite_png)


def main() -> int:
    if len(sys.argv) != 2:
        print("Uso: generate_runtime_fixture.py <directorio_salida>", file=sys.stderr)
        return 2

    output_dir = Path(sys.argv[1])
    output_dir.mkdir(parents=True, exist_ok=True)

    level_buffer = build_level_buffer()
    animation_buffer = build_animation_buffer()
    sprite_png = make_sprite_png()
    large_png = make_large_png()

    (output_dir / "level.bin").write_bytes(level_buffer)
    (output_dir / "animation.bin").write_bytes(animation_buffer)

    valid = build_pak(
        [
            (LEVEL_ID, 1, level_buffer),
            (SPRITE_ID, 3, sprite_png),
            (LARGE_PNG_ID, 3, large_png),
            (ANIMATION_ID, 2, animation_buffer),
        ]
    )
    (output_dir / "runtime_test.pak").write_bytes(valid)
    (output_dir / "large.png").write_bytes(large_png)
    make_variants(output_dir, valid, sprite_png)

    print(f"Fixture T2 generado en {output_dir}")
    print(f"  runtime_test.pak: {len(valid)} bytes")
    print(f"  level.bin: {len(level_buffer)} bytes")
    print(f"  sprite.png: {len(sprite_png)} bytes")
    print(f"  large.png: {len(large_png)} bytes")
    print(f"  animation.bin: {len(animation_buffer)} bytes")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
