// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-identifier-naming,readability-identifier-length,bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <cstdint>
#include <string>

#include "editor/document.h"
#include "editor/validation.h"

namespace
{

k_engine::editor::Level MakeValidLevel()
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

    k_engine::editor::Layer background;
    background.id = 11;
    background.name = "fondo";
    background.kind = k_engine::editor::LayerKind::TILE;
    background.visible = true;
    background.tileset_id = 10;
    background.tiles.assign(12, 1);
    level.layers.push_back(background);

    k_engine::editor::Layer collision;
    collision.id = 12;
    collision.name = "colision";
    collision.kind = k_engine::editor::LayerKind::COLLISION;
    collision.visible = false;
    collision.tileset_id = 10;
    collision.tiles.assign(12, 0);
    level.layers.push_back(collision);
    return level;
}

k_engine::editor::Animation MakeValidAnimation()
{
    k_engine::editor::Animation animation;
    animation.id = 3;
    animation.name = "idle";
    animation.image_id = 1;
    animation.frame_width = 32;
    animation.frame_height = 32;
    animation.fps = 8;
    animation.loop = true;
    animation.pivot = k_engine::editor::Vec2i{16, 32};
    animation.hitbox = k_engine::editor::Recti{0, 0, 32, 32};
    animation.frames.push_back(k_engine::editor::Frame{0, 0, 32, 32, 0});
    animation.frames.push_back(k_engine::editor::Frame{32, 0, 32, 32, 0});
    return animation;
}

} // namespace

TEST_CASE("ParseId acepta IDs decimales")
{
    std::string error;
    std::uint64_t value = 0;
    CHECK(k_engine::editor::ParseId("42", value, error));
    CHECK(value == 42);
    CHECK(k_engine::editor::ParseId("18446744073709551615", value, error));
    CHECK(value == 18446744073709551615ULL);
}

TEST_CASE("ParseId rechaza valores inválidos")
{
    std::string error;
    std::uint64_t value = 0;
    CHECK_FALSE(k_engine::editor::ParseId("", value, error));
    CHECK_FALSE(k_engine::editor::ParseId("01", value, error));
    CHECK_FALSE(k_engine::editor::ParseId("-1", value, error));
    CHECK_FALSE(k_engine::editor::ParseId("1x", value, error));
    CHECK_FALSE(k_engine::editor::ParseId("18446744073709551616", value, error));
}

TEST_CASE("IsSafeRelativePath rechaza rutas peligrosas")
{
    std::string error;
    CHECK(k_engine::editor::IsSafeRelativePath("levels/level_1.json", error));
    CHECK(k_engine::editor::IsSafeRelativePath("assets/tiles.png", error));
    CHECK_FALSE(k_engine::editor::IsSafeRelativePath("../tiles.png", error));
    CHECK_FALSE(k_engine::editor::IsSafeRelativePath("assets/../tiles.png", error));
    CHECK_FALSE(k_engine::editor::IsSafeRelativePath("/tmp/tiles.png", error));
    CHECK_FALSE(k_engine::editor::IsSafeRelativePath("assets\\tiles.png", error));
    CHECK_FALSE(k_engine::editor::IsSafeRelativePath("assets//tiles.png", error));
}

TEST_CASE("ValidateLevel acepta un nivel mínimo válido")
{
    const auto LEVEL = MakeValidLevel();
    std::string error;
    CHECK(k_engine::editor::ValidateLevel(LEVEL, error));
    CHECK(error.empty());
}

TEST_CASE("ValidateLevel rechaza tiles con longitud incorrecta")
{
    auto level = MakeValidLevel();
    level.layers.front().tiles.pop_back();
    std::string error;
    CHECK_FALSE(k_engine::editor::ValidateLevel(level, error));
}

TEST_CASE("ValidateLevel rechaza índices fuera del tileset")
{
    auto level = MakeValidLevel();
    level.layers.front().tiles.front() = 5;
    std::string error;
    CHECK_FALSE(k_engine::editor::ValidateLevel(level, error));
}

TEST_CASE("ValidateLevel rechaza IDs locales duplicados")
{
    auto level = MakeValidLevel();
    level.layers.back().id = level.layers.front().id;
    std::string error;
    CHECK_FALSE(k_engine::editor::ValidateLevel(level, error));
}

TEST_CASE("ValidateLevel rechaza una capa sin tileset")
{
    auto level = MakeValidLevel();
    level.layers.front().tileset_id = 999;
    std::string error;
    CHECK_FALSE(k_engine::editor::ValidateLevel(level, error));
}

TEST_CASE("ValidateAnimation acepta una animación mínima válida")
{
    const auto ANIMATION = MakeValidAnimation();
    std::string error;
    CHECK(k_engine::editor::ValidateAnimation(ANIMATION, error));
}

TEST_CASE("ValidateAnimation rechaza pivote y hitbox fuera del fotograma")
{
    auto animation = MakeValidAnimation();
    animation.pivot.x = 33;
    std::string error;
    CHECK_FALSE(k_engine::editor::ValidateAnimation(animation, error));

    animation = MakeValidAnimation();
    animation.hitbox.width = 40;
    CHECK_FALSE(k_engine::editor::ValidateAnimation(animation, error));
}

TEST_CASE("ValidateAnimation exige al menos un fotograma")
{
    auto animation = MakeValidAnimation();
    animation.frames.clear();
    std::string error;
    CHECK_FALSE(k_engine::editor::ValidateAnimation(animation, error));
}

TEST_CASE("AllocateGlobalId evita reutilizar IDs")
{
    k_engine::editor::Project project;
    project.next_id = 1;
    project.assets.push_back(k_engine::editor::AssetRef{5, "assets/a.png"});
    std::string error;
    const std::uint64_t FIRST = k_engine::editor::AllocateGlobalId(project, error);
    CHECK(FIRST == 6);
    CHECK(project.next_id == 7);
    const std::uint64_t SECOND = k_engine::editor::AllocateGlobalId(project, error);
    CHECK(SECOND == 7);
}

TEST_CASE("AllocateLocalId usa el máximo local")
{
    const auto LEVEL = MakeValidLevel();
    std::string error;
    CHECK(k_engine::editor::AllocateLocalId(LEVEL, error) == 13);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-identifier-naming,readability-identifier-length,bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)
