#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <array>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

#include "resource/pak_reader.h"

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

} // namespace

TEST_CASE("El paquete válido abre y entrega sus recursos")
{
    k_engine::resource::PakReader pak;
    REQUIRE(pak.Open(FixturePath("runtime_test.pak")));
    REQUIRE(pak.Entries().size() == 4);

    CHECK(pak.Entries()[0].id == 2);
    CHECK(pak.Entries()[0].type == k_engine::resource::PakResourceType::LEVEL);
    CHECK(pak.Entries()[1].id == 100);
    CHECK(pak.Entries()[1].type == k_engine::resource::PakResourceType::PNG);
    CHECK(pak.Entries()[2].id == 101);
    CHECK(pak.Entries()[2].type == k_engine::resource::PakResourceType::PNG);
    CHECK(pak.Entries()[3].id == 200);
    CHECK(pak.Entries()[3].type == k_engine::resource::PakResourceType::ANIMATION);
    CHECK(pak.Entries()[2].size > 64ULL * 1024ULL);

    std::vector<std::uint8_t> actual;
    std::vector<std::uint8_t> expected;

    REQUIRE(pak.Read(2, k_engine::resource::PakResourceType::LEVEL, actual));
    REQUIRE(ReadFile(FixturePath("level.bin"), expected));
    CHECK(actual == expected);

    REQUIRE(pak.Read(100, k_engine::resource::PakResourceType::PNG, actual));
    REQUIRE(ReadFile(FixturePath("sprite.png"), expected));
    CHECK(actual == expected);

    REQUIRE(pak.Read(101, k_engine::resource::PakResourceType::PNG, actual));
    REQUIRE(ReadFile(FixturePath("large.png"), expected));
    CHECK(actual == expected);

    REQUIRE(pak.Read(200, k_engine::resource::PakResourceType::ANIMATION, actual));
    REQUIRE(ReadFile(FixturePath("animation.bin"), expected));
    CHECK(actual == expected);
}

TEST_CASE("La apertura no valida los CRC de los payloads")
{
    k_engine::resource::PakReader pak;
    REQUIRE(pak.Open(FixturePath("bad_resource_crc.pak")));

    std::vector<std::uint8_t> data;
    CHECK_FALSE(pak.Read(2, k_engine::resource::PakResourceType::LEVEL, data));
    CHECK(data.empty());
    CHECK(pak.LastError().find("CRC") != std::string::npos);
}

TEST_CASE("Los paquetes inválidos se rechazan al abrir")
{
    constexpr std::array<std::string_view, 18> INVALID_PACKAGES = {
        "bad_signature.pak",    "bad_version.pak",   "bad_count.pak",
        "bad_toc_crc.pak",      "truncated.pak",     "truncated_payload.pak",
        "bad_id_zero.pak",      "duplicate_id.pak",  "unsorted_id.pak",
        "bad_type.pak",         "bad_flags.pak",     "bad_reserved.pak",
        "unaligned_offset.pak", "offset_in_toc.pak", "range_outside.pak",
        "overlap.pak",          "zero_size.pak",     "size_over_limit.pak",
    };

    for (const std::string_view NAME : INVALID_PACKAGES)
    {
        INFO("Paquete: " << NAME);
        k_engine::resource::PakReader pak;
        CHECK_FALSE(pak.Open(FixturePath(NAME)));
        CHECK_FALSE(pak.LastError().empty());
    }
}

TEST_CASE("Un paquete sin entradas es válido a nivel de formato")
{
    k_engine::resource::PakReader pak;
    REQUIRE(pak.Open(FixturePath("empty.pak")));
    CHECK(pak.Entries().empty());
}

TEST_CASE("Las variantes con CRC correcto abren y permiten leer")
{
    constexpr std::array<std::string_view, 3> SEMANTIC_VARIANTS = {
        "bad_level_buffer.pak",
        "bad_png.pak",
        "bad_animation_buffer.pak",
    };

    for (const std::string_view NAME : SEMANTIC_VARIANTS)
    {
        INFO("Paquete: " << NAME);
        k_engine::resource::PakReader pak;
        REQUIRE(pak.Open(FixturePath(NAME)));

        std::vector<std::uint8_t> data;
        CHECK(pak.Read(2, k_engine::resource::PakResourceType::LEVEL, data));
        CHECK_FALSE(data.empty());
    }
}

TEST_CASE("Se comprueba el tipo esperado y la existencia del ID")
{
    k_engine::resource::PakReader pak;
    REQUIRE(pak.Open(FixturePath("runtime_test.pak")));

    std::vector<std::uint8_t> data;
    CHECK_FALSE(pak.Read(2, k_engine::resource::PakResourceType::PNG, data));
    CHECK(data.empty());
    CHECK(pak.LastError().find("Tipo inesperado") != std::string::npos);

    CHECK_FALSE(pak.Read(999, k_engine::resource::PakResourceType::LEVEL, data));
    CHECK(data.empty());
    CHECK(pak.LastError().find("No existe") != std::string::npos);
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
