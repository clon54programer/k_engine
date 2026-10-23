# K_engine

## Descripción breve

K_engine es un engine 2D personal escrito principalmente en C++17. Busca combinar control de bajo nivel con herramientas para crear y probar niveles y animaciones. El runtime usa Vulkan; el editor es una aplicación de escritorio y el empaquetador es una CLI en Python.

## Estructura actual del proyecto

```text
.
├── deps.md                         # Dependencias del MVP y cómo obtenerlas
├── docs/
│   ├── image.png                   # Recurso gráfico de documentación
│   └── project-planning-template.md # Plantilla de planificación
├── general_description.md          # Descripción inicial y contexto del proyecto
├── k_engine.md                     # Requisitos, alcance y decisiones técnicas
├── readme.md                       # Resumen del proyecto
├── research.md                     # Temas de investigación y aprendizaje
└── todo.md                         # Tareas y criterios de aceptación del MVP Linux
```

Actualmente el repositorio contiene documentación y planificación; la implementación del runtime, el editor y el empaquetador todavía no está organizada en directorios de código.

## MVP

El primer MVP valida en Linux Mint 22.2 x86_64 el flujo completo de creación y ejecución de contenido: editar un nivel 2D y una animación en el editor, guardar los datos de desarrollo en JSON, convertirlos a FlatBuffers y empaquetarlos en un `.pak`, y cargar y mostrar ese contenido desde un runtime Vulkan. El MVP incluye runtime, editor y empaquetador para Linux, además de instrucciones básicas de prueba.

## Fuera del MVP

El runtime Android se desarrollará después de completar el MVP Linux; Windows, WebAssembly y otras plataformas se evaluarán más adelante. También quedan fuera de esta primera entrega la física y el combate, el audio de runtime, la red, el cifrado y la compresión adicional del `.pak`, la animación esquelética y las interfaces de juego.

Para ver el alcance detallado, las dependencias, las tareas y los temas de investigación, consultar `k_engine.md`, `deps.md`, `todo.md` y `research.md`.
