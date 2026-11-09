# K_engine

## Descripción breve

K_engine es un engine 2D personal escrito principalmente en C++17. Busca combinar control de bajo nivel con herramientas para crear y probar niveles y animaciones. El runtime usa Vulkan; el editor es una aplicación de escritorio y el empaquetador es una CLI en Python.

## Estructura del proyecto

```text
.
├── engine/                         # Núcleo/runtime del engine
│   ├── CMakeLists.txt              # Objetivos y pruebas del engine
│   ├── include/
│   ├── schema/                     # Esquemas FlatBuffers
│   ├── src/
│   ├── test/
│   └── k_engine.h                  # Cabecera pública principal
├── editor/                         # Editor de escritorio
│   ├── CMakeLists.txt              # Objetivo y pruebas del editor
│   ├── include/
│   ├── src/
│   ├── test/
│   └── main.cpp                    # Punto de entrada del editor
├── packer/                         # CLI empaquetadora en Python
│   └── tests/                      # Pruebas del componente Python
├── CMakeLists.txt                  # Orquesta dependencias y componentes
├── deps.md                         # Dependencias del MVP y cómo obtenerlas
├── docs/
│   ├── image.png                   # Recurso gráfico de documentación
│   ├── json-schemas.md             # Esquemas JSON de desarrollo
│   ├── opencode-opcional.md        # Tareas opcionales de OpenCode
│   ├── pak-v1.md                   # Contrato del contenedor .pak v1
│   └── project-planning-template.md # Plantilla de planificación
├── general_description.md          # Descripción inicial y contexto del proyecto
├── k_engine.md                     # Requisitos, alcance y decisiones técnicas
├── readme.md                       # Resumen del proyecto
├── research.md                     # Temas de investigación y aprendizaje
└── todo.md                         # Tareas y criterios de aceptación del MVP Linux
```

Los directorios `engine/src/` y `editor/src/` se mantienen separados para
conservar independientes el código del núcleo y el del editor. `engine/k_engine.h`
y `editor/main.cpp` están en la raíz de su componente, fuera de `src/`, para poder
localizarlos rápidamente sin tener que entrar en el código fuente interno de cada
componente.

Cada componente C++ define sus objetivos y pruebas en su propio
`CMakeLists.txt`. El CMake raíz solo configura las dependencias comunes,
`enable_testing()` y `add_subdirectory()`; no contiene targets ni `add_test()` de
componentes. La estructura de directorios y el build mínimo ya están definidos;
el resto de la implementación se incorporará en `engine/` y `editor/`, y la CLI
en `packer/`.

## MVP

El primer MVP valida en Linux Mint 22.2 x86_64 el flujo completo de creación y ejecución de contenido: editar un nivel 2D y una animación en el editor, guardar los datos de desarrollo en JSON, convertirlos a FlatBuffers y empaquetarlos en un `.pak`, y cargar y mostrar ese contenido desde un runtime Vulkan. El MVP incluye runtime, editor y empaquetador para Linux, además de instrucciones básicas de prueba.

## Fuera del MVP

El runtime Android se desarrollará después de completar el MVP Linux; Windows, WebAssembly y otras plataformas se evaluarán más adelante. También quedan fuera de esta primera entrega la física y el combate, el audio de runtime, la red, el cifrado y la compresión adicional del `.pak`, la animación esquelética y las interfaces de juego.

Para ver el alcance detallado, las dependencias, las tareas y los temas de investigación, consultar `k_engine.md`, `deps.md`, `todo.md` y `research.md`.
