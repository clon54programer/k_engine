// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-identifier-naming,readability-identifier-length,bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include <nlohmann/json.hpp>

#include "editor/document.h"
#include "editor/project_io.h"
#include "editor/tile_tool.h"
#include "editor/validation.h"
#include "test_png.h"

namespace
{

class TempDirectory
{
  public:
    TempDirectory()
    {
        static std::uint64_t counter = 0;
        const auto STAMP = std::chrono::steady_clock::now().time_since_epoch().count();
        _path = std::filesystem::temp_directory_path() /
                ("k_engine_editor_test_" + std::to_string(STAMP) + "_" +
                 std::to_string(counter++));
        std::error_code error;
        std::filesystem::remove_all(_path, error);
        std::filesystem::create_directories(_path, error);
    }

    ~TempDirectory()
    {
        std::error_code error;
        std::filesystem::remove_all(_path, error);
    }

    TempDirectory(const TempDirectory &) = delete;
    TempDirectory &operator=(const TempDirectory &) = delete;
    TempDirectory(TempDirectory &&) = delete;
    TempDirectory &operator=(TempDirectory &&) = delete;

    [[nodiscard]] const std::filesystem::path &Path() const noexcept
    {
        return _path;
    }

  private:
    std::filesystem::path _path;
};

bool WriteBytes(const std::filesystem::path &path, const std::uint8_t *data,
                std::size_t size)
{
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file)
    {
        return false;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    file.write(reinterpret_cast<const char *>(data), static_cast<std::streamsize>(size));
    return file.good();
}

bool WriteText(const std::filesystem::path &path, const std::string &text)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file)
    {
        return false;
    }
    file << text;
    return file.good();
}

