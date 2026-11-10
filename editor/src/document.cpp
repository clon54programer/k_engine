#include "editor/document.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>

namespace k_engine::editor
{

namespace
{

std::uint64_t max_local_id(const Level &level) noexcept
{
    std::uint64_t maximum = 0;
    for (const Tileset &tileset : level.tilesets)
    {
        maximum = std::max(maximum, tileset.id);
    }
    for (const Layer &layer : level.layers)
    {
        maximum = std::max(maximum, layer.id);
    }
    for (const Entity &entity : level.entities)
    {
        maximum = std::max(maximum, entity.id);
    }
    return maximum;
}

std::uint64_t max_global_id(const Project &project) noexcept
{
    std::uint64_t maximum = 0;
    for (const AssetRef &asset : project.assets)
    {
        maximum = std::max(maximum, asset.id);
    }
    for (const ResourceRef &level : project.levels)
    {
        maximum = std::max(maximum, level.id);
    }
    for (const ResourceRef &animation : project.animations)
    {
        maximum = std::max(maximum, animation.id);
    }
    return maximum;
}

} // namespace

const AssetRef *FindAsset(const ProjectBundle &bundle, std::uint64_t asset_id) noexcept
{
    const auto ITERATOR =
        std::find_if(bundle.project.assets.begin(), bundle.project.assets.end(),
                     [asset_id](const AssetRef &asset) { return asset.id == asset_id; });
    return ITERATOR == bundle.project.assets.end() ? nullptr : &(*ITERATOR);
}

LevelDocument *FindLevel(ProjectBundle &bundle, std::uint64_t level_id) noexcept
{
    const auto ITERATOR = std::find_if(bundle.levels.begin(), bundle.levels.end(),
                                       [level_id](const LevelDocument &document) {
                                           return document.data.id == level_id;
                                       });
    return ITERATOR == bundle.levels.end() ? nullptr : &(*ITERATOR);
}

const LevelDocument *FindLevel(const ProjectBundle &bundle,
                               std::uint64_t level_id) noexcept
{
    const auto ITERATOR = std::find_if(bundle.levels.begin(), bundle.levels.end(),
                                       [level_id](const LevelDocument &document) {
                                           return document.data.id == level_id;
                                       });
    return ITERATOR == bundle.levels.end() ? nullptr : &(*ITERATOR);
}

AnimationDocument *FindAnimation(ProjectBundle &bundle,
                                 std::uint64_t animation_id) noexcept
{
    const auto ITERATOR = std::find_if(bundle.animations.begin(), bundle.animations.end(),
                                       [animation_id](const AnimationDocument &document) {
                                           return document.data.id == animation_id;
                                       });
    return ITERATOR == bundle.animations.end() ? nullptr : &(*ITERATOR);
}

const AnimationDocument *FindAnimation(const ProjectBundle &bundle,
                                       std::uint64_t animation_id) noexcept
{
    const auto ITERATOR = std::find_if(bundle.animations.begin(), bundle.animations.end(),
                                       [animation_id](const AnimationDocument &document) {
                                           return document.data.id == animation_id;
                                       });
    return ITERATOR == bundle.animations.end() ? nullptr : &(*ITERATOR);
}

bool operator==(const AssetRef &left, const AssetRef &right) noexcept
{
    return left.id == right.id && left.path == right.path;
}

bool operator==(const ResourceRef &left, const ResourceRef &right) noexcept
{
    return left.id == right.id && left.path == right.path;
}

bool operator==(const Grid &left, const Grid &right) noexcept
{
    return left.tile_width == right.tile_width && left.tile_height == right.tile_height &&
           left.columns == right.columns && left.rows == right.rows;
}

bool operator==(const Tileset &left, const Tileset &right) noexcept
{
    return left.id == right.id && left.image_id == right.image_id &&
           left.tile_width == right.tile_width && left.tile_height == right.tile_height &&
           left.columns == right.columns && left.rows == right.rows;
}

bool operator==(const Layer &left, const Layer &right) noexcept
{
    return left.id == right.id && left.name == right.name && left.kind == right.kind &&
           left.visible == right.visible && left.tileset_id == right.tileset_id &&
           left.tiles == right.tiles;
}

bool operator==(const Entity &left, const Entity &right) noexcept
{
    return left.id == right.id && left.image_id == right.image_id &&
           left.animation_id == right.animation_id && left.x == right.x &&
           left.y == right.y && left.layer_id == right.layer_id;
}

bool operator==(const Level &left, const Level &right) noexcept
{
    return left.schema_version == right.schema_version && left.id == right.id &&
           left.name == right.name && left.grid == right.grid &&
           left.tilesets == right.tilesets && left.layers == right.layers &&
           left.entities == right.entities;
}

bool operator==(const Vec2i &left, const Vec2i &right) noexcept
{
    return left.x == right.x && left.y == right.y;
}

bool operator==(const Recti &left, const Recti &right) noexcept
{
    return left.x == right.x && left.y == right.y && left.width == right.width &&
           left.height == right.height;
}

bool operator==(const Frame &left, const Frame &right) noexcept
{
    return left.x == right.x && left.y == right.y && left.width == right.width &&
           left.height == right.height && left.duration_ms == right.duration_ms;
}

bool operator==(const Animation &left, const Animation &right) noexcept
{
    return left.schema_version == right.schema_version && left.id == right.id &&
           left.name == right.name && left.image_id == right.image_id &&
           left.frame_width == right.frame_width &&
           left.frame_height == right.frame_height && left.fps == right.fps &&
           left.loop == right.loop && left.pivot == right.pivot &&
           left.hitbox == right.hitbox && left.frames == right.frames;
}

std::uint64_t AllocateGlobalId(Project &project, std::string &error)
{
    const std::uint64_t MAXIMUM = max_global_id(project);
    if (project.next_id <= MAXIMUM)
    {
        if (MAXIMUM == std::numeric_limits<std::uint64_t>::max())
        {
            error = "No queda ningún ID global disponible.";
            return 0;
        }
        project.next_id = MAXIMUM + 1;
    }

    if (project.next_id == 0 ||
        project.next_id == std::numeric_limits<std::uint64_t>::max())
    {
        error = "No queda ningún ID global disponible.";
        return 0;
    }

    const std::uint64_t ALLOCATED = project.next_id;
    ++project.next_id;
    return ALLOCATED;
}

std::uint64_t AllocateLocalId(const Level &level, std::string &error)
{
    const std::uint64_t MAXIMUM = max_local_id(level);
    if (MAXIMUM == std::numeric_limits<std::uint64_t>::max())
    {
        error = "No queda ningún ID local disponible.";
        return 0;
    }

    return MAXIMUM + 1;
}

} // namespace k_engine::editor
