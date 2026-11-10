# Guía de pruebas: runtime y editor de K_engine

Esta guía explica cómo probar el estado actual de K_engine paso a paso. Está
pensada para dos públicos:

- **Personas:** pruebas manuales con ventana, ratón y teclado.
- **Agentes de IA:** comandos reproducibles, salidas esperadas, códigos de
  salida y límites de automatización.

## 1. Estado actual que debes tener presente

Ahora mismo existen dos flujos separados:

```text
Runtime:  .pak v1  ───────────────►  k_engine_runtime
Editor:   PNG + JSON de desarrollo ─►  editor
```

El editor todavía **no genera `.pak`**. El empaquetador de T4 está pendiente.
Por eso:

- El runtime se prueba con el fixture `.pak` de T2.
- El editor se prueba creando y reabriendo un proyecto JSON.
- No intentes abrir el `project.json` del editor con el runtime: no es un
  `.pak`.
- La integración editor → JSON → FlatBuffers/`.pak` → runtime pertenece a T4/T5.

---

## 2. Requisitos

Sistema de referencia:

- Linux Mint 22.2 Xfce, x86_64, sesión X11.
- GPU AMD Radeon Graphics, PCI `1002:15e7`, con RADV.
- Vulkan SDK y `VK_LAYER_KHRONOS_validation`.
- GCC 13, CMake 3.28, Ninja y Python 3.

Antes de compilar o ejecutar, carga el entorno del Vulkan SDK en la terminal:

```bash
source /ruta/al/VulkanSDK/1.4.363.0/setup-env.sh
```

Comprueba las herramientas:

```bash
g++ --version
cmake --version
ninja --version
python3 --version
command -v glslc
command -v vulkaninfo
```

Si falta alguna dependencia del sistema, consulta `deps.md`. No instales
paquetes ni submodules por cuenta propia si estás trabajando como agente.

---

## 3. Compilar y ejecutar las pruebas automáticas

Desde la raíz del repositorio:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Resultado esperado:

```text
100% tests passed, 0 tests failed out of 15
```

Las pruebas generan los fixtures de runtime en:

```text
build/engine/test_runtime_fixture/
```

El paquete válido que usa esta guía es:

```text
build/engine/test_runtime_fixture/runtime_test.pak
```

Si ese archivo no existe, ejecuta primero `ctest`.

### Pruebas útiles por separado

```bash
ctest --test-dir build --output-on-failure -R k_engine_pak_reader_test
ctest --test-dir build --output-on-failure -R k_engine_runtime_content_test
ctest --test-dir build --output-on-failure -R k_engine_ui_builder_test
ctest --test-dir build --output-on-failure -R k_editor_
```

---

## 4. Probar el runtime

### 4.1. Prueba de humo no interactiva

Presenta un solo frame y termina:

```bash
./build/engine/k_engine_runtime \
  --pak build/engine/test_runtime_fixture/runtime_test.pak \
  --frames 1
```

Código de salida esperado: `0`.

Salida esperada, con variaciones menores según la sesión:

```text
GPU seleccionada: AMD Radeon Graphics (RADV RENOIR).
Swapchain creado: 1280x720, 4 imágenes, formato ...
Paquete cargado: .../runtime_test.pak (4 recursos).
Nivel cargado: 2 "runtime_level".
Sprite cargado: PNG 100 (64x64).
Validación: 0 errores, 0 advertencias del engine, 0 advertencias del ICD sobre VK_KHR_display.
```

Comprueba el código de salida:

```bash
echo $?
```

Debe imprimir `0`.

### 4.2. Prueba de varios frames

```bash
./build/engine/k_engine_runtime \
  --pak build/engine/test_runtime_fixture/runtime_test.pak \
  --frames 300
```

Debe presentar 300 frames y terminar con `0`. Sirve como comprobación básica de
estabilidad y presentación. No es todavía la prueba de rendimiento de T5.

### 4.3. Seleccionar nivel por ID

El fixture contiene el nivel `2`:

```bash
./build/engine/k_engine_runtime \
  --pak build/engine/test_runtime_fixture/runtime_test.pak \
  --level-id 2 \
  --frames 1
```

