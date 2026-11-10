// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-identifier-length,readability-avoid-nested-conditional-operator)
#include "ui/builder.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include <stb_easy_font.h>

namespace k_engine::ui
{

namespace
{

constexpr float TEXT_SCALE = 1.1F;
constexpr float LABEL_SCALE = 1.0F;
constexpr float FIELD_LABEL_HEIGHT = 14.0F;
constexpr float FIELD_PADDING = 4.0F;
constexpr std::size_t TEXT_BUFFER_BYTES_PER_CHARACTER = 320;
constexpr std::size_t TEXT_BUFFER_EXTRA_BYTES = 64;
constexpr std::size_t EASY_FONT_VERTEX_BYTES = 16;
constexpr std::size_t EASY_FONT_VERTICES_PER_QUAD = 4;
constexpr std::size_t TRIANGLE_VERTICES_PER_QUAD = 6;
constexpr std::size_t MAX_NUMBER_LENGTH = 12;

constexpr Color PANEL_BACKGROUND = Color::FromBytes(38, 42, 50);
constexpr Color PANEL_BORDER = Color::FromBytes(70, 78, 92);
constexpr Color BUTTON_BACKGROUND = Color::FromBytes(55, 62, 74);
constexpr Color BUTTON_HOVER = Color::FromBytes(72, 82, 98);
constexpr Color BUTTON_SELECTED = Color::FromBytes(43, 103, 135);
constexpr Color BUTTON_DISABLED = Color::FromBytes(48, 52, 60);
constexpr Color TEXT_COLOR = Color::FromBytes(225, 230, 238);
constexpr Color MUTED_TEXT = Color::FromBytes(160, 168, 180);
constexpr Color FIELD_BACKGROUND = Color::FromBytes(28, 31, 37);

struct EasyFontVertex
{
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;
    std::array<unsigned char, 4> color = {255, 255, 255, 255};
};

static_assert(sizeof(EasyFontVertex) == EASY_FONT_VERTEX_BYTES,
              "stb_easy_font debe usar vértices de 16 bytes");

std::string sanitize_text(std::string_view text)
{
    std::string result;
    result.reserve(text.size());
    for (const char CHARACTER : text)
    {
        const auto VALUE = static_cast<unsigned char>(CHARACTER);
        result.push_back(VALUE >= 32 && VALUE <= 126 ? CHARACTER : '?');
    }
    return result;
}

bool parse_int(std::string_view text, int &value)
{
    if (text.empty())
    {
        return false;
    }

    int parsed = 0;
    const auto RESULT = std::from_chars(text.data(), text.data() + text.size(), parsed);
    if (RESULT.ec != std::errc{} || RESULT.ptr != text.data() + text.size())
    {
        return false;
    }

    value = parsed;
    return true;
}

bool same_clip(const Rect &left, const Rect &right) noexcept
{
    return left.x == right.x && left.y == right.y && left.width == right.width &&
           left.height == right.height;
}

} // namespace

void Builder::BeginFrame(Rect viewport, const Input &input)
{
    _viewport = viewport;
    _clip_rect = viewport;
    _input = input;
    _draw_list.Clear();
    _hot_id = INVALID_ID;
    _clicked_id = INVALID_ID;
}

void Builder::EndFrame()
{
    if (_input.enter || _input.escape)
    {
        clear_focus();
    }

    if (_input.left_pressed && _hot_id == INVALID_ID)
    {
        clear_focus();
    }
}

const DrawList &Builder::GetDrawList() const noexcept
{
    return _draw_list;
}

void Builder::FillRect(Id id, Rect rect, Color color)
{
    (void)id;
    add_quad(rect, TextureId{}, UvRect{}, color);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void Builder::Panel(Id id, Rect rect, Color background, Color border)
{
    (void)id;
    add_quad(rect, TextureId{}, UvRect{}, background);

    constexpr float BORDER = 1.0F;
    add_quad(Rect{rect.x, rect.y, rect.width, BORDER}, TextureId{}, UvRect{}, border);
    add_quad(Rect{rect.x, rect.y + rect.height - BORDER, rect.width, BORDER}, TextureId{},
             UvRect{}, border);
    add_quad(Rect{rect.x, rect.y, BORDER, rect.height}, TextureId{}, UvRect{}, border);
    add_quad(Rect{rect.x + rect.width - BORDER, rect.y, BORDER, rect.height}, TextureId{},
             UvRect{}, border);
}

void Builder::Text(Id id, Rect rect, std::string_view text, float scale, Color color,
                   TextAlign align)
{
    (void)id;
    add_text(rect, text, scale, color, align);
}

void Builder::Image(Id id, Rect rect, TextureId texture, UvRect uv, Color tint)
{
    (void)id;
    add_quad(rect, texture, uv, tint);
}

void Builder::SetClip(Rect rect) noexcept
{
    _clip_rect = rect;
}

void Builder::ResetClip() noexcept
{
    _clip_rect = _viewport;
}

bool Builder::Button(Id id, Rect rect, std::string_view text, bool selected, bool enabled)
{
    Interaction interaction;
    if (enabled)
    {
        interaction = update_interaction(id, rect);
    }

    Color background = BUTTON_DISABLED;
    if (enabled)
    {
        background = selected ? BUTTON_SELECTED
                              : (interaction.hovered ? BUTTON_HOVER : BUTTON_BACKGROUND);
    }

    Panel(id, rect, background, PANEL_BORDER);
    add_text(rect, text, TEXT_SCALE, enabled ? TEXT_COLOR : MUTED_TEXT,
             TextAlign::CENTER);
    return enabled && interaction.clicked;
}

bool Builder::Toggle(Id id, Rect rect, std::string_view text, bool &value)
{
    const Interaction INTERACTION = update_interaction(id, rect);
    if (INTERACTION.clicked)
    {
        value = !value;
    }

    const std::string LABEL = std::string(value ? "[x] " : "[ ] ") + std::string(text);
    const Color BACKGROUND = INTERACTION.hovered
                                 ? BUTTON_HOVER
                                 : (value ? BUTTON_SELECTED : BUTTON_BACKGROUND);
    Panel(id, rect, BACKGROUND, PANEL_BORDER);
    add_text(rect, LABEL, TEXT_SCALE, TEXT_COLOR, TextAlign::LEFT);
    return INTERACTION.clicked;
}

bool Builder::NumberField(Id id, Rect rect, std::string_view label, int &value,
                          int minimum, int maximum)
{
    const Interaction INTERACTION = update_interaction(id, rect);
    bool changed = false;

    if (INTERACTION.pressed)
    {
        _focus_id = id;
        _number_buffers[id].clear();
    }

    if (_focus_id == id)
    {
        std::string &buffer = _number_buffers[id];
        if (_input.backspace && !buffer.empty())
        {
            buffer.pop_back();
        }

        for (const char CHARACTER : _input.text)
        {
            if ((CHARACTER >= '0' && CHARACTER <= '9') || CHARACTER == '-')
            {
                if (buffer.size() < MAX_NUMBER_LENGTH)
                {
                    buffer.push_back(CHARACTER);
                }
            }
        }

        int parsed = 0;
        if (parse_int(buffer, parsed) && parsed >= minimum && parsed <= maximum)
        {
            if (value != parsed)
            {
                value = parsed;
                changed = true;
            }
        }
    }

    const float LABEL_HEIGHT = FIELD_LABEL_HEIGHT;
    add_text(Rect{rect.x, rect.y, rect.width, LABEL_HEIGHT}, label, LABEL_SCALE,
             MUTED_TEXT, TextAlign::LEFT);

    const Rect FIELD_RECT{rect.x, rect.y + LABEL_HEIGHT, rect.width,
                          std::max(rect.height - LABEL_HEIGHT, 16.0F)};
    Panel(id, FIELD_RECT, FIELD_BACKGROUND,
          _focus_id == id ? BUTTON_SELECTED : PANEL_BORDER);

    const std::string DISPLAY =
        _focus_id == id ? _number_buffers[id] : std::to_string(value);
    add_text(Rect{FIELD_RECT.x + FIELD_PADDING, FIELD_RECT.y,
                  FIELD_RECT.width - (2.0F * FIELD_PADDING), FIELD_RECT.height},
             DISPLAY, TEXT_SCALE, TEXT_COLOR, TextAlign::LEFT);
    return changed;
}

bool Builder::TextField(Id id, Rect rect, std::string_view label, std::string &value,
                        std::size_t maximum_length)
{
    const Interaction INTERACTION = update_interaction(id, rect);
    bool changed = false;

    if (INTERACTION.pressed)
    {
        _focus_id = id;
    }

    if (_focus_id == id)
    {
        if (_input.backspace && !value.empty())
        {
            value.pop_back();
            changed = true;
        }

        for (const char CHARACTER : _input.text)
        {
            const auto BYTE = static_cast<unsigned char>(CHARACTER);
            if (BYTE >= 32 && BYTE <= 126 && value.size() < maximum_length)
            {
                value.push_back(CHARACTER);
                changed = true;
            }
        }
    }

    const float LABEL_HEIGHT = FIELD_LABEL_HEIGHT;
    add_text(Rect{rect.x, rect.y, rect.width, LABEL_HEIGHT}, label, LABEL_SCALE,
             MUTED_TEXT, TextAlign::LEFT);

    const Rect FIELD_RECT{rect.x, rect.y + LABEL_HEIGHT, rect.width,
                          std::max(rect.height - LABEL_HEIGHT, 16.0F)};
    Panel(id, FIELD_RECT, FIELD_BACKGROUND,
          _focus_id == id ? BUTTON_SELECTED : PANEL_BORDER);
    add_text(Rect{FIELD_RECT.x + FIELD_PADDING, FIELD_RECT.y,
                  FIELD_RECT.width - (2.0F * FIELD_PADDING), FIELD_RECT.height},
             value, TEXT_SCALE, TEXT_COLOR, TextAlign::LEFT);
    return changed;
}

bool Builder::IsHovered(Id id) const noexcept
{
    return _hot_id == id;
}

bool Builder::WasClicked(Id id) const noexcept
{
    return _clicked_id == id;
}

Builder::Interaction Builder::update_interaction(Id id, Rect rect)
{
    Interaction interaction;
    interaction.hovered = rect.Contains(_input.mouse);
    if (interaction.hovered)
    {
        _hot_id = id;
    }

    if (_input.left_pressed)
    {
        if (interaction.hovered)
        {
            _active_id = id;
            if (_focus_id != INVALID_ID && _focus_id != id)
            {
                clear_focus();
            }
        }
        else if (_focus_id != INVALID_ID && _focus_id != id)
        {
            clear_focus();
        }
        interaction.pressed = interaction.hovered;
    }

    if (_input.left_released)
    {
        if (_active_id == id && interaction.hovered)
        {
            interaction.clicked = true;
            _clicked_id = id;
        }
        if (_active_id == id)
        {
            _active_id = INVALID_ID;
        }
    }

    return interaction;
}

void Builder::clear_focus() noexcept
{
    _focus_id = INVALID_ID;
    _number_buffers.clear();
}

void Builder::add_quad(Rect rect, TextureId texture, UvRect uv, Color color)
{
    if (rect.width <= 0.0F || rect.height <= 0.0F)
    {
        return;
    }

    const float LEFT = rect.x;
    const float TOP = rect.y;
    const float RIGHT = rect.x + rect.width;
    const float BOTTOM = rect.y + rect.height;

    const std::size_t FIRST_VERTEX = _draw_list.vertices.size();
    _draw_list.vertices.push_back(
        DrawVertex{LEFT, TOP, uv.u0, uv.v0, color.r, color.g, color.b, color.a});
    _draw_list.vertices.push_back(
        DrawVertex{RIGHT, TOP, uv.u1, uv.v0, color.r, color.g, color.b, color.a});
    _draw_list.vertices.push_back(
        DrawVertex{RIGHT, BOTTOM, uv.u1, uv.v1, color.r, color.g, color.b, color.a});
    _draw_list.vertices.push_back(
        DrawVertex{LEFT, TOP, uv.u0, uv.v0, color.r, color.g, color.b, color.a});
    _draw_list.vertices.push_back(
        DrawVertex{RIGHT, BOTTOM, uv.u1, uv.v1, color.r, color.g, color.b, color.a});
    _draw_list.vertices.push_back(
        DrawVertex{LEFT, BOTTOM, uv.u0, uv.v1, color.r, color.g, color.b, color.a});

    constexpr std::uint32_t QUAD_VERTEX_COUNT = 6;
    if (!_draw_list.commands.empty() &&
        _draw_list.commands.back().texture.value == texture.value &&
        same_clip(_draw_list.commands.back().clip, _clip_rect) &&
        _draw_list.commands.back().first_vertex +
                _draw_list.commands.back().vertex_count ==
            FIRST_VERTEX)
    {
        _draw_list.commands.back().vertex_count += QUAD_VERTEX_COUNT;
        return;
    }

    _draw_list.commands.push_back(DrawCommand{texture,
                                              static_cast<std::uint32_t>(FIRST_VERTEX),
                                              QUAD_VERTEX_COUNT, _clip_rect});
}

void Builder::add_text(Rect rect, std::string_view text, float scale, Color color,
                       TextAlign align)
{
    if (text.empty() || rect.width <= 0.0F || rect.height <= 0.0F)
    {
        return;
    }

    std::string sanitized = sanitize_text(text);
    std::vector<char> text_buffer(sanitized.begin(), sanitized.end());
    text_buffer.push_back('\0');

    const int TEXT_WIDTH = stb_easy_font_width(text_buffer.data());
    const int TEXT_HEIGHT = stb_easy_font_height(text_buffer.data());
    const float SCALED_WIDTH = static_cast<float>(TEXT_WIDTH) * scale;
    const float SCALED_HEIGHT = static_cast<float>(TEXT_HEIGHT) * scale;

    float offset_x = rect.x;
    if (align == TextAlign::CENTER)
    {
        offset_x = rect.x + ((rect.width - SCALED_WIDTH) / 2.0F);
    }
    else if (align == TextAlign::RIGHT)
    {
        offset_x = rect.x + rect.width - SCALED_WIDTH;
    }
    const float OFFSET_Y = rect.y + ((rect.height - SCALED_HEIGHT) / 2.0F);

    const std::size_t BUFFER_SIZE =
        (sanitized.size() * TEXT_BUFFER_BYTES_PER_CHARACTER) + TEXT_BUFFER_EXTRA_BYTES;
    std::vector<char> vertices(BUFFER_SIZE);
    const int QUAD_COUNT =
        stb_easy_font_print(offset_x, OFFSET_Y, text_buffer.data(), nullptr,
                            vertices.data(), static_cast<int>(vertices.size()));
    if (QUAD_COUNT <= 0)
    {
        return;
    }

    const std::size_t FIRST_VERTEX = _draw_list.vertices.size();
    for (int quad = 0; quad < QUAD_COUNT; ++quad)
    {
        const std::size_t QUAD_OFFSET = static_cast<std::size_t>(quad) *
                                        EASY_FONT_VERTICES_PER_QUAD *
                                        EASY_FONT_VERTEX_BYTES;
        if (QUAD_OFFSET + (EASY_FONT_VERTICES_PER_QUAD * EASY_FONT_VERTEX_BYTES) >
            vertices.size())
        {
            break;
        }

        std::array<EasyFontVertex, EASY_FONT_VERTICES_PER_QUAD> quad_vertices{};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::memcpy(quad_vertices.data(), vertices.data() + QUAD_OFFSET,
                    sizeof(quad_vertices));

        constexpr std::array<std::size_t, TRIANGLE_VERTICES_PER_QUAD> INDICES = {0, 1, 2,
                                                                                 0, 2, 3};
        for (const std::size_t INDEX : INDICES)
        {
            const EasyFontVertex &vertex_value = quad_vertices.at(INDEX);
            _draw_list.vertices.push_back(
                DrawVertex{offset_x + ((vertex_value.x - offset_x) * scale),
                           OFFSET_Y + ((vertex_value.y - OFFSET_Y) * scale), 0.0F, 0.0F,
                           color.r, color.g, color.b, color.a});
        }
    }

    const auto ADDED_VERTICES =
        static_cast<std::uint32_t>(_draw_list.vertices.size() - FIRST_VERTEX);
    if (ADDED_VERTICES == 0)
    {
        return;
    }

    if (!_draw_list.commands.empty() &&
        _draw_list.commands.back().texture.value == TextureId{}.value &&
        same_clip(_draw_list.commands.back().clip, _clip_rect) &&
        _draw_list.commands.back().first_vertex +
                _draw_list.commands.back().vertex_count ==
            FIRST_VERTEX)
    {
        _draw_list.commands.back().vertex_count += ADDED_VERTICES;
        return;
    }

    _draw_list.commands.push_back(DrawCommand{TextureId{},
                                              static_cast<std::uint32_t>(FIRST_VERTEX),
                                              ADDED_VERTICES, _clip_rect});
}

} // namespace k_engine::ui

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-identifier-length,readability-avoid-nested-conditional-operator)
