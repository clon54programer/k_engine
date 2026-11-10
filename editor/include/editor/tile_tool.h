#pragma once

#include <cstddef>
#include <cstdint>

#include "editor/document.h"

namespace k_engine::editor
{

bool PaintTile(Level &level, std::size_t layer_index, std::uint32_t column,
               std::uint32_t row, std::uint32_t tile_index);
bool EraseTile(Level &level, std::size_t layer_index, std::uint32_t column,
               std::uint32_t row);
bool ResizeGrid(Level &level, std::uint32_t columns, std::uint32_t rows);
bool PickTile(const Tileset &tileset, std::uint32_t local_x, std::uint32_t local_y,
              std::uint32_t &tile_index);

} // namespace k_engine::editor
