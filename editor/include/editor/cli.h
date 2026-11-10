#pragma once

#include <cstdint>
#include <string>

namespace k_engine::editor
{

struct EditorOptions
{
    std::string project_path;
    std::string new_directory;
    std::string import_png;
    std::uint32_t frame_limit = 0;
};

enum class CliResult : std::uint8_t
{
    OK,
    HELP,
    ERROR,
};

CliResult ParseArguments(int argc, char **argv, EditorOptions &options,
                         std::string &error);
void PrintUsage();

} // namespace k_engine::editor
