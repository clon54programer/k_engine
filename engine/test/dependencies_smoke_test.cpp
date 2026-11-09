#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <nlohmann/json.hpp>
#include <stb_image.h>
#include <stb_truetype.h>

TEST_CASE("Dependencias de código del MVP")
{
    const auto PAYLOAD = nlohmann::json::parse(R"({"ok": true})");
    CHECK(PAYLOAD["ok"].get<bool>());
}