Si se omite `--level-id`, el runtime usa el primer nivel del paquete.

### 4.4. Prueba interactiva

```bash
./build/engine/k_engine_runtime \
  --pak build/engine/test_runtime_fixture/runtime_test.pak
```

Controles:

- Flechas o W/A/S/D: mover el sprite.
- Clic izquierdo: recolocar el sprite centrado en el cursor.
- ESC: cerrar.
- Botón de cierre de la ventana: cerrar.
- Redimensionar y minimizar la ventana: deben funcionar sin errores.

Al cerrar, la salida debe ser `0` y la Validation Layer debe mostrar:

```text
0 errores, 0 advertencias del engine
```

### 4.5. Comprobar que el runtime rechaza paquetes corruptos

El fixture incluye variantes inválidas. Ejemplo:

```bash
./build/engine/k_engine_runtime \
  --pak build/engine/test_runtime_fixture/bad_signature.pak \
  --frames 1
```

Debe terminar con código distinto de cero y mostrar un error descriptivo. No
debe abrir una ventana con contenido válido.

Otros casos disponibles:

- `bad_version.pak`
- `bad_toc_crc.pak`
- `duplicate_id.pak`
- `unaligned_offset.pak`
- `range_outside.pak`
- `overlap.pak`
- `bad_resource_crc.pak`
- `truncated.pak`
- `truncated_payload.pak`

---

## 5. Probar el editor

### 5.1. Crear un proyecto de prueba

Usa una ruta temporal fuera del repositorio:

```bash
mkdir -p /tmp/opencode/k_engine_editor_demo
```

Crea el proyecto y importa el PNG de prueba de T2:

```bash
./build/editor/editor \
  --new /tmp/opencode/k_engine_editor_demo \
  --import-png build/engine/test_runtime_fixture/sprite.png \
  --frames 0
```

`--frames 0` significa “presenta hasta que se cierre la ventana”. Para editar
manualmente, usa `0` o no pases `--frames`.

### 5.2. Interfaz del editor

La ventana se divide en:

- **Barra superior:** Guardar, Nivel, Animación, Nuevo nivel, Nueva animación y
  estado.
- **Panel izquierdo:** nombre del proyecto, assets, niveles y animaciones.
- **Centro:** lienzo de tiles o previsualización de animación.
- **Panel derecho:** propiedades del nivel, capas, cuadrícula o animación.
- **Panel inferior:** paleta de tiles o tira de fotogramas de la spritesheet.

### 5.3. Crear un nivel con dos capas

1. Comprueba que el asset importado aparece en el panel izquierdo.
2. Pulsa **Nuevo nivel** en la barra superior.
3. El editor crea:
   - una capa `fondo` de tipo `tile`, visible;
   - una capa `colision` de tipo `collision`, oculta.
4. En el panel derecho ajusta, si quieres:
   - nombre del nivel;
   - tile width/height;
   - columnas y filas.
5. En la paleta inferior selecciona un tile.
6. En el lienzo:
   - clic izquierdo: pintar;
   - clic derecho: borrar;
   - rueda: zoom;
   - botón central: pan.
7. Selecciona la capa `colision` en el panel derecho y pinta algunas celdas.
   La capa está oculta, pero al seleccionarla se muestra un overlay para editar.
8. Cambia el nombre o la visibilidad de una capa si lo necesitas.

### 5.4. Crear una animación

1. Pulsa **Nueva animación**.
2. En el panel derecho ajusta:
   - nombre;
   - frame width/height;
   - FPS;
   - loop;
   - pivot;
   - hitbox.
3. En la tira inferior de la spritesheet, haz clic en las celdas que quieras
   añadir como fotogramas.
4. Cada clic añade o quita ese fotograma.
5. Activa **Reproducir** para ver la previsualización en bucle.
6. El panel central muestra el fotograma actual, el pivot y la hitbox.

### 5.5. Guardar, cerrar y reabrir

Guarda con `Ctrl+S` o con el botón **Guardar**.

Cierra la ventana con el gestor de ventanas. Si hay cambios, el editor intenta
guardar antes de cerrar. Si el proyecto no es válido, cancela el cierre y
muestra el error.

