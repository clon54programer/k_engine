# Descripción del proyecto

## 1. Resumen

**Nombre del proyecto:**  
K_engine

**¿Qué estoy creando?**  
Un engine para mis proyectos personales que tengo 
en planeacion o pensando a desarrollar en un futuro.

**¿Qué problema, necesidad o meta resuelve?**  
Tener un Game engine que sea un balance entre control de bajo nivel 
con la facilidad de diseñar y probar niveles y animaciones.
Ademas, Quiero orientar el desarrollo en el lenguaje de C++, pero evitando
macros invasivas, pseudo-estandares de C++ para el engine y configuracion de
compilacion dificiles de crear y depurar.

**Usuario principal:**  
Yo, un usuario intermedio de C++ y estusiasta de la programacion
, con 3 años escribiendo a diario C++.

---

## 2. Objetivos del proyecto

### Objetivo a largo plazo
Quiero que permita realizar juegos comerciales o para mi, usando C++ como
lenguaje principal.

### Objetivo del primer MVP
Para el nucleo, Abrir una ventana con GLFW3, inicializar el pipeline básico de Vulkan para renderizar un sprite/textura 2D en pantalla desde un contenedor binario personalizado, y permitir cerrar la aplicación de forma limpia. Y, para el editor, Crear una herramienta GUI accesible  que permita colocar baldosas (tiles) en una cuadrícula 2D, definir una secuencia de fotogramas (spritesheet) para una animación básica, y exportar/guardar todo en el formato JSON que el motor C++ pueda cargar e interpretar. Desarrollar una CLI en Python que tome los archivos fuente del editor (niveles JSON y recursos multimedia raw como imágenes PNG y audios WAV), los valide, aplique compresión/estructuración básica y los empaquete en un único archivo binario contenedor (ej. .kbin / .pak) con una cabecera (header) e índice de recursos que el motor en C++ pueda leer de forma eficiente.

### Cómo sabré que el MVP está terminado

#### Nucleo
- [ ] Pila de validación limpia: La aplicación ejecuta, renderiza y se cierra sin arrojar advertencias ni errores en las Vulkan Validation Layers.

- [ ] Criterio de memoria y cierre: El engine debe liberar los recursos propiedad de la aplicación al cerrar. Se validará mediante pruebas repetidas de apertura, carga, renderizado y cierre, análisis de memoria y capas de validación de Vulkan en la configuración de desarrollo. Android deberá poder pausar y reanudar el renderizado cuando cambie la disponibilidad de la superficie.

- [ ] Renderizado funcional: Se muestra correctamente en pantalla la entidad, sprite o textura objetivo desde el contenedor binario del motor.

- [ ] Bucle principal funcional: El game loop corre a una tasa de fotogramas estable y responde al menos a una entrada básica del usuario (ej. mover un sprite o cerrar con la tecla ESC).

#### Editor
- [ ] Grid y Colocación: Se puede seleccionar una baldosa de un tileset y colocarla/borrarla en una cuadrícula (grid) de dimensiones configurables (ej. 16 x 16  o 32 x 32).

- [ ] Capas básicas: Permite manejar al menos 2 capas simples (ej. capa de fondo y capa de colisión/paredes).

- [ ] Creación de Clips: Se pueden seleccionar los fotogramas (frames) de un spritesheet, definir la velocidad de reproducción (ej. FPS o tiempo por cuadro) y reproducir una vista previa (preview) en bucle.

- [ ] Marcadores/Metadata: Permite asignar un nombre a la animación (ej. idle, walk_right) e identificar el origen del pivote/hitbox.

- [ ] Persistencia de datos: El editor permite guardar y cargar el estado del proyecto para continuar editando.

- [ ] Exportación para el Motor: Exporta el nivel y los datos de animación a un formato estructurado json que el motor C++ lee sin errores de parseo.

#### Empaquetador

- [ ] Contenedor de recursos: La versión 1 incluirá una cabecera versionada, un índice de recursos y los datos de cada recurso. El índice permitirá localizar y validar las entradas. La política de compresión y el formato de los niveles se definirán explícitamente; el hecho de usar un contenedor binario no implica que todos sus contenidos deban convertirse a binario.

- [ ] Conversión de Niveles: Convierte la estructura lógica del editor (JSON) a un formato binario compacto (evitando procesar cadenas de texto en C++ en tiempo de ejecución).

- [ ] Inclusión Multimedia: Empaqueta imágenes (PNG/JPG) y otros assets (audio, fuentes, etc.) sin corromper sus bytes ni modificar su alineación necesaria para la carga.

- [ ] Ruta de entrada y salida: Acepta argumentos por línea de comandos para especificar el directorio de assets/proyecto del editor y la ruta del archivo .pak resultante.

- [ ] Validación de integridad: Si falta un recurso referenciado en el nivel o un archivo está corrupto, la herramienta lanza un error descriptivo en la terminal antes de empaquetar.

---

## 3. Alcance del MVP

### Debe incluir
- [ ] Ejecutarse en Linux  mint xfce y x86_64.
- [ ] Instalarse y ejecutarse en un dispositivo Android 15 (API 35) o superior
      con arquitectura arm64-v8a y soporte Vulkan requerido.
