# K_engine — Todo del MVP Linux

## Propósito

Este archivo define el trabajo y los criterios de aceptación del primer MVP. Los agentes deben trabajar dentro de este alcance, respetar las decisiones técnicas de abajo y actualizar el estado de las tareas al completarlas.

## Reglas de trabajo para agentes

- Trabajar solo en la tarea asignada y sus dependencias directas.
- No añadir funciones fuera del alcance ni cambiar las decisiones marcadas como `FIJA` sin pedir confirmación.
- Mantener el runtime, el editor y el empaquetador en objetivos de compilación separados cuando sea práctico.
- No depender de `llvmpipe` para las pruebas de rendimiento; la GPU objetivo es la AMD Radeon.
- Validar cada entrega con sus criterios de finalización y registrar los comandos/resultados relevantes.
- No marcar una tarea como completada si no se verificó su criterio de finalización.

## Objetivo de entrega

- **Fecha objetivo:** 2026-12-01.
- **Plataforma del MVP:** Linux Mint 22.2 Xfce, x86_64, sesión X11.
- **Entregables:** runtime Vulkan, editor de escritorio y CLI empaquetadora, con instrucciones de prueba.
- **Prueba de extremo a extremo:** crear contenido en el editor, guardarlo en JSON, convertirlo a FlatBuffers y `.pak`, y cargarlo y mostrarlo desde el runtime.
- **Escena:** contenido técnico pequeño con tilemap, capas básicas, un sprite o una animación y los recursos necesarios para probar el flujo.

## Fuera del MVP

- Runtime Android, editor/empaquetador Android y cualquier backend que no sea Linux.
- Física, Box2D, colisiones de juego, combate, raycasting, sigilo y carriles de profundidad.
- Audio de runtime.
- Multijugador, comunicación entre instancias y determinismo de red.
- Cifrado del `.pak`, compresión adicional, animación esquelética y UI del juego.
- Windows, WebAssembly y soporte de otras distribuciones/dispositivos.

## Contexto técnico confirmado

- GPU: AMD Radeon Graphics (Renoir), PCI `1002:15e7`.
- Driver: RADV de Mesa `25.2.8`.
- `vulkaninfo`: Vulkan de instancia/loader `1.3.275`; API de dispositivo `1.4.318`.
- **API objetivo inicial de la aplicación:** Vulkan 1.3.
- `vulkaninfo` reporta superficies XCB/Xlib, `VK_KHR_swapchain` y una cola gráfica con soporte de presentación. El runtime aún debe probar la creación real de superficie y el swapchain mediante GLFW.
- `llvmpipe` también aparece como dispositivo; no usarlo como GPU objetivo ni para pruebas de rendimiento.
- `VK_LAYER_KHRONOS_validation` está disponible cuando se configura el entorno del Vulkan SDK; `vulkaninfo` muestra la capa versión `1.4.363`. El runtime debe habilitarla en compilaciones de desarrollo.
- La advertencia del ICD sobre `vkGetPhysicalDeviceDisplayPropertiesKHR` y `vkGetPhysicalDeviceDisplayPlanePropertiesKHR` corresponde a presentación directa (`VK_KHR_display`). No es necesaria para presentación XCB/Xlib y no bloquea el objetivo X11. Wayland no está validado.
- En el equipo de referencia, la pantalla es de `1366 × 768` a `60 Hz` bajo X11.

## Decisiones técnicas fijas para el MVP

### Datos de desarrollo y distribución

- Los niveles y las animaciones se editan y guardan como JSON durante el desarrollo.
- Al preparar una distribución, la CLI valida esos JSON, los convierte a FlatBuffers y los empaqueta.
- El runtime lee los datos FlatBuffers del `.pak`; no requiere parsear JSON para ejecutar el contenido empaquetado.

### Formato `.pak` v1

- Estructura: `[cabecera][tabla de contenidos][datos]`.
- Sin cifrado ni compresión adicional.
- Todos los enteros son little-endian.
- Cabecera fija de 16 bytes:
  - Firma ASCII `KPAK`: 4 bytes.
  - Versión mayor y menor: `u16` cada una.
  - Cantidad de archivos: `u32`.
  - CRC-32/ISO-HDLC de los bytes de la tabla: `u32`.
- La tabla empieza inmediatamente después de la cabecera. Cada registro fijo ocupa 40 bytes:
  - ID estable: `u64`.
  - Tipo de recurso: `u32`, con valores enumerados por el formato.
  - Flags: `u32`; cero en v1.
  - Offset absoluto desde el comienzo del `.pak`: `u64` bytes.
  - Tamaño del recurso: `u64` bytes.
  - CRC-32/ISO-HDLC del recurso: `u32`.
  - Campo reservado: `u32`; cero en v1.
