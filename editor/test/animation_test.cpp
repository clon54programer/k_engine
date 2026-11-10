// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-identifier-naming,readability-identifier-length,bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <cstddef>

#include "editor/animation_player.h"
#include "editor/document.h"

namespace
{

k_engine::editor::Animation MakeAnimation()
{
    k_engine::editor::Animation animation;
    animation.id = 3;
    animation.name = "idle";
    animation.image_id = 1;
    animation.frame_width = 32;
    animation.frame_height = 32;
    animation.fps = 8;
    animation.loop = true;
    animation.pivot = k_engine::editor::Vec2i{16, 32};
    animation.hitbox = k_engine::editor::Recti{0, 0, 32, 32};
    animation.frames.push_back(k_engine::editor::Frame{0, 0, 32, 32, 0});
    animation.frames.push_back(k_engine::editor::Frame{32, 0, 32, 32, 0});
    animation.frames.push_back(k_engine::editor::Frame{0, 32, 32, 32, 0});
    animation.frames.push_back(k_engine::editor::Frame{32, 32, 32, 32, 0});
    return animation;
}

} // namespace

TEST_CASE("AnimationPlayer avanza con fps y hace loop")
{
    const auto ANIMATION = MakeAnimation();
    k_engine::editor::AnimationPlayer player;
    player.Reset();
    CHECK(player.CurrentFrameIndex() == 0);
    player.Update(ANIMATION, 0.06);
    CHECK(player.CurrentFrameIndex() == 0);
    player.Update(ANIMATION, 0.07);
    CHECK(player.CurrentFrameIndex() == 1);
    player.Update(ANIMATION, 0.5);
    CHECK(player.CurrentFrameIndex() == 1);
    CHECK(player.CurrentFrame(ANIMATION) != nullptr);
}

TEST_CASE("AnimationPlayer se detiene en el último fotograma sin loop")
{
    auto animation = MakeAnimation();
    animation.loop = false;
    k_engine::editor::AnimationPlayer player;
    player.Update(animation, 10.0);
    CHECK(player.CurrentFrameIndex() == animation.frames.size() - 1);
}

TEST_CASE("AnimationPlayer respeta duration_ms")
{
    auto animation = MakeAnimation();
    animation.frames[0].duration_ms = 1000;
    k_engine::editor::AnimationPlayer player;
    player.Update(animation, 0.5);
    CHECK(player.CurrentFrameIndex() == 0);
    player.Update(animation, 0.6);
    CHECK(player.CurrentFrameIndex() == 1);
}

TEST_CASE("AnimationPlayer sin fotogramas no devuelve frame")
{
    k_engine::editor::Animation animation;
    k_engine::editor::AnimationPlayer player;
    player.Update(animation, 1.0);
    CHECK(player.CurrentFrame(animation) == nullptr);
}

TEST_CASE("FrameDurationSeconds usa fps cuando duration_ms es cero")
{
    const auto ANIMATION = MakeAnimation();
    CHECK(k_engine::editor::AnimationPlayer::FrameDurationSeconds(ANIMATION, 0) ==
          doctest::Approx(0.125));
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-identifier-naming,readability-identifier-length,bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)