- [ ] Renderizar un sprite cargado desde un contenedor creado por el empaquetador.
- [ ] Crear, guardar y cargar un nivel 2D básico desde el editor.
- [ ] Crear y previsualizar una animación básica a partir de una spritesheet.
- [ ] Exportar los datos y recursos del editor y cargarlos correctamente en el runtime.
- [ ] Completar una prueba de extremo a extremo con un nivel pequeño.
- [ ] Proporcionar una versión ejecutable para Linux y un paquete instalable
      para Android, junto con instrucciones básicas de prueba.
- [ ] Multijugador local

### No incluirá en esta primera versión
- [ ] Renderizado 3D ni herramientas de edición 3D.
- [ ] Compilación o distribución para Windows y WebAssembly.
- [ ] Multijugador  en línea.
- [ ] Compatibilidad garantizada con distribuciones o dispositivos no probados.

### Motivo de estas prioridades
El MVP prioriza las plataformas que puedo probar directamente: Linux y mi
dispositivo Android. Windows y WebAssembly quedan para más adelante; este último
también requiere resolver una estrategia gráfica distinta a Vulkan. El alcance
se limita a las funciones 2D necesarias para validar el flujo completo del engine.

---

## 4. Plataformas objetivo

### Linux
- **Distribución objetivo para el MVP:** Linux Mint 22.2 Xfce (Zara).
- **Arquitectura:** x86_64
- **Forma de distribución para el MVP:** Ejecutable nativo para probar en el sistema objetivo; no se requiere instalador.
- **Versión mínima del sistema:** Linux Mint 22.2.
- **Dispositivos de prueba disponibles:** El equipo de desarrollo con Linux Mint 22.2 Xfce.
- **Métodos de entrada:** Teclado y ratón.

### Android
- **Versión mínima:** Android 15 (API 35).
- **SDK objetivo para Google Play:** `targetSdkVersion` 36 o superior para nuevas apps y actualizaciones, según la política vigente.
- **SDK de compilación:** `compileSdkVersion` 36 o superior.
- **Arquitectura objetivo:** arm64-v8a.
- **Dispositivo de prueba:** Moto G34 5G con Android 16 y arquitectura arm64-v8a; 
- **Forma de instalación para el MVP:** APK instalable manualmente en el dispositivo de prueba.
- **Orientación de pantalla:** Horizontal, recomendada para el juego de plataformas;
- **Métodos de entrada:** Pantalla táctil.
- **Requisito gráfico en Android:** Vulkan 1.1 o superior. Verificar las capacidades expuestas por el driver en el Moto G34 5G.


### Plataformas futuras
Windows y WebAssembly quedan fuera del MVP. Se podrán considerar después de validar el engine en Linux Mint y Android. También se podrá ampliar la compatibilidad a otras distribuciones de Linux y dispositivos Android. Ademas, evaluará ampliar la compatibilidad de Android hasta Android 10 (API 29), manteniendo Vulkan 1.1 como requisito gráfico. Este cambio solo se considerará después de probar el engine en dispositivos reales con distintas GPU y drivers; si no se pueden realizar esas pruebas, no se declarará compatibilidad con Android 10
---

## 5. El juego que utilizará el engine

**Género:**  
plataformas de acción 2d y hack and slash

**Descripción breve:**  
El jugador elige 3 personajes y cambia de personaje durante el nivel. Cada personaje
puede usar hechizos comunes, hechizos especificos por naturaleza o faccion, hechizos personales,
armas de fuego y armas blancas, antes de cada nivel, puede elegir 3 tipos de armas y 12 hechizos de cualquier tipo o clasificacion, pero en un nivel, solo puede elegir dos tipos de amras y 4 hechizos, el resto se va al inventario, que puedem reemplazar a un hechizo o arma durante el nivel, al menos que el nivel exiga un arma o hechizo para su realizacion. En cada nivel el jugador va por un objetivo (robar o alcanzar un objecto, o eliminar a alguien), normalmente se lucha de forma lineal (con profundidad, es decir, puede haber carriles en el eje de profundidad) hasta llegar a ese objetivo, y en niveles muy especificos son sigilos, en donde
ser descubierto falla el nivel de forma automatica o pasado x segundos del ser decubierto.
La narrativa sera importante de forma indirecta al gameplay, ya que segun las interaciones o decisiones que tengas antes de cada nivel, podras tener mas o menos recursos, o bajar o aumentar la dificultad al interactuar con el que no debias, o haber tratado mal en una ocasion anterior.

**Características centrales del juego:**
- [ ]
- [ ]
- [ ]

**Modo de juego:**  
Individual offline y multijugador local 1 vs 1 o 3 vs 3 en una red local.

**¿Qué parte del juego se desarrollará para validar el MVP del engine?**  
[Por ejemplo: una pantalla, un nivel corto o una sección jugable.]

---

## 6. Requisitos funcionales del engine

Anota qué debe poder hacer el engine. Marca cada punto como **necesario**, **deseable** o **futuro**.

