// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-identifier-naming,readability-identifier-length,bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)
#include <string>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "ui/builder.h"

namespace
{

constexpr float WINDOW_WIDTH = 800.0F;
constexpr float WINDOW_HEIGHT = 600.0F;
constexpr k_engine::ui::Rect BUTTON_RECT{10.0F, 10.0F, 120.0F, 32.0F};

k_engine::ui::Input MousePress(float x, float y)
{
    k_engine::ui::Input input;
    input.mouse = k_engine::ui::Vec2{x, y};
    input.left_down = true;
    input.left_pressed = true;
    return input;
}

k_engine::ui::Input MouseRelease(float x, float y)
{
    k_engine::ui::Input input;
    input.mouse = k_engine::ui::Vec2{x, y};
    input.left_down = false;
    input.left_released = true;
    return input;
}

k_engine::ui::Input MouseOnly(float x, float y)
{
    k_engine::ui::Input input;
    input.mouse = k_engine::ui::Vec2{x, y};
    return input;
}

} // namespace

TEST_CASE("Rect contiene solo su área interior")
{
    const k_engine::ui::Rect RECT{10.0F, 20.0F, 30.0F, 40.0F};
    CHECK(RECT.Contains(k_engine::ui::Vec2{10.0F, 20.0F}));
    CHECK(RECT.Contains(k_engine::ui::Vec2{39.0F, 59.0F}));
    CHECK_FALSE(RECT.Contains(k_engine::ui::Vec2{40.0F, 30.0F}));
    CHECK_FALSE(RECT.Contains(k_engine::ui::Vec2{20.0F, 60.0F}));
}

TEST_CASE("MakeId es estable y distinto de cero")
{
    const auto FIRST = k_engine::ui::MakeId("guardar");
    const auto SECOND = k_engine::ui::MakeId("guardar");
    const auto OTHER = k_engine::ui::MakeId("cancelar");
    CHECK(FIRST == SECOND);
    CHECK(FIRST != OTHER);
    CHECK(FIRST != k_engine::ui::INVALID_ID);
}

TEST_CASE("Button detecta clic completo")
{
    k_engine::ui::Builder builder;
    const auto ID = k_engine::ui::MakeId("boton");
    const k_engine::ui::Rect VIEWPORT{0.0F, 0.0F, WINDOW_WIDTH, WINDOW_HEIGHT};

    builder.BeginFrame(VIEWPORT, MousePress(20.0F, 20.0F));
    CHECK_FALSE(builder.Button(ID, BUTTON_RECT, "Guardar"));
    builder.EndFrame();

    builder.BeginFrame(VIEWPORT, MouseRelease(20.0F, 20.0F));
    CHECK(builder.Button(ID, BUTTON_RECT, "Guardar"));
    builder.EndFrame();
}

TEST_CASE("Button no dispara si el clic termina fuera")
{
    k_engine::ui::Builder builder;
    const auto ID = k_engine::ui::MakeId("boton");
    const k_engine::ui::Rect VIEWPORT{0.0F, 0.0F, WINDOW_WIDTH, WINDOW_HEIGHT};

    builder.BeginFrame(VIEWPORT, MousePress(20.0F, 20.0F));
    CHECK_FALSE(builder.Button(ID, BUTTON_RECT, "Guardar"));
    builder.EndFrame();

    builder.BeginFrame(VIEWPORT, MouseRelease(300.0F, 300.0F));
    CHECK_FALSE(builder.Button(ID, BUTTON_RECT, "Guardar"));
    builder.EndFrame();
}

TEST_CASE("Toggle cambia el valor al hacer clic")
{
    k_engine::ui::Builder builder;
    const auto ID = k_engine::ui::MakeId("toggle");
    const k_engine::ui::Rect VIEWPORT{0.0F, 0.0F, WINDOW_WIDTH, WINDOW_HEIGHT};
    bool value = false;

    builder.BeginFrame(VIEWPORT, MousePress(20.0F, 20.0F));
    CHECK_FALSE(builder.Toggle(ID, BUTTON_RECT, "visible", value));
    builder.EndFrame();

    builder.BeginFrame(VIEWPORT, MouseRelease(20.0F, 20.0F));
    CHECK(builder.Toggle(ID, BUTTON_RECT, "visible", value));
    builder.EndFrame();

    CHECK(value);
}