- Los registros de la tabla se ordenan por ID. Los IDs duplicados son inválidos.
- Cada payload empieza en un offset absoluto múltiplo de 16 bytes. El empaquetador añade padding de bytes cero; el tamaño de la entrada excluye ese padding.
- El runtime lee los recursos bajo demanda; no carga el `.pak` completo en memoria. Puede verificar CRC por bloques.
- Parsear los campos byte a byte o mediante `memcpy` y conversión explícita de endianness. No convertir offsets arbitrarios del archivo en punteros a estructuras o enteros.
- La alineación de 16 bytes del archivo no reemplaza los requisitos Vulkan: el runtime calcula por separado la alineación requerida para buffers de staging según el dispositivo.
- Validar versiones, CRC, flags/reservados, alineación, IDs duplicados, solapamientos, rangos fuera del archivo, truncamientos y desbordamientos antes de reservar memoria.
- **Pendiente de T0:** fijar un límite de memoria para la tabla y límites por tipo de recurso. No fijar un máximo global del `.pak` por memoria.

## Presupuesto y planificación

- Disponibilidad habitual hasta la fecha objetivo: aproximadamente `143 h` (25 h/semana durante 5,7 semanas).
- Ocho jornadas adicionales en noviembre: estimar `6–8 h` productivas por jornada, aproximadamente `48–64 h` extra.
- Disponibilidad total estimada: aproximadamente `191–207 h`.
- Trabajo planificado: `155 h`.
- Reserva para integración, bloqueos y correcciones: aproximadamente `36–52 h`.
- Las horas son objetivos de planificación, no criterios para forzar el cierre de una fase incompleta.

## Tareas

Estados permitidos: `PENDIENTE`, `EN CURSO`, `BLOQUEADA`, `COMPLETADA`.

### T0 — Congelar contrato y preparar entorno

- **Estado:** PENDIENTE
- **Estimación:** 8 h
- **Dependencias:** ninguna
- **Desglose estimado:** entorno 1 h; dependencias/submodules 1,5 h; contrato `.pak` 2 h; prueba FlatBuffers Python/C++ 1,5 h; build CMake mínimo 2 h.
- **Trabajo, en este orden:**
  1. **Auditar el entorno (1 h):** registrar versiones de GCC, CMake, Python, Vulkan SDK, loader y capa de validación. Cargar `setup-env.sh` del SDK en terminal e IDE. No instalar GLFW ni FlatBuffers globalmente.
  2. **Cerrar dependencias y submodules (1,5 h):** fijar los commits/tags de GLFW y FlatBuffers bajo `third_party/`; documentar cómo se compilan desde CMake. Decidir el parser JSON y el decoder PNG como submodules, no como paquetes globales.
  3. **Cerrar el contrato del `.pak` (2 h):** asignar valores al enum de tipos; fijar límites de memoria de la TOC y límites por tipo; conservar las reglas v1 ya definidas de little-endian, CRC-32/ISO-HDLC, offsets absolutos y alineación de payloads de 16 bytes.
  4. **Probar FlatBuffers (1,5 h):** compilar `flatc` desde el submodule y generar/verificar un buffer mínimo desde Python y C++. Usar `flatc` y runtimes de Python/C++ del mismo commit o versión compatible.
  5. **Verificar el build mínimo (2 h):** configurar C++17/CMake; compilar GLFW desde su submodule y enlazar una aplicación mínima con el Vulkan SDK. Esta prueba verifica el build, no implementa la superficie/swapchain de T1.
- **Propuestas de límites para confirmar durante T0; no son valores aprobados todavía:** TOC de hasta 4 MiB (máximo derivado de 104 857 entradas de 40 bytes); buffers FlatBuffers de hasta 64 MiB por recurso; PNG hasta 4096 × 4096 y 64 MiB de memoria decodificada.
- **Propuesta de dependencias C++ pendiente de confirmar:** `nlohmann/json` para JSON y `stb_image` para PNG, ambos como submodules. CRC-32 puede implementarse en el proyecto para evitar una biblioteca global adicional.
- **Finaliza cuando:** contrato del `.pak`, tipos y límites quedan versionados/documentados; dependencias tienen versiones fijadas como submodules; el ejemplo FlatBuffers Python/C++ funciona; el entorno del SDK está configurado; y el proyecto C++17/CMake compila en Linux sin GLFW/FlatBuffers instalados globalmente.
- **No incluye:** crear ventana, superficie X11, swapchain ni presentar frames; esos trabajos pertenecen a T1.

### T1 — Prototipo técnico Vulkan Linux

- **Estado:** PENDIENTE
- **Estimación:** 20 h
- **Dependencias:** T0
- **Trabajo:**
  - Crear instancia con API Vulkan 1.3 y seleccionar explícitamente la GPU AMD.
  - Crear una ventana GLFW bajo X11, superficie XCB o Xlib, swapchain y cola gráfica/presentación.
  - Habilitar `VK_LAYER_KHRONOS_validation` en desarrollo y recibir mensajes mediante `VK_EXT_debug_utils`.
  - Presentar una imagen de prueba y responder al cierre de ventana.
