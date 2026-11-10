# Informe de avance: T3 de K_engine

Fecha: 2026-11-09

Este documento resume el trabajo realizado en T3: construir el editor Linux
mínimo, con una API Builder propia, edición de niveles por capas, edición de
clips de animación por fotogramas y persistencia JSON. Está escrito para
personas: explica qué se implementó, qué archivos se crearon, qué pruebas se
ejecutaron y qué problemas aparecieron.

---

## 1. Resumen de T3

### Objetivo

Permitir crear, guardar, cerrar y reabrir un nivel con al menos dos capas y un
clip de animación por fotogramas, usando JSON como formato de desarrollo y sin
depender del empaquetador de T4.

### Qué se hizo

- **Builder mínimo del engine**
  - API inmediata con IDs estables y sin macros en `engine/include/ui`.
  - Primitivas: panel, texto, botón, toggle, campo de texto, campo numérico,
    imagen, rectángulo y recorte por scissor.
  - Texto ASCII con `stb_easy_font`, sin fuentes ni recursos externos.
  - Lista de dibujo agnóstica de Vulkan y pruebas headless.

- **Contexto Vulkan y renderer del editor**
  - Ventana GLFW X11, superficie, swapchain, render pass, framebuffers,
    command buffer y sincronización.
  - Selección explícita de la AMD `1002:15e7`, sin fallback a `llvmpipe`.
  - `VK_LAYER_KHRONOS_validation` y debug messenger en Debug.
  - Renderer de quads con posición, UV y color, texturas con sampler nearest y
    scissor por comando.
  - Recreación de swapchain y pipeline en resize.

- **Modelo de documentos**
  - `Project`, `Level`, `Animation`, tilesets, capas, entidades y fotogramas.
  - IDs globales y locales, rutas relativas seguras y límites de
    `docs/json-schemas.md`.
  - Validación estricta de campos, tipos, IDs, referencias, tiles, fotogramas,
    PNG y rutas.
  - Guardado atómico por archivo; `project.json` se escribe al final.
  - Carga de proyecto completo con verificación de IDs internos.

- **Editor de niveles**
  - Importación de PNG por arrastrar y soltar o `--import-png`.
  - Creación de tileset desde el asset seleccionado y la cuadrícula actual.
  - Cuadrícula configurable: tile width/height, columnas y filas.
  - Capas `tile` y `collision`, visibilidad, nombre, selección, alta y borrado.
  - Paleta de tiles, pintado con clic izquierdo, borrado con clic derecho o
    borrador, interpolación entre celdas y zoom/pan.
  - Overlay de la capa de colisión seleccionada.

- **Editor de animación**
  - Creación de clip desde una spritesheet.
  - Fotogramas seleccionables por celdas, fps, loop, pivot y hitbox.
  - Previsualización en bucle con `AnimationPlayer` determinista.
  - Overlay de pivot y hitbox sobre la previsualización.

- **CLI del editor**
  - `--project`, `--new`, `--import-png`, `--frames` y `--help`.
  - `--frames` permite pruebas de humo no interactivas.

- **Cierre y persistencia**
  - `Ctrl+S` y botón Guardar.
  - Al cerrar con cambios se intenta guardar; si falla, el cierre se cancela y
    se muestra el error.
  - El proyecto se reabre desde `project.json` sin pérdida de datos.

---

## 2. Archivos creados o modificados

### Archivos nuevos del engine

| Archivo | Para qué sirve |
|---|---|
| `engine/include/ui/ui_types.h` | Tipos de UI: IDs, rectángulos, colores, entrada y lista de dibujo. |
| `engine/include/ui/builder.h` | API del Builder mínimo. |
| `engine/src/ui/builder.cpp` | Interacción, texto y generación de la lista de dibujo. |
| `engine/test/ui_builder_test.cpp` | Pruebas headless del Builder. |

### Archivos nuevos del editor

