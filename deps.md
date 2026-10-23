# K_engine — Dependencias del MVP Linux

## 1. Alcance

Esta lista cubre el MVP para **Linux Mint 22.2 Xfce, x86_64, X11**: runtime Vulkan, editor de escritorio y empaquetador Python. Android y otras plataformas quedan fuera de esta instalación inicial.

Los agentes deben respetar los estados de las dependencias:

- **Requerida:** instalar o verificar antes de implementar el componente.
- **Ya disponible:** detectada en el entorno; no reinstalar sin necesidad.
- **Alternativa:** elegir una opción, no instalar ambas salvo que el build lo requiera.
- **Pendiente de decisión:** no incorporarla silenciosamente; resolverla antes de depender de ella.
- **Futura:** no instalar para el MVP Linux.

## 2. Herramientas y paquetes del sistema requeridos

Las bibliotecas de código del proyecto se incorporarán como **Git submodules** bajo `third_party/`; no se instalarán globalmente con APT. APT se usará solo para herramientas del sistema, el driver Vulkan y headers/librerías de integración con X11.

Instalación base del sistema en Linux Mint 22.2:

```bash
sudo apt update
sudo apt install \
  build-essential \
  cmake \
  ninja-build \
  pkg-config \
  git \
  mesa-vulkan-drivers \
  libvulkan1 \
  libgl1-mesa-dev \
  libx11-dev \
  libxrandr-dev \
  libxinerama-dev \
  libxcursor-dev \
  libxi-dev \
  libxxf86vm-dev \
  python3 \
  python3-venv \
  python3-pip
```

| Dependencia | Estado | Uso |
|---|---|---|
| GCC/G++ y herramientas de compilación (`build-essential`) | Requerida; GCC 13.3.0 ya está registrado como disponible | Compilar runtime y editor con C++17. |
| CMake | Requerida; versión registrada: 3.28.3 | Configurar y compilar los objetivos C++. |
| Ninja | Recomendada | Generador rápido para CMake; se puede usar Make si el proyecto lo configura así. |
| `pkg-config` | Recomendada | Descubrir bibliotecas del sistema cuando un paquete CMake no proporcione su configuración. |
| Driver Mesa Vulkan (`mesa-vulkan-drivers`) | Requerida para la GPU AMD objetivo | Proporciona RADV para la AMD Radeon Graphics Renoir. Verificar que el dispositivo AMD se enumere y no usar `llvmpipe` como objetivo de rendimiento. |
| Loader Vulkan del sistema (`libvulkan1`) | Requerido como runtime Vulkan del sistema; ya disponible en el entorno | Proporciona la interfaz entre la aplicación y el ICD instalado. No es una biblioteca del engine para convertir en submodule. El SDK ya aporta headers y herramientas de desarrollo. |
| Headers/librerías de X11 y GLX | Requeridos para compilar GLFW con backend X11 | `libgl1-mesa-dev`, `libx11-dev`, `libxrandr-dev`, `libxinerama-dev`, `libxcursor-dev`, `libxi-dev` y `libxxf86vm-dev`. Son dependencias de integración con el sistema; GLFW se compila desde el submodule del proyecto. |
| Python 3, `venv` y `pip` | Requerida | Ejecutar la CLI empaquetadora en un entorno virtual local; no instalar paquetes Python del proyecto globalmente. |

No instalar `libglfw3-dev`, `libvulkan-dev`, `vulkan-tools`, `flatbuffers-compiler` ni `libflatbuffers-dev` globalmente para este proyecto: GLFW y FlatBuffers serán submodules, y el Vulkan SDK ya aporta los headers y herramientas de desarrollo. `mesa-vulkan-drivers` y las bibliotecas/headers de X11 sí son componentes del sistema.

## 3. Bibliotecas de código como Git submodules

Las dependencias de código se guardarán en `third_party/` como submodules fijados a un commit o tag conocido. Al incorporarlas al repositorio, inicializarlas con:

```bash
git submodule update --init --recursive
```

| Submodule | Estado | Uso/build |
|---|---|---|
| GLFW | Requerido | Ventana X11, input y creación de superficie Vulkan. Compilar como parte del build del proyecto; no enlazar a `libglfw3-dev` global. |
| FlatBuffers | Requerido | Compilar `flatc` y usar su runtime C++ desde el submodule. Usar también el runtime Python del mismo submodule para que los buffers escritos por la CLI sean compatibles. |
| Parser JSON C++ | Pendiente de decisión; candidato `nlohmann/json` | Editor: guardar y volver a cargar JSON. Incorporarlo como submodule cuando se apruebe la selección. |
| Decodificador PNG | Pendiente de decisión; candidato `stb_image` | Editor/runtime: importar, previsualizar y cargar imágenes. Incorporarlo como submodule cuando se apruebe la selección. |

No añadir dependencias de código descargadas automáticamente durante la configuración de CMake; usar los submodules fijados para que el build sea reproducible y funcione sin resolver versiones globales.

## 4. Vulkan SDK y Validation Layers

El Vulkan SDK oficial de LunarG **ya está instalado desde el tarball**. Se usará para headers, herramientas de desarrollo, shader compiler y Validation Layers. Extraer el tarball no instala necesariamente sus capas en las rutas globales del loader.

En el entorno de desarrollo, cargar el script del SDK en cada terminal o configurarlo en el IDE:

