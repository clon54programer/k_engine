// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-identifier-naming,readability-identifier-length,bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <cstdint>

#include "editor/document.h"
#include "editor/tile_tool.h"

namespace
{

k_engine::editor::Level MakeLevel()
{
    k_engine::editor::Level level;
    level.id = 2;
    level.name = "Nivel";
    level.grid.tile_width = 32;
    level.grid.tile_height = 32;
    level.grid.columns = 4;
    level.grid.rows = 3;

    k_engine::editor::Tileset tileset;
    tileset.id = 10;
    tileset.image_id = 1;
    tileset.tile_width = 32;
    tileset.tile_height = 32;
    tileset.columns = 2;
    tileset.rows = 2;
    level.tilesets.push_back(tileset);

    k_engine::editor::Layer layer;
    layer.id = 11;
    layer.name = "fondo";
    layer.kind = k_engine::editor::LayerKind::TILE;
    layer.tileset_id = 10;
    layer.tiles.assign(12, 0);
    level.layers.push_back(layer);
    return level;
}

} // namespace

TEST_CASE("PaintTile coloca y borra un tile")
{
    auto level = MakeLevel();
    CHECK(k_engine::editor::PaintTile(level, 0, 1, 1, 4));
    CHECK(level.layers[0].tiles[5] == 4);
    CHECK(k_engine::editor::EraseTile(level, 0, 1, 1));
    CHECK(level.layers[0].tiles[5] == 0);
}

TEST_CASE("PaintTile rechaza coordenadas e índices fuera de rango")
{
    auto level = MakeLevel();
    CHECK_FALSE(k_engine::editor::PaintTile(level, 0, 4, 0, 1));
    CHECK_FALSE(k_engine::editor::PaintTile(level, 0, 0, 3, 1));
    CHECK_FALSE(k_engine::editor::PaintTile(level, 0, 0, 0, 5));
    CHECK_FALSE(k_engine::editor::PaintTile(level, 3, 0, 0, 1));
}

TEST_CASE("PickTile convierte una celda del tileset en índice 1-based")
{
    k_engine::editor::Tileset tileset;
    tileset.columns = 4;
    tileset.rows = 3;
    std::uint32_t tile = 0;
    CHECK(k_engine::editor::PickTile(tileset, 0, 0, tile));
    CHECK(tile == 1);
    CHECK(k_engine::editor::PickTile(tileset, 3, 2, tile));
    CHECK(tile == 12);
    CHECK_FALSE(k_engine::editor::PickTile(tileset, 4, 0, tile));
}

TEST_CASE("ResizeGrid conserva la intersección y descarta el resto")
{
    auto level = MakeLevel();
    level.layers[0].tiles[0] = 1;
    level.layers[0].tiles[5] = 2;
    level.layers[0].tiles[11] = 3;

    REQUIRE(k_engine::editor::ResizeGrid(level, 2, 2));
    CHECK(level.grid.columns == 2);
    CHECK(level.grid.rows == 2);
    CHECK(level.layers[0].tiles.size() == 4);
    CHECK(level.layers[0].tiles[0] == 1);
    CHECK(level.layers[0].tiles[3] == 2);
}

TEST_CASE("ResizeGrid rechaza dimensiones inválidas")
{
    auto level = MakeLevel();
    CHECK_FALSE(k_engine::editor::ResizeGrid(level, 0, 2));
    CHECK_FALSE(k_engine::editor::ResizeGrid(level, 2, 0));
    CHECK_FALSE(k_engine::editor::ResizeGrid(level, 4096, 4096));
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-identifier-naming,readability-identifier-length,bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)
