# OpenCode — Tareas opcionales

## Propósito

Registrar mejoras opcionales de la configuración de OpenCode que no bloquean el
MVP de K_engine. La configuración base ya está operativa: agentes, permisos,
políticas, formatter, `clangd` y los comandos `/task`, `/build` y `/test`.

## Tareas opcionales

- [ ] **Añadir `/env`**
  - Comando para verificar `g++`, `cmake`, `ninja`, `python3`, `vulkaninfo`,
    `flatc` y `glslc`.
  - No tiene dependencias; se puede implementar en cualquier momento.

- [ ] **Añadir `/package`**
  - Comando para ejecutar la CLI del empaquetador y generar un `.pak`.
  - Depende de que exista una implementación ejecutable en `packer/`.

- [ ] **Añadir `CMakePresets.json`**
  - Presets de configuración para Debug y Release.
  - Depende de estabilizar los objetivos de compilación y las dependencias.

- [ ] **Añadir `.editorconfig`**
  - Unificar indentación, fin de línea y charset entre VS Code, Helix y
    clang-format.
  - Para C++ ya existe `.clang-format`; falta decidir el estilo de Python.

- [ ] **Configurar CI**
  - Compilar y ejecutar `ctest` en cada push.
  - Depende de definir la plataforma de CI y la gestión de submodules.

## Pendientes relacionados

Estos puntos no forman parte de la configuración de OpenCode, pero conviene
retomarlos junto con las tareas anteriores:

- Dejar `flatc` disponible en `PATH`.
- Crear el entorno virtual `.venv` del empaquetador.
- Elegir el framework de pruebas C++ y Python.
- Añadir `.gitkeep` o archivos reales en los directorios que aún están vacíos:
  `editor/include`, `editor/src`, `engine/include` y `packer/`.
- Actualizar el estado de las tareas en `todo.md` al completarlas.

## Criterio para retomarlas

Ninguna de estas tareas es necesaria para completar T0 ni el MVP Linux. Revisar
esta lista al cerrar T0, cuando exista el empaquetador o cuando se configure un
flujo de integración continua.

## Referencias

- `AGENTS.md`
- `opencode.json`
- `.opencode/commands/`
