#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "level_generated.h"

namespace
{

constexpr std::uint32_t SCHEMA_VERSION = 1;
constexpr std::uint64_t LEVEL_ID = 42;
constexpr std::uint32_t GRID_TILE_WIDTH = 32;
constexpr std::uint32_t GRID_TILE_HEIGHT = 32;
constexpr std::uint32_t GRID_COLUMNS = 4;
constexpr std::uint32_t GRID_ROWS = 3;
constexpr std::uint64_t TILESET_ID = 10;
constexpr std::uint64_t TILESET_IMAGE_ID = 1;
constexpr std::uint32_t TILESET_TILE_WIDTH = 32;
constexpr std::uint32_t TILESET_TILE_HEIGHT = 32;
constexpr std::uint32_t TILESET_COLUMNS = 16;
constexpr std::uint32_t TILESET_ROWS = 16;
constexpr std::uint64_t LAYER_ID = 11;
constexpr std::uint32_t TILE_COUNT = 12;
constexpr std::array<std::uint32_t, TILE_COUNT> EXPECTED_TILES = {1, 1, 1, 0, 1, 1,
                                                                  0, 0, 1, 0, 0, 1};

bool Check(bool condition, const char *message)
{
    if (!condition)
    {
        std::cerr << "Fallo: " << message << '\n';
        return false;
    }
    return true;
}

bool ReadFile(const std::string &path, std::vector<std::uint8_t> &data)
{
    std::ifstream input(path, std::ios::binary);
    if (!input)
    {
        std::cerr << "No se pudo abrir el buffer: " << path << '\n';
        return false;
    }

    data.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    if (data.empty())
    {
        std::cerr << "El buffer está vacío: " << path << '\n';
        return false;
    }

    return true;
}

bool CheckGrid(const k_engine::schema::Level &level)
{
    const auto *grid = level.grid();
    if (!Check(grid != nullptr, "falta la cuadrícula"))
    {
        return false;
    }

    return Check(grid->tile_width() == GRID_TILE_WIDTH, "tile_width inesperado") &&
           Check(grid->tile_height() == GRID_TILE_HEIGHT, "tile_height inesperado") &&
           Check(grid->columns() == GRID_COLUMNS, "columns inesperado") &&
           Check(grid->rows() == GRID_ROWS, "rows inesperado");
}

bool CheckTileset(const k_engine::schema::Level &level)
{
    const auto *tilesets = level.tilesets();
    if (!Check(tilesets != nullptr && tilesets->size() == 1,
               "cantidad de tilesets inesperada"))
    {
        return false;
    }

    const auto *tileset = tilesets->Get(0);
    if (!Check(tileset != nullptr, "el tileset es null"))
    {
        return false;
    }

    return Check(tileset->id() == TILESET_ID, "id de tileset inesperado") &&
           Check(tileset->image_id() == TILESET_IMAGE_ID, "image_id inesperado") &&
           Check(tileset->tile_width() == TILESET_TILE_WIDTH,
                 "tile_width de tileset inesperado") &&
           Check(tileset->tile_height() == TILESET_TILE_HEIGHT,
                 "tile_height de tileset inesperado") &&
           Check(tileset->columns() == TILESET_COLUMNS,
                 "columns de tileset inesperado") &&
           Check(tileset->rows() == TILESET_ROWS, "rows de tileset inesperado");
}

bool CheckTiles(const k_engine::schema::Layer &layer)
{
    const auto *tiles = layer.tiles();
    if (!Check(tiles != nullptr && tiles->size() == TILE_COUNT,
               "cantidad de tiles inesperada"))
    {
        return false;
    }

    std::uint32_t index = 0;
    for (const auto EXPECTED_TILE : EXPECTED_TILES)
    {
        if (!Check(tiles->Get(index) == EXPECTED_TILE, "valor de tile inesperado"))
        {
            return false;
        }
        ++index;
    }

    return true;
}

bool CheckLayer(const k_engine::schema::Level &level)
{
    const auto *layers = level.layers();
    if (!Check(layers != nullptr && layers->size() == 1, "cantidad de capas inesperada"))
    {
        return false;
    }

    const auto *layer = layers->Get(0);
    if (!Check(layer != nullptr, "la capa es null"))
    {
        return false;
    }

    if (!Check(layer->id() == LAYER_ID, "id de capa inesperado") ||
        !Check(layer->name() != nullptr && layer->name()->str() == "background",
               "nombre de capa inesperado") ||
        !Check(layer->kind() == k_engine::schema::LayerKind_tile,
               "tipo de capa inesperado") ||
        !Check(layer->visible(), "la capa debería ser visible") ||
        !Check(layer->tileset_id() == TILESET_ID, "tileset_id de capa inesperado"))
    {
        return false;
    }

    return CheckTiles(*layer);
}

bool CheckEntities(const k_engine::schema::Level &level)
{
    const auto *entities = level.entities();
    return Check(entities == nullptr || entities->empty(),
                 "el nivel de prueba no debería tener entidades");
}

bool CheckLevel(const k_engine::schema::Level &level)
{
    if (!Check(level.schema_version() == SCHEMA_VERSION, "schema_version inesperado") ||
        !Check(level.id() == LEVEL_ID, "id de nivel inesperado") ||
        !Check(level.name() != nullptr && level.name()->str() == "test_level",
               "nombre de nivel inesperado"))
    {
        return false;
    }

    return CheckGrid(level) && CheckTileset(level) && CheckLayer(level) &&
           CheckEntities(level);
}

bool VerifyLevel(const std::vector<std::uint8_t> &data)
{
    flatbuffers::Verifier verifier(data.data(), data.size());
    if (!Check(k_engine::schema::VerifyLevelBuffer(verifier),
               "el buffer Level no pasó la verificación FlatBuffers"))
    {
        return false;
    }

    return Check(k_engine::schema::LevelBufferHasIdentifier(data.data()),
                 "el identificador del buffer no es KLVL");
}

} // namespace

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Uso: flatbuffers_roundtrip_test <archivo.bin>\n";
        return 2;
    }

    // El acceso a argv[1] es seguro después de comprobar argc y no se usa como
    // offset dentro de un búfer.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const std::string INPUT_PATH = argv[1];

    std::vector<std::uint8_t> data;
    if (!ReadFile(INPUT_PATH, data) || !VerifyLevel(data))
    {
        return 1;
    }

    const auto *level = k_engine::schema::GetLevel(data.data());
    if (!Check(level != nullptr, "GetLevel devolvió null") || !CheckLevel(*level))
    {
        return 1;
    }

    std::cout << "FlatBuffers Python -> C++: OK\n";
    return 0;
}
