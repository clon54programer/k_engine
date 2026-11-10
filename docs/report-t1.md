# Informe de avance: T1 de K_engine

Fecha: 2026-11-09

Este documento resume el trabajo realizado en T1: construir el prototipo técnico
Vulkan para Linux, comprobar la creación de una superficie X11 y presentar frames
en la GPU AMD con `VK_LAYER_KHRONOS_validation` activa. Está escrito para
personas: explica qué se hizo, qué archivos se crearon, qué problemas aparecieron
y qué queda para las siguientes etapas.

---

## 1. Resumen de tareas y estado del proyecto

### T1: prototipo técnico Vulkan Linux

Objetivo: demostrar que K_engine puede crear una ventana GLFW bajo X11, crear una
superficie Vulkan, seleccionar explícitamente la GPU AMD y presentar al menos un
frame sin errores atribuibles al engine.

Qué se hizo:

- **Instancia Vulkan y validación**
  - Instancia creada con `VK_API_VERSION_1_3`.
  - `VK_LAYER_KHRONOS_validation` habilitada en compilaciones de desarrollo.
  - `VK_EXT_debug_utils` y un debug messenger para recibir los mensajes.
  - Contadores separados para errores del engine, advertencias del engine y
    advertencias del ICD relacionadas con `VK_KHR_display`.

- **Ventana y superficie**
  - GLFW inicializado con la plataforma X11 forzada.
  - Ventana de `1280 × 720` sin contexto OpenGL (`GLFW_NO_API`).
  - Superficie creada con `glfwCreateWindowSurface`.
  - Cierre mediante el botón de la ventana y mediante ESC.

- **Selección de GPU y colas**
  - Enumeración de dispositivos Vulkan.
  - Selección exacta de la AMD Radeon Graphics `1002:15e7`.
  - Comprobación de Vulkan 1.3, `VK_KHR_swapchain` y una queue family con
    gráficos y presentación.
  - Fallo explícito si la GPU AMD no está disponible; sin fallback a `llvmpipe`.
  - Creación del dispositivo lógico y obtención de las colas.

- **Swapchain y presentación**
  - Consulta de capacidades, formatos y modos de presentación de la superficie.
  - Formato preferido `VK_FORMAT_B8G8R8A8_SRGB`.
  - `VK_PRESENT_MODE_FIFO_KHR`.
  - Creación de swapchain, imágenes e image views.
  - Render pass de un attachment de color con `loadOp = CLEAR`.
  - Framebuffers, command pool, command buffer, semáforos y fence.
  - Bucle de `vkAcquireNextImageKHR`, `vkQueueSubmit` y `vkQueuePresentKHR`.
  - Un semáforo de “render finished” por imagen del swapchain para evitar la
    reutilización insegura de semáforos.

- **Resize, minimizado y cierre**
  - Callback de framebuffer que marca un flag; la recreación se hace fuera del
    callback.
  - Recreación de swapchain, image views, render pass, framebuffers y objetos de
    sincronización.
  - Espera cuando la ventana está minimizada y el tamaño es `0 × 0`.
  - Destrucción ordenada de recursos y `Shutdown` idempotente.

- **Ejecución no interactiva**
  - Opción `--frames N` para presentar un número limitado de frames.
  - `--frames 0` presenta hasta que se cierre la ventana.
  - `--help` y validación de argumentos.

- **Imagen de prueba**
  - Color de limpieza animado con `glfwGetTime()`.
  - No se añadieron shaders, pipeline gráfico ni sprites; esos trabajos
    pertenecen a T2.

### Estado general y pruebas

- Se ejecutan **5 pruebas automáticas** y todas pasan.
- El target `k_engine_runtime` compila y enlaza correctamente.
- El prototipo presenta frames en la GPU AMD con Validation Layer activa.
- No se registraron errores ni advertencias atribuibles al engine.
- `todo.md` no se modificó: T1 está implementado y verificado, pero su estado
  sigue siendo `PENDIENTE` hasta que se apruebe el cambio de estado.

