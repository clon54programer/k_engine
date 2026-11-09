# Informe de avance: T0 de K_engine

Fecha: 2026-11-09

Este documento resume el trabajo realizado en T0: congelar el contrato del
contenedor `.pak`, preparar las dependencias y dejar un build mínimo reproducible
en Linux. Está escrito para personas: explica qué se hizo, qué archivos se
crearon, qué problemas aparecieron y qué queda para las siguientes etapas.

---

## 1. Resumen de tareas y estado del proyecto

### T0: contrato y entorno

Objetivo: dejar fijados el formato `.pak` v1, las dependencias del MVP y un build
C++17/CMake que compile sin GLFW ni FlatBuffers instalados globalmente.

Qué se hizo:

- **Auditoría del entorno**
  - GCC 13.3.0, CMake 3.28.3, Ninja 1.11.1 y Python 3.12.3.
  - Vulkan SDK/loader/capa de validación 1.4.363.
  - GPU AMD Radeon Graphics (RENOIR), PCI `1002:15e7`, con RADV de Mesa 25.2.8.
  - `llvmpipe` se detectó como dispositivo adicional, pero no se usa como GPU
    objetivo.
  - Se comprobó que `flatc` y GLFW no están instalados globalmente.

- **Submodules fijados**
  - GLFW `3.5.1`.
  - FlatBuffers `v25.12.19`.
  - nlohmann/json `v3.12.0`.
  - stb, commit `4e9b368bfc071e97206ea5c6adba68dd0cbf8897`.
  - doctest `v2.5.3`.

- **Contrato `.pak` v1**
  - Cabecera de 16 bytes y entradas de TOC de 40 bytes.
  - Little-endian, offsets absolutos, payloads alineados a 16 bytes.
  - CRC-32/ISO-HDLC para tabla y recursos.
  - Enum de tipos y valores reservados.
  - Límites de TOC, FlatBuffers, PNG, niveles y animaciones.
  - Política de versión exacta `1.0`.
  - Documentado en `docs/pak-v1.md`.

- **Esquemas de datos**
  - Esquemas JSON de desarrollo para proyecto, nivel y animación.
  - Esquemas FlatBuffers `level.fbs` y `animation.fbs`, con identificadores
    `KLVL` y `KANI`.
  - Documentado en `docs/json-schemas.md`.

- **FlatBuffers Python → C++**
  - `flatc` se compila desde `third_party/flatbuffers`.
  - Se genera código C++ y Python en el directorio de build.
  - Un generador Python crea un `Level` mínimo y un lector C++ lo verifica con
    `flatbuffers::Verifier`.
  - El buffer generado tiene 272 bytes y se valida correctamente.

- **CMake, GLFW y Vulkan**
  - `find_package(Vulkan REQUIRED)` encuentra el SDK 1.4.363.
  - GLFW se compila con X11 y sin Wayland.
  - Se enlaza una prueba mínima contra GLFW y Vulkan, sin crear ventana,
    superficie ni swapchain.
  - Se integran nlohmann/json, stb y doctest.
  - stb tiene sus implementaciones en un único archivo C++.

- **CMake por componente**
  - El CMake raíz solo configura dependencias comunes, `enable_testing()` y
    `add_subdirectory()`.
  - `engine/CMakeLists.txt` define la biblioteca, la generación FlatBuffers y
    todas las pruebas del engine.
  - `editor/CMakeLists.txt` define el ejecutable del editor.
  - La regla quedó documentada en `AGENTS.md` y `readme.md`.

### Estado general y pruebas

- Se ejecutan **5 pruebas automáticas** y todas pasan.
- También se hizo un build limpio desde cero en
  `/tmp/opencode/k_engine_clean_build_policy`, con 5/5 pruebas correctas.
- `clang-tidy` y `clang-format` pasan en los archivos C++ nuevos.
- No se instaló `flatc` ni GLFW globalmente.
- No se creó todavía ninguna ventana, superficie X11, swapchain ni frame; eso
  pertenece a T1.

Comprobar el estado en cualquier momento:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Resultado esperado:

```text
100% tests passed, 0 tests failed out of 5
```

---

## 2. Archivos creados o modificados

### Archivos nuevos

| Archivo | Para qué sirve |
|---|---|
| `docs/pak-v1.md` | Contrato binario del contenedor `.pak` v1. |
| `docs/json-schemas.md` | Esquemas JSON de proyecto, nivel y animación. |
| `docs/report-t0.md` | Este informe de avance. |
| `engine/schema/level.fbs` | Esquema FlatBuffers del nivel. |
| `engine/schema/animation.fbs` | Esquema FlatBuffers de la animación. |
| `engine/CMakeLists.txt` | Biblioteca, generación FlatBuffers y pruebas del engine. |
| `editor/CMakeLists.txt` | Ejecutable y futuras pruebas del editor. |
| `engine/src/stb_impl.cpp` | Implementaciones únicas de `stb_image` y `stb_truetype`. |
| `engine/test/flatbuffers_roundtrip_test.cpp` | Lector C++ que verifica el buffer generado por Python. |
| `engine/test/dependencies_smoke_test.cpp` | Prueba de nlohmann/json, stb y doctest. |
| `engine/test/glfw_vulkan_build_test.cpp` | Prueba de enlace de GLFW y Vulkan. |
| `engine/test/generate_level_buffer.py` | Generador Python del buffer Level de prueba. |