```bash
source /ruta/al/VulkanSDK/<version>/setup-env.sh
```

Después, comprobar qué herramientas y capas detecta el entorno:

```bash
command -v vulkaninfo
vulkaninfo --summary | grep -F "VK_LAYER_KHRONOS_validation"
```

Estado del equipo de referencia:

- `VK_LAYER_KHRONOS_validation` aparece disponible como versión de capa `1.4.363` cuando se configura el entorno del SDK.
- El runtime debe habilitarla en compilaciones de desarrollo al crear `VkInstance`; para recibir los mensajes en el programa debe configurar `VK_EXT_debug_utils` y un debug messenger.
- Las advertencias del ICD relacionadas con `vkGetPhysicalDeviceDisplayPropertiesKHR` y `vkGetPhysicalDeviceDisplayPlanePropertiesKHR` pertenecen a presentación directa (`VK_KHR_display`). No son necesarias para el objetivo X11 con XCB/Xlib.
- Vulkan SDK y paquetes del sistema pueden proporcionar loaders/herramientas de versiones distintas. Antes de depurar, registrar `command -v vulkaninfo`, la versión del loader y las variables de entorno del SDK. No copiar bibliotecas del SDK a rutas globales como solución por defecto.

No instalar `vulkan-validationlayers` desde APT mientras se use la capa del SDK ya verificada, salvo que se decida abandonar el SDK para las capas. Evitar tener rutas de capas ambiguas y registrar cuál se activa.

El compilador de shaders `glslc` suele venir con el Vulkan SDK. Si no está disponible, elegir una alternativa como `glslangValidator` del paquete `glslang-tools`; no instalar ambos compiladores sin una necesidad del build.

## 5. Dependencias Python del empaquetador

Crear un entorno virtual dentro del repositorio:

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip
```

| Paquete Python | Estado | Uso |
|---|---|---|
| Runtime Python de FlatBuffers | Requerido; se importará desde el submodule FlatBuffers, no desde un paquete global de `pip` | Construir buffers FlatBuffers desde el empaquetador. Si hace falta registrarlo en el venv, instalarlo desde la ruta local del submodule y no desde PyPI sin fijar versión. |
| `json` | Incluido en la biblioteca estándar | Leer y validar los JSON de desarrollo; no instalar por `pip`. |
| `zlib` | Incluido en la biblioteca estándar de Python | Calcular CRC-32/ISO-HDLC incremental con `zlib.crc32`; no instalar por `pip`. |
| `pytest` | Opcional, pendiente de elegir framework de pruebas | Automatizar pruebas de la CLI y del empaquetador. |

La versión del runtime Python y `flatc` debe proceder del mismo submodule/commit de FlatBuffers. No depender de una versión flotante de PyPI.

## 6. Dependencias de código pendientes de selección

Estas dependencias son necesarias para las funciones descritas, pero las decisiones concretas aún deben cerrarse antes de instalarlas o añadirlas al build:

| Necesidad | Estado | Opción candidata |
|---|---|---|
| Leer/escribir JSON en el editor C++ | Pendiente de decisión | Candidato: `nlohmann/json` como submodule. El runtime de distribución no necesita JSON. |
| Decodificar y previsualizar PNG en el editor/runtime | Pendiente de decisión | Candidato: `stb_image` como submodule. |
| CRC-32/ISO-HDLC en C++ | Pendiente de decisión de implementación | Implementación pequeña en el proyecto o submodule de zlib; evitar enlazar una biblioteca global sin decisión explícita. |
| Framework de pruebas C++ | Pendiente de decisión | Elegirlo e incorporarlo como submodule cuando se implementen las pruebas automatizadas del runtime/lector. |

Estas decisiones no cambian el alcance del MVP; se deben cerrar antes de que las tareas dependientes queden bloqueadas.

## 7. Dependencias que no se incorporan al MVP

- `miniaudio`, OpenAL y dependencias de audio: audio de runtime aplazado.
- Box2D: física y colisiones de juego aplazadas.
- Android NDK, SDK y herramientas de empaquetado Android: etapa posterior al MVP Linux.
- Bibliotecas de red/multijugador: fuera del MVP.
- Dear ImGui u otra biblioteca GUI externa: el editor valida la API Builder propia y no depende de una GUI externa según el alcance actual.

## 8. Verificación del entorno

Antes de compilar el runtime, comprobar:

```bash
g++ --version
cmake --version
python3 --version
flatc --version
vulkaninfo --summary
```

Para `flatc --version`, `PATH` debe resolver el ejecutable construido desde `third_party/flatbuffers`, no un `flatc` global.

Criterios del entorno Vulkan de referencia:

- Debe enumerarse la AMD Radeon Graphics (`1002:15e7`) con RADV/Mesa.
- Para el runtime se usará Vulkan 1.3, aunque el dispositivo informe Vulkan 1.4.
- Deben aparecer las superficies XCB/Xlib y la capa `VK_LAYER_KHRONOS_validation` en el entorno configurado con el SDK.
- La creación de ventana, superficie, swapchain y presentación debe probarse en el prototipo GLFW; `vulkaninfo` por sí solo no sustituye esta prueba.
- GLFW y FlatBuffers deben compilarse desde los submodules fijados, no desde paquetes globales de desarrollo.
