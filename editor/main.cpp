#include <iostream>
#include <string>

#include "editor/app.h"
#include "editor/cli.h"

int main(int argc, char **argv)
{
    k_engine::editor::EditorOptions options;
    std::string error;
    const k_engine::editor::CliResult RESULT =
        k_engine::editor::ParseArguments(argc, argv, options, error);
    if (RESULT == k_engine::editor::CliResult::HELP)
    {
        k_engine::editor::PrintUsage();
        return 0;
    }
    if (RESULT == k_engine::editor::CliResult::ERROR)
    {
        std::cerr << "Error: " << error << '\n';
        k_engine::editor::PrintUsage();
        return 2;
    }

    k_engine::editor::EditorApp app;
    if (!app.Initialize(options))
    {
        std::cerr << "Error: no se pudo iniciar el editor.\n";
        app.Shutdown();
        return 1;
    }

    const int RUN_RESULT = app.Run(options);
    app.Shutdown();
    return RUN_RESULT;
}
