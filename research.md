# K_engine — Investigación y aprendizaje

## Propósito

Esta hoja organiza los temas que conviene estudiar para tomar decisiones fundamentadas durante el desarrollo de K_engine. Prioriza preguntas concretas y experimentos pequeños sobre lectura sin un resultado verificable.

Usar junto con:

- `todo.md`: trabajo y criterios de aceptación del MVP Linux.
- `deps.md`: herramientas y dependencias aprobadas o pendientes.
- `k_engine.md`: alcance y decisiones técnicas del proyecto.

## Cómo investigar

Para cada tema:

1. Escribir la pregunta que debe resolverse y por qué afecta al proyecto.
2. Consultar primero documentación oficial y especificaciones; usar artículos y videos como explicación complementaria.
3. Crear un experimento pequeño que pruebe la idea en el hardware o build objetivo.
4. Registrar resultado, limitaciones, coste, dependencias y decisión recomendada.
5. Si cambia el alcance, una dependencia o una decisión técnica, actualizar también `k_engine.md`, `deps.md` o `todo.md`.

No investigar una característica solo porque podría ser útil algún día. Los temas P0 bloquean el MVP; P1 y P2 pertenecen a etapas posteriores.

**Estados:** `PENDIENTE`, `EN CURSO`, `BLOQUEADO`, `COMPLETADO`.

---

## P0 — Necesario para el MVP Linux

### R-01 — Vulkan, loader, ICD y presentación X11

