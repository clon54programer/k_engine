#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace k_engine::runtime
{

enum class InputAction : std::uint8_t
{
    MOVE_LEFT = 0,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    COUNT,
};

class InputState
{
  public:
    void SetPressed(InputAction action, bool pressed) noexcept;
    [[nodiscard]] bool IsPressed(InputAction action) const noexcept;

    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    void SetCursor(double cursor_x, double cursor_y) noexcept;
    [[nodiscard]] double CursorX() const noexcept;
    [[nodiscard]] double CursorY() const noexcept;

    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    void RequestLeftClick(double click_x, double click_y) noexcept;
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    bool ConsumeLeftClick(double &click_x, double &click_y) noexcept;

  private:
    static constexpr std::size_t ACTION_COUNT =
        static_cast<std::size_t>(InputAction::COUNT);

    std::array<bool, ACTION_COUNT> _pressed{};
    double _cursor_x = 0.0;
    double _cursor_y = 0.0;
    double _click_x = 0.0;
    double _click_y = 0.0;
    bool _left_click_pending = false;
};

} // namespace k_engine::runtime
