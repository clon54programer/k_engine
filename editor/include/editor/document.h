#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace k_engine::editor
{

enum class LayerKind : std::uint8_t
{
    TILE = 0,
    COLLISION = 1,
};

struct AssetRef
{
    std::uint64_t id = 0;
    std::string path;
};

struct ResourceRef
{
    std::uint64_t id = 0;
    std::string path;
};

struct Project
{
    std::string schema_version = "1.0";
    std::string project_id;
    std::string name;
    std::uint64_t next_id = 1;
    std::uint64_t entry_level_id = 0;
    std::vector<AssetRef> assets;
    std::vector<ResourceRef> levels;
    std::vector<ResourceRef> animations;
};

struct Grid
{
    std::uint32_t tile_width = 32;
    std::uint32_t tile_height = 32;
    std::uint32_t columns = 32;
    std::uint32_t rows = 18;
};

struct Tileset
{
    std::uint64_t id = 0;
    std::uint64_t image_id = 0;
    std::uint32_t tile_width = 32;
    std::uint32_t tile_height = 32;
    std::uint32_t columns = 1;
    std::uint32_t rows = 1;
};

struct Layer
{
    std::uint64_t id = 0;
    std::string name;
    LayerKind kind = LayerKind::TILE;
    bool visible = true;
    std::uint64_t tileset_id = 0;
    std::vector<std::uint32_t> tiles;
};

struct Entity
{
    std::uint64_t id = 0;
    std::uint64_t image_id = 0;
    std::uint64_t animation_id = 0;
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::uint64_t layer_id = 0;
};

struct Level
{
    std::string schema_version = "1.0";
    std::uint64_t id = 0;
    std::string name;
    Grid grid;
    std::vector<Tileset> tilesets;
    std::vector<Layer> layers;
    std::vector<Entity> entities;
};

struct Vec2i
{
    std::int32_t x = 0;
    std::int32_t y = 0;
};

struct Recti
{
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::int32_t width = 0;
    std::int32_t height = 0;
};

struct Frame
{
    std::uint32_t x = 0;
    std::uint32_t y = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t duration_ms = 0;
};

struct Animation
{
    std::string schema_version = "1.0";
    std::uint64_t id = 0;
    std::string name;
    std::uint64_t image_id = 0;
    std::uint32_t frame_width = 32;
    std::uint32_t frame_height = 32;
    std::uint32_t fps = 8;
    bool loop = true;
    Vec2i pivot;
    Recti hitbox;
    std::vector<Frame> frames;
};

struct LevelDocument
{
    Level data;
    std::filesystem::path path;
    std::string relative_path;
};

struct AnimationDocument
{
    Animation data;
    std::filesystem::path path;
    std::string relative_path;
};

struct ProjectBundle
{
    Project project;
    std::filesystem::path project_path;
    std::filesystem::path base_directory;
    std::vector<LevelDocument> levels;
    std::vector<AnimationDocument> animations;
    bool dirty = false;
};

const AssetRef *FindAsset(const ProjectBundle &bundle, std::uint64_t asset_id) noexcept;
LevelDocument *FindLevel(ProjectBundle &bundle, std::uint64_t level_id) noexcept;
const LevelDocument *FindLevel(const ProjectBundle &bundle,
                               std::uint64_t level_id) noexcept;
AnimationDocument *FindAnimation(ProjectBundle &bundle,
                                 std::uint64_t animation_id) noexcept;
const AnimationDocument *FindAnimation(const ProjectBundle &bundle,
                                       std::uint64_t animation_id) noexcept;

bool operator==(const AssetRef &left, const AssetRef &right) noexcept;
bool operator==(const ResourceRef &left, const ResourceRef &right) noexcept;
bool operator==(const Grid &left, const Grid &right) noexcept;
bool operator==(const Tileset &left, const Tileset &right) noexcept;
bool operator==(const Layer &left, const Layer &right) noexcept;
bool operator==(const Entity &left, const Entity &right) noexcept;
bool operator==(const Level &left, const Level &right) noexcept;
bool operator==(const Vec2i &left, const Vec2i &right) noexcept;
bool operator==(const Recti &left, const Recti &right) noexcept;
bool operator==(const Frame &left, const Frame &right) noexcept;
bool operator==(const Animation &left, const Animation &right) noexcept;

std::uint64_t AllocateGlobalId(Project &project, std::string &error);
std::uint64_t AllocateLocalId(const Level &level, std::string &error);

} // namespace k_engine::editor
