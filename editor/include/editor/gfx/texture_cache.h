#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>

#include "editor/gfx/quad_renderer.h"

namespace k_engine::editor::gfx
{

class TextureCache
{
  public:
    struct Entry
    {
        ui::TextureId texture{};
        std::uint32_t width = 0;
        std::uint32_t height = 0;
    };

    void Initialize(QuadRenderer *renderer) noexcept;
    void Clear() noexcept;

    const Entry *Find(std::uint64_t asset_id) const noexcept;
    const Entry *Load(std::uint64_t asset_id, const std::filesystem::path &path,
                      std::string &error);

  private:
    QuadRenderer *_renderer = nullptr;
    std::unordered_map<std::uint64_t, Entry> _entries;
};

} // namespace k_engine::editor::gfx