TEST_CASE("NumberField acepta un valor dentro del rango")
{
    k_engine::ui::Builder builder;
    const auto ID = k_engine::ui::MakeId("numero");
    const k_engine::ui::Rect FIELD_RECT{10.0F, 10.0F, 160.0F, 42.0F};
    const k_engine::ui::Rect VIEWPORT{0.0F, 0.0F, WINDOW_WIDTH, WINDOW_HEIGHT};
    int value = 4;

    builder.BeginFrame(VIEWPORT, MousePress(20.0F, 20.0F));
    CHECK_FALSE(builder.NumberField(ID, FIELD_RECT, "columnas", value, 1, 100));
    builder.EndFrame();

    k_engine::ui::Input input = MouseOnly(20.0F, 20.0F);
    input.text = "42";
    builder.BeginFrame(VIEWPORT, input);
    CHECK(builder.NumberField(ID, FIELD_RECT, "columnas", value, 1, 100));
    builder.EndFrame();

    CHECK(value == 42);
}

TEST_CASE("NumberField no acepta valores fuera del rango")
{
    k_engine::ui::Builder builder;
    const auto ID = k_engine::ui::MakeId("numero");
    const k_engine::ui::Rect FIELD_RECT{10.0F, 10.0F, 160.0F, 42.0F};
    const k_engine::ui::Rect VIEWPORT{0.0F, 0.0F, WINDOW_WIDTH, WINDOW_HEIGHT};
    int value = 4;

    builder.BeginFrame(VIEWPORT, MousePress(20.0F, 20.0F));
    builder.NumberField(ID, FIELD_RECT, "columnas", value, 1, 100);
    builder.EndFrame();

    k_engine::ui::Input input = MouseOnly(20.0F, 20.0F);
    input.text = "500";
    builder.BeginFrame(VIEWPORT, input);
    CHECK_FALSE(builder.NumberField(ID, FIELD_RECT, "columnas", value, 1, 100));
    builder.EndFrame();

    CHECK(value == 4);
}

TEST_CASE("TextField edita una cadena")
{
    k_engine::ui::Builder builder;
    const auto ID = k_engine::ui::MakeId("texto");
    const k_engine::ui::Rect FIELD_RECT{10.0F, 10.0F, 200.0F, 42.0F};
    const k_engine::ui::Rect VIEWPORT{0.0F, 0.0F, WINDOW_WIDTH, WINDOW_HEIGHT};
    std::string value = "nivel";

    builder.BeginFrame(VIEWPORT, MousePress(20.0F, 20.0F));
    builder.TextField(ID, FIELD_RECT, "nombre", value);
    builder.EndFrame();

    k_engine::ui::Input input = MouseOnly(20.0F, 20.0F);
    input.text = "_01";
    builder.BeginFrame(VIEWPORT, input);
    CHECK(builder.TextField(ID, FIELD_RECT, "nombre", value));
    builder.EndFrame();

    CHECK(value == "nivel_01");
}

TEST_CASE("La lista de dibujo agrupa comandos de la misma textura")
{
    k_engine::ui::Builder builder;
    const k_engine::ui::Rect VIEWPORT{0.0F, 0.0F, WINDOW_WIDTH, WINDOW_HEIGHT};
    builder.BeginFrame(VIEWPORT, MouseOnly(0.0F, 0.0F));
    builder.FillRect(k_engine::ui::MakeId("fondo"),
                     k_engine::ui::Rect{0.0F, 0.0F, 50.0F, 50.0F},
                     k_engine::ui::Color::White());
    builder.Text(k_engine::ui::MakeId("texto"),
                 k_engine::ui::Rect{0.0F, 0.0F, 100.0F, 20.0F}, "hola", 1.0F,
                 k_engine::ui::Color::White());
    builder.EndFrame();

    const auto &DRAW_LIST = builder.GetDrawList();
    CHECK_FALSE(DRAW_LIST.vertices.empty());
    REQUIRE(DRAW_LIST.commands.size() == 1);
    CHECK(DRAW_LIST.commands.front().texture.value == 0);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-identifier-naming,readability-identifier-length,bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)