| Capacidad | Prioridad | Descripción o ejemplo |
|---|---|---|
| Renderizado 2D | [ ] | [ ] |
| Ventanas y pantalla completa | [ ] | [ ] |
| Entrada | [ ] | [Teclado, táctil, mando…] |
| Audio | [ ] | [ ] |
| Física y colisiones | [ ] | [ ] |
| Animaciones | [ ] | [ ] |
| Interfaz de usuario | [ ] | [ ] |
| Carga de recursos | [ ] | [ ] |
| Escenas o niveles | [ ] | [ ] |
| Guardado de datos | [ ] | [ ] |
| Red | [ ] | [Si aplica al MVP] |
| Otras | [ ] | [ ] |

---

## 7. Editor y herramientas

**¿El MVP necesita un editor visual?**  
[No / Sí / Solo herramientas básicas]

**¿Qué se podrá crear o modificar con el editor?**
- [ ] Niveles
- [ ] Interfaz
- [ ] Animaciones
- [ ] Diálogos
- [ ] Otros: [ ]

**Si el editor no está en el MVP, ¿cómo se crearán esos datos inicialmente?**  
[Archivos escritos a mano, herramientas existentes, código u otra opción.]

**Herramientas adicionales necesarias:**  
[Empaquetador, conversor de recursos, depurador, etc.]

---

## 8. Datos y recursos

**Tipos de recursos necesarios:**  
[Imágenes, audio, fuentes, mapas, diálogos, etc.]

**Formato editable durante el desarrollo:**  
[JSON, archivos propios u otro. Por decidir si no se sabe.]

**Formato utilizado al ejecutar el juego:**  
[Igual al editable / formato convertido / por decidir.]

**¿Se necesita agrupar o comprimir recursos?**  
[Qué necesidad concreta debe resolver.]

**¿Se necesita cifrar recursos?**  
[Qué información se intenta proteger y por qué.]

**Identificación de recursos:**  
[Nombre, ID único, hash u otro. Por decidir.]

---

## 9. Decisiones técnicas

Completa cada decisión con su motivo, alternativas y nivel de certeza. No hace falta cerrar ahora las que todavía no estén investigadas.

| Tema | Decisión actual | Motivo | Alternativas consideradas | Estado |
|---|---|---|---|---|
| Lenguaje y estándar | [C++17 u otro] | [ ] | [ ] | [Decidido / Investigar / Abierto] |
| API gráfica en Linux | [ ] | [ ] | [ ] | [ ] |
| API gráfica en Android 15+ | [ ] | [ ] | [ ] | [ ] |
| Librería de ventanas/entrada | [ ] | [ ] | [ ] | [ ] |
| Librería de audio | [ ] | [ ] | [ ] | [ ] |
| Física | [ ] | [ ] | [ ] | [ ] |
| Formato de datos | [ ] | [ ] | [ ] | [ ] |
| Sistema de compilación | [ ] | [ ] | [ ] | [ ] |
| Dependencias externas | [ ] | [ ] | [ ] | [ ] |

---

## 10. Requisitos de calidad

Indica qué importa para el MVP y cómo se comprobará.

| Requisito | Prioridad | Cómo comprobarlo |
|---|---|---|
| Rendimiento | [ ] | [ ] |
| Tiempo de carga | [ ] | [ ] |
| Estabilidad | [ ] | [ ] |
| Tamaño de la aplicación | [ ] | [ ] |
| Facilidad para añadir contenido | [ ] | [ ] |
| Compatibilidad entre Linux y Android | [ ] | [ ] |
| Facilidad de depuración | [ ] | [ ] |

---

## 11. Pruebas y validación

**Dispositivos y sistemas disponibles para probar:**  
[ ]

**Qué debe probarse en Linux:**  
[ ]

**Qué debe probarse en Android 15+:**  
[ ]

**Pruebas manuales necesarias:**  
[ ]

**Pruebas automatizadas necesarias:**  
[ ]

**Problemas conocidos o riesgos:**
- [ ]
- [ ]

---

## 12. Plan de desarrollo

| Etapa | Resultado esperado | Criterio para darla por terminada |
|---|---|---|
| 1. Investigación | [ ] | [ ] |
| 2. Prototipo técnico | [ ] | [ ] |
| 3. Funciones esenciales del engine | [ ] | [ ] |
| 4. Prueba con contenido del juego | [ ] | [ ] |
| 5. Pruebas en Linux y Android | [ ] | [ ] |
| 6. MVP | [ ] | [ ] |

**Horas disponibles por semana:**  
[ ]

**Fecha objetivo:**  
[Incluye día, mes y año.]

**Qué significa “terminado” en esa fecha:**  
[ ]

---

## 13. Suposiciones y preguntas abiertas

### Suposiciones actuales
- [ ]
- [ ]

### Preguntas que necesito resolver
- [ ]
- [ ]

### Decisiones que no necesito tomar todavía
- [ ]
- [ ]

---

## 14. Contexto del desarrollador

**Experiencia relevante:**  
[ ]

**Herramientas y sistema de desarrollo:**  
[ ]

**Limitaciones importantes de tiempo, equipo o conocimientos:**  
[ ]

**Preferencias de colaboración o revisión:**  
[ ]
