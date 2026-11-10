#include "editor/cli.h"

#include <charconv>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <system_error>

namespace k_engine::editor
{

namespace
{

bool ParseFrameLimit(std::string_view value, std::uint32_t &frame_limit)
{
    if (value.empty())
    {
        return false;
    }
    std::uint32_t parsed = 0;
    const auto RESULT =
        std::from_chars(value.data(), value.data() + value.size(), parsed);
    if (RESULT.ec != std::errc{} || RESULT.ptr != value.data() + value.size())
    {
        return false;
    }
    frame_limit = parsed;
    return true;
}

} // namespace

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
CliResult ParseArguments(int argc, char **argv, EditorOptions &options,
                         std::string &error)
{
    for (int index = 1; index < argc; ++index)
    {
        // Los argumentos se leen con los límites comprobados por argc.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const std::string_view ARGUMENT(argv[index]);
        if (ARGUMENT == "--help" || ARGUMENT == "-h")
        {
            return CliResult::HELP;
        }
        if (ARGUMENT == "--frames")
        {
            if (index + 1 >= argc)
            {
                error = "Falta el valor de --frames.";
                return CliResult::ERROR;
            }
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            const std::string_view VALUE(argv[index + 1]);
            if (!ParseFrameLimit(VALUE, options.frame_limit))
            {
                error = "Valor inválido para --frames: " + std::string(VALUE);
                return CliResult::ERROR;
            }
            ++index;
            continue;
        }
        if (ARGUMENT == "--project")
        {
            if (index + 1 >= argc)
            {
                error = "Falta el valor de --project.";
                return CliResult::ERROR;
            }
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            options.project_path = argv[index + 1];
            ++index;
            continue;
        }
        if (ARGUMENT == "--new")
        {
            if (index + 1 >= argc)
            {
                error = "Falta el valor de --new.";
                return CliResult::ERROR;
            }
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            options.new_directory = argv[index + 1];
            ++index;
            continue;
        }
        if (ARGUMENT == "--import-png")
        {
            if (index + 1 >= argc)
            {
                error = "Falta el valor de --import-png.";
                return CliResult::ERROR;
            }
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            options.import_png = argv[index + 1];
            ++index;
            continue;
        }

        error = "Argumento no reconocido: " + std::string(ARGUMENT);
        return CliResult::ERROR;
    }

    if (!options.project_path.empty() && !options.new_directory.empty())
    {
        error = "--project y --new no pueden usarse juntos.";
        return CliResult::ERROR;
    }
    if (!options.import_png.empty() && options.project_path.empty() &&
        options.new_directory.empty())
    {
        error = "--import-png requiere --project o --new.";
        return CliResult::ERROR;
    }
    return CliResult::OK;
}

void PrintUsage()
{
    std::cout << "Uso: editor [--project RUTA] [--new DIRECTORIO] [--frames N]\n"
              << "             [--import-png RUTA]\n"
              << "  --project RUTA     Abre el project.json indicado.\n"
              << "  --new DIRECTORIO   Crea un proyecto nuevo en el directorio.\n"
              << "  --frames N         Cierra tras presentar N frames; 0 hasta cerrar.\n"
              << "  --import-png RUTA  Importa un PNG al proyecto al arrancar.\n";
}

} // namespace k_engine::editor
