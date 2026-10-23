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

Instalación base recomendada en Linux Mint 22.2:

```bash
sudo apt update
sudo apt install \
  build-essential \
  cmake \
  ninja-build \
  pkg-config \
  libglfw3-dev \
  libvulkan-dev \
  mesa-vulkan-drivers \
  vulkan-tools \
  flatbuffers-compiler \
  libflatbuffers-dev \
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
| GLFW (`libglfw3-dev`) | Requerida | Ventana X11, eventos de teclado/ratón y creación de superficie Vulkan mediante GLFW. El paquete de desarrollo instala sus dependencias de sistema necesarias. |
| Vulkan loader/headers (`libvulkan-dev`, `libvulkan1`) | Requerida | Headers para compilar y loader para ejecutar. `libvulkan1` normalmente se instala como dependencia de `libvulkan-dev`. |
| Driver Mesa Vulkan (`mesa-vulkan-drivers`) | Requerida para la GPU AMD objetivo | Proporciona RADV para la AMD Radeon Graphics Renoir. Verificar que el dispositivo AMD se enumere y no usar `llvmpipe` como objetivo de rendimiento. |
| Herramientas Vulkan (`vulkan-tools`) | Requerida para diagnóstico | Proporciona `vulkaninfo`; usarlo para consultar el loader, GPU, superficies y capas. |
| FlatBuffers (`flatbuffers-compiler`, `libflatbuffers-dev`) | Requerida | `flatc` genera código desde `.fbs`; los headers/runtime C++ permiten verificar y leer buffers. Mantener la versión de `flatc` y la biblioteca C++ coherentes. |
| Python 3, `venv` y `pip` | Requerida | Ejecutar la CLI empaquetadora en un entorno aislado del sistema. |

No hace falta instalar dependencias de desarrollo de Xlib/XCB a mano si se usa `libglfw3-dev` desde APT; el gestor instala las dependencias requeridas por ese paquete. Si GLFW se compila desde fuentes en el futuro, reevaluar las dependencias de X11 correspondientes.

## 3. Vulkan SDK y Validation Layers

El Vulkan SDK oficial de LunarG es una distribución aparte de APT. El tarball contiene herramientas de desarrollo y Validation Layers, pero extraerlo no instala necesariamente sus capas en las rutas globales del loader.

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

**Alternativa a la capa del SDK:** instalar `vulkan-validationlayers` desde APT. No es obligatoria si la capa del SDK ya se enumera y funciona en el entorno del proyecto. Evitar seleccionar accidentalmente una capa distinta de la que se probó.

El compilador de shaders `glslc` suele venir con el Vulkan SDK. Si no está disponible, elegir una alternativa como `glslangValidator` del paquete `glslang-tools`; no instalar ambos compiladores sin una necesidad del build.

## 4. Dependencias Python del empaquetador

Crear un entorno virtual dentro del repositorio:

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip
```

| Paquete Python | Estado | Uso |
|---|---|---|
| `flatbuffers` | Requerido para la CLI; fijar versión junto con `flatc` en `requirements.txt` cuando se cierre T0 | Construir buffers FlatBuffers desde el empaquetador. |
| `json` | Incluido en la biblioteca estándar | Leer y validar los JSON de desarrollo; no instalar por `pip`. |
| `zlib` | Incluido en la biblioteca estándar de Python | Calcular CRC-32/ISO-HDLC incremental con `zlib.crc32`; no instalar por `pip`. |
| `pytest` | Opcional, pendiente de elegir framework de pruebas | Automatizar pruebas de la CLI y del empaquetador. |

La versión del runtime Python `flatbuffers` debe ser compatible con el código Python generado por la versión de `flatc`. Registrar ambas versiones y fijar la dependencia en `requirements.txt`; los agentes no deben dejar una instalación flotante sin versión al finalizar la fase del empaquetador.

## 5. Dependencias de C++ pendientes de selección

Estas dependencias son necesarias para las funciones descritas, pero las decisiones concretas aún deben cerrarse antes de instalarlas o añadirlas al build:

| Necesidad | Estado | Opción candidata |
|---|---|---|
| Leer/escribir JSON en el editor C++ | Pendiente de decisión | `nlohmann-json3-dev` (header-only). Confirmar en T0 o elegir otra biblioteca; el runtime de distribución no necesita JSON. |
| Decodificar y previsualizar PNG en el editor/runtime | Pendiente de decisión | `stb_image` (por ejemplo, `libstb-dev` o una copia fijada en `third_party`). Confirmar cómo se versiona antes de depender de ella. |
| CRC-32/ISO-HDLC en C++ | Pendiente de decisión de implementación | Usar una implementación pequeña en el proyecto o enlazar zlib (`zlib1g-dev`). No añadir zlib si se implementa y prueba el algoritmo internamente. |
| Framework de pruebas C++ | Pendiente de decisión | Elegirlo cuando se implementen las pruebas automatizadas del runtime/lector. |

Estas decisiones no cambian el alcance del MVP; se deben cerrar antes de que las tareas dependientes queden bloqueadas.

## 6. Dependencias que no se instalan para el MVP

- `miniaudio`, OpenAL y dependencias de audio: audio de runtime aplazado.
- Box2D: física y colisiones de juego aplazadas.
- Android NDK, SDK y herramientas de empaquetado Android: etapa posterior al MVP Linux.
- Bibliotecas de red/multijugador: fuera del MVP.
- Dear ImGui u otra biblioteca GUI externa: el editor valida la API Builder propia y no depende de una GUI externa según el alcance actual.

## 7. Verificación del entorno

Antes de compilar el runtime, comprobar:

```bash
g++ --version
cmake --version
python3 --version
flatc --version
vulkaninfo --summary
```

Criterios del entorno Vulkan de referencia:

- Debe enumerarse la AMD Radeon Graphics (`1002:15e7`) con RADV/Mesa.
- Para el runtime se usará Vulkan 1.3, aunque el dispositivo informe Vulkan 1.4.
- Deben aparecer las superficies XCB/Xlib y la capa `VK_LAYER_KHRONOS_validation` en el entorno configurado.
- La creación de ventana, superficie, swapchain y presentación debe probarse en el prototipo GLFW; `vulkaninfo` por sí solo no sustituye esta prueba.
