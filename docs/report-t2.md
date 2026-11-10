# Informe de avance: T2 de K_engine

Fecha: 2026-11-09

Este documento resume el trabajo realizado en T2: convertir el prototipo Vulkan
de T1 en un runtime Linux mínimo capaz de leer un `.pak` v1, cargar un sprite y
presentarlo con input de teclado y ratón. Está escrito para personas: explica
qué se implementó, qué archivos se crearon, qué problemas aparecieron y qué
queda para las siguientes etapas.

---

## 1. Resumen de T2

### Objetivo

Implementar el bucle principal, el input, el lector `.pak`, la verificación de
FlatBuffers y el renderizado de un sprite usando un paquete de prueba, sin
esperar a que exista la CLI empaquetadora de T4.

### Qué se hizo

- **Lector `.pak` v1**
  - Cabecera de 16 bytes y tabla de contenidos de entradas de 40 bytes.
  - Parseo little-endian byte a byte.
  - Validación de firma, versión `1.0`, cantidad de entradas, tamaño de TOC,
    CRC de tabla, flags, campo reservado, IDs, tipos, offsets, tamaños, rangos
    y solapamientos.
  - Lectura de payloads bajo demanda con `std::ifstream::seekg` y bloques de
    64 KiB.
  - Verificación de CRC de cada recurso antes de entregarlo.
  - Límites del contrato: TOC de 4 MiB, Level/Animation/PNG codificado de
    64 MiB y máximo de 104 857 entradas.
  - `Open` no lee payloads: solo cabecera y TOC. La prueba de CRC incorrecto
    demuestra que el paquete abre y el fallo aparece al leer el recurso.

- **CRC-32/ISO-HDLC**
  - Implementación interna incremental, sin zlib ni dependencias nuevas.
  - Polinomio reflejado `0xEDB88320`, valor inicial y XOR final `0xFFFFFFFF`.
  - Vectores conocidos verificados, incluido `"123456789"` → `0xCBF43926`.
  - Prueba de actualización por bloques.

- **Carga de contenido**
  - Selección del nivel por `--level-id` o por la primera entrada Level.
  - Verificación del buffer Level con `flatbuffers::Verifier` e identificador
    `KLVL` antes de leer sus campos.
  - Comprobación de `schema_version`, ID interno, cuadrícula, tilesets y capas.
  - Selección del sprite desde la primera entidad con `image_id` o, si no
    existe, desde el primer tileset con imagen.
  - Lectura y decodificación PNG con `stb_image`.
  - Validación de dimensiones 1..4096 y de memoria decodificada ≤ 64 MiB antes
    de reservar.
  - Verificación de buffers Animation con `VerifyAnimationBuffer` e
    identificador `KANI` en las pruebas. T2 no reproduce animaciones.

- **Renderizado 2D**
  - Shaders GLSL para un quad texturizado.
  - Compilación a SPIR-V con `glslc` y embebido en una cabecera C++ generada.
  - Descriptor set para sampler e imagen.
  - Subida de textura a `VkImage` con staging buffer y transiciones de layout.
  - Pipeline gráfico con vertex buffer, push constants, viewport/scissor
    dinámicos y blending.
  - Recreación del pipeline al recrear el render pass de la swapchain; la
    textura y el descriptor set sobreviven al resize.
  - El modo sin `--pak` conserva el color de limpieza animado de T1.

- **Input y bucle principal**
  - `InputState` independiente de GLFW; el runtime traduce los callbacks.
  - Flechas y WASD mueven el sprite a 200 px/s.
  - Clic izquierdo recoloca el sprite centrado en el cursor.
  - ESC y el cierre de ventana terminan el bucle.
  - Cierre idempotente y liberación ordenada de textura, descriptor set,
    pipeline, swapchain y dispositivo.

- **CLI del runtime**
  - `--frames N`, `--pak RUTA`, `--level-id N` y `--help`.
  - `--frames 0` presenta hasta cerrar.
  - Los argumentos se validan y los errores devuelven código distinto de cero.

### Estado general y pruebas

