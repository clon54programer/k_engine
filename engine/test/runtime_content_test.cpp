#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

#include <flatbuffers/verifier.h>

#include "animation_generated.h"
#include "runtime/content_loader.h"

namespace
{

std::string g_fixture_dir;

std::string FixturePath(std::string_view name)
{
    return g_fixture_dir + "/" + std::string(name);
}

bool ReadFile(const std::string &path, std::vector<std::uint8_t> &data)
{
    std::ifstream input(path, std::ios::binary);
    if (!input)
    {
        return false;
    }

    data.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    return true;
}

void CheckPixel(const k_engine::runtime::LoadedContent &content, std::size_t x,
                std::size_t y, std::uint8_t red, std::uint8_t green, std::uint8_t blue,
                std::uint8_t alpha)
{
    const std::size_t INDEX =
        ((y * static_cast<std::size_t>(content.sprite.width)) + x) * 4U;
    REQUIRE(INDEX + 3U < content.sprite.rgba.size());
    CHECK(content.sprite.rgba[INDEX] == red);
    CHECK(content.sprite.rgba[INDEX + 1U] == green);
    CHECK(content.sprite.rgba[INDEX + 2U] == blue);
    CHECK(content.sprite.rgba[INDEX + 3U] == alpha);
}

} // namespace

TEST_CASE("El nivel y el sprite se cargan desde el paquete")
{
    k_engine::resource::PakReader pak;
    REQUIRE(pak.Open(FixturePath("runtime_test.pak")));

    k_engine::runtime::LoadedContent content;
    std::string error;

    REQUIRE_MESSAGE(k_engine::runtime::ContentLoader::Load(pak, 2, content, error),
                    error);
    CHECK(content.level_id == 2);
    CHECK(content.level_name == "runtime_level");
    CHECK(content.sprite.image_id == 100);
    CHECK(content.sprite.width == 64);
    CHECK(content.sprite.height == 64);
    CHECK(content.sprite.rgba.size() == 64U * 64U * 4U);
    CHECK(content.sprite_x == 64);
    CHECK(content.sprite_y == 64);

    CheckPixel(content, 0, 0, 220, 40, 40, 255);
    CheckPixel(content, 63, 0, 40, 200, 80, 255);
    CheckPixel(content, 0, 63, 40, 80, 220, 255);
    CheckPixel(content, 63, 63, 240, 240, 240, 255);
}

TEST_CASE("Un buffer Level corrupto se rechaza antes de leerlo")
{
    k_engine::resource::PakReader pak;
    REQUIRE(pak.Open(FixturePath("bad_level_buffer.pak")));

    k_engine::runtime::LoadedContent content;
    std::string error;

    CHECK_FALSE(k_engine::runtime::ContentLoader::Load(pak, 2, content, error));
    CHECK(error.find("FlatBuffers") != std::string::npos);
}

TEST_CASE("Un PNG corrupto se rechaza con error descriptivo")
{
    k_engine::resource::PakReader pak;
    REQUIRE(pak.Open(FixturePath("bad_png.pak")));

    k_engine::runtime::LoadedContent content;
    std::string error;

    CHECK_FALSE(k_engine::runtime::ContentLoader::Load(pak, 2, content, error));
    CHECK(error.find("PNG") != std::string::npos);
}

TEST_CASE("Un nivel inexistente se rechaza")
{
    k_engine::resource::PakReader pak;
    REQUIRE(pak.Open(FixturePath("runtime_test.pak")));

    k_engine::runtime::LoadedContent content;
    std::string error;

    CHECK_FALSE(k_engine::runtime::ContentLoader::Load(pak, 999, content, error));
    CHECK(error.find("No existe") != std::string::npos);
}

TEST_CASE("El buffer Animation se verifica antes de leerlo")
{
    std::vector<std::uint8_t> animation_data;
    REQUIRE(ReadFile(FixturePath("animation.bin"), animation_data));
    REQUIRE_FALSE(animation_data.empty());

    flatbuffers::Verifier verifier(animation_data.data(), animation_data.size());
    CHECK(k_engine::schema::VerifyAnimationBuffer(verifier));
    CHECK(k_engine::schema::AnimationBufferHasIdentifier(animation_data.data()));

    k_engine::resource::PakReader pak;
    REQUIRE(pak.Open(FixturePath("bad_animation_buffer.pak")));

    std::vector<std::uint8_t> corrupt_data;
    REQUIRE(pak.Read(200, k_engine::resource::PakResourceType::ANIMATION, corrupt_data));

    flatbuffers::Verifier corrupt_verifier(corrupt_data.data(), corrupt_data.size());
    CHECK_FALSE(k_engine::schema::VerifyAnimationBuffer(corrupt_verifier));
    CHECK_FALSE(k_engine::schema::AnimationBufferHasIdentifier(corrupt_data.data()));
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }

    // El acceso a argv[1] es seguro después de comprobar argc.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    g_fixture_dir = argv[1];

    doctest::Context context;
    context.setOption("no-breaks", true);
    return context.run();
}
