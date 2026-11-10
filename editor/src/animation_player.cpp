// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-identifier-naming)
#include "editor/animation_player.h"

#include <algorithm>
#include <cstddef>

namespace k_engine::editor
{

void AnimationPlayer::Reset() noexcept
{
    _elapsed_seconds = 0.0;
    _frame_index = 0;
}

void AnimationPlayer::Update(const Animation &animation, double delta_seconds) noexcept
{
    if (animation.frames.empty())
    {
        Reset();
        return;
    }
    if (_frame_index >= animation.frames.size())
    {
        _frame_index = 0;
        _elapsed_seconds = 0.0;
    }
    if (delta_seconds <= 0.0)
    {
        return;
    }

    _elapsed_seconds += delta_seconds;
    double duration = FrameDurationSeconds(animation, _frame_index);
    if (duration <= 0.0)
    {
        return;
    }

    while (_elapsed_seconds >= duration)
    {
        _elapsed_seconds -= duration;
        if (_frame_index + 1 < animation.frames.size())
        {
            ++_frame_index;
        }
        else if (animation.loop)
        {
            _frame_index = 0;
        }
        else
        {
            _frame_index = animation.frames.size() - 1;
            _elapsed_seconds = 0.0;
            return;
        }

        duration = FrameDurationSeconds(animation, _frame_index);
        if (duration <= 0.0)
        {
            return;
        }
    }
}

std::size_t AnimationPlayer::CurrentFrameIndex() const noexcept
{
    return _frame_index;
}

const Frame *AnimationPlayer::CurrentFrame(const Animation &animation) const noexcept
{
    if (animation.frames.empty() || _frame_index >= animation.frames.size())
    {
        return nullptr;
    }
    return &animation.frames[_frame_index];
}

double AnimationPlayer::FrameDurationSeconds(const Animation &animation,
                                             std::size_t frame_index) noexcept
{
    if (frame_index >= animation.frames.size())
    {
        return 0.0;
    }

    const Frame &FRAME = animation.frames[frame_index];
    if (FRAME.duration_ms > 0)
    {
        return static_cast<double>(FRAME.duration_ms) / 1000.0;
    }
    if (animation.fps == 0)
    {
        return 0.125;
    }
    return 1.0 / static_cast<double>(animation.fps);
}

} // namespace k_engine::editor

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-identifier-naming)
