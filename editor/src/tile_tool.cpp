#include "editor/tile_tool.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "editor/validation.h"

namespace k_engine::editor
{

namespace
{

const Tileset *FindTileset(const Level &level, std::uint64_t tileset_id) noexcept
{
    const auto ITERATOR = std::find_if(
        level.tilesets.begin(), level.tilesets.end(),
        [tileset_id](const Tileset &tileset) { return tileset.id == tileset_id; });
    return ITERATOR == level.tilesets.end() ? nullptr : &(*ITERATOR);
}

} // namespace

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
bool PaintTile(Level &level, std::size_t layer_index, std::uint32_t column,
               std::uint32_t row, // NOLINT(bugprone-easily-swappable-parameters)
               std::uint32_t tile_index)
{
    if (layer_index >= level.layers.size() || column >= level.grid.columns ||
        row >= level.grid.rows)
    {
        return false;
    }

    Layer &layer = level.layers[layer_index];
    const Tileset *const TILESET = FindTileset(level, layer.tileset_id);
    if (TILESET == nullptr)
    {
        return false;
    }

    const std::uint64_t TILE_LIMIT =
        static_cast<std::uint64_t>(TILESET->columns) * TILESET->rows;
    if (static_cast<std::uint64_t>(tile_index) > TILE_LIMIT)
    {
        return false;
    }

    const std::size_t INDEX =
        (static_cast<std::size_t>(row) * level.grid.columns) + column;
    if (INDEX >= layer.tiles.size())
    {
        return false;
    }

    layer.tiles[INDEX] = tile_index;
    return true;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
bool EraseTile(Level &level, std::size_t layer_index, std::uint32_t column,
               std::uint32_t row)
{
    return PaintTile(level, layer_index, column, row, 0);
}

bool ResizeGrid(Level &level, std::uint32_t columns, std::uint32_t rows)
{
    if (columns == 0 || columns > MAX_GRID_DIMENSION || rows == 0 ||
        rows > MAX_GRID_DIMENSION)
    {
        return false;
    }
    const std::uint64_t CELL_COUNT = static_cast<std::uint64_t>(columns) * rows;
    if (CELL_COUNT > MAX_GRID_CELLS)
    {
        return false;
    }

    const std::uint32_t OLD_COLUMNS = level.grid.columns;
    const std::uint32_t OLD_ROWS = level.grid.rows;
    const auto NEW_COUNT = static_cast<std::size_t>(CELL_COUNT);

    for (Layer &layer : level.layers)
    {
        std::vector<std::uint32_t> resized(NEW_COUNT, 0);
        const std::uint32_t COPY_COLUMNS = std::min(columns, OLD_COLUMNS);
        const std::uint32_t COPY_ROWS = std::min(rows, OLD_ROWS);
        for (std::uint32_t row = 0; row < COPY_ROWS; ++row)
        {
            for (std::uint32_t column = 0; column < COPY_COLUMNS; ++column)
            {
                const std::size_t OLD_INDEX =
                    (static_cast<std::size_t>(row) * OLD_COLUMNS) + column;
                const std::size_t NEW_INDEX =
                    (static_cast<std::size_t>(row) * columns) + column;
                if (OLD_INDEX < layer.tiles.size())
                {
                    resized[NEW_INDEX] = layer.tiles[OLD_INDEX];
                }
            }
        }
        layer.tiles = std::move(resized);
    }

    level.grid.columns = columns;
    level.grid.rows = rows;
    return true;
}

bool PickTile(const Tileset &tileset, std::uint32_t local_x, std::uint32_t local_y,
              std::uint32_t &tile_index)
{
    if (local_x >= tileset.columns || local_y >= tileset.rows)
    {
        return false;
    }

    tile_index = (local_y * tileset.columns) + local_x + 1U;
    return true;
}

} // namespace k_engine::editor