- **Estado:** EN CURSO
- **Pregunta:** ¿Cómo se conectan loader, ICD/driver, instancia, dispositivo físico, colas, superficie y swapchain en este runtime?
- **Contexto confirmado:** GPU AMD Radeon Graphics (Renoir), RADV/Mesa 25.2.8; loader/instancia reporta Vulkan 1.3.275 y dispositivo Vulkan 1.4.318. `vulkaninfo` reporta XCB/Xlib presentables y `VK_KHR_swapchain`.
- **Aprender:** versionado de Vulkan, extensiones de instancia/dispositivo, selección de GPU, queue families, soporte de presentación y ciclo de vida de swapchain.
- **Experimento:** prototipo GLFW en X11 que seleccione la AMD, cree superficie y swapchain, presente un frame, redimensione la ventana y cierre.
- **Resultado esperado:** criterio de selección de dispositivo y cola; lista mínima de extensiones; pasos correctos de creación, recreación y destrucción de la swapchain.
- **Fuentes:** [Vulkan Guide](https://docs.vulkan.org/guide/latest/), [Vulkan Specification](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html), [GLFW Documentation](https://www.glfw.org/docs/latest/).

### R-02 — Validation Layers y diagnóstico Vulkan

- **Estado:** PENDIENTE
- **Pregunta:** ¿Cómo se habilitan las comprobaciones en desarrollo y cómo se reciben sus mensajes sin depender de la salida incidental del loader?
- **Contexto confirmado:** `VK_LAYER_KHRONOS_validation` está disponible desde el Vulkan SDK después de cargar `setup-env.sh`; la versión informada de la capa es 1.4.363. La capa debe habilitarse al crear `VkInstance`.
- **Aprender:** enumeración y activación de capas, `VK_EXT_debug_utils`, debug messenger, severidades/tipos de mensaje y separación entre mensajes del ICD y errores del engine.
- **Experimento:** habilitar la capa y el callback en el prototipo; verificar que recibe un mensaje de prueba y que el uso correcto de Vulkan no genera diagnósticos atribuibles al engine.
- **Resultado esperado:** configuración de compilación de desarrollo, callback de diagnóstico y criterio reproducible para clasificar mensajes.
- **Nota:** las advertencias sobre `vkGetPhysicalDeviceDisplayPropertiesKHR` y `vkGetPhysicalDeviceDisplayPlanePropertiesKHR` son de presentación directa (`VK_KHR_display`); no son necesarias para XCB/Xlib. Wayland sigue fuera del MVP.
- **Fuentes:** [Validation Overview](https://docs.vulkan.org/guide/latest/validation_overview.html), [Validation Layers Tutorial](https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/02_Validation_layers.html).

### R-03 — Sincronización, presentación y ciclo de vida de recursos Vulkan

- **Estado:** PENDIENTE
- **Pregunta:** ¿Qué sincronización mínima necesita el render loop para no leer/escribir recursos antes de tiempo y cómo se recrean los recursos de presentación?
- **Aprender:** command buffers, semáforos, fences, acquire/present, estados de imagen, swapchain out-of-date/suboptimal y destrucción en el orden correcto.
- **Experimento:** presentar de forma continua; forzar redimensionamiento y cierre; verificar Validation Layers y destrucción limpia.
- **Resultado esperado:** un ciclo de frame mínimo documentado, reglas de propiedad de recursos y procedimiento de recreación de swapchain.
- **Fuentes:** [Vulkan Guide](https://docs.vulkan.org/guide/latest/), [Vulkan Specification](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html).

### R-04 — Renderizado de texturas y alineación de memoria

- **Estado:** PENDIENTE
- **Pregunta:** ¿Cómo cargar una imagen, transferirla a una imagen Vulkan y muestrearla sin asumir requisitos que dependan de una GPU específica?
- **Aprender:** formatos sRGB/UNORM, staging buffers, memoria host-visible/device-local, transiciones de layout, descriptor/sampler y alineaciones de transferencia/mapeo.
- **Contexto:** el `.pak` fija offsets de payload alineados a 16 bytes. Esa regla del archivo no sustituye los requisitos Vulkan; el runtime debe alinear por separado staging y flush/invalidate según el dispositivo.
- **Experimento:** cargar una textura PNG de prueba desde memoria, subirla y dibujarla; comprobar color y validar límites/formato.
- **Resultado esperado:** ruta mínima de carga de textura, formatos compatibles requeridos y errores legibles para formato/dimensiones no admitidos.
- **Fuentes:** [Vulkan Specification](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html), documentación oficial del decodificador PNG que se seleccione.

### R-05 — Contrato del `.pak` y lectura segura bajo demanda

- **Estado:** PENDIENTE
- **Pregunta:** ¿Cómo implementar el formato v1 de manera segura, compatible con C++17 y sin cargar el paquete entero en memoria?
- **Decisiones existentes:** cabecera `KPAK` de 16 bytes; TOC con entradas fijas de 40 bytes; little-endian; offsets absolutos; payloads alineados a 16 bytes; CRC-32/ISO-HDLC; sin cifrado ni compresión.
- **Aprender:** lectura binaria explícita, `memcpy`/conversión de endianness, aritmética comprobada, rangos/solapamientos, lectura por offsets, checksum incremental, escritura atómica y manejo de archivos truncados.
- **Experimento:** crear un `.pak` mínimo, leer una sola entrada por offset, alterar/truncar sus bytes y verificar los rechazos. Confirmar que abrir el paquete no lee todos los payloads.
- **Decisiones pendientes:** límite de memoria de la TOC y límites por tipo de recurso. No imponer un máximo global del `.pak` basado en RAM.
- **Resultado esperado:** especificación y pruebas compartidas entre escritor Python y lector C++.

### R-06 — FlatBuffers y evolución de esquemas

- **Estado:** PENDIENTE
- **Pregunta:** ¿Cómo generar y verificar buffers de niveles/animaciones desde Python y leerlos en C++ sin desincronizar versiones?
- **Aprender:** `.fbs`, `flatc`, código generado para C++ y Python, verificador, límites del buffer, evolución compatible del schema y política de versiones.
- **Experimento:** definir un esquema pequeño de nivel y animación; crear buffer desde el empaquetador Python; verificarlo/leerlo desde C++; probar buffer truncado e incompatible.
- **Restricción:** FlatBuffers y su runtime se gestionarán desde un Git submodule fijado; Python/C++/`flatc` deben proceder del mismo commit o versión compatible.
- **Resultado esperado:** flujo de generación reproducible, versión fijada y reglas para cambiar esquemas sin romper contenido existente.
- **Fuentes:** [FlatBuffers Documentation](https://flatbuffers.dev/).

### R-07 — JSON de desarrollo y validación del proyecto

- **Estado:** PENDIENTE
- **Pregunta:** ¿Qué estructura/versionado necesita el JSON editable para que el editor lo guarde y la CLI lo valide de forma consistente?
- **Aprender:** separación entre formato editable y formato de distribución, validación, campos obligatorios/opcionales, IDs estables y errores que permitan corregir el proyecto.
- **Experimento:** guardar y volver a abrir un nivel con tiles/capas y una animación; validar las mismas estructuras desde Python y convertirlas a FlatBuffers.
- **Decisión pendiente:** elegir parser JSON C++ para el editor; el runtime de distribución no necesita parsear JSON.
- **Resultado esperado:** ejemplos JSON válidos/inválidos, versión del esquema y reglas de compatibilidad.
- **Fuentes:** [Python `json`](https://docs.python.org/3/library/json.html) y documentación oficial de la biblioteca C++ que se elija.

### R-08 — Build reproducible con submodules

- **Estado:** PENDIENTE
- **Pregunta:** ¿Cómo compilar GLFW y FlatBuffers desde submodules y mantener el build local, reproducible y sin dependencias globales de esas bibliotecas?
- **Aprender:** objetivos CMake, `add_subdirectory`, opciones de build de GLFW/FlatBuffers, rutas de headers/libraries y configuración de Python para importar FlatBuffers desde el submodule.
- **Experimento:** clonar/init submodules y construir runtime/editor/CLI desde una configuración limpia usando solo toolchain del sistema, Vulkan SDK ya instalado y dependencias del repositorio.
- **Resultado esperado:** comandos de configuración/build y versiones exactas de submodules documentadas en `deps.md`.
- **Fuentes:** [CMake Documentation](https://cmake.org/cmake/help/latest/), documentación oficial de GLFW y FlatBuffers.

### R-09 — Decodificación de imagen y memoria de recursos

- **Estado:** PENDIENTE
- **Pregunta:** ¿Qué biblioteca y política de límites permiten previsualizar PNG en el editor y cargar texturas en el runtime sin decodificar recursos excesivos?
- **Aprender:** bytes comprimidos frente a tamaño decodificado, dimensiones máximas Vulkan, espacio de color, uso de memoria y errores de imágenes corruptas.
- **Decisión pendiente:** elegir biblioteca de PNG como submodule. `deps.md` tiene `stb_image` como candidata, no como decisión aprobada.
- **Resultado esperado:** dependencias, formatos/dimensiones aceptados y límites por tipo de recurso documentados.

---

## P1 — Etapa posterior: runtime Android

### R-10 — Vulkan Android, NDK y ciclo de vida

- **Estado:** PENDIENTE; no bloquea el MVP Linux
- **Pregunta:** ¿Cómo crear/recrear superficies nativas y suspender/reanudar el runtime con el ciclo de vida de Android 16/API 36+?
- **Aprender:** NDK, `ANativeWindow`, input táctil, Activity lifecycle, Vulkan surface/swapchain, APK arm64-v8a y requisitos de dispositivo.
- **Experimento:** portar el prototipo Linux mínimo a Moto G34 5G; pausar, volver, recrear superficie y seguir renderizando el mismo `.pak`.
- **Resultado esperado:** backend Android y lista explícita de capacidades Vulkan requeridas/consultadas en runtime.

### R-11 — Compatibilidad de formatos y recursos entre Linux y Android

- **Estado:** PENDIENTE; iniciar después de validar Linux
- **Pregunta:** ¿Qué límites deben consultarse por dispositivo para texturas, memoria, formatos y presentación, sin depender de las capacidades altas del equipo Linux?
- **Aprender:** formatos Vulkan requeridos, límites de textura/memoria, perfiles de GPU y carga de recursos en memoria limitada.
- **Resultado esperado:** política de fallback/error para recursos que el teléfono no pueda cargar.

---

## P2 — Futuro: funciones de juego y otras plataformas

### R-12 — Editor Builder y diseño de UI

- **Estado:** PENDIENTE; mantenerlo fuera del MVP más allá de los controles necesarios del editor
- **Pregunta:** ¿Qué partes del sistema Builder deben ser declarativas, cuáles son eventos y cómo se separa layout de renderizado?
- **Aprender:** layout inmediato/declarativo, input, coordenadas, escalado, DPI y límites de construir una UI genérica.
- **Resultado esperado:** criterios para ampliar el Builder sin convertirlo prematuramente en un sistema de UI de juego.

### R-13 — Física 2D y carriles de profundidad

- **Estado:** PENDIENTE; se investiga si una futura vertical slice necesita física
- **Pregunta:** ¿Cómo representar carriles discretos, conexiones y filtros de colisión sin romper el modelo de movimiento 2D?
- **Aprender:** paso fijo de simulación, colisiones, filtros/máscaras de Box2D, detección de impactos y separación entre carril lógico y orden de dibujado.
- **Resultado esperado:** prototipo pequeño que compare un mundo Box2D con filtros por carril frente a una alternativa simple, antes de decidir la arquitectura final.
- **Fuentes:** [Box2D Documentation](https://box2d.org/documentation/).

### R-14 — Audio

- **Estado:** PENDIENTE; fuera del MVP
- **Pregunta:** ¿Qué API mínima necesita el juego para efectos/música, streaming y pausa/reanudación de aplicación?
- **Aprender:** decode vs playback, mezcla, buffers y ciclo de vida Linux/Android.
- **Resultado esperado:** prototipo de un efecto WAV y una decisión de biblioteca, solo si una prueba de juego requiere audio.

### R-15 — Preparación y multijugador en red

- **Estado:** PENDIENTE; fuera del MVP
- **Pregunta:** ¿Qué requisitos reales hay para intercambiar acciones, sincronizar estado y manejar desconexiones en una LAN?
- **Aprender:** comandos de juego, paso de simulación, identificadores de entidad, latencia, autoridad y determinismo; no asumir que todos son necesarios antes del prototipo.
- **Resultado esperado:** prototipo de dos instancias y decisión de arquitectura antes de implementar partidas 1 vs. 1 o 3 vs. 3.

### R-16 — Seguridad e integridad de paquetes

- **Estado:** PENDIENTE; cifrado fuera del MVP
- **Pregunta:** ¿Qué amenaza concreta se desea mitigar y qué protección aporta frente a la complejidad y la clave embebida en el juego?
- **Aprender:** CRC frente a hash/MAC/AEAD, gestión de claves y límites del cifrado del índice si los recursos quedan sin cifrar.
- **Resultado esperado:** amenaza y beneficio definidos antes de reabrir la decisión de cifrar `.pak`.

### R-17 — Otros backends y distribución

- **Estado:** PENDIENTE; después de Android
- **Pregunta:** ¿Qué plataformas adicionales justifican el coste de otro backend, formato gráfico o empaquetado?
- **Aprender:** requisitos de Windows/WebAssembly, diferencias de WSI, Vulkan en navegador y estrategia de distribución.
- **Resultado esperado:** decisión por plataforma basada en un caso de uso y un prototipo, no solo en portabilidad teórica.

---

## Registro de investigación

Añadir una entrada cuando una investigación cambie una decisión:

```text
ID:
Fecha:
Pregunta:
Opciones consideradas:
Fuentes primarias:
Experimento y entorno:
Resultado observado:
Decisión y motivo:
Impacto en deps.md / k_engine.md / todo.md:
Siguientes pasos:
```
