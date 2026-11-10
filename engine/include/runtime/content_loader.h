#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "resource/pak_reader.h"

namespace k_engine::runtime
{

struct SpriteAsset
{
    std::uint64_t image_id = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::vector<std::uint8_t> rgba;
};

struct LoadedContent
{
    std::uint64_t level_id = 0;
    std::string level_name;
    SpriteAsset sprite;
    std::int32_t sprite_x = 0;
    std::int32_t sprite_y = 0;
};

class ContentLoader
{
  public:
    static bool Load(const resource::PakReader &pak, std::uint64_t level_id,
                     LoadedContent &out, std::string &error);
};

} // namespace k_engine::runtime
