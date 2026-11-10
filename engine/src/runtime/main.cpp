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

void PrintUsage()
{
    std::cout << "Uso: k_engine_runtime [--frames N]\n"
              << "  --frames N  Presenta N frames y termina; 0 presenta hasta cerrar.\n";
}

} // namespace

int main(int argc, char **argv)
{
    k_engine::runtime::RuntimeOptions options{};
    options.frame_limit = DEFAULT_FRAME_LIMIT;

    for (int index = 1; index < argc; ++index)
    {
        // Los argumentos se leen con los límites comprobados por argc.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const std::string_view ARGUMENT(argv[index]);

        if (ARGUMENT == "--help" || ARGUMENT == "-h")
        {
            PrintUsage();
            return 0;
        }

        if (ARGUMENT == "--frames")
        {
            if (index + 1 >= argc)
            {
                std::cerr << "Falta el valor de --frames.\n";
                return 2;
            }

            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            const std::string_view VALUE(argv[index + 1]);
            if (!ParseFrameLimit(VALUE, options.frame_limit))
            {
                std::cerr << "Valor inválido para --frames: " << VALUE << '\n';
                return 2;
            }

            ++index;
            continue;
        }

        std::cerr << "Argumento no reconocido: " << ARGUMENT << '\n';
        PrintUsage();
        return 2;
    }

    k_engine::runtime::VulkanRuntime runtime;
    if (!runtime.Initialize())
    {
        runtime.Shutdown();
        return 1;
    }

    const int RESULT = runtime.Run(options);
    runtime.Shutdown();
    return RESULT;
}