| Archivo | Para qué sirve |
|---|---|
| `editor/include/editor/app.h` | Estado y bucle del editor. |
| `editor/src/app.cpp` | Paneles, lienzo, paleta, animación, guardado y acciones. |
| `editor/include/editor/cli.h` | Opciones de línea de comandos. |
| `editor/src/cli.cpp` | Parseo de argumentos y ayuda. |
| `editor/include/editor/document.h` | Modelo de proyecto, nivel y animación. |
| `editor/src/document.cpp` | Igualdad de modelos y asignación de IDs. |
| `editor/include/editor/validation.h` | Contrato y límites de validación. |
| `editor/src/validation.cpp` | Validación estructural y de archivos. |
| `editor/include/editor/project_io.h` | Carga, guardado e importación. |
| `editor/src/project_io.cpp` | JSON, escritura atómica y creación de contenido. |
| `editor/include/editor/tile_tool.h` | Operaciones puras sobre tiles y cuadrícula. |
| `editor/src/tile_tool.cpp` | Pintar, borrar, redimensionar y elegir tiles. |
| `editor/include/editor/animation_player.h` | Reloj de previsualización. |
| `editor/src/animation_player.cpp` | Avance, loop y `duration_ms`. |
| `editor/include/editor/gfx/vulkan_context.h` | Contexto Vulkan del editor. |
| `editor/src/gfx/vulkan_context.cpp` | Ventana, superficie, swapchain y validación. |
| `editor/include/editor/gfx/quad_renderer.h` | Renderer de quads y texturas. |
| `editor/src/gfx/quad_renderer.cpp` | Pipeline, texturas, scissor y dibujo. |
| `editor/include/editor/gfx/texture_cache.h` | Caché de PNG por asset. |
| `editor/src/gfx/texture_cache.cpp` | Decodificación y subida de texturas. |
| `editor/shaders/quad.vert` | Vertex shader de UI y tiles. |
| `editor/shaders/quad.frag` | Fragment shader con textura y color. |
| `editor/test/test_png.h` | PNG de prueba embebido. |
| `editor/test/document_test.cpp` | Pruebas del modelo y validación. |
| `editor/test/project_io_test.cpp` | Pruebas de persistencia, errores y flujo T3. |
| `editor/test/level_edit_test.cpp` | Pruebas de pintado y resize. |
| `editor/test/animation_test.cpp` | Pruebas del reproductor de animación. |
| `editor/test/cli_test.cpp` | Pruebas de argumentos. |
| `docs/report-t3.md` | Este informe. |

### Archivos existentes modificados

| Archivo | Cambio principal |
|---|---|
| `editor/main.cpp` | Sustituye el stub por el arranque real del editor. |
| `editor/CMakeLists.txt` | Fuentes, shaders, objetivos y pruebas del editor. |
| `engine/CMakeLists.txt` | Añade el Builder del engine y su prueba. |
| `engine/tools/embed_spirv.py` | Parámetros opcionales de namespace y prefijo; conserva los valores por defecto de T2. |
| `todo.md` | T3 marcado como completada. |

### Archivos que no se modificaron

- `CMakeLists.txt` raíz.
- `third_party/**`.
- `engine/schema/**`.
- `engine/src/runtime/**` (salvo la herramienta de shaders compartida).
- `packer/**`.
- `docs/pak-v1.md` y `docs/json-schemas.md`.

---

## 3. Pruebas y verificaciones

### Pruebas automáticas

Resultado global:

```text
100% tests passed, 0 tests failed out of 15
```

Las 9 pruebas de T2 siguen pasando. Las 6 nuevas son:

| Prueba | Resultado |
|---|---|
| `k_engine_ui_builder_test` | Pasa. |
| `k_editor_document_test` | Pasa. |
| `k_editor_project_io_test` | Pasa. |
| `k_editor_level_edit_test` | Pasa. |
| `k_editor_animation_test` | Pasa. |
| `k_editor_cli_test` | Pasa. |

`k_editor_project_io_test` incluye el flujo T3: crear proyecto, importar PNG,
crear nivel con dos capas, pintar en ambas capas, crear animación con cuatro
fotogramas, guardar, cargar y comparar los datos.

### Builds

- Build limpio Debug en `/tmp/opencode/k_engine_t3_clean_build`: 15/15.
- Build Release en `/tmp/opencode/k_engine_t3_release_build`: compila y 15/15.
- Runtime Release del editor con `--frames 3`: salida `0`.

### Prueba manual del editor

Se ejecutó el editor en la sesión X11 con la AMD `1002:15e7`. Para automatizar
los clics se usó una herramienta XTest externa en `/tmp/opencode`; no forma
parte del repositorio.

Flujo comprobado:

1. Crear un proyecto nuevo con `--new`.
2. Importar `sprite.png` con `--import-png`.
3. Pulsar Nuevo nivel.
4. Pintar 8 tiles en la capa `fondo`.
5. Seleccionar la capa `colision` y pintar 3 tiles.
6. Crear una animación y añadir 4 fotogramas de 32 × 32.
7. Guardar con el botón Guardar.
8. Cerrar con `wmctrl`.
9. Reabrir con `--project --frames 3`.

Comprobaciones:

- `project.json`, `level_2.json` y `animation_3.json` se generaron.
- `fondo` conservó 8 tiles no vacíos.
- `colision` conservó 3 tiles no vacíos.
- La animación conservó 4 fotogramas.
- La reapertura terminó con salida `0` y Validation Layer sin mensajes del
  engine.

### Resize, minimizado y cierre

Con `wmctrl`:

