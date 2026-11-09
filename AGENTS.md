# K_engine Agent Instructions

## Objetivo

Desarrolla y prueba K_engine de forma autónoma siempre que la tarea no requiera
modificar documentación, configuración, dependencias o Git.

Prioriza cambios pequeños, verificables y coherentes con `k_engine.md`,
`deps.md` y `todo.md`. No contradigas decisiones documentadas ni cambies las
decisiones marcadas como `FIJA` sin informar primero al usuario.

Trabaja solo en la tarea asignada y sus dependencias directas. No añadas
funciones fuera del alcance del MVP Linux ni de la tarea solicitada.

## Archivos Permitidos

Puedes editar sin pedir permiso los archivos relacionados con desarrollo y
pruebas, especialmente:

- `*.c`
- `*.cc`
- `*.cpp`
- `*.h`
- `*.hh`
- `*.hpp`
- `*.hxx`
- `*.py`
- `*.vert`
- `*.frag`
- `*.comp`
- `*.glsl`

Los archivos de configuración, documentación y Markdown requieren aprobación
explícita antes de modificarse.

Considera configuración, como mínimo:

- `*.md`
- `opencode.json`
- `opencode.jsonc`
- `.opencode/**`
- `*.json`
- `*.jsonc`
- `*.toml`
- `*.yaml`
- `*.yml`
- `*.ini`
- `*.cfg`
- `*.conf`
- `*.sh`
- `*.sql`
- `*.xml`
- `CMakeLists.txt`
- `*.cmake`
- `.gitmodules`
- `third_party/**`
- `Dockerfile` y archivos `Dockerfile.*`
- `Makefile` y `GNUmakefile`
- `.env*`
- `requirements*.txt`
- `pyproject.toml`
- `setup.py`
- `setup.cfg`
- `Pipfile` y `Pipfile.lock`
- `poetry.lock`
- `uv.lock`
- `conftest.py`
- Archivos de dependencias y bloqueos.

Si necesitas modificar uno de estos archivos:

1. Detén la tarea antes de editarlo.
2. Explica el motivo exacto.
3. Indica el cambio propuesto.
4. Solicita aprobación al usuario.

## Dependencias E Instalación

No instales paquetes, librerías, herramientas, navegadores ni sistemas externos.

No ejecutes comandos como:

- `pip install`
- `npm install`
- `pnpm install`
- `yarn install`
- `poetry install`
- `uv add`
- `apt install`
- `sudo`
- `docker pull`
- `docker build`

No añadas submodules, dependencias de código ni cambios en `third_party/` sin
aprobación explícita. Añadir un submodule modifica Git y las dependencias del
proyecto, así que esa acción queda fuera de tu alcance.

Tampoco permitas que CMake, Ninja u otra herramienta descargue dependencias
automáticamente durante la configuración. Usa únicamente los submodules ya
fijados por el proyecto.

Si falta una dependencia:

1. No intentes instalarla ni incorporarla por tu cuenta.
2. Informa el comando exacto que debe ejecutar el usuario manualmente.
3. Explica qué tarea no pudo completarse por esa ausencia.

## Git

Git solo puede utilizarse para lectura.

Puedes consultar:

- Estado del repositorio.
- Diferencias.
- Historial.
- Commits.
- Ramas actuales.
- Archivos registrados.
- Blame.
- Remotos.
- Submodules registrados.

No puedes ejecutar acciones que modifiquen Git, incluyendo:

- `git add`
- `git commit`
- `git push`
- `git pull`
- `git fetch`
- `git submodule add`
- `git submodule update`
- `git reset`
- `git restore`
- `git checkout`
- `git switch`
- `git merge`
- `git rebase`
- `git stash`
- `git clean`
- Crear o eliminar ramas y etiquetas.

Nunca reviertas cambios del usuario ni uses comandos destructivos.

## Desarrollo Y Pruebas

