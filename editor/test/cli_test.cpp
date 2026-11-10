#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-identifier-naming,readability-identifier-length)
#include <cstdint>
#include <initializer_list>
#include <string>
#include <vector>

#include "editor/cli.h"

namespace
{

k_engine::editor::CliResult Parse(std::initializer_list<std::string> arguments,
                                  k_engine::editor::EditorOptions &options,
                                  std::string &error)
{
    std::vector<std::string> storage(arguments);
    std::vector<char *> pointers;
    pointers.reserve(storage.size());
    for (std::string &argument : storage)
    {
        pointers.push_back(argument.data());
    }
    return k_engine::editor::ParseArguments(static_cast<int>(pointers.size()),
                                            pointers.data(), options, error);
}

} // namespace

TEST_CASE("CLI acepta proyecto y frames")
{
    k_engine::editor::EditorOptions options;
    std::string error;
    CHECK(Parse({"editor", "--project", "proyecto/project.json", "--frames", "10"},
                options, error) == k_engine::editor::CliResult::OK);
    CHECK(options.project_path == "proyecto/project.json");
    CHECK(options.frame_limit == 10);
    CHECK(error.empty());
}

TEST_CASE("CLI acepta proyecto nuevo e importación")
{
    k_engine::editor::EditorOptions options;
    std::string error;
    CHECK(Parse({"editor", "--new", "demo", "--import-png", "sprite.png"}, options,
                error) == k_engine::editor::CliResult::OK);
    CHECK(options.new_directory == "demo");
    CHECK(options.import_png == "sprite.png");
}

TEST_CASE("CLI rechaza --project y --new juntos")
{
    k_engine::editor::EditorOptions options;
    std::string error;
    CHECK(Parse({"editor", "--project", "a", "--new", "b"}, options, error) ==
          k_engine::editor::CliResult::ERROR);
    CHECK_FALSE(error.empty());
}

TEST_CASE("CLI rechaza frames inválidos")
{
    k_engine::editor::EditorOptions options;
    std::string error;
    CHECK(Parse({"editor", "--frames", "abc"}, options, error) ==
          k_engine::editor::CliResult::ERROR);
}

TEST_CASE("CLI acepta ayuda")
{
    k_engine::editor::EditorOptions options;
    std::string error;
    CHECK(Parse({"editor", "--help"}, options, error) ==
          k_engine::editor::CliResult::HELP);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-identifier-naming,readability-identifier-length)
