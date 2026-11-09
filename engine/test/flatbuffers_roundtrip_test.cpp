#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "level_generated.h"

namespace
{

bool Check(bool condition, const char *message)
{
    if (!condition)
    {
        std::cerr << "Fallo: " << message << '\n';
        return false;
    }
    return true;
}

bool ReadFile(const char *path, std::vector<std::uint8_t> &data)
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

} // namespace

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Uso: flatbuffers_roundtrip_test <archivo.bin>\n";
        return 2;
    }

    std::vector<std::uint8_t> data;
    if (!ReadFile(argv[1], data))
    {
        return 1;
    }

    flatbuffers::Verifier verifier(data.data(), data.size());
    if (!Check(k_engine::schema::VerifyLevelBuffer(verifier),
               "el buffer Level no pasó la verificación FlatBuffers"))
    {
        return 1;
    }

    if (!Check(k_engine::schema::LevelBufferHasIdentifier(data.data()),
               "el identificador del buffer no es KLVL"))
    {
        return 1;
    }

    const auto *level = k_engine::schema::GetLevel(data.data());
    if (!Check(level != nullptr, "GetLevel devolvió null"))
    {
        return 1;
    }

    if (!Check(level->schema_version() == 1, "schema_version inesperado") ||
        !Check(level->id() == 42, "id de nivel inesperado") ||
        !Check(level->name() != nullptr && level->name()->str() == "test_level",
               "nombre de nivel inesperado") ||
        !Check(level->grid() != nullptr, "falta la cuadrícula"))
    {
        return 1;
    }

    const auto *grid = level->grid();
    if (!Check(grid->tile_width() == 32, "tile_width inesperado") ||
        !Check(grid->tile_height() == 32, "tile_height inesperado") ||
        !Check(grid->columns() == 4, "columns inesperado") ||
        !Check(grid->rows() == 3, "rows inesperado"))
    {
        return 1;
    }

    if (!Check(level->tilesets() != nullptr && level->tilesets()->size() == 1,
               "cantidad de tilesets inesperada") ||
        !Check(level->layers() != nullptr && level->layers()->size() == 1,
               "cantidad de capas inesperada"))
    {
        return 1;
    }

    const auto *tileset = level->tilesets()->Get(0);
    if (!Check(tileset != nullptr, "el tileset es null") ||
        !Check(tileset->id() == 10, "id de tileset inesperado") ||
        !Check(tileset->image_id() == 1, "image_id inesperado") ||
        !Check(tileset->tile_width() == 32, "tile_width de tileset inesperado") ||
        !Check(tileset->tile_height() == 32, "tile_height de tileset inesperado") ||
        !Check(tileset->columns() == 16, "columns de tileset inesperado") ||
        !Check(tileset->rows() == 16, "rows de tileset inesperado"))
    {
        return 1;
    }

    const auto *layer = level->layers()->Get(0);
    if (!Check(layer != nullptr, "la capa es null") ||
        !Check(layer->id() == 11, "id de capa inesperado") ||
        !Check(layer->name() != nullptr && layer->name()->str() == "background",
               "nombre de capa inesperado") ||
        !Check(layer->kind() == k_engine::schema::LayerKind_tile,
               "tipo de capa inesperado") ||
        !Check(layer->visible(), "la capa debería ser visible") ||
        !Check(layer->tileset_id() == 10, "tileset_id de capa inesperado"))
    {
        return 1;
    }

    const auto *tiles = layer->tiles();
    const std::uint32_t expected_tiles[] = {1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1};
    if (!Check(tiles != nullptr && tiles->size() == 12, "cantidad de tiles inesperada"))
    {
        return 1;
    }

    for (std::uint32_t index = 0; index < 12; ++index)
    {
        if (!Check(tiles->Get(index) == expected_tiles[index],
                   "valor de tile inesperado"))
        {
            return 1;
        }
    }

    if (!Check(level->entities() == nullptr || level->entities()->size() == 0,
               "el nivel de prueba no debería tener entidades"))
    {
        return 1;
    }

    std::cout << "FlatBuffers Python -> C++: OK\n";
    return 0;
}
