#pragma once

#include <cstddef>

#include "editor/document.h"

namespace k_engine::editor
{

// Reloj determinista para la previsualización de un clip. Se mantiene separado
// del render para poder probarlo sin abrir una ventana.
class AnimationPlayer
{
  public:
    void Reset() noexcept;
    void Update(const Animation &animation, double delta_seconds) noexcept;

    [[nodiscard]] std::size_t CurrentFrameIndex() const noexcept;
    [[nodiscard]] const Frame *CurrentFrame(const Animation &animation) const noexcept;
    [[nodiscard]] static double FrameDurationSeconds(const Animation &animation,
                                                     std::size_t frame_index) noexcept;

  private:
    double _elapsed_seconds = 0.0;
    std::size_t _frame_index = 0;
};

} // namespace k_engine::editor
