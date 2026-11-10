#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "resource/crc32.h"

TEST_CASE("CRC-32/ISO-HDLC vectores conocidos")
{
    CHECK(k_engine::resource::ComputeCrc32(nullptr, 0) == 0U);

    const std::string CHECK_VECTOR = "123456789";
    const std::vector<std::uint8_t> CHECK_BYTES(CHECK_VECTOR.begin(), CHECK_VECTOR.end());
    CHECK(k_engine::resource::ComputeCrc32(CHECK_BYTES.data(), CHECK_BYTES.size()) ==
          0xCBF43926U);

    const std::string FOX = "The quick brown fox jumps over the lazy dog";
    const std::vector<std::uint8_t> FOX_BYTES(FOX.begin(), FOX.end());
    CHECK(k_engine::resource::ComputeCrc32(FOX_BYTES.data(), FOX_BYTES.size()) ==
          0x414FA339U);
}

TEST_CASE("CRC-32 por bloques coincide con una sola actualización")
{
    constexpr std::array<std::uint8_t, 10> DATA = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    const std::uint32_t EXPECTED =
        k_engine::resource::ComputeCrc32(DATA.data(), DATA.size());

    k_engine::resource::Crc32 crc;
    crc.Update(DATA.data(), 3);
    crc.Update(&DATA[3], 4);
    crc.Update(&DATA[7], 3);
    CHECK(crc.Value() == EXPECTED);
}