Comprobar el estado en cualquier momento:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/engine/k_engine_runtime --frames 1
```

Resultado esperado de las pruebas:

```text
100% tests passed, 0 tests failed out of 5
```

Salida relevante del prototipo:

```text
GPU seleccionada: AMD Radeon Graphics (RADV RENOIR).
Swapchain creado: 1280x720, 4 imágenes, formato 50.
Validación: 0 errores, 0 advertencias del engine, 0 advertencias del ICD sobre VK_KHR_display.
```

Verificaciones realizadas:

- `./build/engine/k_engine_runtime --frames 1`: salida `0`.
- `./build/engine/k_engine_runtime --frames 300`: salida `0` en 5,196 s.
- Redimensionado de `1280 × 720` a `800 × 600` y cierre con `wmctrl`: salida `0`.
- Minimizado, restaurado y cierre: salida `0`.
- Tres ejecuciones consecutivas de `--frames 1`: salida `0` en las tres.
- Build Release en `/tmp/opencode/k_engine_t1_release_build`: salida `0`, sin
  Validation Layer.
- Build limpio Debug en `/tmp/opencode/k_engine_t1_clean_build`: `ctest` 5/5 y
  runtime con salida `0`.
- `clang-format --dry-run --Werror`: correcto.
- `clang-tidy` con `-header-filter='engine/src/runtime/.*'`: correcto.

Las pruebas de presentación, resize y cierre se hicieron manualmente porque
requieren una sesión X11 y la GPU AMD; no se añadió una prueba CTest que abra una
ventana.

---

## 2. Archivos creados o modificados

### Archivos nuevos

| Archivo | Para qué sirve |
|---|---|
| `engine/src/runtime/main.cpp` | Punto de entrada del prototipo y parseo de `--frames`. |
| `engine/src/runtime/vulkan_runtime.h` | Interfaz interna del runtime Vulkan de T1. |
| `engine/src/runtime/vulkan_runtime.cpp` | Instancia, validación, ventana, superficie, GPU, swapchain, render y bucle de presentación. |
| `docs/report-t1.md` | Este informe de avance. |

### Archivos que ya existían y se modificaron

| Archivo | Cambio principal |
|---|---|
| `engine/CMakeLists.txt` | Nuevo target `k_engine_runtime`, enlazado con `k_engine`, `glfw` y `Vulkan::Vulkan`. |

### Archivos movidos

Ninguno.

### Archivos que no se modificaron

- `CMakeLists.txt` raíz.
- `third_party/**`.
- `engine/schema/**`.
- `editor/**`.
- `packer/**`.
- `todo.md`.
- `docs/report-t0.md`.
- `opencode.json` y `.opencode/**`.

---

## 3. Errores encontrados y cómo se resolvieron

1. **Reutilización insegura del semáforo de presentación**
   - Qué pasó: con un único semáforo `render finished`, Validation Layer detectó
     el error `VUID-vkQueueSubmit-pSignalSemaphores-00067` al presentar varias
     imágenes del swapchain. El semáforo podía seguir en uso por una presentación
     anterior.
   - Solución: se creó un semáforo `render finished` por imagen del swapchain y
     se recrean los objetos de sincronización al recrear la swapchain.
   - Estado: **resuelto**.

2. **Resumen de validación duplicado**
   - Qué pasó: `main` llamaba a `Shutdown` y el destructor de `VulkanRuntime`
     volvía a llamarlo, por lo que el resumen se imprimía dos veces.
   - Solución: se añadió el flag `_shutdown_complete` para que `Shutdown` sea
     idempotente y el resumen se imprima una sola vez.
   - Estado: **resuelto**.

3. **Nombres privados fuera de la configuración de `clang-tidy`**
   - Qué pasó: los métodos privados usaban `CamelCase` y los miembros privados
     tenían sufijo `_` en lugar del prefijo `_` exigido por `.clang-tidy`.
   - Solución: métodos privados renombrados a `lower_case` y miembros privados
     renombrados con prefijo `_`, por ejemplo `_window`, `_instance` y
     `_command_pool`. Los métodos públicos `Initialize`, `Run` y `Shutdown`
     conservan `CamelCase`.
   - Estado: **resuelto**.

4. **Errores de `clang-tidy` en el código nuevo**
   - Qué pasó: aparecieron avisos tratados como error por aritmética de punteros,
     decaimiento de arrays a punteros, `modernize-use-auto`, parámetros
     fácilmente intercambiables y complejidad cognitiva del bucle principal.
   - Solución:
     - `std::copy_n` para copiar extensiones de GLFW.
     - `std::data` para leer `deviceName`, `layerName` y `extensionName`.
     - `auto` en la conversión de `glfwGetTime()`.
     - `std::any_of` en las comprobaciones de capas y extensiones.
     - `NOLINT` justificados para las firmas impuestas por GLFW y para
       `prepare_frame`.
     - División del bucle principal en `prepare_frame`, `submit_frame` y
       `handle_frame_result` para reducir la complejidad.
   - Estado: **resuelto**.

5. **`RecordCommandBuffer` no propagaba errores**
   - Qué pasó: la grabación del command buffer era `void` y no permitía detener
     el bucle si `vkBeginCommandBuffer` o `vkEndCommandBuffer` fallaban.
   - Solución: la función ahora devuelve `bool` y el bucle comprueba el resultado.
   - Estado: **resuelto**.

6. **Recreación doble de la swapchain en algunos resize**
   - Qué pasó: el callback de tamaño y el resultado `VK_SUBOPTIMAL_KHR` podían
     provocar dos recreaciones seguidas.
   - Solución: se mantiene el comportamiento seguro; la segunda recreación no
     genera errores y deja la swapchain en el tamaño correcto. No se optimizó en
     T1 porque no afecta a los criterios de aceptación.
   - Estado: **aceptado como comportamiento del prototipo**.

### Pendientes (no son fallos actuales, son mejoras futuras)

- **Registrar T1 en `todo.md`:** el prototipo está implementado y verificado,
  pero el estado de la tarea sigue en `PENDIENTE` hasta que se apruebe la
  actualización del Markdown.
- **Prueba automática de presentación:** no se añadió a CTest porque requiere
  sesión X11 y GPU AMD. La verificación se hizo manualmente.
- **Triángulo o pipeline gráfico:** no forma parte de T1. Los shaders, el pipeline
  y el renderizado de sprites pertenecen a T2.
- **Simulación de ESC:** la ruta está implementada, pero no se simuló la pulsación
  de tecla por falta de una herramienta de automatización de teclado. El cierre
  por gestor de ventanas sí se verificó.
- **Advertencias del ICD:** el prototipo registra un contador separado para
  `VK_KHR_display`. En esta ejecución el contador quedó a cero; si un driver
  futuro emite esas advertencias, se seguirán clasificando aparte.
- **T2:** implementar el runtime mínimo, el lector `.pak` y la carga de un sprite
  de prueba.
