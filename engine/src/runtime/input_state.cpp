#include "runtime/input_state.h"

namespace k_engine::runtime
{

void InputState::SetPressed(InputAction action, bool pressed) noexcept
{
    const auto INDEX = static_cast<std::size_t>(action);
    if (INDEX < ACTION_COUNT)
    {
        _pressed.at(INDEX) = pressed;
    }
}

bool InputState::IsPressed(InputAction action) const noexcept
{
    const auto INDEX = static_cast<std::size_t>(action);
    return INDEX < ACTION_COUNT && _pressed.at(INDEX);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void InputState::SetCursor(double cursor_x, double cursor_y) noexcept
{
    _cursor_x = cursor_x;
    _cursor_y = cursor_y;
}

double InputState::CursorX() const noexcept
{
    return _cursor_x;
}

double InputState::CursorY() const noexcept
{
    return _cursor_y;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void InputState::RequestLeftClick(double click_x, double click_y) noexcept
{
    _click_x = click_x;
    _click_y = click_y;
    _left_click_pending = true;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
bool InputState::ConsumeLeftClick(double &click_x, double &click_y) noexcept
{
    if (!_left_click_pending)
    {
        return false;
    }

    click_x = _click_x;
    click_y = _click_y;
    _left_click_pending = false;
    return true;
}

} // namespace k_engine::runtime