Archivos esperados:

```text
/tmp/opencode/k_engine_editor_demo/project.json
/tmp/opencode/k_engine_editor_demo/levels/level_<id>.json
/tmp/opencode/k_engine_editor_demo/animations/animation_<id>.json
/tmp/opencode/k_engine_editor_demo/assets/asset_<id>_sprite.png
```

Reabre el proyecto:

```bash
./build/editor/editor \
  --project /tmp/opencode/k_engine_editor_demo/project.json \
  --frames 0
```

Comprueba que siguen estando:

- los tiles pintados en `fondo`;
- los tiles pintados en `colision`;
- los fotogramas de la animación;
- los nombres, fps, loop, pivot y hitbox.

### 5.6. Prueba de humo del editor

Sin interacción:

```bash
./build/editor/editor \
  --project /tmp/opencode/k_engine_editor_demo/project.json \
  --frames 3
```

Código de salida esperado: `0`.

Salida esperada:

```text
Instancia Vulkan 1.3 del editor creada.
GPU seleccionada: AMD Radeon Graphics (RADV RENOIR).
Swapchain del editor: ... 
Validación: 0 errores, 0 advertencias del engine, 0 advertencias del ICD sobre VK_KHR_display.
```

Importante: `--frames` es solo para pruebas de humo. Si usas un límite de
frames, el editor termina al alcanzarlo y no ejecuta el guardado de cierre.

### 5.7. Resize, minimizado y cierre

Ejemplo con `wmctrl`:

```bash
./build/editor/editor \
  --project /tmp/opencode/k_engine_editor_demo/project.json \
  --frames 0 &

wmctrl -r "K_engine - Editor T3" -e 0,100,100,800,600
wmctrl -r "K_engine - Editor T3" -b add,hidden
wmctrl -r "K_engine - Editor T3" -b remove,hidden
wmctrl -c "K_engine - Editor T3"
```

El proceso debe terminar con `0` y sin errores de validación atribuibles al
engine.

---

## 6. Validar los JSON generados

Puedes revisar los JSON con Python, sin instalar nada:

```bash
python3 -m json.tool /tmp/opencode/k_engine_editor_demo/project.json
python3 -m json.tool /tmp/opencode/k_engine_editor_demo/levels/level_2.json
python3 -m json.tool /tmp/opencode/k_engine_editor_demo/animations/animation_3.json
```

Comprobación rápida del estado del nivel:

```bash
python3 - <<'PY'
import json
from pathlib import Path

base = Path("/tmp/opencode/k_engine_editor_demo")
project = json.loads((base / "project.json").read_text())
level_path = base / project["levels"][0]["path"]
level = json.loads(level_path.read_text())

print("capas:", [(l["name"], l["kind"], l["visible"]) for l in level["layers"]])
for layer in level["layers"]:
    print(layer["name"], "tiles no vacíos:", sum(1 for tile in layer["tiles"] if tile))
PY
```

Comprobación de la animación:

```bash
python3 - <<'PY'
import json
from pathlib import Path

base = Path("/tmp/opencode/k_engine_editor_demo")
project = json.loads((base / "project.json").read_text())
animation_path = base / project["animations"][0]["path"]
animation = json.loads(animation_path.read_text())

print("fps:", animation["fps"])
print("loop:", animation["loop"])
print("fotogramas:", len(animation["frames"]))
print("pivot:", animation["pivot"])
print("hitbox:", animation["hitbox"])
PY
```

---

## 7. Guía específica para agentes de IA

### 7.1. Flujo no interactivo recomendado

```bash
source /ruta/al/VulkanSDK/1.4.363.0/setup-env.sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure

./build/engine/k_engine_runtime \
  --pak build/engine/test_runtime_fixture/runtime_test.pak \
  --frames 1

./build/editor/editor \
  --project /tmp/opencode/k_engine_editor_demo/project.json \
  --frames 3
```

Criterios de éxito:

- `ctest` termina con `100% tests passed`.
- Runtime y editor terminan con código `0`.
- La salida no contiene errores de Vulkan atribuibles al engine.
- El resumen de validación muestra `0 errores` y `0 advertencias del engine`.

