#include "runtime/content_loader.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <flatbuffers/verifier.h>
#include <stb_image.h>

#include "animation_generated.h"
#include "level_generated.h"

namespace k_engine::runtime
{

namespace
{

constexpr std::uint32_t SCHEMA_VERSION = 1;
constexpr std::uint32_t MAX_IMAGE_DIMENSION = 4096;
constexpr std::uint64_t MAX_DECODED_IMAGE_BYTES = 64ULL * 1024ULL * 1024ULL;
constexpr int CHANNELS_IN_RGBA = 4;

bool Fail(std::string message, std::string &error)
{
    error = std::move(message);
    return false;
}

const resource::PakEntry *FindLevelEntry(const resource::PakReader &pak,
                                         std::uint64_t level_id)
{
    const auto &entries = pak.Entries();
    if (level_id != 0)
    {
        auto iterator = std::find_if(
            entries.begin(), entries.end(),
            [level_id](const resource::PakEntry &entry) { return entry.id == level_id; });
        if (iterator == entries.end() ||
            iterator->type != resource::PakResourceType::LEVEL)
        {
            return nullptr;
        }
        return &(*iterator);
    }

    auto iterator =
        std::find_if(entries.begin(), entries.end(), [](const resource::PakEntry &entry) {
            return entry.type == resource::PakResourceType::LEVEL;
        });
    if (iterator == entries.end())
    {
        return nullptr;
    }

    return &(*iterator);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
bool FindSpriteReference(const schema::Level &level, std::uint64_t &image_id,
                         std::int32_t &position_x, std::int32_t &position_y)
{
    const auto *const ENTITIES = level.entities();
    if (ENTITIES != nullptr)
    {
        for (std::uint32_t index = 0; index < ENTITIES->size(); ++index)
        {
            const auto *const ENTITY = ENTITIES->Get(index);
            if (ENTITY != nullptr && ENTITY->image_id() != 0)
            {
                image_id = ENTITY->image_id();
                position_x = ENTITY->x();
                position_y = ENTITY->y();
                return true;
            }
        }
    }

    const auto *const TILESETS = level.tilesets();
    if (TILESETS != nullptr)
    {
        for (std::uint32_t index = 0; index < TILESETS->size(); ++index)
        {
            const auto *const TILESET = TILESETS->Get(index);
            if (TILESET != nullptr && TILESET->image_id() != 0)
            {
                image_id = TILESET->image_id();
                position_x = 0;
                position_y = 0;
                return true;
            }
        }
    }

    return false;
}

std::string StbFailureReason()
{
    const char *const REASON = stbi_failure_reason();
    return REASON != nullptr ? std::string(REASON) : std::string("sin detalle");
}

} // namespace

bool ContentLoader::Load(const resource::PakReader &pak, std::uint64_t level_id,
                         LoadedContent &out, std::string &error)
{
    out = LoadedContent{};
    error.clear();

    const resource::PakEntry *const LEVEL_ENTRY = FindLevelEntry(pak, level_id);
    if (LEVEL_ENTRY == nullptr)
    {
        if (level_id == 0)
        {
            return Fail("El paquete " + pak.Path() + " no contiene ningún nivel.", error);
        }
        return Fail("No existe el nivel " + std::to_string(level_id) + " en " +
                        pak.Path() + ".",
                    error);
    }

    std::vector<std::uint8_t> level_data;
    if (!pak.Read(LEVEL_ENTRY->id, resource::PakResourceType::LEVEL, level_data))
    {
        return Fail(pak.LastError(), error);
    }

    flatbuffers::Verifier verifier(level_data.data(), level_data.size());
    if (!schema::VerifyLevelBuffer(verifier))
    {
        return Fail("El buffer Level del nivel " + std::to_string(LEVEL_ENTRY->id) +
                        " no pasó la verificación FlatBuffers en " + pak.Path() + ".",
                    error);
    }

    if (!schema::LevelBufferHasIdentifier(level_data.data()))
    {
        return Fail("El buffer del nivel " + std::to_string(LEVEL_ENTRY->id) +
                        " no tiene el identificador KLVL en " + pak.Path() + ".",
                    error);
    }

    const auto *const LEVEL = schema::GetLevel(level_data.data());
    if (LEVEL == nullptr)
    {
        return Fail("No se pudo leer el nivel " + std::to_string(LEVEL_ENTRY->id) +
                        " en " + pak.Path() + ".",
                    error);
    }

    if (LEVEL->schema_version() != SCHEMA_VERSION)
    {
        return Fail("Versión de esquema no soportada en el nivel " +
                        std::to_string(LEVEL_ENTRY->id) + " de " + pak.Path() + ".",
                    error);
    }

    if (LEVEL->id() != LEVEL_ENTRY->id)
    {
        return Fail("El ID interno del nivel no coincide con la tabla de " + pak.Path() +
                        ".",
                    error);
    }

    if (LEVEL->grid() == nullptr || LEVEL->tilesets() == nullptr ||
        LEVEL->tilesets()->empty() || LEVEL->layers() == nullptr ||
        LEVEL->layers()->empty())
    {
        return Fail("El nivel " + std::to_string(LEVEL->id()) +
                        " no contiene cuadrícula, tilesets o capas en " + pak.Path() +
                        ".",
                    error);
    }

    std::uint64_t image_id = 0;
    std::int32_t sprite_x = 0;
    std::int32_t sprite_y = 0;
    if (!FindSpriteReference(*LEVEL, image_id, sprite_x, sprite_y))
    {
        return Fail("El nivel " + std::to_string(LEVEL->id()) +
                        " no contiene ninguna entidad o tileset con imagen en " +
                        pak.Path() + ".",
                    error);
    }

    std::vector<std::uint8_t> png_data;
    if (!pak.Read(image_id, resource::PakResourceType::PNG, png_data))
    {
        return Fail(pak.LastError(), error);
    }

    int width = 0;
    int height = 0;
    int components = 0;
    if (stbi_info_from_memory(png_data.data(), static_cast<int>(png_data.size()), &width,
                              &height, &components) == 0)
    {
        return Fail("El PNG " + std::to_string(image_id) + " de " + pak.Path() +
                        " no es válido: " + StbFailureReason() + ".",
                    error);
    }

    if (components < 1 || components > CHANNELS_IN_RGBA)
    {
        return Fail("El PNG " + std::to_string(image_id) + " de " + pak.Path() +
                        " tiene un número de canales no soportado.",
                    error);
    }

    if (width <= 0 || height <= 0 ||
        static_cast<std::uint32_t>(width) > MAX_IMAGE_DIMENSION ||
        static_cast<std::uint32_t>(height) > MAX_IMAGE_DIMENSION)
    {
        return Fail("El PNG " + std::to_string(image_id) + " de " + pak.Path() +
                        " tiene dimensiones fuera de límite (" + std::to_string(width) +
                        "x" + std::to_string(height) + ").",
                    error);
    }

    const std::uint64_t DECODED_BYTES = static_cast<std::uint64_t>(width) *
                                        static_cast<std::uint64_t>(height) *
                                        static_cast<std::uint64_t>(CHANNELS_IN_RGBA);
    if (DECODED_BYTES > MAX_DECODED_IMAGE_BYTES)
    {
        return Fail("El PNG " + std::to_string(image_id) + " de " + pak.Path() +
                        " supera el límite de memoria decodificada.",
                    error);
    }

    stbi_uc *const PIXELS =
        stbi_load_from_memory(png_data.data(), static_cast<int>(png_data.size()), &width,
                              &height, &components, CHANNELS_IN_RGBA);
    if (PIXELS == nullptr)
    {
        return Fail("No se pudo decodificar el PNG " + std::to_string(image_id) + " de " +
                        pak.Path() + ": " + StbFailureReason() + ".",
                    error);
    }

    out.level_id = LEVEL->id();
    out.level_name = LEVEL->name() != nullptr ? LEVEL->name()->str() : std::string();
    out.sprite.image_id = image_id;
    out.sprite.width = static_cast<std::uint32_t>(width);
    out.sprite.height = static_cast<std::uint32_t>(height);
    out.sprite.rgba.resize(static_cast<std::size_t>(DECODED_BYTES));
    std::copy_n(PIXELS, static_cast<std::size_t>(DECODED_BYTES), out.sprite.rgba.begin());
    stbi_image_free(PIXELS);

    out.sprite_x = sprite_x;
    out.sprite_y = sprite_y;
    return true;
}

} // namespace k_engine::runtime