std::string ReadText(const std::filesystem::path &path)
{
    std::ifstream file(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

struct ProjectFixture
{
    TempDirectory directory;
    std::filesystem::path project_directory;
    std::filesystem::path source_png;
    k_engine::editor::ProjectBundle bundle;
    std::uint64_t asset_id = 0;
    std::uint64_t level_id = 0;
    std::uint64_t animation_id = 0;
};

bool BuildValidProject(ProjectFixture &fixture, std::string &error)
{
    fixture.project_directory = fixture.directory.Path() / "project";
    fixture.source_png = fixture.directory.Path() / "source.png";
    if (!WriteBytes(fixture.source_png, k_engine::editor::test::TEST_PNG.data(),
                    k_engine::editor::test::TEST_PNG.size()))
    {
        error = "No se pudo escribir el PNG de prueba.";
        return false;
    }

    if (!k_engine::editor::ProjectIo::Create(fixture.project_directory, fixture.bundle,
                                             error))
    {
        return false;
    }
    if (!k_engine::editor::ProjectIo::ImportPng(fixture.bundle, fixture.source_png,
                                                fixture.asset_id, error))
    {
        return false;
    }
    if (!k_engine::editor::ProjectIo::CreateLevel(fixture.bundle, fixture.asset_id,
                                                  fixture.level_id, error))
    {
        return false;
    }
    if (!k_engine::editor::ProjectIo::CreateAnimation(fixture.bundle, fixture.asset_id,
                                                      fixture.animation_id, error))
    {
        return false;
    }

    auto *const ANIMATION =
        k_engine::editor::FindAnimation(fixture.bundle, fixture.animation_id);
    if (ANIMATION == nullptr)
    {
        error = "No se encontró la animación creada.";
        return false;
    }
    ANIMATION->data.frames.push_back(k_engine::editor::Frame{0, 0, 32, 32, 0});
    ANIMATION->data.frames.push_back(k_engine::editor::Frame{32, 0, 32, 32, 0});
    ANIMATION->data.frames.push_back(k_engine::editor::Frame{0, 32, 32, 32, 0});
    ANIMATION->data.frames.push_back(k_engine::editor::Frame{32, 32, 32, 32, 0});
    return true;
}

} // namespace

TEST_CASE("Proyecto completo se guarda y se vuelve a cargar sin pérdida")
{
    ProjectFixture fixture;
    std::string error;
    REQUIRE(BuildValidProject(fixture, error));

    const auto ORIGINAL_LEVEL = fixture.bundle.levels.front().data;
    const auto ORIGINAL_ANIMATION = fixture.bundle.animations.front().data;

    REQUIRE(k_engine::editor::ProjectIo::Save(fixture.bundle, error));
    CHECK_FALSE(fixture.bundle.dirty);
    CHECK(std::filesystem::is_regular_file(fixture.bundle.project_path));

    k_engine::editor::ProjectBundle loaded;
    REQUIRE(
        k_engine::editor::ProjectIo::Load(fixture.bundle.project_path, loaded, error));

    CHECK(loaded.project.name == fixture.bundle.project.name);
    CHECK(loaded.project.entry_level_id == fixture.level_id);
    CHECK(loaded.levels.size() == 1);
    CHECK(loaded.animations.size() == 1);
    CHECK(loaded.levels.front().data == ORIGINAL_LEVEL);
    CHECK(loaded.animations.front().data == ORIGINAL_ANIMATION);
    CHECK_FALSE(loaded.dirty);
}

TEST_CASE("El JSON guardado contiene el esquema esperado")
{
    ProjectFixture fixture;
    std::string error;
    REQUIRE(BuildValidProject(fixture, error));
    REQUIRE(k_engine::editor::ProjectIo::Save(fixture.bundle, error));

    std::ifstream file(fixture.bundle.project_path);
    REQUIRE(file.good());
    nlohmann::json project;
    file >> project;
    CHECK(project.at("schema_version") == "1.0");
    CHECK(project.at("assets").size() == 1);
    CHECK(project.at("levels").size() == 1);
    CHECK(project.at("animations").size() == 1);

    const auto LEVEL_PATH =
        fixture.bundle.base_directory / fixture.bundle.levels.front().relative_path;
    std::ifstream level_file(LEVEL_PATH);
    REQUIRE(level_file.good());
    nlohmann::json level;
    level_file >> level;
    CHECK(level.at("layers").size() == 2);
    CHECK(level.at("grid").at("columns") == 32);
    CHECK(level.at("tilesets").size() == 1);
}

TEST_CASE("Load rechaza campos JSON desconocidos")
{
    ProjectFixture fixture;
    std::string error;
    REQUIRE(BuildValidProject(fixture, error));
    REQUIRE(k_engine::editor::ProjectIo::Save(fixture.bundle, error));

    std::ifstream file(fixture.bundle.project_path);
    nlohmann::json project;
    file >> project;
    file.close();
    project["campo_extra"] = true;
    REQUIRE(WriteText(fixture.bundle.project_path, project.dump(2)));

    k_engine::editor::ProjectBundle loaded;
    CHECK_FALSE(
        k_engine::editor::ProjectIo::Load(fixture.bundle.project_path, loaded, error));
    CHECK_FALSE(error.empty());
}

TEST_CASE("Load rechaza rutas con traversal")
{
    ProjectFixture fixture;
    std::string error;
    REQUIRE(BuildValidProject(fixture, error));
    REQUIRE(k_engine::editor::ProjectIo::Save(fixture.bundle, error));

    std::ifstream file(fixture.bundle.project_path);
    nlohmann::json project;
    file >> project;
    file.close();
    project["assets"][0]["path"] = "../escape.png";
    REQUIRE(WriteText(fixture.bundle.project_path, project.dump(2)));

    k_engine::editor::ProjectBundle loaded;
    CHECK_FALSE(
        k_engine::editor::ProjectIo::Load(fixture.bundle.project_path, loaded, error));
}

TEST_CASE("Load rechaza un asset ausente")
{
    ProjectFixture fixture;
    std::string error;
    REQUIRE(BuildValidProject(fixture, error));
    REQUIRE(k_engine::editor::ProjectIo::Save(fixture.bundle, error));
    std::filesystem::remove(fixture.bundle.base_directory /
                            fixture.bundle.project.assets.front().path);

    k_engine::editor::ProjectBundle loaded;
    CHECK_FALSE(
        k_engine::editor::ProjectIo::Load(fixture.bundle.project_path, loaded, error));
}

TEST_CASE("Save no destruye el proyecto anterior si la validación falla")
{
    ProjectFixture fixture;
    std::string error;
    REQUIRE(BuildValidProject(fixture, error));
    REQUIRE(k_engine::editor::ProjectIo::Save(fixture.bundle, error));

    const std::string ORIGINAL = ReadText(fixture.bundle.project_path);
    fixture.bundle.animations.front().data.frames.front().width = 0;
    CHECK_FALSE(k_engine::editor::ProjectIo::Save(fixture.bundle, error));
    CHECK(ReadText(fixture.bundle.project_path) == ORIGINAL);
}

TEST_CASE("ImportPng copia un archivo externo al proyecto")
{
    ProjectFixture fixture;
    std::string error;
    REQUIRE(BuildValidProject(fixture, error));
    CHECK(fixture.bundle.project.assets.size() == 1);
    CHECK(std::filesystem::is_regular_file(fixture.bundle.base_directory /
                                           fixture.bundle.project.assets.front().path));
    CHECK(fixture.bundle.project.assets.front().path.rfind("assets/", 0) == 0);
}

TEST_CASE("CreateLevel crea dos capas y un tileset")
{
    ProjectFixture fixture;
    std::string error;
    REQUIRE(BuildValidProject(fixture, error));
    REQUIRE(fixture.bundle.levels.size() == 1);
    const auto &LEVEL = fixture.bundle.levels.front().data;
    CHECK(LEVEL.tilesets.size() == 1);
    CHECK(LEVEL.layers.size() == 2);
    CHECK(LEVEL.layers.front().kind == k_engine::editor::LayerKind::TILE);
    CHECK(LEVEL.layers.back().kind == k_engine::editor::LayerKind::COLLISION);
    CHECK(LEVEL.layers.front().tiles.size() ==
          static_cast<std::size_t>(LEVEL.grid.columns) * LEVEL.grid.rows);
}

TEST_CASE("CreateAnimation crea un clip con metadatos")
{
    ProjectFixture fixture;
    std::string error;
    REQUIRE(BuildValidProject(fixture, error));
    const auto &ANIMATION = fixture.bundle.animations.front().data;
    CHECK(ANIMATION.image_id == fixture.asset_id);
    CHECK(ANIMATION.fps == 8);
    CHECK(ANIMATION.loop);
    CHECK(ANIMATION.frames.size() == 4);
}

TEST_CASE("Flujo T3: editar capas y fotogramas, guardar y reabrir")
{
    ProjectFixture fixture;
    std::string error;
    REQUIRE(BuildValidProject(fixture, error));

    auto &LEVEL = fixture.bundle.levels.front().data;
    REQUIRE(k_engine::editor::PaintTile(LEVEL, 0, 1, 1, 4));
    REQUIRE(k_engine::editor::PaintTile(LEVEL, 1, 2, 2, 2));
    fixture.bundle.levels.front().data.layers[0].tiles[1] = 3;
    fixture.bundle.dirty = true;

    REQUIRE(k_engine::editor::ProjectIo::Save(fixture.bundle, error));

    k_engine::editor::ProjectBundle loaded;
    REQUIRE(
        k_engine::editor::ProjectIo::Load(fixture.bundle.project_path, loaded, error));
    const auto &LOADED_LEVEL = loaded.levels.front().data;
    CHECK(LOADED_LEVEL.layers[0].tiles[1] == 3);
    CHECK(LOADED_LEVEL.layers[0].tiles[33] == 4);
    CHECK(LOADED_LEVEL.layers[1].tiles[66] == 2);
    CHECK(loaded.animations.front().data.frames.size() == 4);
    CHECK(loaded.animations.front().data.frames[0] ==
          fixture.bundle.animations.front().data.frames[0]);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-identifier-naming,readability-identifier-length,bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)