### 7.2. Comandos que no debes ejecutar sin supervisión

- No ejecutes el editor con `--frames 0` en un proceso sin ventana X11.
- No intentes automatizar clics si no tienes una herramienta X11/XTest
  disponible.
- No uses `llvmpipe`.
- No ejecutes pruebas de rendimiento todavía; pertenecen a T5.
- No edites Markdown, CMake, JSON de configuración, dependencias o Git sin
  aprobación.

### 7.3. Automatización de UI

Las pruebas automáticas actuales son headless para el modelo, el Builder y la
persistencia. La ventana Vulkan se verifica con `--frames` y con herramientas
externas como `wmctrl`.

Si necesitas simular clics, usa una herramienta XTest externa en
`/tmp/opencode`; no añadas esa herramienta al repositorio. Antes de cada clic:

```bash
wmctrl -a "K_engine - Editor T3"
```

Espera a que la ventana esté activa y mapeada antes de enviar eventos.

### 7.4. Rutas temporales

Usa siempre `/tmp/opencode` para:

- proyectos de prueba;
- builds limpios;
- capturas;
- registros.

No dejes proyectos de prueba ni artefactos dentro del repositorio.

### 7.5. Interpretación de errores frecuentes

| Mensaje o síntoma | Causa probable | Acción |
|---|---|---|
| `glslc no encontrado` | SDK no cargado | Ejecuta `setup-env.sh` y vuelve a configurar. |
| `Falta la capa de validación` | SDK/layer no disponible | Carga el SDK; no instales paquetes globales sin aprobación. |
| `No se encontró la GPU AMD ... 1002:15e7` | GPU/entorno distinto | Usa el equipo de referencia; no cambies a `llvmpipe`. |
| `No existe build/.../runtime_test.pak` | Fixtures no generados | Ejecuta `ctest`. |
| `El proyecto debe tener al menos un nivel` | Proyecto nuevo sin nivel | Importa un PNG y crea un nivel antes de guardar/cerrar. |
| `El PNG ... no es válido` | Asset corrupto o formato no admitido | Usa un PNG válido, por ejemplo el fixture `sprite.png`. |
| La ventana se queda minimizada | Tamaño de framebuffer `0×0` | Restaura la ventana o cierra con el gestor. |
| `Validation error` | Uso incorrecto de Vulkan | Revisa la llamada indicada; no ignorares el error. |

---

## 8. Limitaciones actuales

- No existe todavía la CLI empaquetadora de T4.
- El editor no convierte JSON a FlatBuffers ni genera `.pak`.
- El runtime solo lee `.pak` v1.
- El editor no crea ni edita entidades; conserva las entidades existentes.
- No hay undo/redo, copiar/pegar ni autotiling.
- La UI permite añadir tilesets, pero no reasignar visualmente una capa a otro
  tileset.
- El texto de UI es ASCII; algunos mensajes con acentos pueden verse con `?`.
- No hay UI de juego, layouts responsive ni editor de diálogos.

---

## 9. Lista de comprobación final

Para dar por buena una prueba del estado actual:

- [ ] `ctest --test-dir build --output-on-failure` pasa 15/15.
- [ ] El runtime abre `runtime_test.pak` y presenta 1 frame con salida `0`.
- [ ] El runtime presenta 300 frames con salida `0`.
- [ ] El runtime responde a flechas/WASD, clic izquierdo y ESC.
- [ ] El runtime rechaza al menos un `.pak` corrupto con error descriptivo.
- [ ] El editor crea un proyecto nuevo e importa un PNG.
- [ ] El editor crea un nivel con capas `fondo` y `colision`.
- [ ] Se pintan tiles en ambas capas.
- [ ] Se crea una animación con al menos un fotograma y se previsualiza.
- [ ] Se guarda con `Ctrl+S` o con el botón Guardar.
- [ ] Se cierra y se reabre el proyecto sin pérdida de datos.
- [ ] El editor soporta resize, minimizado y cierre con salida `0`.
- [ ] Runtime y editor muestran 0 errores y 0 advertencias del engine en
      Validation Layer.
- [ ] No se usó `llvmpipe`.
- [ ] No se modificaron archivos protegidos sin aprobación.