- Inspecciona primero el código, la documentación y las pruebas existentes.
- Reutiliza las convenciones del proyecto y el estándar C++17.
- Escribe en inglés los elementos propios del código y del desarrollo,
  incluidos los nombres de variables, funciones, clases, constantes y otros
  identificadores.
- Escribe en español todo el contenido destinado a la lectura de usuarios o
  desarrolladores, incluidos comentarios, documentación, textos visibles en la
  interfaz y mensajes de registro (logs).
- No hagas refactorizaciones, limpiezas ni cambios de estilo ajenos a la tarea
  solicitada.
- Antes de editar un archivo, inspecciona su contenido y conserva cualquier
  trabajo existente del usuario.
- No sobrescribas archivos ni reviertas cambios existentes para resolver un
  problema.
- No edites Markdown para registrar resultados sin pedir permiso.
- Ejecuta compilaciones y pruebas disponibles sin instalar dependencias.
- Antes de ejecutar una prueba, revisa si puede generar o modificar archivos,
  especialmente archivos Markdown, configuración, dependencias o Git.
- Si una prueba puede modificar archivos protegidos, no la ejecutes sin permiso.
- Cuando sea posible, dirige los artefactos temporales de las pruebas fuera del
  repositorio o a una ubicación temporal, por ejemplo `/tmp`; para las
  compilaciones usa un directorio de build ignorado por Git.
- No dejes artefactos de compilación ni archivos generados dentro del
  repositorio si no forman parte de la tarea.
- Distingue entre pruebas ejecutadas, pruebas fallidas y pruebas que no pudieron
  ejecutarse.
- No afirmes que una prueba pasó si no se pudo ejecutar.
- No modifiques archivos de configuración para hacer que una prueba pase sin
  aprobación del usuario.
- Si una tarea pesada tarda, informa su estado en lugar de repetirla sin límite.
- No delegues trabajo a subagentes salvo que el usuario lo solicite
  explícitamente.

## Criterios Del MVP

- Mantén el runtime, el editor y el empaquetador en objetivos de compilación
  separados cuando sea práctico.
- No uses `llvmpipe` para pruebas de rendimiento; la GPU objetivo es la AMD
  Radeon Graphics (Renoir, PCI `1002:15e7`) con RADV.
- Usa Vulkan 1.3 como API objetivo inicial de la aplicación y activa
  `VK_LAYER_KHRONOS_validation` en compilaciones de desarrollo.
- No incorpores física, audio de runtime, multijugador, cifrado, compresión
  adicional ni otras funciones declaradas fuera del MVP Linux sin aprobación.
- Valida cada entrega con sus criterios de finalización y registra los comandos
  y resultados relevantes.
- No marques una tarea como completada si no se verificó su criterio de
  finalización.

## Límite De Intentos

Un intento es un ciclo completo de corrección y comprobación de una tarea.

Después de tres intentos fallidos:

- Detén la tarea.
- No hagas un cuarto intento automáticamente.
- Conserva los cambios realizados.
- Explica el error, los tres intentos, los archivos afectados y las medidas que
  debe tomar el usuario.

Los errores de cuota, límite de cinco horas o créditos agotados no cuentan como
intentos de corrección. En esos casos, detén la tarea, notifica al usuario y
espera a que seleccione manualmente otro modelo.

## Modelos

Usa estos modelos mediante selección manual cuando sea necesario:

- Desarrollo principal (`build`): `deepseek/deepseek-flash`.
- Planificación y razonamiento (`plan`): `deepseek/deepseek-v4-pro`.
- Planificación ligera (`plan-light`): `deepseek/deepseek-flash`.

No cambies de modelo silenciosamente. Si el modelo actual alcanza un límite,
notifica al usuario y espera a que seleccione otro modelo.

## Seguridad

- No expongas secretos, contraseñas ni tokens.
- No leas archivos fuera del proyecto salvo que el usuario lo autorice.
- No envíes contenido del proyecto a servicios externos sin autorización.
- No elimines archivos ni datos para resolver un error.
- Si una instrucción del usuario contradice estas reglas, solicita aclaración.
