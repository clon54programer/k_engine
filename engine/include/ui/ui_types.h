#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace k_engine::ui
{

using Id = std::uint64_t;

inline constexpr Id INVALID_ID = 0;

// Genera un ID estable a partir de un nombre. No se usan punteros ni direcciones.
constexpr Id MakeId(std::string_view text) noexcept
{
    std::uint64_t hash = 1469598103934665603ULL;
    for (const char CHARACTER : text)
    {
        hash ^= static_cast<unsigned char>(CHARACTER);
        hash *= 1099511628211ULL;
    }
    return hash == INVALID_ID ? 1 : hash;
}

struct Vec2
{
    float x = 0.0F;
    float y = 0.0F;
};

struct Rect
{
    float x = 0.0F;
    float y = 0.0F;
    float width = 0.0F;
    float height = 0.0F;

    [[nodiscard]] bool Contains(Vec2 point) const noexcept
    {
        return point.x >= x && point.x < x + width && point.y >= y &&
               point.y < y + height;
    }
};

// Los colores se guardan como valores 0..1 en el mismo espacio que la
// superficie UNORM del editor.
struct Color
{
    float r = 0.0F;
    float g = 0.0F;
    float b = 0.0F;
    float a = 1.0F;

    static constexpr Color FromBytes(std::uint8_t red, std::uint8_t green,
                                     std::uint8_t blue, std::uint8_t alpha = 255) noexcept
    {
        return Color{static_cast<float>(red) / 255.0F, static_cast<float>(green) / 255.0F,
                     static_cast<float>(blue) / 255.0F,
                     static_cast<float>(alpha) / 255.0F};
    }

    static constexpr Color White() noexcept
    {
        return Color{1.0F, 1.0F, 1.0F, 1.0F};
    }
};

struct TextureId
{
    std::uint32_t value = 0;
};

struct UvRect
{
    float u0 = 0.0F;
    float v0 = 0.0F;
    float u1 = 1.0F;
    float v1 = 1.0F;
};

enum class TextAlign : std::uint8_t
{
    LEFT = 0,
    CENTER,
    RIGHT,
};

struct DrawVertex
{
    float x = 0.0F;
    float y = 0.0F;
    float u = 0.0F;
    float v = 0.0F;
    float r = 1.0F;
    float g = 1.0F;
    float b = 1.0F;
    float a = 1.0F;
};

struct DrawCommand
{
    TextureId texture{};
    std::uint32_t first_vertex = 0;
    std::uint32_t vertex_count = 0;
    Rect clip{};
};

struct DrawList
{
    std::vector<DrawVertex> vertices;
    std::vector<DrawCommand> commands;

    void Clear() noexcept
    {
        vertices.clear();
        commands.clear();
    }
};

struct Input
{
    Vec2 mouse{};
    bool left_down = false;
    bool left_pressed = false;
    bool left_released = false;
    bool right_down = false;
    bool right_pressed = false;
    bool right_released = false;
    bool middle_down = false;
    bool backspace = false;
    bool enter = false;
    bool escape = false;
    bool ctrl = false;
    bool shift = false;
    float wheel = 0.0F;
    std::string text;
};

} // namespace k_engine::ui
