---
description: Configura y compila K_engine
agent: build
---

Configura y compila K_engine.

1. Ejecuta `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug`.
2. Ejecuta `cmake --build build`.
3. No añadas targets ni pruebas al CMake raíz: cada componente C++ las define en
   su propio `CMakeLists.txt`.
4. Si falla, analiza el error, corrige solo los archivos permitidos y vuelve a
   intentarlo.
5. Informa los objetivos compilados, los errores y las advertencias relevantes.

No instales dependencias ni modifiques archivos de configuración sin aprobación.
