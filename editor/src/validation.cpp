// NOLINTBEGIN(readability-identifier-naming,readability-identifier-length,readability-function-cognitive-complexity,readability-else-after-return)
#include "editor/validation.h"

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <set>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include <stb_image.h>

namespace k_engine::editor
{

namespace
{

constexpr std::string_view SCHEMA_VERSION = "1.0";
constexpr int CHANNELS_IN_RGBA = 4;

bool Fail(std::string &error, std::string message)
{
    error = std::move(message);
    return false;
}

bool CheckNonEmptyString(const std::string &value, std::string_view field,
                         std::string &error)
{
    if (value.empty())
    {
        return Fail(error, "El campo '" + std::string(field) + "' no puede estar vacío.");
    }
    if (value.size() > MAX_STRING_BYTES)
    {
        return Fail(error, "El campo '" + std::string(field) + "' supera el límite.");
    }
    return true;
}

bool CheckSchemaVersion(const std::string &version, std::string_view kind,
                        std::string &error)
{
    if (version != SCHEMA_VERSION)
    {
        return Fail(error, "Versión de esquema no soportada en " + std::string(kind) +
                               ": '" + version + "'.");
    }
    return true;
}

bool CheckUniqueId(std::set<std::uint64_t> &ids, std::uint64_t id, std::string_view kind,
                   std::string &error)
{
    if (id == 0)
    {
        return Fail(error, "El ID de " + std::string(kind) + " no puede ser cero.");
    }
    if (!ids.insert(id).second)
    {
        return Fail(error, "El ID " + std::to_string(id) + " está duplicado en " +
                               std::string(kind) + ".");
    }
    return true;
}

bool CheckGridDimension(std::uint32_t value, std::string_view field, std::string &error)
{
    if (value == 0 || value > MAX_GRID_DIMENSION)
    {
        return Fail(error,
                    "El campo '" + std::string(field) + "' debe estar entre 1 y 4096.");
    }
    return true;
}

const Tileset *FindTileset(const Level &level, std::uint64_t tileset_id) noexcept
{
    const auto ITERATOR = std::find_if(
        level.tilesets.begin(), level.tilesets.end(),
        [tileset_id](const Tileset &tileset) { return tileset.id == tileset_id; });
    return ITERATOR == level.tilesets.end() ? nullptr : &(*ITERATOR);
}

const Layer *FindLayer(const Level &level, std::uint64_t layer_id) noexcept
{
    const auto ITERATOR =
        std::find_if(level.layers.begin(), level.layers.end(),
                     [layer_id](const Layer &layer) { return layer.id == layer_id; });
    return ITERATOR == level.layers.end() ? nullptr : &(*ITERATOR);
}

} // namespace

bool ParseId(std::string_view text, std::uint64_t &value, std::string &error)
{
    if (text.empty())
    {
        return Fail(error, "El ID no puede estar vacío.");
    }
    if (text.size() > 1 && text.front() == '0')
    {
        return Fail(error, "El ID '" + std::string(text) +
                               "' no puede tener ceros a la izquierda.");
    }

    std::uint64_t parsed = 0;
    const auto RESULT = std::from_chars(text.data(), text.data() + text.size(), parsed);
    if (RESULT.ec != std::errc{} || RESULT.ptr != text.data() + text.size())
    {
        return Fail(error, "El ID '" + std::string(text) + "' no es decimal válido.");
    }

    value = parsed;
    return true;
}

std::string FormatId(std::uint64_t value)
{
    return std::to_string(value);
}

bool IsSafeRelativePath(std::string_view path, std::string &error)
{
    if (path.empty())
    {
        return Fail(error, "La ruta no puede estar vacía.");
    }
    if (path.size() > MAX_PATH_BYTES)
    {
        return Fail(error, "La ruta supera el límite de 4096 bytes.");
    }
    if (path.front() == '/' || path.find('\\') != std::string_view::npos)
    {
        return Fail(error, "La ruta debe ser relativa y usar '/' como separador.");
    }

    std::size_t start = 0;
    while (start <= path.size())
    {
        const std::size_t END = path.find('/', start);
        const std::string_view COMPONENT = path.substr(
            start, END == std::string_view::npos ? path.size() - start : END - start);
        if (COMPONENT.empty() || COMPONENT == "." || COMPONENT == "..")
        {
            return Fail(error, "La ruta contiene un componente no permitido: '" +
                                   std::string(COMPONENT) + "'.");
        }
        if (END == std::string_view::npos)
        {
            break;
        }
        start = END + 1;
    }

    return true;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
bool ReadPngInfo(const std::filesystem::path &path, std::uint32_t &width,
                 std::uint32_t &height, std::string &error)
{
    std::error_code file_error;
    if (!std::filesystem::is_regular_file(path, file_error) || file_error)
    {
        return Fail(error, "No existe el archivo PNG " + path.string() + ".");
    }

    const std::uintmax_t FILE_SIZE = std::filesystem::file_size(path, file_error);
    if (file_error || FILE_SIZE > MAX_DOCUMENT_FILE_BYTES)
    {
        return Fail(error, "El PNG " + path.string() + " supera el límite de tamaño.");
    }

    int image_width = 0;
    int image_height = 0;
    int components = 0;
    if (stbi_info(path.string().c_str(), &image_width, &image_height, &components) == 0)
    {
        const char *const REASON = stbi_failure_reason();
        return Fail(error, "El PNG " + path.string() + " no es válido: " +
                               (REASON != nullptr ? std::string(REASON)
                                                  : std::string("sin detalle")) +
                               ".");
    }

    if (image_width <= 0 || image_height <= 0 ||
        static_cast<std::uint32_t>(image_width) > MAX_IMAGE_DIMENSION ||
        static_cast<std::uint32_t>(image_height) > MAX_IMAGE_DIMENSION)
    {
        return Fail(error,
                    "El PNG " + path.string() + " tiene dimensiones fuera de límite.");
    }
    if (components < 1 || components > CHANNELS_IN_RGBA)
    {
        return Fail(error, "El PNG " + path.string() + " usa canales no soportados.");
    }
    if (static_cast<std::uint64_t>(image_width) *
            static_cast<std::uint64_t>(image_height) *
            static_cast<std::uint64_t>(CHANNELS_IN_RGBA) >
        MAX_DECODED_IMAGE_BYTES)
    {
        return Fail(error, "El PNG " + path.string() +
                               " supera el límite de memoria decodificada.");
    }

    width = static_cast<std::uint32_t>(image_width);
    height = static_cast<std::uint32_t>(image_height);
    return true;
}

bool ValidateLevel(const Level &level, std::string &error)
{
    if (!CheckSchemaVersion(level.schema_version, "el nivel", error))
    {
        return false;
    }
    if (level.id == 0)
    {
        return Fail(error, "El nivel no tiene un ID válido.");
    }
    if (!CheckNonEmptyString(level.name, "level.name", error))
    {
        return false;
    }

    const Grid &GRID = level.grid;
    if (!CheckGridDimension(GRID.tile_width, "grid.tile_width", error) ||
        !CheckGridDimension(GRID.tile_height, "grid.tile_height", error) ||
        !CheckGridDimension(GRID.columns, "grid.columns", error) ||
        !CheckGridDimension(GRID.rows, "grid.rows", error))
    {
        return false;
    }

    const std::uint64_t CELL_COUNT = static_cast<std::uint64_t>(GRID.columns) * GRID.rows;
    if (CELL_COUNT > MAX_GRID_CELLS)
    {
        return Fail(error, "La cuadrícula del nivel supera 1 048 576 celdas.");
    }

    if (level.tilesets.empty() || level.tilesets.size() > MAX_TILESETS)
    {
        return Fail(error, "El nivel debe tener entre 1 y 64 tilesets.");
    }
    if (level.layers.empty() || level.layers.size() > MAX_LAYERS)
    {
        return Fail(error, "El nivel debe tener entre 1 y 16 capas.");
    }
    if (level.entities.size() > MAX_ENTITIES)
    {
        return Fail(error, "El nivel supera el límite de 65 536 entidades.");
    }

    std::set<std::uint64_t> local_ids;
    for (const Tileset &TILESET : level.tilesets)
    {
        if (!CheckUniqueId(local_ids, TILESET.id, "tileset", error))
        {
            return false;
        }
        if (TILESET.image_id == 0)
        {
            return Fail(error, "El tileset " + std::to_string(TILESET.id) +
                                   " no referencia una imagen.");
        }
        if (!CheckGridDimension(TILESET.tile_width, "tileset.tile_width", error) ||
            !CheckGridDimension(TILESET.tile_height, "tileset.tile_height", error) ||
            !CheckGridDimension(TILESET.columns, "tileset.columns", error) ||
            !CheckGridDimension(TILESET.rows, "tileset.rows", error))
        {
            return false;
        }
        if (TILESET.tile_width != GRID.tile_width ||
            TILESET.tile_height != GRID.tile_height)
        {
            return Fail(error,
                        "El tileset " + std::to_string(TILESET.id) +
                            " no coincide con el tamaño de tile de la cuadrícula.");
        }
    }

    for (const Layer &LAYER : level.layers)
    {
        if (!CheckUniqueId(local_ids, LAYER.id, "capa", error))
        {
            return false;
        }
        if (!CheckNonEmptyString(LAYER.name, "layer.name", error))
        {
            return false;
        }
        if (LAYER.kind != LayerKind::TILE && LAYER.kind != LayerKind::COLLISION)
        {
            return Fail(error, "La capa " + std::to_string(LAYER.id) +
                                   " tiene un tipo inválido.");
        }

        const Tileset *const TILESET = FindTileset(level, LAYER.tileset_id);
        if (TILESET == nullptr)
        {
            return Fail(error, "La capa " + std::to_string(LAYER.id) +
                                   " referencia un tileset inexistente.");
        }
        if (LAYER.tiles.size() != CELL_COUNT)
        {
            return Fail(error, "La capa " + std::to_string(LAYER.id) + " tiene " +
                                   std::to_string(LAYER.tiles.size()) +
                                   " tiles; se esperaban " + std::to_string(CELL_COUNT) +
                                   ".");
        }

        const std::uint64_t TILE_LIMIT =
            static_cast<std::uint64_t>(TILESET->columns) * TILESET->rows;
        for (const std::uint32_t TILE : LAYER.tiles)
        {
            if (static_cast<std::uint64_t>(TILE) > TILE_LIMIT)
            {
                return Fail(error, "La capa " + std::to_string(LAYER.id) +
                                       " usa un índice de tile fuera del tileset.");
            }
        }
    }

    for (const Entity &ENTITY : level.entities)
    {
        if (!CheckUniqueId(local_ids, ENTITY.id, "entidad", error))
        {
            return false;
        }
        const bool HAS_IMAGE = ENTITY.image_id != 0;
        const bool HAS_ANIMATION = ENTITY.animation_id != 0;
        if (HAS_IMAGE == HAS_ANIMATION)
        {
            return Fail(error, "La entidad " + std::to_string(ENTITY.id) +
                                   " debe tener exactamente image_id o animation_id.");
        }
        if (FindLayer(level, ENTITY.layer_id) == nullptr)
        {
            return Fail(error, "La entidad " + std::to_string(ENTITY.id) +
                                   " referencia una capa inexistente.");
        }
    }

    return true;
}

bool ValidateAnimation(const Animation &animation, std::string &error)
{
    if (!CheckSchemaVersion(animation.schema_version, "la animación", error))
    {
        return false;
    }
    if (animation.id == 0)
    {
        return Fail(error, "La animación no tiene un ID válido.");
    }
    if (!CheckNonEmptyString(animation.name, "animation.name", error))
    {
        return false;
    }
    if (animation.image_id == 0)
    {
        return Fail(error, "La animación no referencia una imagen.");
    }
    if (!CheckGridDimension(animation.frame_width, "frame_width", error) ||
        !CheckGridDimension(animation.frame_height, "frame_height", error))
    {
        return false;
    }
    if (animation.fps < MIN_FPS || animation.fps > MAX_FPS)
    {
        return Fail(error, "La animación debe tener fps entre 1 y 240.");
    }
    if (animation.pivot.x < 0 || animation.pivot.y < 0 ||
        static_cast<std::uint32_t>(animation.pivot.x) > animation.frame_width ||
        static_cast<std::uint32_t>(animation.pivot.y) > animation.frame_height)
    {
        return Fail(error, "El pivote de la animación está fuera del fotograma.");
    }

    const std::int64_t HITBOX_RIGHT =
        static_cast<std::int64_t>(animation.hitbox.x) + animation.hitbox.width;
    const std::int64_t HITBOX_BOTTOM =
        static_cast<std::int64_t>(animation.hitbox.y) + animation.hitbox.height;
    if (animation.hitbox.x < 0 || animation.hitbox.y < 0 || animation.hitbox.width <= 0 ||
        animation.hitbox.height <= 0 || HITBOX_RIGHT > animation.frame_width ||
        HITBOX_BOTTOM > animation.frame_height)
    {
        return Fail(error, "La hitbox de la animación está fuera del fotograma.");
    }

    if (animation.frames.empty() || animation.frames.size() > MAX_FRAMES)
    {
        return Fail(error, "La animación debe tener entre 1 y 4096 fotogramas.");
    }
    for (const Frame &FRAME : animation.frames)
    {
        if (FRAME.width == 0 || FRAME.width > MAX_IMAGE_DIMENSION || FRAME.height == 0 ||
            FRAME.height > MAX_IMAGE_DIMENSION)
        {
            return Fail(error,
                        "Un fotograma de la animación tiene dimensiones inválidas.");
        }
        if (FRAME.duration_ms > MAX_FRAME_DURATION_MS)
        {
            return Fail(error, "Un fotograma supera la duración máxima de 60 000 ms.");
        }
    }

    return true;
}

bool ValidateProject(const ProjectBundle &bundle, std::string &error)
{
    const Project &PROJECT = bundle.project;
    if (!CheckSchemaVersion(PROJECT.schema_version, "el proyecto", error))
    {
        return false;
    }
    if (!CheckNonEmptyString(PROJECT.project_id, "project_id", error) ||
        !CheckNonEmptyString(PROJECT.name, "project.name", error))
    {
        return false;
    }
    if (PROJECT.next_id == 0)
    {
        return Fail(error, "next_id debe ser mayor que cero.");
    }

    std::set<std::uint64_t> global_ids;
    std::set<std::string> paths;
    std::uint64_t maximum_id = 0;

    for (const AssetRef &ASSET : PROJECT.assets)
    {
        if (!CheckUniqueId(global_ids, ASSET.id, "asset", error))
        {
            return false;
        }
        maximum_id = std::max(maximum_id, ASSET.id);
        if (!IsSafeRelativePath(ASSET.path, error))
        {
            return false;
        }
        if (!paths.insert(ASSET.path).second)
        {
            return Fail(error, "La ruta '" + ASSET.path + "' está repetida.");
        }
    }

    for (const ResourceRef &LEVEL : PROJECT.levels)
    {
        if (!CheckUniqueId(global_ids, LEVEL.id, "nivel", error))
        {
            return false;
        }
        maximum_id = std::max(maximum_id, LEVEL.id);
        if (!IsSafeRelativePath(LEVEL.path, error) || !paths.insert(LEVEL.path).second)
        {
            if (error.empty())
            {
                return Fail(error, "La ruta '" + LEVEL.path + "' está repetida.");
            }
            return false;
        }
    }

    for (const ResourceRef &ANIMATION : PROJECT.animations)
    {
        if (!CheckUniqueId(global_ids, ANIMATION.id, "animación", error))
        {
            return false;
        }
        maximum_id = std::max(maximum_id, ANIMATION.id);
        if (!IsSafeRelativePath(ANIMATION.path, error) ||
            !paths.insert(ANIMATION.path).second)
        {
            if (error.empty())
            {
                return Fail(error, "La ruta '" + ANIMATION.path + "' está repetida.");
            }
            return false;
        }
    }

    if (PROJECT.next_id <= maximum_id)
    {
        return Fail(error, "next_id debe ser mayor que todos los IDs globales.");
    }
    if (bundle.levels.empty())
    {
        return Fail(error, "El proyecto debe tener al menos un nivel para guardarse.");
    }

    bool entry_found = false;
    for (const LevelDocument &DOCUMENT : bundle.levels)
    {
        if (DOCUMENT.data.id == PROJECT.entry_level_id)
        {
            entry_found = true;
        }
        if (!ValidateLevel(DOCUMENT.data, error))
        {
            return false;
        }
        if (!IsSafeRelativePath(DOCUMENT.relative_path, error))
        {
            return false;
        }
    }
    if (!entry_found)
    {
        return Fail(error, "entry_level_id no existe en los niveles cargados.");
    }

    if (bundle.levels.size() != PROJECT.levels.size() ||
        bundle.animations.size() != PROJECT.animations.size())
    {
        return Fail(error, "Las referencias de project.json no coinciden con los "
                           "documentos cargados.");
    }

    for (const LevelDocument &DOCUMENT : bundle.levels)
    {
        const auto ITERATOR =
            std::find_if(PROJECT.levels.begin(), PROJECT.levels.end(),
                         [&DOCUMENT](const ResourceRef &reference) {
                             return reference.id == DOCUMENT.data.id &&
                                    reference.path == DOCUMENT.relative_path;
                         });
        if (ITERATOR == PROJECT.levels.end())
        {
            return Fail(error, "El nivel " + std::to_string(DOCUMENT.data.id) +
                                   " no está declarado en project.json.");
        }
    }

    for (const AnimationDocument &DOCUMENT : bundle.animations)
    {
        if (!ValidateAnimation(DOCUMENT.data, error))
        {
            return false;
        }
        if (!IsSafeRelativePath(DOCUMENT.relative_path, error))
        {
            return false;
        }
        const auto ITERATOR =
            std::find_if(PROJECT.animations.begin(), PROJECT.animations.end(),
                         [&DOCUMENT](const ResourceRef &reference) {
                             return reference.id == DOCUMENT.data.id &&
                                    reference.path == DOCUMENT.relative_path;
                         });
        if (ITERATOR == PROJECT.animations.end())
        {
            return Fail(error, "La animación " + std::to_string(DOCUMENT.data.id) +
                                   " no está declarada en project.json.");
        }
    }

    return true;
}

bool ValidateProjectFiles(const ProjectBundle &bundle, std::string &error)
{
    std::set<std::uint64_t> image_ids;
    std::vector<std::pair<std::uint64_t, std::pair<std::uint32_t, std::uint32_t>>>
        image_sizes;

    for (const AssetRef &ASSET : bundle.project.assets)
    {
        const std::filesystem::path ABSOLUTE = bundle.base_directory / ASSET.path;
        std::uint32_t width = 0;
        std::uint32_t height = 0;
        if (!ReadPngInfo(ABSOLUTE, width, height, error))
        {
            return false;
        }
        image_ids.insert(ASSET.id);
        image_sizes.emplace_back(ASSET.id, std::make_pair(width, height));
    }

    const auto FIND_IMAGE = [&image_sizes](std::uint64_t image_id) {
        const auto ITERATOR = std::find_if(
            image_sizes.begin(), image_sizes.end(),
            [image_id](const auto &entry) { return entry.first == image_id; });
        return ITERATOR == image_sizes.end()
                   ? std::pair<std::uint32_t, std::uint32_t>{0, 0}
                   : ITERATOR->second;
    };

    for (const LevelDocument &DOCUMENT : bundle.levels)
    {
        for (const Tileset &TILESET : DOCUMENT.data.tilesets)
        {
            if (image_ids.count(TILESET.image_id) == 0)
            {
                return Fail(error, "El tileset " + std::to_string(TILESET.id) +
                                       " referencia un asset inexistente.");
            }
            const auto [WIDTH, HEIGHT] = FIND_IMAGE(TILESET.image_id);
            if (static_cast<std::uint64_t>(TILESET.columns) * TILESET.tile_width >
                    WIDTH ||
                static_cast<std::uint64_t>(TILESET.rows) * TILESET.tile_height > HEIGHT)
            {
                return Fail(error, "El tileset " + std::to_string(TILESET.id) +
                                       " no cabe en su imagen.");
            }
        }
    }

    for (const AnimationDocument &DOCUMENT : bundle.animations)
    {
        if (image_ids.count(DOCUMENT.data.image_id) == 0)
        {
            return Fail(error, "La animación " + std::to_string(DOCUMENT.data.id) +
                                   " referencia un asset inexistente.");
        }
        const auto [WIDTH, HEIGHT] = FIND_IMAGE(DOCUMENT.data.image_id);
        for (const Frame &FRAME : DOCUMENT.data.frames)
        {
            if (static_cast<std::uint64_t>(FRAME.x) + FRAME.width > WIDTH ||
                static_cast<std::uint64_t>(FRAME.y) + FRAME.height > HEIGHT)
            {
                return Fail(error, "Un fotograma de la animación " +
                                       std::to_string(DOCUMENT.data.id) +
                                       " queda fuera de la imagen.");
            }
        }
    }

    return true;
}

bool ValidateAll(const ProjectBundle &bundle, std::string &error)
{
    return ValidateProject(bundle, error) && ValidateProjectFiles(bundle, error);
}

} // namespace k_engine::editor

// NOLINTEND(readability-identifier-naming,readability-identifier-length,readability-function-cognitive-complexity,readability-else-after-return)
