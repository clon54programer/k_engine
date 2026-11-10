// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-identifier-naming,readability-identifier-length,readability-function-cognitive-complexity,readability-else-after-return,performance-move-const-arg)
#include "editor/project_io.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include <nlohmann/json.hpp>

#include "editor/validation.h"

namespace k_engine::editor
{

namespace
{

using Json = nlohmann::ordered_json;

constexpr std::uint32_t DEFAULT_GRID_COLUMNS = 32;
constexpr std::uint32_t DEFAULT_GRID_ROWS = 18;
constexpr std::uint32_t DEFAULT_TILE_SIZE = 32;
constexpr std::uint32_t DEFAULT_FPS = 8;
constexpr std::string_view DEFAULT_SCHEMA_VERSION = "1.0";
constexpr std::string_view PNG_TYPE = "png";

bool Fail(std::string &error, std::string message)
{
    error = std::move(message);
    return false;
}

const Json *FindField(const Json &object, std::string_view key)
{
    const auto ITERATOR = object.find(std::string(key));
    return ITERATOR == object.end() ? nullptr : &(*ITERATOR);
}

bool CheckKeys(const Json &object, std::initializer_list<std::string_view> allowed,
               std::string_view context, std::string &error)
{
    if (!object.is_object())
    {
        return Fail(error, std::string(context) + " debe ser un objeto.");
    }

    for (const auto &ITEM : object.items())
    {
        bool known = false;
        for (const std::string_view KEY : allowed)
        {
            if (ITEM.key() == KEY)
            {
                known = true;
                break;
            }
        }
        if (!known)
        {
            return Fail(error, std::string(context) + " contiene el campo desconocido '" +
                                   ITEM.key() + "'.");
        }
    }
    return true;
}

bool ExpectObject(const Json &value, std::string_view context, std::string &error)
{
    if (!value.is_object())
    {
        return Fail(error, std::string(context) + " debe ser un objeto.");
    }
    return true;
}

bool ExpectArray(const Json &value, std::string_view context, std::string &error)
{
    if (!value.is_array())
    {
        return Fail(error, std::string(context) + " debe ser un array.");
    }
    return true;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
bool ExpectString(const Json &value, std::string_view context, std::string &out,
                  std::string &error)
{
    if (!value.is_string())
    {
        return Fail(error, std::string(context) + " debe ser una cadena.");
    }
    out = value.get<std::string>();
    if (out.size() > MAX_STRING_BYTES)
    {
        return Fail(error, std::string(context) + " supera el límite de cadena.");
    }
    return true;
}

bool ExpectBool(const Json &value, std::string_view context, bool &out,
                std::string &error)
{
    if (!value.is_boolean())
    {
        return Fail(error, std::string(context) + " debe ser un booleano.");
    }
    out = value.get<bool>();
    return true;
}

bool ExpectUint(const Json &value, std::string_view context, std::uint64_t maximum,
                std::uint64_t &out, std::string &error)
{
    if (value.is_number_unsigned())
    {
        out = value.get<std::uint64_t>();
    }
    else if (value.is_number_integer())
    {
        const std::int64_t SIGNED = value.get<std::int64_t>();
        if (SIGNED < 0)
        {
            return Fail(error, std::string(context) + " no puede ser negativo.");
        }
        out = static_cast<std::uint64_t>(SIGNED);
    }
    else
    {
        return Fail(error, std::string(context) + " debe ser un entero.");
    }

    if (out > maximum)
    {
        return Fail(error, std::string(context) + " está fuera de rango.");
    }
    return true;
}

bool ExpectInt32(const Json &value, std::string_view context, std::int32_t &out,
                 std::string &error)
{
    if (!value.is_number_integer())
    {
        return Fail(error, std::string(context) + " debe ser un entero.");
    }

    const std::int64_t SIGNED = value.get<std::int64_t>();
    if (SIGNED < std::numeric_limits<std::int32_t>::min() ||
        SIGNED > std::numeric_limits<std::int32_t>::max())
    {
        return Fail(error, std::string(context) + " está fuera de rango int32.");
    }
    out = static_cast<std::int32_t>(SIGNED);
    return true;
}

bool ExpectId(const Json &value, std::string_view context, std::uint64_t &out,
              std::string &error)
{
    std::string text;
    if (!ExpectString(value, context, text, error))
    {
        return false;
    }
    return ParseId(text, out, error);
}

const Json &RequiredField(const Json &object, std::string_view key,
                          std::string_view context, std::string &error, bool &ok)
{
    const Json *const FIELD = FindField(object, key);
    if (FIELD == nullptr)
    {
        ok = Fail(error,
                  std::string(context) + " no contiene '" + std::string(key) + "'.");
        static const Json EMPTY = Json::object();
        return EMPTY;
    }
    ok = true;
    return *FIELD;
}

bool ReadJsonFile(const std::filesystem::path &path, std::uint64_t maximum_bytes,
                  Json &out, std::string &error)
{
    std::error_code file_error;
    if (!std::filesystem::is_regular_file(path, file_error) || file_error)
    {
        return Fail(error, "No existe el archivo " + path.string() + ".");
    }
    const std::uintmax_t FILE_SIZE = std::filesystem::file_size(path, file_error);
    if (file_error || FILE_SIZE > maximum_bytes)
    {
        return Fail(error, "El archivo " + path.string() + " supera el límite.");
    }

    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        return Fail(error, "No se pudo abrir " + path.string() + ".");
    }
    const std::string TEXT((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
    if (!file.good() && !file.eof())
    {
        return Fail(error, "No se pudo leer " + path.string() + ".");
    }

    out = Json::parse(TEXT, nullptr, false);
    if (out.is_discarded())
    {
        return Fail(error, "El JSON " + path.string() + " no es válido.");
    }
    if (!out.is_object())
    {
        return Fail(error, "El JSON " + path.string() + " debe ser un objeto.");
    }
    return true;
}

bool WriteJsonAtomic(const std::filesystem::path &path, const Json &document,
                     std::string &error)
{
    std::error_code file_error;
    const std::filesystem::path PARENT = path.parent_path();
    if (!PARENT.empty())
    {
        std::filesystem::create_directories(PARENT, file_error);
        if (file_error)
        {
            return Fail(error, "No se pudo crear el directorio " + PARENT.string() + ".");
        }
    }

    std::filesystem::path temporary = path;
    temporary += ".tmp";
    {
        std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
        if (!file)
        {
            return Fail(error, "No se pudo escribir " + temporary.string() + ".");
        }
        file << document.dump(2) << '\n';
        file.flush();
        if (!file)
        {
            return Fail(error, "No se pudo escribir " + temporary.string() + ".");
        }
    }

    std::filesystem::rename(temporary, path, file_error);
    if (file_error)
    {
        std::error_code remove_error;
        std::filesystem::remove(temporary, remove_error);
        return Fail(error, "No se pudo publicar " + path.string() + ".");
    }
    return true;
}

Json ProjectToJson(const Project &project)
{
    Json assets = Json::array();
    for (const AssetRef &ASSET : project.assets)
    {
        assets.push_back(Json{{"id", FormatId(ASSET.id)},
                              {"type", std::string(PNG_TYPE)},
                              {"path", ASSET.path}});
    }

    Json levels = Json::array();
    for (const ResourceRef &LEVEL : project.levels)
    {
        levels.push_back(Json{{"id", FormatId(LEVEL.id)}, {"path", LEVEL.path}});
    }

    Json animations = Json::array();
    for (const ResourceRef &ANIMATION : project.animations)
    {
        animations.push_back(
            Json{{"id", FormatId(ANIMATION.id)}, {"path", ANIMATION.path}});
    }

    return Json{{"schema_version", project.schema_version},
                {"project_id", project.project_id},
                {"name", project.name},
                {"next_id", FormatId(project.next_id)},
                {"entry_level_id", FormatId(project.entry_level_id)},
                {"assets", std::move(assets)},
                {"levels", std::move(levels)},
                {"animations", std::move(animations)}};
}

Json LevelToJson(const Level &level)
{
    Json grid{{"tile_width", level.grid.tile_width},
              {"tile_height", level.grid.tile_height},
              {"columns", level.grid.columns},
              {"rows", level.grid.rows}};

    Json tilesets = Json::array();
    for (const Tileset &TILESET : level.tilesets)
    {
        tilesets.push_back(Json{{"id", FormatId(TILESET.id)},
                                {"image_id", FormatId(TILESET.image_id)},
                                {"tile_width", TILESET.tile_width},
                                {"tile_height", TILESET.tile_height},
                                {"columns", TILESET.columns},
                                {"rows", TILESET.rows}});
    }

    Json layers = Json::array();
    for (const Layer &LAYER : level.layers)
    {
        layers.push_back(
            Json{{"id", FormatId(LAYER.id)},
                 {"name", LAYER.name},
                 {"kind", LAYER.kind == LayerKind::COLLISION ? "collision" : "tile"},
                 {"visible", LAYER.visible},
                 {"tileset_id", FormatId(LAYER.tileset_id)},
                 {"tiles", LAYER.tiles}});
    }

    Json entities = Json::array();
    for (const Entity &ENTITY : level.entities)
    {
        Json entity{{"id", FormatId(ENTITY.id)},
                    {"x", ENTITY.x},
                    {"y", ENTITY.y},
                    {"layer_id", FormatId(ENTITY.layer_id)}};
        if (ENTITY.image_id != 0)
        {
            entity["image_id"] = FormatId(ENTITY.image_id);
        }
        else
        {
            entity["animation_id"] = FormatId(ENTITY.animation_id);
        }
        entities.push_back(std::move(entity));
    }

    return Json{{"schema_version", level.schema_version},
                {"id", FormatId(level.id)},
                {"name", level.name},
                {"grid", std::move(grid)},
                {"tilesets", std::move(tilesets)},
                {"layers", std::move(layers)},
                {"entities", std::move(entities)}};
}

Json AnimationToJson(const Animation &animation)
{
    Json pivot{{"x", animation.pivot.x}, {"y", animation.pivot.y}};
    Json hitbox{{"x", animation.hitbox.x},
                {"y", animation.hitbox.y},
                {"w", animation.hitbox.width},
                {"h", animation.hitbox.height}};

    Json frames = Json::array();
    for (const Frame &FRAME : animation.frames)
    {
        frames.push_back(Json{{"x", FRAME.x},
                              {"y", FRAME.y},
                              {"w", FRAME.width},
                              {"h", FRAME.height},
                              {"duration_ms", FRAME.duration_ms}});
    }

    return Json{{"schema_version", animation.schema_version},
                {"id", FormatId(animation.id)},
                {"name", animation.name},
                {"image_id", FormatId(animation.image_id)},
                {"frame_width", animation.frame_width},
                {"frame_height", animation.frame_height},
                {"fps", animation.fps},
                {"loop", animation.loop},
                {"pivot", std::move(pivot)},
                {"hitbox", std::move(hitbox)},
                {"frames", std::move(frames)}};
}

bool ParseProject(const Json &root, Project &project, std::string &error)
{
    if (!CheckKeys(root,
                   {"schema_version", "project_id", "name", "next_id", "entry_level_id",
                    "assets", "levels", "animations"},
                   "project.json", error))
    {
        return false;
    }

    bool ok = false;
    const Json &SCHEMA = RequiredField(root, "schema_version", "project.json", error, ok);
    if (!ok || !ExpectString(SCHEMA, "project.json.schema_version",
                             project.schema_version, error))
    {
        return false;
    }
    const Json &PROJECT_ID = RequiredField(root, "project_id", "project.json", error, ok);
    if (!ok ||
        !ExpectString(PROJECT_ID, "project.json.project_id", project.project_id, error))
    {
        return false;
    }
    const Json &NAME = RequiredField(root, "name", "project.json", error, ok);
    if (!ok || !ExpectString(NAME, "project.json.name", project.name, error))
    {
        return false;
    }
    const Json &NEXT_ID = RequiredField(root, "next_id", "project.json", error, ok);
    if (!ok || !ExpectId(NEXT_ID, "project.json.next_id", project.next_id, error))
    {
        return false;
    }
    const Json &ENTRY_ID =
        RequiredField(root, "entry_level_id", "project.json", error, ok);
    if (!ok ||
        !ExpectId(ENTRY_ID, "project.json.entry_level_id", project.entry_level_id, error))
    {
        return false;
    }

    const Json &ASSETS = RequiredField(root, "assets", "project.json", error, ok);
    if (!ok || !ExpectArray(ASSETS, "project.json.assets", error))
    {
        return false;
    }
    for (const Json &ASSET : ASSETS)
    {
        if (!ExpectObject(ASSET, "project.json.assets[]", error) ||
            !CheckKeys(ASSET, {"id", "type", "path"}, "project.json.assets[]", error))
        {
            return false;
        }
        const Json &ID = RequiredField(ASSET, "id", "project.json.assets[]", error, ok);
        AssetRef asset;
        if (!ok || !ExpectId(ID, "project.json.assets[].id", asset.id, error))
        {
            return false;
        }
        const Json &TYPE =
            RequiredField(ASSET, "type", "project.json.assets[]", error, ok);
        std::string type;
        if (!ok || !ExpectString(TYPE, "project.json.assets[].type", type, error) ||
            type != PNG_TYPE)
        {
            if (error.empty())
            {
                error = "project.json.assets[].type debe ser 'png'.";
            }
            return false;
        }
        const Json &PATH =
            RequiredField(ASSET, "path", "project.json.assets[]", error, ok);
        if (!ok || !ExpectString(PATH, "project.json.assets[].path", asset.path, error))
        {
            return false;
        }
        project.assets.push_back(std::move(asset));
    }

    const Json &LEVELS = RequiredField(root, "levels", "project.json", error, ok);
    if (!ok || !ExpectArray(LEVELS, "project.json.levels", error))
    {
        return false;
    }
    for (const Json &LEVEL : LEVELS)
    {
        if (!ExpectObject(LEVEL, "project.json.levels[]", error) ||
            !CheckKeys(LEVEL, {"id", "path"}, "project.json.levels[]", error))
        {
            return false;
        }
        const Json &ID = RequiredField(LEVEL, "id", "project.json.levels[]", error, ok);
        ResourceRef reference;
        if (!ok || !ExpectId(ID, "project.json.levels[].id", reference.id, error))
        {
            return false;
        }
        const Json &PATH =
            RequiredField(LEVEL, "path", "project.json.levels[]", error, ok);
        if (!ok ||
            !ExpectString(PATH, "project.json.levels[].path", reference.path, error))
        {
            return false;
        }
        project.levels.push_back(std::move(reference));
    }

    if (const Json *const ANIMATIONS = FindField(root, "animations");
        ANIMATIONS != nullptr)
    {
        if (!ExpectArray(*ANIMATIONS, "project.json.animations", error))
        {
            return false;
        }
        for (const Json &ANIMATION : *ANIMATIONS)
        {
            if (!ExpectObject(ANIMATION, "project.json.animations[]", error) ||
                !CheckKeys(ANIMATION, {"id", "path"}, "project.json.animations[]", error))
            {
                return false;
            }
            const Json &ID =
                RequiredField(ANIMATION, "id", "project.json.animations[]", error, ok);
            ResourceRef reference;
            if (!ok || !ExpectId(ID, "project.json.animations[].id", reference.id, error))
            {
                return false;
            }
            const Json &PATH =
                RequiredField(ANIMATION, "path", "project.json.animations[]", error, ok);
            if (!ok || !ExpectString(PATH, "project.json.animations[].path",
                                     reference.path, error))
            {
                return false;
            }
            project.animations.push_back(std::move(reference));
        }
    }

    return true;
}

bool ParseGrid(const Json &root, Grid &grid, std::string &error)
{
    if (!CheckKeys(root, {"tile_width", "tile_height", "columns", "rows"}, "level.grid",
                   error))
    {
        return false;
    }

    bool ok = false;
    std::uint64_t value = 0;
    const Json &TILE_WIDTH = RequiredField(root, "tile_width", "level.grid", error, ok);
    if (!ok || !ExpectUint(TILE_WIDTH, "level.grid.tile_width", 4096, value, error))
    {
        return false;
    }
    grid.tile_width = static_cast<std::uint32_t>(value);
    const Json &TILE_HEIGHT = RequiredField(root, "tile_height", "level.grid", error, ok);
    if (!ok || !ExpectUint(TILE_HEIGHT, "level.grid.tile_height", 4096, value, error))
    {
        return false;
    }
    grid.tile_height = static_cast<std::uint32_t>(value);
    const Json &COLUMNS = RequiredField(root, "columns", "level.grid", error, ok);
    if (!ok || !ExpectUint(COLUMNS, "level.grid.columns", 4096, value, error))
    {
        return false;
    }
    grid.columns = static_cast<std::uint32_t>(value);
    const Json &ROWS = RequiredField(root, "rows", "level.grid", error, ok);
    if (!ok || !ExpectUint(ROWS, "level.grid.rows", 4096, value, error))
    {
        return false;
    }
    grid.rows = static_cast<std::uint32_t>(value);
    return true;
}

bool ParseLevel(const Json &root, Level &level, std::string &error)
{
    if (!CheckKeys(
            root,
            {"schema_version", "id", "name", "grid", "tilesets", "layers", "entities"},
            "level.json", error))
    {
        return false;
    }

    bool ok = false;
    const Json &SCHEMA = RequiredField(root, "schema_version", "level.json", error, ok);
    if (!ok ||
        !ExpectString(SCHEMA, "level.json.schema_version", level.schema_version, error))
    {
        return false;
    }
    const Json &ID = RequiredField(root, "id", "level.json", error, ok);
    if (!ok || !ExpectId(ID, "level.json.id", level.id, error))
    {
        return false;
    }
    const Json &NAME = RequiredField(root, "name", "level.json", error, ok);
    if (!ok || !ExpectString(NAME, "level.json.name", level.name, error))
    {
        return false;
    }

    const Json &GRID = RequiredField(root, "grid", "level.json", error, ok);
    if (!ok || !ParseGrid(GRID, level.grid, error))
    {
        return false;
    }

    const Json &TILESETS = RequiredField(root, "tilesets", "level.json", error, ok);
    if (!ok || !ExpectArray(TILESETS, "level.json.tilesets", error))
    {
        return false;
    }
    for (const Json &VALUE : TILESETS)
    {
        if (!ExpectObject(VALUE, "level.json.tilesets[]", error) ||
            !CheckKeys(VALUE,
                       {"id", "image_id", "tile_width", "tile_height", "columns", "rows"},
                       "level.json.tilesets[]", error))
        {
            return false;
        }
        Tileset tileset;
        const Json &TILESET_ID =
            RequiredField(VALUE, "id", "level.json.tilesets[]", error, ok);
        if (!ok || !ExpectId(TILESET_ID, "level.json.tilesets[].id", tileset.id, error))
        {
            return false;
        }
        const Json &IMAGE_ID =
            RequiredField(VALUE, "image_id", "level.json.tilesets[]", error, ok);
        if (!ok || !ExpectId(IMAGE_ID, "level.json.tilesets[].image_id", tileset.image_id,
                             error))
        {
            return false;
        }
        std::uint64_t number = 0;
        const Json &TILE_WIDTH =
            RequiredField(VALUE, "tile_width", "level.json.tilesets[]", error, ok);
        if (!ok || !ExpectUint(TILE_WIDTH, "level.json.tilesets[].tile_width", 4096,
                               number, error))
        {
            return false;
        }
        tileset.tile_width = static_cast<std::uint32_t>(number);
        const Json &TILE_HEIGHT =
            RequiredField(VALUE, "tile_height", "level.json.tilesets[]", error, ok);
        if (!ok || !ExpectUint(TILE_HEIGHT, "level.json.tilesets[].tile_height", 4096,
                               number, error))
        {
            return false;
        }
        tileset.tile_height = static_cast<std::uint32_t>(number);
        const Json &COLUMNS =
            RequiredField(VALUE, "columns", "level.json.tilesets[]", error, ok);
        if (!ok ||
            !ExpectUint(COLUMNS, "level.json.tilesets[].columns", 4096, number, error))
        {
            return false;
        }
        tileset.columns = static_cast<std::uint32_t>(number);
        const Json &ROWS =
            RequiredField(VALUE, "rows", "level.json.tilesets[]", error, ok);
        if (!ok || !ExpectUint(ROWS, "level.json.tilesets[].rows", 4096, number, error))
        {
            return false;
        }
        tileset.rows = static_cast<std::uint32_t>(number);
        level.tilesets.push_back(std::move(tileset));
    }

    const Json &LAYERS = RequiredField(root, "layers", "level.json", error, ok);
    if (!ok || !ExpectArray(LAYERS, "level.json.layers", error))
    {
        return false;
    }
    for (const Json &VALUE : LAYERS)
    {
        if (!ExpectObject(VALUE, "level.json.layers[]", error) ||
            !CheckKeys(VALUE, {"id", "name", "kind", "visible", "tileset_id", "tiles"},
                       "level.json.layers[]", error))
        {
            return false;
        }
        Layer layer;
        const Json &LAYER_ID =
            RequiredField(VALUE, "id", "level.json.layers[]", error, ok);
        if (!ok || !ExpectId(LAYER_ID, "level.json.layers[].id", layer.id, error))
        {
            return false;
        }
        const Json &NAME_VALUE =
            RequiredField(VALUE, "name", "level.json.layers[]", error, ok);
        if (!ok ||
            !ExpectString(NAME_VALUE, "level.json.layers[].name", layer.name, error))
        {
            return false;
        }
        const Json &KIND = RequiredField(VALUE, "kind", "level.json.layers[]", error, ok);
        std::string kind;
        if (!ok || !ExpectString(KIND, "level.json.layers[].kind", kind, error))
        {
            return false;
        }
        if (kind == "tile")
        {
            layer.kind = LayerKind::TILE;
        }
        else if (kind == "collision")
        {
            layer.kind = LayerKind::COLLISION;
        }
        else
        {
            return Fail(error, "level.json.layers[].kind debe ser 'tile' o "
                               "'collision'.");
        }

        layer.visible = true;
        if (const Json *const VISIBLE = FindField(VALUE, "visible");
            VISIBLE != nullptr &&
            !ExpectBool(*VISIBLE, "level.json.layers[].visible", layer.visible, error))
        {
            return false;
        }

        const Json &TILESET_ID =
            RequiredField(VALUE, "tileset_id", "level.json.layers[]", error, ok);
        if (!ok || !ExpectId(TILESET_ID, "level.json.layers[].tileset_id",
                             layer.tileset_id, error))
        {
            return false;
        }

        const Json &TILES =
            RequiredField(VALUE, "tiles", "level.json.layers[]", error, ok);
        if (!ok || !ExpectArray(TILES, "level.json.layers[].tiles", error))
        {
            return false;
        }
        for (const Json &TILE : TILES)
        {
            std::uint64_t tile = 0;
            if (!ExpectUint(TILE, "level.json.layers[].tiles[]",
                            std::numeric_limits<std::uint32_t>::max(), tile, error))
            {
                return false;
            }
            layer.tiles.push_back(static_cast<std::uint32_t>(tile));
        }
        level.layers.push_back(std::move(layer));
    }

    if (const Json *const ENTITIES = FindField(root, "entities"); ENTITIES != nullptr)
    {
        if (!ExpectArray(*ENTITIES, "level.json.entities", error))
        {
            return false;
        }
        for (const Json &VALUE : *ENTITIES)
        {
            if (!ExpectObject(VALUE, "level.json.entities[]", error) ||
                !CheckKeys(VALUE,
                           {"id", "image_id", "animation_id", "x", "y", "layer_id"},
                           "level.json.entities[]", error))
            {
                return false;
            }
            Entity entity;
            const Json &ENTITY_ID =
                RequiredField(VALUE, "id", "level.json.entities[]", error, ok);
            if (!ok || !ExpectId(ENTITY_ID, "level.json.entities[].id", entity.id, error))
            {
                return false;
            }
            if (const Json *const IMAGE_ID = FindField(VALUE, "image_id");
                IMAGE_ID != nullptr &&
                !ExpectId(*IMAGE_ID, "level.json.entities[].image_id", entity.image_id,
                          error))
            {
                return false;
            }
            if (const Json *const ANIMATION_ID = FindField(VALUE, "animation_id");
                ANIMATION_ID != nullptr &&
                !ExpectId(*ANIMATION_ID, "level.json.entities[].animation_id",
                          entity.animation_id, error))
            {
                return false;
            }
            const Json &X = RequiredField(VALUE, "x", "level.json.entities[]", error, ok);
            if (!ok || !ExpectInt32(X, "level.json.entities[].x", entity.x, error))
            {
                return false;
            }
            const Json &Y = RequiredField(VALUE, "y", "level.json.entities[]", error, ok);
            if (!ok || !ExpectInt32(Y, "level.json.entities[].y", entity.y, error))
            {
                return false;
            }
            const Json &LAYER_ID =
                RequiredField(VALUE, "layer_id", "level.json.entities[]", error, ok);
            if (!ok || !ExpectId(LAYER_ID, "level.json.entities[].layer_id",
                                 entity.layer_id, error))
            {
                return false;
            }
            level.entities.push_back(entity);
        }
    }

    return true;
}

bool ParseAnimation(const Json &root, Animation &animation, std::string &error)
{
    if (!CheckKeys(root,
                   {"schema_version", "id", "name", "image_id", "frame_width",
                    "frame_height", "fps", "loop", "pivot", "hitbox", "frames"},
                   "animation.json", error))
    {
        return false;
    }

    bool ok = false;
    const Json &SCHEMA =
        RequiredField(root, "schema_version", "animation.json", error, ok);
    if (!ok || !ExpectString(SCHEMA, "animation.json.schema_version",
                             animation.schema_version, error))
    {
        return false;
    }
    const Json &ID = RequiredField(root, "id", "animation.json", error, ok);
    if (!ok || !ExpectId(ID, "animation.json.id", animation.id, error))
    {
        return false;
    }
    const Json &NAME = RequiredField(root, "name", "animation.json", error, ok);
    if (!ok || !ExpectString(NAME, "animation.json.name", animation.name, error))
    {
        return false;
    }
    const Json &IMAGE_ID = RequiredField(root, "image_id", "animation.json", error, ok);
    if (!ok || !ExpectId(IMAGE_ID, "animation.json.image_id", animation.image_id, error))
    {
        return false;
    }

    std::uint64_t number = 0;
    const Json &FRAME_WIDTH =
        RequiredField(root, "frame_width", "animation.json", error, ok);
    if (!ok ||
        !ExpectUint(FRAME_WIDTH, "animation.json.frame_width", 4096, number, error))
    {
        return false;
    }
    animation.frame_width = static_cast<std::uint32_t>(number);
    const Json &FRAME_HEIGHT =
        RequiredField(root, "frame_height", "animation.json", error, ok);
    if (!ok ||
        !ExpectUint(FRAME_HEIGHT, "animation.json.frame_height", 4096, number, error))
    {
        return false;
    }
    animation.frame_height = static_cast<std::uint32_t>(number);
    const Json &FPS = RequiredField(root, "fps", "animation.json", error, ok);
    if (!ok || !ExpectUint(FPS, "animation.json.fps", 240, number, error))
    {
        return false;
    }
    animation.fps = static_cast<std::uint32_t>(number);

    animation.loop = true;
    if (const Json *const LOOP = FindField(root, "loop");
        LOOP != nullptr &&
        !ExpectBool(*LOOP, "animation.json.loop", animation.loop, error))
    {
        return false;
    }

    const Json &PIVOT = RequiredField(root, "pivot", "animation.json", error, ok);
    if (!ok || !CheckKeys(PIVOT, {"x", "y"}, "animation.json.pivot", error))
    {
        return false;
    }
    const Json &PIVOT_X = RequiredField(PIVOT, "x", "animation.json.pivot", error, ok);
    if (!ok || !ExpectInt32(PIVOT_X, "animation.json.pivot.x", animation.pivot.x, error))
    {
        return false;
    }
    const Json &PIVOT_Y = RequiredField(PIVOT, "y", "animation.json.pivot", error, ok);
    if (!ok || !ExpectInt32(PIVOT_Y, "animation.json.pivot.y", animation.pivot.y, error))
    {
        return false;
    }

    const Json &HITBOX = RequiredField(root, "hitbox", "animation.json", error, ok);
    if (!ok || !CheckKeys(HITBOX, {"x", "y", "w", "h"}, "animation.json.hitbox", error))
    {
        return false;
    }
    const Json &HITBOX_X = RequiredField(HITBOX, "x", "animation.json.hitbox", error, ok);
    if (!ok ||
        !ExpectInt32(HITBOX_X, "animation.json.hitbox.x", animation.hitbox.x, error))
    {
        return false;
    }
    const Json &HITBOX_Y = RequiredField(HITBOX, "y", "animation.json.hitbox", error, ok);
    if (!ok ||
        !ExpectInt32(HITBOX_Y, "animation.json.hitbox.y", animation.hitbox.y, error))
    {
        return false;
    }
    const Json &HITBOX_W = RequiredField(HITBOX, "w", "animation.json.hitbox", error, ok);
    if (!ok ||
        !ExpectInt32(HITBOX_W, "animation.json.hitbox.w", animation.hitbox.width, error))
    {
        return false;
    }
    const Json &HITBOX_H = RequiredField(HITBOX, "h", "animation.json.hitbox", error, ok);
    if (!ok ||
        !ExpectInt32(HITBOX_H, "animation.json.hitbox.h", animation.hitbox.height, error))
    {
        return false;
    }

    const Json &FRAMES = RequiredField(root, "frames", "animation.json", error, ok);
    if (!ok || !ExpectArray(FRAMES, "animation.json.frames", error))
    {
        return false;
    }
    for (const Json &VALUE : FRAMES)
    {
        if (!ExpectObject(VALUE, "animation.json.frames[]", error) ||
            !CheckKeys(VALUE, {"x", "y", "w", "h", "duration_ms"},
                       "animation.json.frames[]", error))
        {
            return false;
        }
        Frame frame;
        std::uint64_t frame_number = 0;
        const Json &X = RequiredField(VALUE, "x", "animation.json.frames[]", error, ok);
        if (!ok || !ExpectUint(X, "animation.json.frames[].x", 4096, frame_number, error))
        {
            return false;
        }
        frame.x = static_cast<std::uint32_t>(frame_number);
        const Json &Y = RequiredField(VALUE, "y", "animation.json.frames[]", error, ok);
        if (!ok || !ExpectUint(Y, "animation.json.frames[].y", 4096, frame_number, error))
        {
            return false;
        }
        frame.y = static_cast<std::uint32_t>(frame_number);
        const Json &W = RequiredField(VALUE, "w", "animation.json.frames[]", error, ok);
        if (!ok || !ExpectUint(W, "animation.json.frames[].w", 4096, frame_number, error))
        {
            return false;
        }
        frame.width = static_cast<std::uint32_t>(frame_number);
        const Json &H = RequiredField(VALUE, "h", "animation.json.frames[]", error, ok);
        if (!ok || !ExpectUint(H, "animation.json.frames[].h", 4096, frame_number, error))
        {
            return false;
        }
        frame.height = static_cast<std::uint32_t>(frame_number);
        if (const Json *const DURATION = FindField(VALUE, "duration_ms");
            DURATION != nullptr &&
            !ExpectUint(*DURATION, "animation.json.frames[].duration_ms", 60000,
                        frame_number, error))
        {
            return false;
        }
        else if (DURATION != nullptr)
        {
            frame.duration_ms = static_cast<std::uint32_t>(frame_number);
        }
        animation.frames.push_back(frame);
    }

    return true;
}

void SyncProjectReferences(ProjectBundle &bundle)
{
    bundle.project.levels.clear();
    for (const LevelDocument &DOCUMENT : bundle.levels)
    {
        bundle.project.levels.push_back(
            ResourceRef{DOCUMENT.data.id, DOCUMENT.relative_path});
    }

    bundle.project.animations.clear();
    for (const AnimationDocument &DOCUMENT : bundle.animations)
    {
        bundle.project.animations.push_back(
            ResourceRef{DOCUMENT.data.id, DOCUMENT.relative_path});
    }
}

bool IsPathInside(const std::filesystem::path &base, const std::filesystem::path &path,
                  std::filesystem::path &relative)
{
    std::error_code error;
    relative = std::filesystem::relative(path, base, error);
    if (error || relative.empty() || relative.is_absolute())
    {
        return false;
    }
    const std::string TEXT = relative.generic_string();
    return TEXT.rfind("..", 0) != 0;
}

} // namespace

bool ProjectIo::Create(const std::filesystem::path &directory, ProjectBundle &bundle,
                       std::string &error)
{
    if (directory.empty())
    {
        return Fail(error, "La ruta del proyecto no puede estar vacía.");
    }

    std::error_code file_error;
    const std::filesystem::path ABSOLUTE =
        std::filesystem::absolute(directory, file_error);
    if (file_error)
    {
        return Fail(error, "No se pudo resolver la ruta del proyecto.");
    }

    std::filesystem::create_directories(ABSOLUTE / "assets", file_error);
    if (file_error)
    {
        return Fail(error, "No se pudo crear el directorio del proyecto.");
    }
    std::filesystem::create_directories(ABSOLUTE / "levels", file_error);
    std::filesystem::create_directories(ABSOLUTE / "animations", file_error);
    if (file_error)
    {
        return Fail(error, "No se pudieron crear los directorios del proyecto.");
    }

    const std::filesystem::path PROJECT_PATH = ABSOLUTE / "project.json";
    if (std::filesystem::exists(PROJECT_PATH, file_error))
    {
        return Fail(error, "Ya existe " + PROJECT_PATH.string() + ".");
    }

    bundle = ProjectBundle{};
    bundle.base_directory = ABSOLUTE;
    bundle.project_path = PROJECT_PATH;
    bundle.project.schema_version = std::string(DEFAULT_SCHEMA_VERSION);
    bundle.project.project_id = ABSOLUTE.filename().string();
    if (bundle.project.project_id.empty())
    {
        bundle.project.project_id = "k_engine_project";
    }
    bundle.project.name = bundle.project.project_id;
    bundle.project.next_id = 1;
    bundle.project.entry_level_id = 0;
    bundle.dirty = true;
    return true;
}

bool ProjectIo::Load(const std::filesystem::path &project_path, ProjectBundle &bundle,
                     std::string &error)
{
    std::error_code file_error;
    const std::filesystem::path ABSOLUTE =
        std::filesystem::absolute(project_path, file_error);
    if (file_error)
    {
        return Fail(error, "No se pudo resolver la ruta del proyecto.");
    }

    Json root;
    if (!ReadJsonFile(ABSOLUTE, MAX_PROJECT_FILE_BYTES, root, error))
    {
        return false;
    }

    ProjectBundle loaded;
    loaded.project_path = ABSOLUTE;
    loaded.base_directory = ABSOLUTE.parent_path();
    if (!ParseProject(root, loaded.project, error))
    {
        return false;
    }

    for (const ResourceRef &REFERENCE : loaded.project.levels)
    {
        LevelDocument document;
        document.relative_path = REFERENCE.path;
        document.path = loaded.base_directory / REFERENCE.path;
        Json level_json;
        if (!ReadJsonFile(document.path, MAX_DOCUMENT_FILE_BYTES, level_json, error) ||
            !ParseLevel(level_json, document.data, error))
        {
            return false;
        }
        if (document.data.id != REFERENCE.id)
        {
            return Fail(error, "El ID del nivel en " + document.path.string() +
                                   " no coincide con project.json.");
        }
        loaded.levels.push_back(std::move(document));
    }

    for (const ResourceRef &REFERENCE : loaded.project.animations)
    {
        AnimationDocument document;
        document.relative_path = REFERENCE.path;
        document.path = loaded.base_directory / REFERENCE.path;
        Json animation_json;
        if (!ReadJsonFile(document.path, MAX_DOCUMENT_FILE_BYTES, animation_json,
                          error) ||
            !ParseAnimation(animation_json, document.data, error))
        {
            return false;
        }
        if (document.data.id != REFERENCE.id)
        {
            return Fail(error, "El ID de la animación en " + document.path.string() +
                                   " no coincide con project.json.");
        }
        loaded.animations.push_back(std::move(document));
    }

    if (!ValidateAll(loaded, error))
    {
        return false;
    }

    loaded.dirty = false;
    bundle = std::move(loaded);
    return true;
}

bool ProjectIo::Save(ProjectBundle &bundle, std::string &error)
{
    if (bundle.project_path.empty() || bundle.base_directory.empty())
    {
        return Fail(error, "El proyecto no tiene una ruta de guardado.");
    }

    SyncProjectReferences(bundle);
    if (!ValidateAll(bundle, error))
    {
        return false;
    }

    for (const LevelDocument &DOCUMENT : bundle.levels)
    {
        if (!WriteJsonAtomic(bundle.base_directory / DOCUMENT.relative_path,
                             LevelToJson(DOCUMENT.data), error))
        {
            return false;
        }
    }
    for (const AnimationDocument &DOCUMENT : bundle.animations)
    {
        if (!WriteJsonAtomic(bundle.base_directory / DOCUMENT.relative_path,
                             AnimationToJson(DOCUMENT.data), error))
        {
            return false;
        }
    }

    if (!WriteJsonAtomic(bundle.project_path, ProjectToJson(bundle.project), error))
    {
        return false;
    }

    bundle.dirty = false;
    return true;
}

bool ProjectIo::ImportPng(ProjectBundle &bundle, const std::filesystem::path &source_path,
                          std::uint64_t &asset_id, std::string &error)
{
    if (bundle.base_directory.empty())
    {
        return Fail(error, "No hay un proyecto abierto.");
    }

    std::error_code file_error;
    const std::filesystem::path SOURCE =
        std::filesystem::absolute(source_path, file_error);
    if (file_error)
    {
        return Fail(error, "No se pudo resolver la ruta del PNG.");
    }

    std::uint32_t width = 0;
    std::uint32_t height = 0;
    if (!ReadPngInfo(SOURCE, width, height, error))
    {
        return false;
    }

    const std::uint64_t NEW_ID = AllocateGlobalId(bundle.project, error);
    if (NEW_ID == 0)
    {
        return false;
    }

    std::filesystem::path relative_path;
    const bool INSIDE = IsPathInside(bundle.base_directory, SOURCE, relative_path);
    std::filesystem::path target_path;
    if (INSIDE)
    {
        target_path = SOURCE;
        relative_path = relative_path.generic_string();
    }
    else
    {
        const std::filesystem::path ASSETS = bundle.base_directory / "assets";
        std::filesystem::create_directories(ASSETS, file_error);
        if (file_error)
        {
            return Fail(error, "No se pudo crear el directorio de assets.");
        }
        const std::string FILENAME = SOURCE.filename().string();
        if (FILENAME.empty())
        {
            return Fail(error, "El PNG no tiene un nombre de archivo válido.");
        }
        target_path = ASSETS / ("asset_" + FormatId(NEW_ID) + "_" + FILENAME);
        std::filesystem::copy_file(SOURCE, target_path,
                                   std::filesystem::copy_options::none, file_error);
        if (file_error)
        {
            return Fail(error, "No se pudo copiar el PNG al proyecto.");
        }
        relative_path =
            std::filesystem::relative(target_path, bundle.base_directory, file_error);
        if (file_error)
        {
            return Fail(error, "No se pudo calcular la ruta relativa del PNG.");
        }
    }

    const std::string RELATIVE_TEXT = relative_path.generic_string();
    if (!IsSafeRelativePath(RELATIVE_TEXT, error))
    {
        return false;
    }

    const auto DUPLICATE = std::find_if(
        bundle.project.assets.begin(), bundle.project.assets.end(),
        [&RELATIVE_TEXT](const AssetRef &asset) { return asset.path == RELATIVE_TEXT; });
    if (DUPLICATE != bundle.project.assets.end())
    {
        return Fail(error, "El PNG ya está importado en el proyecto.");
    }

    bundle.project.assets.push_back(AssetRef{NEW_ID, RELATIVE_TEXT});
    bundle.dirty = true;
    asset_id = NEW_ID;
    return true;
}

bool ProjectIo::AddTileset(ProjectBundle &bundle, Level &level, std::uint64_t image_id,
                           std::uint64_t &tileset_id, std::string &error)
{
    if (level.tilesets.size() >= MAX_TILESETS)
    {
        return Fail(error, "El nivel ya tiene el máximo de 64 tilesets.");
    }

    const AssetRef *const ASSET = FindAsset(bundle, image_id);
    if (ASSET == nullptr)
    {
        return Fail(error, "El asset " + FormatId(image_id) + " no existe.");
    }

    std::uint32_t width = 0;
    std::uint32_t height = 0;
    if (!ReadPngInfo(bundle.base_directory / ASSET->path, width, height, error))
    {
        return false;
    }

    const std::uint32_t COLUMNS = width / level.grid.tile_width;
    const std::uint32_t ROWS = height / level.grid.tile_height;
    if (COLUMNS == 0 || ROWS == 0)
    {
        return Fail(error, "La imagen no contiene ningún tile del tamaño configurado.");
    }

    const std::uint64_t NEW_ID = AllocateLocalId(level, error);
    if (NEW_ID == 0)
    {
        return false;
    }

    Tileset tileset;
    tileset.id = NEW_ID;
    tileset.image_id = image_id;
    tileset.tile_width = level.grid.tile_width;
    tileset.tile_height = level.grid.tile_height;
    tileset.columns = COLUMNS;
    tileset.rows = ROWS;
    level.tilesets.push_back(tileset);
    tileset_id = NEW_ID;
    bundle.dirty = true;
    return true;
}

bool ProjectIo::CreateLevel(ProjectBundle &bundle, std::uint64_t image_id,
                            std::uint64_t &level_id, std::string &error)
{
    const AssetRef *const ASSET = FindAsset(bundle, image_id);
    if (ASSET == nullptr)
    {
        return Fail(error, "Selecciona un PNG importado para crear el nivel.");
    }

    std::uint32_t image_width = 0;
    std::uint32_t image_height = 0;
    if (!ReadPngInfo(bundle.base_directory / ASSET->path, image_width, image_height,
                     error))
    {
        return false;
    }

    const std::uint64_t NEW_ID = AllocateGlobalId(bundle.project, error);
    if (NEW_ID == 0)
    {
        return false;
    }

    Level level;
    level.id = NEW_ID;
    level.name = "Nivel " + FormatId(NEW_ID);
    level.grid.columns = DEFAULT_GRID_COLUMNS;
    level.grid.rows = DEFAULT_GRID_ROWS;
    level.grid.tile_width = std::min(DEFAULT_TILE_SIZE, image_width);
    level.grid.tile_height = std::min(DEFAULT_TILE_SIZE, image_height);
    if (level.grid.tile_width == 0 || level.grid.tile_height == 0)
    {
        return Fail(error, "La imagen no tiene dimensiones válidas.");
    }

    Tileset tileset;
    tileset.id = 1;
    tileset.image_id = image_id;
    tileset.tile_width = level.grid.tile_width;
    tileset.tile_height = level.grid.tile_height;
    tileset.columns = image_width / level.grid.tile_width;
    tileset.rows = image_height / level.grid.tile_height;
    if (tileset.columns == 0 || tileset.rows == 0)
    {
        return Fail(error, "La imagen no contiene ningún tile utilizable.");
    }
    level.tilesets.push_back(tileset);

    const std::uint64_t CELL_COUNT =
        static_cast<std::uint64_t>(level.grid.columns) * level.grid.rows;
    if (CELL_COUNT > MAX_GRID_CELLS)
    {
        return Fail(error, "La cuadrícula por defecto supera el límite de celdas.");
    }

    const std::uint64_t TILE_LAYER_ID = AllocateLocalId(level, error);
    if (TILE_LAYER_ID == 0)
    {
        return false;
    }
    Layer tile_layer;
    tile_layer.id = TILE_LAYER_ID;
    tile_layer.name = "fondo";
    tile_layer.kind = LayerKind::TILE;
    tile_layer.visible = true;
    tile_layer.tileset_id = tileset.id;
    tile_layer.tiles.assign(static_cast<std::size_t>(CELL_COUNT), 0);
    level.layers.push_back(std::move(tile_layer));

    const std::uint64_t COLLISION_LAYER_ID = AllocateLocalId(level, error);
    if (COLLISION_LAYER_ID == 0)
    {
        return false;
    }
    Layer collision_layer;
    collision_layer.id = COLLISION_LAYER_ID;
    collision_layer.name = "colision";
    collision_layer.kind = LayerKind::COLLISION;
    collision_layer.visible = false;
    collision_layer.tileset_id = tileset.id;
    collision_layer.tiles.assign(static_cast<std::size_t>(CELL_COUNT), 0);
    level.layers.push_back(std::move(collision_layer));

    LevelDocument document;
    document.data = std::move(level);
    document.relative_path = "levels/level_" + FormatId(NEW_ID) + ".json";
    document.path = bundle.base_directory / document.relative_path;
    bundle.levels.push_back(std::move(document));
    if (bundle.project.entry_level_id == 0)
    {
        bundle.project.entry_level_id = NEW_ID;
    }
    bundle.dirty = true;
    level_id = NEW_ID;
    return true;
}

bool ProjectIo::CreateAnimation(ProjectBundle &bundle, std::uint64_t image_id,
                                std::uint64_t &animation_id, std::string &error)
{
    const AssetRef *const ASSET = FindAsset(bundle, image_id);
    if (ASSET == nullptr)
    {
        return Fail(error, "Selecciona un PNG importado para crear la animación.");
    }

    std::uint32_t image_width = 0;
    std::uint32_t image_height = 0;
    if (!ReadPngInfo(bundle.base_directory / ASSET->path, image_width, image_height,
                     error))
    {
        return false;
    }

    const std::uint64_t NEW_ID = AllocateGlobalId(bundle.project, error);
    if (NEW_ID == 0)
    {
        return false;
    }

    Animation animation;
    animation.id = NEW_ID;
    animation.name = "Animacion " + FormatId(NEW_ID);
    animation.image_id = image_id;
    animation.frame_width = std::min(DEFAULT_TILE_SIZE, image_width);
    animation.frame_height = std::min(DEFAULT_TILE_SIZE, image_height);
    animation.fps = DEFAULT_FPS;
    animation.loop = true;
    animation.pivot.x = static_cast<std::int32_t>(animation.frame_width / 2U);
    animation.pivot.y = static_cast<std::int32_t>(animation.frame_height);
    animation.hitbox.x = 0;
    animation.hitbox.y = 0;
    animation.hitbox.width = static_cast<std::int32_t>(animation.frame_width);
    animation.hitbox.height = static_cast<std::int32_t>(animation.frame_height);

    AnimationDocument document;
    document.data = std::move(animation);
    document.relative_path = "animations/animation_" + FormatId(NEW_ID) + ".json";
    document.path = bundle.base_directory / document.relative_path;
    bundle.animations.push_back(std::move(document));
    bundle.dirty = true;
    animation_id = NEW_ID;
    return true;
}

} // namespace k_engine::editor

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-identifier-naming,readability-identifier-length,readability-function-cognitive-complexity,readability-else-after-return,performance-move-const-arg)