### Archivos que ya existían y se modificaron

| Archivo | Cambio principal |
|---|---|
| `CMakeLists.txt` | Ahora solo configura dependencias y añade los componentes `engine` y `editor`. |
| `engine/CMakeLists.txt` | Recibe la biblioteca, FlatBuffers y todas las pruebas del engine. |
| `deps.md` | Dependencias X11 completas, versiones de submodules y decisiones de JSON/PNG/doctest. |
| `AGENTS.md` | Nueva política de CMake y pruebas por componente. |
| `readme.md` | Estructura actualizada con CMake por componente y documentos nuevos. |
| `.opencode/commands/build.md` | Regla de no añadir targets al CMake raíz. |
| `.opencode/commands/test.md` | Regla de registrar pruebas en el CMake del componente. |
| `todo.md` | T0 marcado como completado y propuestas pendientes actualizadas. |

### Archivos movidos

| Archivo | Cambio |
|---|---|
| `packer/tests/generate_level_buffer.py` | Se movió a `engine/test/generate_level_buffer.py`, porque es un fixture de la prueba de integración del engine. |

---

## 3. Errores encontrados y cómo se resolvieron

1. **Faltaban los headers de desarrollo de X11**
   - Qué pasó: CMake no encontraba `X11_X11_LIB` y GLFW no podía configurarse.
   - Solución: se instalaron los paquetes X11 indicados en `deps.md`. Se añadió
     `libxext-dev`, necesario para `X11/extensions/shape.h`, que exige GLFW 3.5.1.
   - Estado: **resuelto**.

2. **GLFW activaba Wayland por defecto**
   - Qué pasó: el entorno no tiene `libwayland-dev`, así que el build podía
     fallar aunque el MVP use X11.
   - Solución: se configuró `GLFW_BUILD_WAYLAND=OFF` y se documentó en
     `deps.md`.
   - Estado: **resuelto**.

3. **Importación incorrecta de `LayerKind` en Python**
   - Qué pasó: `LayerKind` se importaba como módulo y no como clase, por lo que
     `LayerKind.tile` no existía.
   - Solución: importar la clase desde
     `k_engine.schema.LayerKind`.
   - Estado: **resuelto**.

4. **Struct FlatBuffers fuera de su posición inline**
   - Qué pasó: el `Grid` se creaba antes de `Level.Start()` y FlatBuffers lo
     rechazaba por no estar en la posición actual del builder.
   - Solución: crear el struct inmediatamente antes de `Level.AddGrid()`.
   - Estado: **resuelto**.

5. **`clang-tidy` detectaba errores en el test C++**
   - Qué pasó: números mágicos, arreglo C, acceso por índice no constante,
     `size() == 0` y complejidad de `main`.
   - Solución: constantes con nombre, `std::array`, bucle `range-for`,
     `empty()`, funciones auxiliares y un `NOLINT` justificado para `argv[1]`.
   - Estado: **resuelto**.

6. **Conflicto entre el ejecutable antiguo `build/editor` y el nuevo directorio del componente**
   - Qué pasó: al mover el target del editor a `editor/CMakeLists.txt`, el build
     necesitaba crear `build/editor/` y ya existía el ejecutable anterior.
   - Solución: se eliminó el artefacto antiguo `build/editor` y se verificó con
     un build limpio.
   - Estado: **resuelto**.

7. **`flatc` no estaba disponible globalmente**
   - Qué pasó: el proyecto no depende de un `flatc` instalado en el sistema.
   - Solución: se compila desde el submodule FlatBuffers y se ejecuta desde el
     directorio de build.
   - Estado: **resuelto**.

### Pendientes (no son fallos actuales, son mejoras futuras)

- **T1:** instancia Vulkan 1.3, selección explícita de la AMD, Validation Layer,
  ventana GLFW, superficie X11, swapchain y primer frame.
- **T2–T5:** runtime mínimo, editor, CLI empaquetadora, integración end-to-end,
  rendimiento y entrega.
- **CRC-32 en C++:** deps.md mantiene pendiente la decisión de implementación
  (implementación propia o zlib como submodule).
- **CLI del packer:** todavía no existe; la prueba FlatBuffers actual es un
  fixture del engine.
- **Pruebas de rendimiento:** no se han ejecutado porque aún no hay escena ni
  presentación Vulkan.
