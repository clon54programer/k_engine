---
description: Compila y ejecuta las pruebas de K_engine
agent: build
---

Compila y ejecuta las pruebas de K_engine.

1. Si no existe `build/`, configura el proyecto con
   `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug`.
2. Compila con `cmake --build build`.
3. Ejecuta `ctest --test-dir build --output-on-failure`.
4. Informa las pruebas ejecutadas, las fallidas y las que no pudieron ejecutarse,
   además de los errores pendientes.

No marques una prueba como aprobada si no se ejecutó.
