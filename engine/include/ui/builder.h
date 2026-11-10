#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>

#include "ui/ui_types.h"

namespace k_engine::ui
{

// Builder inmediato mínimo. No depende de Vulkan ni de GLFW: genera una lista de
// dibujo y resuelve interacción con IDs estables. El editor usa solo las
// primitivas que necesita; no es un sistema de UI de juego.
class Builder
{
  public:
    void BeginFrame(Rect viewport, const Input &input);
    void EndFrame();

    [[nodiscard]] const DrawList &GetDrawList() const noexcept;

    void FillRect(Id id, Rect rect, Color color);
    void Panel(Id id, Rect rect, Color background, Color border);
    void Text(Id id, Rect rect, std::string_view text, float scale, Color color,
              TextAlign align = TextAlign::LEFT);
    void Image(Id id, Rect rect, TextureId texture, UvRect uv,
               Color tint = Color::White());

    void SetClip(Rect rect) noexcept;
    void ResetClip() noexcept;

    bool Button(Id id, Rect rect, std::string_view text, bool selected = false,
                bool enabled = true);
    bool Toggle(Id id, Rect rect, std::string_view text, bool &value);
    bool NumberField(Id id, Rect rect, std::string_view label, int &value, int minimum,
                     int maximum);
    bool TextField(Id id, Rect rect, std::string_view label, std::string &value,
                   std::size_t maximum_length = 256);

    [[nodiscard]] bool IsHovered(Id id) const noexcept;
    [[nodiscard]] bool WasClicked(Id id) const noexcept;

  private:
    struct Interaction
    {
        bool hovered = false;
        bool pressed = false;
        bool clicked = false;
    };

    Interaction update_interaction(Id id, Rect rect);
    void clear_focus() noexcept;
    void add_quad(Rect rect, TextureId texture, UvRect uv, Color color);
    void add_text(Rect rect, std::string_view text, float scale, Color color,
                  TextAlign align);

    Rect _viewport{};
    Rect _clip_rect{};
    Input _input{};
    DrawList _draw_list;
    Id _hot_id = INVALID_ID;
    Id _active_id = INVALID_ID;
    Id _focus_id = INVALID_ID;
    Id _clicked_id = INVALID_ID;
    std::unordered_map<Id, std::string> _number_buffers;
};

} // namespace k_engine::ui