- Se ejecutan **9 pruebas automáticas** y todas pasan.
- El runtime carga el paquete de prueba y muestra el sprite en la GPU AMD.
- La Validation Layer no registra errores ni advertencias atribuibles al
  engine.
- La verificación de input, resize y cierre se hizo en la sesión X11 con la GPU
  AMD de referencia.

Comprobar el estado en cualquier momento:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure

./build/engine/k_engine_runtime \
  --pak build/engine/test_runtime_fixture/runtime_test.pak \
  --frames 1
```

Resultado esperado de las pruebas:

```text
100% tests passed, 0 tests failed out of 9
```

Salida relevante del runtime:

```text
GPU seleccionada: AMD Radeon Graphics (RADV RENOIR).
Swapchain creado: 1280x720, 4 imágenes, formato 50.
Paquete cargado: .../runtime_test.pak (4 recursos).
Nivel cargado: 2 "runtime_level".
Sprite cargado: PNG 100 (64x64).
Validación: 0 errores, 0 advertencias del engine, 0 advertencias del ICD sobre VK_KHR_display.
```

---

## 2. Archivos creados o modificados

### Archivos nuevos

| Archivo | Para qué sirve |
|---|---|
| `engine/include/resource/crc32.h` | Interfaz del CRC-32/ISO-HDLC incremental. |
| `engine/include/resource/pak_reader.h` | Interfaz y entradas del lector `.pak` v1. |
| `engine/include/runtime/content_loader.h` | Interfaz de carga de nivel, sprite y animación verificada. |
| `engine/include/runtime/input_state.h` | Estado abstracto de teclado, cursor y clic. |
| `engine/include/runtime/sprite_renderer.h` | Interfaz del pipeline, textura y draw del sprite. |
| `engine/src/resource/crc32.cpp` | Implementación del CRC-32/ISO-HDLC. |
| `engine/src/resource/pak_reader.cpp` | Cabecera, TOC, validaciones y lectura bajo demanda. |
| `engine/src/runtime/content_loader.cpp` | Verificación FlatBuffers y decodificación PNG. |
| `engine/src/runtime/input_state.cpp` | Implementación del estado de input. |
| `engine/src/runtime/sprite_renderer.cpp` | Textura, descriptor set, pipeline y draw. |
| `engine/shaders/sprite.vert` | Vertex shader del quad del sprite. |
| `engine/shaders/sprite.frag` | Fragment shader con muestreo de textura. |
| `engine/tools/embed_spirv.py` | Convierte SPIR-V en una cabecera C++ embebida. |
| `engine/test/crc32_test.cpp` | Vectores CRC y actualización por bloques. |
| `engine/test/pak_reader_test.cpp` | Pruebas del contrato y de las variantes inválidas. |
| `engine/test/runtime_content_test.cpp` | Verificación de Level/Animation y PNG. |
| `engine/test/generate_runtime_fixture.py` | Fixture de T2 y variantes corruptas, solo para tests. |
| `docs/report-t2.md` | Este informe de avance. |

### Archivos que ya existían y se modificaron

| Archivo | Cambio principal |
|---|---|
| `engine/CMakeLists.txt` | Fuentes de T2, shaders, objetivos de prueba y fixture. |
| `engine/src/runtime/main.cpp` | `--pak`, `--level-id` y nueva inicialización. |
| `engine/src/runtime/vulkan_runtime.cpp` | Carga de contenido, sprite, input, draw y limpieza. |
| `todo.md` | T2 marcado como completado y enlace a este informe. |

### Archivos movidos

| Archivo | Cambio |
|---|---|
| `engine/src/runtime/vulkan_runtime.h` | Se movió a `engine/include/runtime/vulkan_runtime.h` para separar cabeceras de implementaciones. |

### Archivos que no se modificaron

- `CMakeLists.txt` raíz.
- `third_party/**`.
- `engine/schema/**`.
- `editor/**`.
- `packer/**`.
- `docs/pak-v1.md` y `docs/json-schemas.md`.
- `docs/report-t0.md` y `docs/report-t1.md`.

---

## 3. Pruebas y verificaciones

### Pruebas automáticas

| Prueba | Resultado |
|---|---|
| `k_engine_version_test` | Pasa. |
| `k_engine_flatbuffers_python_generate` | Pasa. |
| `k_engine_flatbuffers_python_to_cpp` | Pasa. |
| `k_engine_dependencies_smoke_test` | Pasa. |
| `k_engine_glfw_vulkan_build_test` | Pasa. |
| `k_engine_t2_fixture_generate` | Pasa. |
| `k_engine_crc32_test` | Pasa. |
| `k_engine_pak_reader_test` | Pasa: 6 casos, 81 aserciones. |
| `k_engine_runtime_content_test` | Pasa. |

Resultado global:

```text
100% tests passed, 0 tests failed out of 9
```

### Pruebas del lector `.pak`

Se cubrieron, entre otras, estas variantes:

- firma incorrecta;
- versión no soportada;
- tabla truncada;
- cantidad de entradas fuera de límite;
- CRC de TOC incorrecto;
- ID cero, duplicado o desordenado;
- tipo inválido o reservado;
- flags o campo reservado distintos de cero;
- offset no alineado o dentro de la cabecera/TOC;
- rango fuera del archivo;
- solapamiento de payloads;
- tamaño cero o mayor que el límite del tipo;
- CRC de recurso incorrecto, con apertura correcta y fallo al leer;
- payload truncado;
- paquete sin entradas;
- lectura por bloques con un PNG de más de 64 KiB;
- Level, Animation y PNG corruptos con CRC de recurso correcto, rechazados por
  la verificación semántica correspondiente.

El fixture usa `zlib.crc32` de Python para que el CRC de las pruebas sea
independiente de la implementación C++.

### Runtime y GPU

- `./build/engine/k_engine_runtime --pak ... --frames 1`: salida `0`.
- `./build/engine/k_engine_runtime --pak ... --frames 300`: salida `0` en
  5,19 s.
- Build limpio Debug en `/tmp/opencode/k_engine_t2_clean_build_final`:
  `ctest` 9/9; runtime de 1 y 300 frames con salida `0`.
- Build Release en `/tmp/opencode/k_engine_t2_release_build_final`: compila y
  `ctest` 9/9; runtime de 1 frame con salida `0`, sin Validation Layer por
  `NDEBUG`.
- Resize de `1280 × 720` a `800 × 600`, minimizado, restaurado y cierre:
  salida `0`, con 0 errores y 0 advertencias del engine.
- Tres aperturas/cierres consecutivos: sin errores de Vulkan ni resumen de
  validación duplicado.

### Verificación de input

La comprobación se hizo en la sesión X11 con la AMD de referencia:

- Flecha derecha durante 1 s: el cuadrante rojo del sprite pasó de
  `x = 64..95` a `x = 264..295`, es decir, 200 px.
- Clic izquierdo en `(600, 300)`: el sprite quedó centrado y su esquina
  superior izquierda pasó a `(568, 268)`.
- Escape: el proceso terminó con salida `0`.

No se añadió una prueba CTest de ventana/input porque requiere una sesión X11
y la GPU AMD. No se instaló `xdotool`; la verificación se hizo con herramientas
ya disponibles en el entorno.

### Calidad de código

- `clang-format --dry-run --Werror`: correcto en los archivos nuevos y
  modificados.
- `clang-tidy -p build ...`: sin errores ni advertencias en el código nuevo,
  con `header-filter` sobre `engine/src` y `engine/include`.

---

## 4. Errores encontrados y cómo se resolvieron

1. **Sprite invertido verticalmente**
   - Qué pasó: el shader aplicaba `ndc.y = -ndc.y`, por lo que el sprite
     aparecía cerca del borde inferior en lugar de en la posición del nivel.
   - Solución: se eliminó la inversión; Vulkan ya usa el origen superior
     izquierdo en la transformación de viewport.
   - Estado: **resuelto**; verificado con captura y posiciones de píxeles.

2. **`PakReader::Read` no era const y `ContentLoader` recibía `const PakReader&`**
   - Qué pasó: la compilación rechazaba la llamada porque `Read` modificaba el
     flujo y el mensaje de error.
   - Solución: `Read` pasó a ser lógico-const con `mutable std::ifstream` y
     `mutable std::string`, ya que solo cambia la posición de lectura y el
     último error, no la tabla.
   - Estado: **resuelto**.

3. **Cabeceras dentro de `engine/src`**
   - Qué pasó: las cabeceras nuevas y `vulkan_runtime.h` estaban mezcladas con
     las implementaciones.
   - Solución: se movieron a `engine/include/resource` y
     `engine/include/runtime`, y se actualizaron los directorios de include de
     CMake.
   - Estado: **resuelto**.

4. **Struct FlatBuffers fuera de su posición inline**
   - Qué pasó: el fixture de T2 debía respetar la regla ya conocida de T0:
     `Grid`, `Vec2i`, `Recti` y `Frame` se crean inmediatamente antes de su
     campo.
   - Solución: el generador crea los structs en el orden correcto.
   - Estado: **resuelto**.

5. **Avisos de `clang-tidy` en el parser binario**
   - Qué pasó: mágic numbers, aritmética de punteros, parámetros
     intercambiables y `enum` con tipo base innecesariamente ancho.
   - Solución: constantes con nombre, `std::array::at`, enums con base
     `std::uint8_t`, `NOLINT` justificados para el decodificador de bytes y
     separación de la función de parseo.
   - Estado: **resuelto**.

6. **Fixture Release sin generar antes de ejecutar**
   - Qué pasó: el build Release no ejecuta CTest, así que el `.pak` de prueba
     todavía no existía.
   - Solución: ejecutar `ctest` en el build Release antes de lanzar el runtime
     de comprobación.
   - Estado: **resuelto**; no era un fallo del runtime.

### Pendientes (no son fallos de T2)

- **T3:** editor de niveles y animaciones.
- **T4:** CLI empaquetadora y conversión JSON → FlatBuffers → `.pak`.
- **T5:** integración end-to-end, pruebas de rendimiento y entrega.
- **Animaciones en runtime:** T2 verifica el buffer Animation, pero no
  reproduce clips; la reproducción pertenece a T3/T5.
- **Tilemap:** T2 dibuja un sprite; el renderizado de capas y tilesets se
  integrará con el contenido de T3/T4.
- **Fixture de pruebas:** es una herramienta test-only. T4/T5 deben sustituirlo
  por el packer real y verificar el mismo contrato con ambos.

---

## 5. Decisiones registradas

- **CRC-32 C++:** implementación interna en `engine/src/resource/crc32.cpp`.
  No se añadió zlib ni una dependencia global.
- **Shaders:** GLSL compilado con `glslc` y embebido en una cabecera C++
  generada. El ejecutable no depende de archivos `.spv` externos.
- **Paquete de prueba:** generador Python en `engine/test/`, limitado a tests.
  No es la CLI de T4.
- **Selección de nivel:** `--level-id N`; por defecto, primera entrada Level.
- **Selección de sprite:** primera entidad con `image_id`; si no existe, primer
  tileset con imagen.
- **Input:** flechas/WASD, clic izquierdo y ESC; la lógica está separada de
  GLFW mediante `InputState`.
- **Modo sin paquete:** mantiene el color de limpieza de T1.

---

## 6. Criterios de finalización de T2

- [x] El runtime carga un `.pak` de prueba.
- [x] El sprite se muestra correctamente.
- [x] El paquete se abre sin leer los payloads.
- [x] Los recursos se leen bajo demanda y su CRC se verifica por bloques.
- [x] Level y Animation se verifican con `flatbuffers::Verifier` antes de
      leerlos.
- [x] El runtime responde a teclado y ratón.
- [x] El cierre es limpio y no deja errores de Vulkan atribuibles al engine.
- [x] Las pruebas automáticas pasan 9/9.
- [x] Build limpio Debug, build Release y calidad de código verificados.
