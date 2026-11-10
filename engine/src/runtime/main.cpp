#include <charconv>
#include <cstdint>
#include <iostream>
#include <string_view>
#include <system_error>

#include "runtime/vulkan_runtime.h"

namespace
{

constexpr std::uint32_t DEFAULT_FRAME_LIMIT = 0;

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

bool ParseLevelId(std::string_view value, std::uint64_t &level_id)
{
    if (value.empty())
    {
        return false;
    }

    std::uint64_t parsed = 0;
    const auto RESULT =
        std::from_chars(value.data(), value.data() + value.size(), parsed);
    if (RESULT.ec != std::errc{} || RESULT.ptr != value.data() + value.size())
    {
        return false;
    }

    level_id = parsed;
    return true;
}

void PrintUsage()
{
    std::cout << "Uso: k_engine_runtime [--frames N] [--pak RUTA] [--level-id N]\n"
              << "  --frames N    Presenta N frames y termina; 0 presenta hasta cerrar.\n"
              << "  --pak RUTA    Carga el paquete y renderiza su sprite.\n"
              << "  --level-id N  Nivel a cargar; 0 o ausente usa el primero.\n";
}

enum class ArgumentResult : std::uint8_t
{
    OK,
    HELP,
    ERROR,
};

ArgumentResult ParseArguments(int argc, char **argv,
                              k_engine::runtime::RuntimeOptions &options)
{
    for (int index = 1; index < argc; ++index)
    {
        // Los argumentos se leen con los límites comprobados por argc.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const std::string_view ARGUMENT(argv[index]);

        if (ARGUMENT == "--help" || ARGUMENT == "-h")
        {
            PrintUsage();
            return ArgumentResult::HELP;
        }

        if (ARGUMENT == "--frames")
        {
            if (index + 1 >= argc)
            {
                std::cerr << "Falta el valor de --frames.\n";
                return ArgumentResult::ERROR;
            }

            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            const std::string_view VALUE(argv[index + 1]);
            if (!ParseFrameLimit(VALUE, options.frame_limit))
            {
                std::cerr << "Valor inválido para --frames: " << VALUE << '\n';
                return ArgumentResult::ERROR;
            }

            ++index;
            continue;
        }

        if (ARGUMENT == "--pak")
        {
            if (index + 1 >= argc)
            {
                std::cerr << "Falta el valor de --pak.\n";
                return ArgumentResult::ERROR;
            }

            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            options.package_path = argv[index + 1];
            ++index;
            continue;
        }

        if (ARGUMENT == "--level-id")
        {
            if (index + 1 >= argc)
            {
                std::cerr << "Falta el valor de --level-id.\n";
                return ArgumentResult::ERROR;
            }

            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            const std::string_view VALUE(argv[index + 1]);
            if (!ParseLevelId(VALUE, options.level_id))
            {
                std::cerr << "Valor inválido para --level-id: " << VALUE << '\n';
                return ArgumentResult::ERROR;
            }

            ++index;
            continue;
        }

        std::cerr << "Argumento no reconocido: " << ARGUMENT << '\n';
        PrintUsage();
        return ArgumentResult::ERROR;
    }

    return ArgumentResult::OK;
}

} // namespace

int main(int argc, char **argv)
{
    k_engine::runtime::RuntimeOptions options{};
    options.frame_limit = DEFAULT_FRAME_LIMIT;

    const ArgumentResult ARGUMENT_RESULT = ParseArguments(argc, argv, options);
    if (ARGUMENT_RESULT == ArgumentResult::HELP)
    {
        return 0;
    }
    if (ARGUMENT_RESULT == ArgumentResult::ERROR)
    {
        return 2;
    }

    k_engine::runtime::VulkanRuntime runtime;
    if (!runtime.Initialize(options))
    {
        runtime.Shutdown();
        return 1;
    }

    const int RESULT = runtime.Run(options);
    runtime.Shutdown();
    return RESULT;
}
