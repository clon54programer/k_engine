---
description: Trabaja una tarea de todo.md respetando AGENTS.md
agent: build
---

Trabaja en la tarea `$ARGUMENTS` de `todo.md` para K_engine. Si no se indicó
ninguna tarea, pregunta cuál corresponde antes de empezar.

1. Lee `AGENTS.md`, `k_engine.md`, `deps.md` y la sección correspondiente de
   `todo.md`.
2. Confirma el alcance, las dependencias y los criterios de finalización de la
   tarea.
3. Revisa el estado actual del repositorio y del código relacionado.
4. Implementa solo los cambios necesarios dentro de los archivos permitidos.
5. Ejecuta las comprobaciones disponibles sin instalar dependencias.
6. Si necesitas editar un archivo protegido, detente y solicita aprobación.
7. Al terminar, informa: cambios realizados, archivos afectados, pruebas
   ejecutadas, pruebas no ejecutadas, errores pendientes y siguiente paso.

No marques la tarea como completada si no se verificó su criterio de
finalización.
