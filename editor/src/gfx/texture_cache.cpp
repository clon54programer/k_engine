#include "editor/gfx/texture_cache.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>

#include <stb_image.h>

#include "editor/validation.h"

namespace k_engine::editor::gfx
{

namespace
{

constexpr int CHANNELS_IN_RGBA = 4;

} // namespace

void TextureCache::Initialize(QuadRenderer *renderer) noexcept
{
    Clear();
    _renderer = renderer;
}

void TextureCache::Clear() noexcept
{
    if (_renderer != nullptr)
    {
        for (auto &pair : _entries)
        {
            _renderer->DestroyTexture(pair.second.texture);
        }
    }
    _entries.clear();
}

const TextureCache::Entry *TextureCache::Find(std::uint64_t asset_id) const noexcept
{
    const auto ITERATOR = _entries.find(asset_id);
    return ITERATOR == _entries.end() ? nullptr : &ITERATOR->second;
}

const TextureCache::Entry *TextureCache::Load(std::uint64_t asset_id,
                                              const std::filesystem::path &path,
                                              std::string &error)
{
    if (const Entry *const EXISTING = Find(asset_id); EXISTING != nullptr)
    {
        return EXISTING;
    }
    if (_renderer == nullptr)
    {
        error = "El renderer no está inicializado.";
        return nullptr;
    }

    int width = 0;
    int height = 0;
    int components = 0;
    stbi_uc *const PIXELS =
        stbi_load(path.string().c_str(), &width, &height, &components, CHANNELS_IN_RGBA);
    if (PIXELS == nullptr)
    {
        const char *const REASON = stbi_failure_reason();
        error = "No se pudo cargar el PNG " + path.string() + ": " +
                (REASON != nullptr ? std::string(REASON) : std::string("sin detalle")) +
                ".";
        return nullptr;
    }

    const std::uint64_t BYTES = static_cast<std::uint64_t>(width) *
                                static_cast<std::uint64_t>(height) *
                                static_cast<std::uint64_t>(CHANNELS_IN_RGBA);
    if (width <= 0 || height <= 0 ||
        static_cast<std::uint32_t>(width) > MAX_IMAGE_DIMENSION ||
        static_cast<std::uint32_t>(height) > MAX_IMAGE_DIMENSION ||
        BYTES > MAX_DECODED_IMAGE_BYTES)
    {
        stbi_image_free(PIXELS);
        error = "El PNG " + path.string() + " supera los límites del editor.";
        return nullptr;
    }

    ui::TextureId texture;
    const bool CREATED =
        _renderer->CreateTexture(PIXELS, static_cast<std::uint32_t>(width),
                                 static_cast<std::uint32_t>(height), texture, error);
    stbi_image_free(PIXELS);
    if (!CREATED)
    {
        return nullptr;
    }

    Entry entry;
    entry.texture = texture;
    entry.width = static_cast<std::uint32_t>(width);
    entry.height = static_cast<std::uint32_t>(height);
    const auto RESULT = _entries.emplace(asset_id, entry);
    return &RESULT.first->second;
}

} // namespace k_engine::editor::gfx
