#!/usr/bin/env python3
"""Genera un buffer Level FlatBuffers para la prueba Python -> C++."""

from pathlib import Path
import sys

import flatbuffers

from k_engine.schema import Grid, Layer, Level, Tileset
from k_engine.schema.LayerKind import LayerKind


def build_level_buffer() -> bytes:
    """Construye un nivel mínimo y devuelve sus bytes serializados."""
    builder = flatbuffers.Builder(1024)
    level_name = builder.CreateString("test_level")
    layer_name = builder.CreateString("background")

    tiles = [1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1]
    Layer.StartTilesVector(builder, len(tiles))
    for tile in reversed(tiles):
        builder.PrependUint32(tile)
    tiles_vector = builder.EndVector()

    Layer.Start(builder)
    Layer.AddId(builder, 11)
    Layer.AddName(builder, layer_name)
    Layer.AddKind(builder, LayerKind.tile)
    Layer.AddVisible(builder, True)
    Layer.AddTilesetId(builder, 10)
    Layer.AddTiles(builder, tiles_vector)
    layer = Layer.End(builder)

    Tileset.Start(builder)
    Tileset.AddId(builder, 10)
    Tileset.AddImageId(builder, 1)
    Tileset.AddTileWidth(builder, 32)
    Tileset.AddTileHeight(builder, 32)
    Tileset.AddColumns(builder, 16)
    Tileset.AddRows(builder, 16)
    tileset = Tileset.End(builder)

    Level.StartTilesetsVector(builder, 1)
    builder.PrependUOffsetTRelative(tileset)
    tilesets = builder.EndVector()

    Level.StartLayersVector(builder, 1)
    builder.PrependUOffsetTRelative(layer)
    layers = builder.EndVector()

    Level.Start(builder)
    Level.AddSchemaVersion(builder, 1)
    Level.AddId(builder, 42)
    Level.AddName(builder, level_name)
    grid = Grid.CreateGrid(builder, 32, 32, 4, 3)
    Level.AddGrid(builder, grid)
    Level.AddTilesets(builder, tilesets)
    Level.AddLayers(builder, layers)
    level = Level.End(builder)

    builder.Finish(level, file_identifier=b"KLVL")
    return bytes(builder.Output())


def main() -> int:
    if len(sys.argv) != 2:
        print("Uso: generate_level_buffer.py <archivo.bin>", file=sys.stderr)
        return 2

    output = Path(sys.argv[1])
    output.write_bytes(build_level_buffer())
    print(f"Buffer Level generado: {output} ({output.stat().st_size} bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
