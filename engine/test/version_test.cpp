#include <cstring>
#include <iostream>

#include "k_engine.h"

int main()
{
    if (k_engine::VERSION_MAJOR != 0 || k_engine::VERSION_MINOR != 1 ||
        k_engine::VERSION_PATCH != 0)
    {
        std::cerr << "Versión inesperada.\n";
        return 1;
    }

    if (std::strcmp(k_engine::VersionString(), "0.1.0") != 0)
    {
        std::cerr << "Cadena de versión inesperada.\n";
        return 1;
    }

    std::cout << "Pruebas de versión correctas.\n";
    return 0;
}
