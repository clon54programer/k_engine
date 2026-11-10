#!/usr/bin/env python3
"""Convierte dos archivos SPIR-V en una cabecera C++ con arrays de palabras."""

from __future__ import annotations

import struct
import sys
from pathlib import Path


def read_words(path: Path) -> list[int]:
    data = path.read_bytes()
    if not data or len(data) % 4 != 0:
        raise ValueError(f"SPIR-V inválido o vacío: {path}")
    return list(struct.unpack(f"<{len(data) // 4}I", data))


def format_array(name: str, words: list[int]) -> str:
    values = ", ".join(f"0x{word:08X}U" for word in words)
    return (
        f"inline constexpr std::array<std::uint32_t, {len(words)}> {name} = {{\n"
        f"    {values},\n"
        "};\n"
    )


def main() -> int:
    if len(sys.argv) != 4:
        print(
            "Uso: embed_spirv.py <cabecera_salida> <vert.spv> <frag.spv>",
            file=sys.stderr,
        )
        return 2

    output = Path(sys.argv[1])
    vertex_words = read_words(Path(sys.argv[2]))
    fragment_words = read_words(Path(sys.argv[3]))

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(
        "#pragma once\n\n"
        "#include <array>\n"
        "#include <cstdint>\n\n"
        "namespace k_engine::runtime::shaders\n"
        "{\n\n"
        + format_array("SPRITE_VERT", vertex_words)
        + "\n"
        + format_array("SPRITE_FRAG", fragment_words)
        + "\n} // namespace k_engine::runtime::shaders\n",
        encoding="utf-8",
    )
    print(f"Cabecera de shaders generada: {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