- **Finaliza cuando:** el prototipo presenta al menos un frame en la GPU AMD; la capa de validación está activa; no hay errores ni advertencias atribuibles al engine. Registrar por separado las advertencias del ICD sobre `VK_KHR_display`.
- **Punto de control:** si no se presenta una imagen después de esta estimación, detener la expansión del runtime/editor y resolver primero la integración Vulkan/GLFW/X11.

### T2 — Runtime Linux mínimo

- **Estado:** PENDIENTE
- **Estimación:** 32 h
- **Dependencias:** T1 y T0
- **Trabajo:**
  - Implementar el bucle principal, input de teclado/ratón y cierre limpio.
  - Implementar lector `.pak` conforme al contrato v1.
  - Leer recursos bajo demanda; verificar cabecera, tabla, rangos, alineación y CRC.
  - Verificar buffers FlatBuffers antes de leerlos.
  - Cargar y renderizar un sprite usando un paquete de prueba, sin esperar a que termine la CLI.
- **Finaliza cuando:** el runtime carga un `.pak` de prueba, muestra correctamente el sprite, responde a input básico y cierra sin errores de Vulkan atribuibles al engine.

### T3 — Editor Linux mínimo

- **Estado:** PENDIENTE
- **Estimación:** 40 h
- **Dependencias:** T1; T0 para los formatos JSON
- **Trabajo:**
  - Implementar solo los componentes Builder requeridos por el editor.
  - Importar y seleccionar tiles de imágenes/tilesets.
  - Colocar y borrar tiles en una cuadrícula de dimensiones configurables.
  - Editar al menos dos capas básicas.
  - Definir clips de animación por fotogramas desde una spritesheet y previsualizarlos en bucle.
  - Guardar y volver a abrir el proyecto, nivel y animación en JSON.
- **Finaliza cuando:** un nivel con capas y un clip de animación se pueden crear, guardar, cerrar y reabrir sin pérdida de datos.
- **Límite:** no implementar UI de juego, layouts responsive para móvil, editor de diálogos ni explorador avanzado.

### T4 — CLI empaquetadora y FlatBuffers

- **Estado:** PENDIENTE
- **Estimación:** 30 h
- **Dependencias:** T0 y T2; coordinar el JSON que produce T3
- **Trabajo:**
  - Implementar CLI Python con rutas de entrada/salida explícitas.
  - Validar JSON, versiones de esquema, IDs y referencias a recursos.
  - Convertir niveles y animaciones JSON a FlatBuffers.
  - Escribir cabecera, TOC y payloads del `.pak` v1, con padding hasta alineación de 16 bytes y CRC-32/ISO-HDLC.
  - Escribir a archivo temporal y publicar el destino final solo si el empaquetado concluye correctamente.
- **Finaliza cuando:** genera un paquete que el runtime abre; rechaza entradas inválidas, referencias ausentes y datos corruptos con errores descriptivos.

### T5 — Integración end-to-end, QA y entrega

- **Estado:** PENDIENTE
- **Estimación:** 25 h
- **Dependencias:** T2, T3 y T4
- **Trabajo:**
  - Ejecutar flujo editor → JSON → FlatBuffers/`.pak` → runtime con un nivel técnico pequeño.
  - Añadir pruebas del lector para versiones inválidas, tabla truncada, offsets no alineados, IDs duplicados, rangos fuera del archivo, solapamientos, CRC incorrecto y tamaños desbordados.
  - Verificar que abrir el `.pak` no lee todos los payloads y que solicitar un recurso lee su rango; probar CRC por bloques.
  - Ejecutar la escena de referencia a `1366 × 768` durante 5 minutos y registrar tiempos; objetivo inicial: percentil 95 ≤ `16,67 ms`.
  - Ejecutar la escena durante 10 minutos y repetir 10 veces abrir/cargar/renderizar/cerrar.
  - Preparar versiones ejecutables Linux e instrucciones de prueba.
- **Finaliza cuando:** se cumplen los criterios de aceptación de T0–T4 y se entregan runtime, editor, empaquetador e instrucciones en Linux Mint 22.2.

## Hitos de integración

1. Ventana GLFW presenta imagen usando la GPU AMD y Validation Layer.
2. Runtime muestra sprite de un `.pak` de prueba.
3. Editor guarda y reabre nivel/animación JSON.
4. CLI genera `.pak` que el runtime carga.
5. Flujo end-to-end y pruebas de errores pasan en Linux Mint 22.2.

## Siguiente etapa, fuera de este todo del MVP

1. Portar el runtime a Android 16/API 36+ y probar entrada táctil, ciclo de vida, Vulkan y el mismo `.pak`.
2. Después de Android, evaluar otros sistemas y backends (por ejemplo Windows y WebAssembly).