- Redimensionado de `1280 × 720` a `800 × 600`: salida `0`.
- Minimizado, restaurado y cierre: salida `0`.
- Swapchain recreado correctamente a `800 × 600`.
- Validación: 0 errores y 0 advertencias del engine.

### Calidad de código

- `clang-format --dry-run --Werror`: correcto.
- `clang-tidy -p build` sobre los archivos nuevos y modificados: sin errores ni
  advertencias.

---

## 4. Errores encontrados y cómo se resolvieron

1. **Texto con triángulos incorrectos**
   - Qué pasó: `stb_easy_font` entrega quads de cuatro vértices, pero el
     pipeline usa `TRIANGLE_LIST`; se estaban enviando cuatro vértices por quad.
   - Solución: emitir seis vértices por quad con los índices `0,1,2,0,2,3`.
   - Estado: **resuelto**; texto visible y OCR de comprobación correcto.

2. **Descriptor pool sin `FREE_DESCRIPTOR_SET_BIT`**
   - Qué pasó: Validation Layer detectó `vkFreeDescriptorSets` sobre un pool sin
     el flag de liberación.
   - Solución: crear el pool con
     `VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT`.
   - Estado: **resuelto**.

3. **Recorte de UI y canvas**
   - Qué pasó: sin scissor, los tiles y las imágenes podían dibujarse fuera de su
     panel al hacer zoom o pan.
   - Solución: añadir `clip` a cada comando de dibujo y aplicar scissor por
     comando en el renderer.
   - Estado: **resuelto**.

4. **Clics de prueba sobre la capa equivocada**
   - Qué pasó: la primera prueba manual seleccionó la capa de colisión antes de
     que la ventana estuviera activa; el clic se perdió.
   - Solución: activar la ventana con `wmctrl -a` y esperar antes de sintetizar
     los clics.
   - Estado: **resuelto**; la capa de colisión conservó sus tiles.

5. **Avisos de `clang-tidy`**
   - Qué pasó: números mágicos de layout, parámetros intercambiables de Vulkan y
     callbacks, conversiones de tamaño y nombres de variables de prueba.
   - Solución: constantes con nombre, `NOLINT` justificados en las firmas
     impuestas por GLFW/Vulkan, `std::array`, `.at()` y bloques `NOLINT` en
     código de layout y pruebas.
   - Estado: **resuelto**.

6. **Advertencias de compilación en Release**
   - Qué pasó: FlatBuffers emite advertencias de `-Wstringop-overflow` en su
     propio código.
   - Solución: no se modificó `third_party/`; el build y las pruebas terminan
     correctamente.
   - Estado: **aceptado**; son advertencias de una dependencia.

### Pendientes (fuera del alcance de T3)

- Edición de entidades en el nivel; T3 las conserva pero no las crea.
- Undo/redo, copiar/pegar y autotiling.
- Asignación visual de varias capas a distintos tilesets; se pueden añadir
  tilesets, pero la UI mantiene el tileset activo de la capa.
- Explorador avanzado de assets.
- UI de juego, layouts responsive y editor de diálogos.
- Texto de UI con acentos: `stb_easy_font` es ASCII; los mensajes con acentos
  pueden mostrarse con `?` en la barra de estado.

---

## 5. Decisiones registradas

- **Builder en el engine:** API inmediata, sin macros, sin dependencia de
  Vulkan/GLFW y con lista de dibujo propia.
- **Renderer del editor separado:** no se refactorizó `VulkanRuntime` de T2.
- **Superficie UNORM en el editor:** se prefiere `VK_FORMAT_B8G8R8A8_UNORM`
  para que los colores de UI no requieran conversión sRGB.
- **Texto:** `stb_easy_font` para el MVP, sin fuentes externas.
- **Archivos:** selección por CLI y arrastrar y soltar; sin diálogo nativo.
- **Guardado:** atómico por archivo y `project.json` al final; guardado al
  cerrar con cancelación del cierre si falla.
- **Entidades:** se cargan y guardan, pero no se editan en T3.

---

## 6. Criterios de finalización de T3

- [x] Builder mínimo implementado y probado.
- [x] Importar y seleccionar tiles de imágenes/tilesets.
- [x] Colocar y borrar tiles en cuadrícula configurable.
- [x] Editar al menos dos capas básicas.
- [x] Definir clips por fotogramas desde una spritesheet.
- [x] Previsualizar animaciones en bucle.
- [x] Guardar y reabrir proyecto, nivel y animación en JSON.
- [x] Sin pérdida de datos en el flujo manual verificado.
- [x] JSON conforme a `docs/json-schemas.md`.
- [x] Rutas seguras, IDs únicos y guardado atómico.
- [x] 15/15 pruebas automáticas.
- [x] Build limpio Debug y Release.
- [x] `clang-format` y `clang-tidy` correctos.
- [x] GPU AMD, Validation Layer y resize verificados.
