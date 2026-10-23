# Descripción del proyecto

## 1. Resumen

**Nombre del proyecto:**  
K_engine

**¿Qué estoy creando?**  
Un engine para mis proyectos personales que tengo 
en planeación o que pienso desarrollar en el futuro. El runtime se ejecutará en todas las plataformas objetivo; el editor y el empaquetador serán herramientas de escritorio.

**¿Qué problema, necesidad o meta resuelve?**  
Tener un Game engine que sea un balance entre control de bajo nivel 
con la facilidad de diseñar y probar niveles y animaciones.
Además, quiero orientar el desarrollo en el lenguaje C++, pero evitando
macros invasivas, pseudoestándares de C++ para el engine y configuraciones de
compilación difíciles de crear y depurar.

**Usuario principal:**  
Yo, un usuario intermedio de C++ y entusiasta de la programación,
con 3 años escribiendo a diario C++.

**Terminología:**  
En este documento, «núcleo» y «runtime» se usan como sinónimos, y «motor» y «engine» se usan como sinónimos.

---

## 2. Objetivos del proyecto

### Objetivo a largo plazo
Quiero que permita realizar juegos comerciales o para mi, usando C++ como
lenguaje principal.

### Objetivo del primer MVP
Completar en Linux Mint x86_64 el flujo de trabajo del engine: crear y ejecutar un runtime Vulkan que lea un nivel y sus recursos desde un `.pak`; crear un editor de escritorio para niveles en cuadrícula y animaciones por fotogramas, con datos editables en JSON; y crear una CLI de escritorio que valide los datos, convierta niveles y animaciones JSON a FlatBuffers y genere el `.pak` de distribución. La prueba del MVP será una escena técnica pequeña que permita verificar el flujo completo, no una vertical slice del juego final. Android será la siguiente etapa de plataforma, después de cerrar este MVP Linux.

### Cómo sabré que el MVP está terminado

#### Núcleo
- [ ] Pila de validación limpia: La aplicación ejecuta, renderiza y se cierra sin errores ni advertencias atribuibles al engine en las Vulkan Validation Layers.

- [ ] Criterio de memoria y cierre: El engine debe liberar los recursos propiedad de la aplicación al cerrar. Se validará mediante aperturas y cierres repetidos, análisis de memoria y capas de validación de Vulkan en Linux. La pausa, reanudación y recreación de superficies Android se validarán en la etapa de esa plataforma.

- [ ] Renderizado funcional: Se muestra correctamente en pantalla la entidad, sprite o textura objetivo desde el contenedor binario del motor.

- [ ] Bucle principal funcional: El game loop corre a una tasa de fotogramas estable y responde al menos a una entrada básica del usuario (ej. mover un sprite o cerrar con la tecla ESC).

#### Editor
- [ ] Grid y Colocación: Se puede seleccionar una baldosa de un tileset y colocarla/borrarla en una cuadrícula (grid) de dimensiones configurables (ej. tiles de 16 × 16 o 32 × 32 píxeles).

- [ ] Capas básicas: Permite manejar al menos 2 capas simples (ej. capa de fondo y capa de colisión/paredes; esta última puede ser solo de datos, sin dibujarse).

- [ ] Creación de Clips: Se pueden seleccionar los fotogramas (frames) de un spritesheet, definir la velocidad de reproducción (ej. FPS o tiempo por cuadro) y reproducir una vista previa (preview) en bucle.

- [ ] Marcadores/Metadata: Permite asignar un nombre a la animación (ej. idle, walk_right) e identificar el origen del pivote y la caja de colisión (hitbox).

- [ ] Persistencia de datos: El editor permite guardar y cargar el estado del proyecto para continuar editando.

- [ ] Datos de desarrollo y distribución: El editor guarda los niveles y las animaciones en JSON durante el desarrollo. Para distribuir un juego, el empaquetador valida esos datos, los convierte a FlatBuffers y los incluye en el `.pak` que lee el runtime C++.

#### Empaquetador

- [ ] Contenedor de recursos: La versión 1 incluirá una cabecera versionada, un índice de recursos y los datos de cada recurso. El índice permitirá localizar y validar las entradas. La política de compresión y el formato de los niveles se definirán explícitamente; el hecho de usar un contenedor binario no implica que todos sus contenidos deban convertirse a binario.

- [ ] Conversión para distribución: Al preparar un juego para distribución, convierte la estructura lógica del editor (JSON) a FlatBuffers, evitando procesar cadenas de texto en C++ en tiempo de ejecución.

- [ ] Inclusión Multimedia: Empaqueta imágenes (PNG/JPG) y otros assets (audio, fuentes, etc.) sin corromper sus bytes ni modificar su alineación necesaria para la carga.

- [ ] Ruta de entrada y salida: Acepta argumentos por línea de comandos para especificar el directorio de assets/proyecto del editor y la ruta del archivo .pak resultante.

- [ ] Validación de integridad: Si falta un recurso referenciado en el nivel o un archivo está corrupto, la herramienta lanza un error descriptivo en la terminal antes de empaquetar.

---

## 3. Alcance del MVP

### Debe incluir
- [ ] Ejecutar el runtime, el editor y el empaquetador en Linux Mint 22.2 x86_64. El editor y el empaquetador son herramientas de escritorio.
- [ ] Renderizar un sprite cargado desde un contenedor creado por el empaquetador.
- [ ] Crear, guardar y cargar un nivel 2D básico desde el editor.
- [ ] Crear y previsualizar una animación básica a partir de una spritesheet.
- [ ] Guardar los datos del editor en JSON durante el desarrollo; al preparar la distribución, convertirlos a FlatBuffers y empaquetarlos con los recursos para cargarlos correctamente en el runtime.
- [ ] Completar una prueba de extremo a extremo: crear contenido en el editor, guardarlo, empaquetarlo y cargarlo en el runtime Linux.
- [ ] Proporcionar versiones ejecutables del runtime y de las herramientas para Linux, junto con instrucciones básicas de prueba.

### No incluirá en esta primera versión
- [ ] Runtime Android, editor o empaquetador Android; Android se abordará después del MVP Linux.
- [ ] Física de juego, combate, raycasting y una vertical slice del juego. Se podrán añadir después si el flujo técnico del engine ya está validado.
- [ ] Audio como requisito de entrega del MVP.
- [ ] Implementación de red o prototipo de comunicación entre instancias. La arquitectura se mantendrá desacoplada de la entrada física, sin exigir una solución de red en esta etapa.
- [ ] Cifrado del índice del `.pak`.
- [ ] Renderizado 3D ni herramientas de edición 3D.
- [ ] Compilación o distribución para Windows y WebAssembly.
- [ ] Compatibilidad garantizada con distribuciones o dispositivos no probados.

### Motivo de estas prioridades
El MVP prioriza completar y validar un flujo vertical en el entorno Linux disponible antes de añadir un segundo backend de plataforma. Android será la siguiente etapa; Windows, WebAssembly y otras plataformas se considerarán después. El MVP se limita a las herramientas y funciones 2D necesarias para crear, empaquetar y ejecutar contenido.

---

## 4. Plataformas objetivo

### Linux
- **Distribución objetivo para el MVP:** Linux Mint 22.2 Xfce (Zara).
- **Arquitectura:** x86_64
- **Forma de distribución para el MVP:** Runtime y herramientas de escritorio ejecutables de forma nativa para probar en el sistema objetivo; no se requiere instalador.
- **Versión mínima del sistema:** Linux Mint 22.2.
- **Dispositivos de prueba disponibles:** El equipo de desarrollo con Linux Mint 22.2 Xfce.
- **Métodos de entrada:** Teclado y ratón.

### Android
- **Etapa prevista:** Posterior al MVP Linux.
- **Versión mínima:** Android 16 (API 36).
- **SDK objetivo para Google Play:** `targetSdkVersion` 36 o superior para nuevas apps y actualizaciones, según la política vigente.
- **SDK de compilación:** `compileSdkVersion` 36 o superior.
- **Arquitectura objetivo:** arm64-v8a.
- **Dispositivo de prueba:** Moto G34 5G con Android 16 y arquitectura arm64-v8a.
- **Forma de instalación prevista:** APK instalable manualmente en el dispositivo de prueba; contiene el runtime, no el editor ni el empaquetador.
- **Orientación de pantalla:** Horizontal, recomendada para el juego de plataformas.
- **Métodos de entrada:** Pantalla táctil.
- **Requisito gráfico en Android:** Vulkan 1.1 o superior. Verificar las capacidades expuestas por el driver en el Moto G34 5G.


### Plataformas futuras
Después del MVP Linux, se trabajará en el runtime Android 16 (API 36) o superior y se validará en el Moto G34 5G. Tras Android se podrán considerar otras plataformas, como Windows y WebAssembly, además de ampliar la compatibilidad a otras distribuciones de Linux y dispositivos Android. Reducir más adelante el mínimo Android hasta Android 10 (API 29) solo se evaluará después de probar dispositivos reales con distintas GPU y drivers; si esas pruebas no se realizan, no se declarará esa compatibilidad.
---

## 5. El juego que utilizará el engine

**Género:**  
plataformas de acción 2D y hack and slash

**Descripción breve:**  
Un juego de plataformas de acción 2D con combate hack and slash. El jugador elige 3 personajes y cambia de personaje durante el nivel. Cada personaje
puede usar hechizos comunes, hechizos específicos por naturaleza o facción, hechizos personales,
armas de fuego y armas blancas. Antes de cada nivel, el jugador prepara hasta 3 tipos de armas y 12 hechizos de cualquier tipo o clasificación; durante el nivel solo puede equipar dos tipos de armas y 4 hechizos. El resto va al inventario, un menú que no pausa el nivel y que permite reemplazar un hechizo o arma durante el nivel, a menos que el nivel exija un arma o hechizo para su realización. 

En cada nivel el jugador va por un objetivo (robar o alcanzar un objeto, o eliminar a alguien), normalmente se lucha de forma lineal (con profundidad, es decir, puede haber carriles en el eje de profundidad) hasta llegar a ese objetivo, y en niveles muy específicos son de sigilo, en donde
si un NPC de baja rango (dependiendo de la facción) te visualiza y logra alertar al resto falla, pero si es de alto rango, falla de forma automática. No se puede especificar un tiempo, ya que dependerá del entorno (abierto o cerrado) y de que la forma de comunicación sea a voz o con un sistema de comunicaciones.

La narrativa será importante de forma indirecta al gameplay, ya que según las interacciones o decisiones que tengas antes de cada nivel, podrás tener más o menos recursos, o bajar o aumentar la dificultad al interactuar con quien no debías, o haber tratado mal en una ocasión anterior.

**Características centrales del juego:**
- El jugador controla un equipo de tres personajes y puede cambiar entre ellos durante el nivel. Los personajes pueden usar hechizos comunes, hechizos asociados a su naturaleza o facción, y hechizos personales.
- Antes de cada nivel, el jugador prepara hasta tres tipos de armas y doce hechizos. Durante el nivel puede seleccionar hasta dos tipos de armas y cuatro hechizos; el resto queda en el inventario y puede sustituirse desde un menú sin pausar el juego.
- Los niveles combinan plataformas de acción 2D y combate hack and slash. Algunos incluyen carriles de profundidad que afectan el movimiento y los combates, pero que no están conectados de forma continua por todo el nivel.
- Algunos niveles incluyen sigilo. Si un NPC de rango bajo detecta al jugador y consigue alertar a los demás, el nivel falla. Si lo detecta un NPC de rango alto, el nivel falla automáticamente. El tiempo para transmitir una alerta depende del entorno y del medio de comunicación, como la voz o un sistema de comunicaciones.
- Las decisiones e interacciones narrativas anteriores a un nivel pueden afectar los recursos disponibles o la dificultad de niveles posteriores.

**Definición de «carril» (carril de profundidad):**  
En este juego, un carril es un plano de profundidad discreto dentro de la vista lateral 2D (típicamente 2 o 3: fondo, medio y primer plano). Cada personaje o enemigo ocupa un carril: se mueve lateralmente y salta dentro de él, y cambia de carril solo en puntos de conexión definidos. El carril condiciona el combate, las colisiones y el orden de dibujado (los del carril de fondo se dibujan detrás). La lógica discreta (índice de carril) y la transición visual entre carriles se diseñarán al implementar la prueba de juego; no forman parte del MVP del engine.

**Modo de juego:**  
Individual offline y multijugador competitivo mediante una red local, con modos 1 vs. 1 y 3 vs. 3. En el multijugador cada jugador elige un personaje disponible del modo offline.

**¿Qué parte del juego se desarrollará para validar el MVP del engine?**   
No se desarrollará una vertical slice del juego como criterio del MVP del engine. La prueba técnica contendrá un nivel pequeño con una cuadrícula, una capa adicional, un sprite o una animación básica y los recursos necesarios para validar la edición, el guardado JSON, la conversión a FlatBuffers, el empaquetado y la carga en el runtime.

La física de juego, el combate, los carriles de profundidad, los personajes intercambiables, el inventario, el sigilo y el multijugador quedarán fuera de esta prueba inicial.

---

## 6. Requisitos funcionales del engine


| Capacidad | Prioridad | Descripción o ejemplo |
|---|---|---|
| Renderizado 2D | Necesario para el MVP Linux | Renderizar sprites y texturas con Vulkan desde los recursos empaquetados. Ordenar sprites por capas de dibujo del nivel. El backend Android y sus requisitos de Vulkan se implementarán en la etapa Android. |
| Ventanas y superficies | Necesario para el MVP Linux | Crear una ventana Linux, gestionar el redimensionamiento y cerrar limpiamente. La superficie nativa y el ciclo de pausa/reanudación Android se implementarán en la etapa Android. |
| Entrada | Necesario para el MVP Linux | Admitir teclado y ratón y convertirlos en acciones del runtime. Mantener la lógica del juego separada de los eventos físicos; la pantalla táctil se añadirá en Android y el mando en una etapa posterior. |
| Física y colisiones | Fuera del MVP del engine; necesaria para una futura prueba de combate | La primera prueba técnica no requiere física. Si una vertical slice posterior incluye movimiento y ataques, añadir colisiones y detección de impactos; evaluar Box2D detrás de una interfaz del engine. Raycasting, visión y sigilo quedan para etapas posteriores. |
| Animaciones | Necesario: animación por fotogramas en el MVP; Deseable: animación esquelética en una fase posterior | El MVP permite definir secuencias desde spritesheets y previsualizarlas. En una fase posterior, añadir un sistema 2D de huesos jerárquicos, slots asociados a huesos y attachments —por ejemplo, sprites que se puedan cambiar—, con clips que animen transformaciones y cambios de attachment. |
| Interfaz de usuario | Necesaria para el editor Linux | Implementar la API Builder propia sin macros y solo los componentes que necesita el editor: paneles, botones, paleta, cuadrícula y controles de animación. Adaptar el editor a tamaños de ventana de escritorio; el diseño responsive para dispositivos Android y las interfaces del juego se abordarán después. |
| Carga de recursos | Necesario | Leer y validar recursos bajo demanda desde un contenedor versionado, incluyendo imágenes y datos de niveles, sin cargar el `.pak` completo en memoria. Informar errores descriptivos si faltan recursos o el contenedor no es compatible. |
| Carga de niveles | Necesario | Cargar un nivel 2D con tiles, capas y datos básicos de objetos. Los carriles de profundidad y sus conexiones se añadirán para una futura prueba del juego. |
| Guardado de datos | Necesario | Guardar y cargar los proyectos editables del editor. El guardado de progreso, decisiones narrativas y recursos de campaña queda para una fase posterior. |
| Red | Fuera del MVP | Mantener las acciones del juego separadas de la entrada física cuando se implemente gameplay. El paso de simulación, identificadores estables, prototipos de intercambio y partidas LAN se definirán en una fase posterior; no se exige determinismo ni comunicación entre instancias para el MVP. |
| Audio | Fuera del MVP; deseable para una futura prueba de juego | El `.pak` podrá incluir recursos de audio, pero el runtime del MVP no necesita reproducirlos. En una fase posterior se podrá empezar por reproducir un efecto WAV con miniaudio; música, mezcla avanzada y ciclo de pausa Android quedan para después. |
| Editor de niveles y animaciones | Necesario; herramienta de escritorio | Permitir crear un nivel en cuadrícula, seleccionar y colocar tiles, editar capas y crear una animación básica. Guardar el proyecto y sus datos de desarrollo en JSON. |
| Empaquetador | Necesario; herramienta de escritorio | Incluir una CLI en Python que valide los recursos y niveles, detecte referencias ausentes, convierta los JSON de desarrollo a FlatBuffers para distribución y genere un contenedor que el runtime C++ pueda leer. |
| Otras | Deseable | Facilitar mensajes de diagnóstico útiles para errores de carga, inicialización de Vulkan y recursos incompatibles. |

---

## 7. Editor y herramientas

**¿El MVP necesita un editor visual?**  
Sí. Será una herramienta de escritorio (en el MVP, para Linux) para crear y guardar niveles y animaciones básicas en JSON durante el desarrollo. No se instalará ni ejecutará en Android; el runtime se portará a Android después de cerrar el MVP Linux.

**¿Qué se podrá crear o modificar con el editor?**
- [x] Niveles: tilemaps en cuadrícula y capas básicas. Los carriles y sus conexiones quedan fuera del MVP del engine.
- [x] Interfaz de usuario (del editor): el editor de escritorio construye su propia interfaz (paneles, botones, paleta de tiles, cuadrícula y controles de animación) con la API Builder del engine, lo que valida el sistema de UI. La lógica y los eventos del juego se implementarán en C++.
- [ ] Interfaz de usuario (del juego) y generador de C++: diseñar visualmente layouts responsive para el juego (por ejemplo, el inventario que no pausa el nivel) y generarlos como código C++ mediante la API Builder quedará para una fase posterior.
- [x] Animaciones: clips básicos creados a partir de fotogramas de una spritesheet.
- [ ] Diálogos: no habrá un editor visual de diálogos en el MVP. Si se necesitan para una prueba, su contenido se escribirá manualmente en archivos JSON.
- [x] Otros: explorador básico para importar, organizar y previsualizar imágenes y spritesheets, y detectar recursos faltantes o referencias rotas. La previsualización de audio queda para después.


**Herramientas adicionales necesarias:**  
- CLI en Python de escritorio para validar niveles y recursos, convertir los JSON de desarrollo a FlatBuffers para distribución y generar el `.pak`.
- Validación de referencias a recursos y errores descriptivos antes de generar código o paquetes.
- (Post-MVP) Generador de código C++ para convertir los layouts de interfaz del juego en código que use la API Builder.
---

## 8. Datos y recursos

**Tipos de recursos necesarios:**
- Imágenes y spritesheets, principalmente PNG.
- Efectos de sonido y música, inicialmente WAV.
- Fuentes y shaders compilados para Vulkan.
- Niveles y datos de animación creados visualmente en el editor.
- Layouts de interfaz creados visualmente en el editor y convertidos a código C++ (post-MVP).
- Diálogos y lógica del juego escritos directamente en C++.

**Formato editable durante el desarrollo:**
- Los niveles y las animaciones se guardarán en archivos JSON editables, con una versión de esquema definida. Estos son los formatos de trabajo durante el desarrollo.
- Los archivos multimedia se conservarán en su formato original dentro del proyecto.
- (Post-MVP) El editor será la fuente editable de los layouts de interfaz del juego. El generador de código los convertirá en C++ que use la API Builder del engine.
- Los diálogos y la lógica del juego se mantendrán en archivos C++.

**Formato utilizado para distribuir y ejecutar el juego:**
- Al preparar una distribución, el empaquetador validará los JSON de niveles y animaciones y los convertirá a buffers binarios FlatBuffers usando los esquemas `.fbs`.
- Los esquemas FlatBuffers (`.fbs`) definirán el formato binario de distribución de los niveles y las animaciones.
- El runtime C++ utilizará el código de acceso generado por `flatc` y verificará cada buffer FlatBuffers antes de leerlo.
- Los buffers de niveles y animaciones, junto con los recursos multimedia, se incluirán como entradas de un archivo `.pak` versionado.
- (Post-MVP) El código C++ generado a partir de los layouts de interfaz del juego se compilará con el juego; no se incluirá como layout dentro del `.pak`.

**¿Se necesita agrupar o comprimir recursos?**
- Sí. Para distribuir el juego, el empaquetador agrupará los recursos y los buffers FlatBuffers generados desde los JSON de desarrollo en un archivo `.pak`.
- La versión 1 no aplicará compresión adicional. Los recursos conservarán sus bytes originales; los niveles y las animaciones se almacenarán como buffers FlatBuffers.
- La estructura será `[cabecera][tabla de contenidos][datos]`. La cabecera tendrá 16 bytes: firma mágica `KPAK` (4 bytes ASCII), versión mayor y menor (u16 cada una), cantidad de archivos (u32) y CRC-32/ISO-HDLC de la tabla (u32). La tabla empieza inmediatamente después de la cabecera.
- Cada entrada de la tabla tendrá 40 bytes: ID estable (u64), tipo (u32 con valores enumerados por el formato), flags/reservado (u32, cero en v1), offset absoluto en bytes (u64), tamaño (u64), CRC-32/ISO-HDLC del recurso (u32) y reservado (u32, cero en v1). Los enteros se serializan en little-endian. Las entradas se ordenan por ID; el área de datos sigue a la tabla. El CRC de la tabla cubre únicamente los bytes de sus entradas.
- Los datos de cada recurso comienzan en un offset múltiplo de 16 bytes. El empaquetador insertará padding de bytes cero para cumplirlo; el tamaño registrado corresponde solo a los bytes del recurso, no al padding. Los offsets son absolutos desde el inicio del archivo y el índice es legible, no cifrado.
- El runtime abrirá el `.pak` y leerá los recursos bajo demanda usando los offsets del índice; no cargará el paquete completo en memoria. La lectura y comprobación CRC-32 de los datos puede hacerse por bloques. El índice se cargará o procesará por separado y tendrá un límite de memoria propio.
- El runtime leerá los campos como bytes o mediante `memcpy` y conversión explícita de endianness; no convertirá offsets arbitrarios del archivo directamente en punteros a estructuras o enteros. La alineación de 16 bytes del paquete no sustituye los requisitos específicos de Vulkan: el runtime alineará por separado los offsets del buffer de staging según el dispositivo.
- El empaquetador validará referencias, offsets, tamaños y sumas de comprobación antes de generar el paquete; escribirá a un archivo temporal y solo publicará el `.pak` final si termina correctamente.
- Se rechazarán IDs duplicados, referencias ausentes, versiones no compatibles, flags/reservados distintos de cero, offsets no alineados, rangos solapados o fuera de los límites del archivo, CRC incorrectos y paquetes o datos truncados. Antes de cargar o recorrer el índice, el lector calculará su tamaño con aritmética comprobada y validará que quepa en el archivo y dentro del límite de memoria del índice.
- No se fijará un límite global de tamaño del `.pak` por memoria: el runtime no carga todos sus datos a la vez. El tamaño total seguirá limitado por el sistema de archivos y las APIs de acceso; sí se definirán límites prácticos para el índice y para cada tipo de recurso.

**¿Se necesita cifrar recursos?**
No en el MVP. El cifrado AES-GCM del índice se aplaza: la clave incorporada al juego podría extraerse y los recursos permanecerían legibles, por lo que el beneficio no justifica el coste inicial. CRC-32/ISO-HDLC se utilizará únicamente para detectar corrupción accidental, no como protección criptográfica. El cifrado se reconsiderará después si existe una necesidad concreta.

**Identificación de recursos:**
- Cada recurso tendrá un ID estable asignado y guardado en los datos del proyecto.
- La ruta relativa del archivo servirá para localizarlo durante el desarrollo.
- El índice no cifrado asociará cada ID con el tipo, offset, tamaño y suma de comprobación de la entrada correspondiente.
- Los hashes de contenido no se usarán como identificadores principales, porque cambian cuando cambia el archivo. Una suma de comprobación podrá usarse aparte para detectar corrupción accidental.
---

## 9. Decisiones técnicas



| Tema | Decisión actual | Motivo | Alternativas consideradas | Estado |
|---|---|---|---|---|
| Lenguaje y estándar | C++17 para el núcleo (runtime) y el editor | Es el estándar elegido para compartir código entre Linux y Android | Otros estándares de C++ | Decidido |
| GPU Linux de referencia | AMD Radeon Graphics (Renoir), dispositivo `1002:15e7`, controlador RADV de Mesa 25.2.8, con Linux Mint 22.2 Xfce; pantalla de 1366 × 768 a 60 Hz bajo X11 | Define el hardware y el controlador de referencia para probar el runtime Linux | Otras GPU y entornos de pantalla | Identificados con `vulkaninfo --summary`; falta probar presentación en X11 |
| API gráfica en Linux | Vulkan 1.3 como versión objetivo inicial de la aplicación. `vulkaninfo` informa versión de instancia 1.3.275 y API 1.4.318 para la GPU AMD. | La GPU soporta una versión superior a la necesaria; se elige 1.3 para alinearse con la versión informada por el loader y evitar depender de funciones 1.4 | Vulkan 1.4; OpenGL; otro backend | Versión y controlador identificados; falta comprobar superficie X11 y swapchain |
| API gráfica en Android | Vulkan 1.1 o superior en arm64-v8a; consultar las capacidades del dispositivo en tiempo de ejecución | El Moto G34 5G con Snapdragon 695/Adreno 619 admite Vulkan 1.1 | Vulkan 1.0; otro backend | Decisión para la etapa Android; aún no implementada |
| Android SDK | `minSdkVersion` 36, `targetSdkVersion` 36 y `compileSdkVersion` 36 o superior | Define el objetivo de compatibilidad de la etapa Android | Revisar cuando cambien los requisitos de Google Play | Pendiente de implementar después del MVP Linux |
| Ventanas y entrada | Runtime Linux con GLFW3 y una API del engine que mantenga la lógica separada de la biblioteca de ventanas y de los eventos físicos. El backend Android nativo se añadirá en su etapa. | Reduce dependencias de plataforma en el juego sin anticipar la implementación de todos los backends | SDL3 como backend común; otros backends por plataforma | Linux para el MVP; Android posterior |
| Audio | Audio fuera del MVP; evaluar miniaudio para reproducir efectos WAV en una futura prueba de juego | Evita ampliar el alcance del primer flujo técnico | OpenAL; audio de SDL3 | Aplazado |
| Física y colisiones | Física fuera del MVP del engine; evaluar Box2D detrás de una interfaz propia si una futura prueba de juego requiere movimiento y ataques | No se requiere física para validar el flujo editor-empaquetador-runtime | Física propia acotada; otra biblioteca 2D | Aplazado; raycasting también posterior |
| Datos de niveles y animaciones | JSON editable durante el desarrollo; al preparar una distribución, conversión a buffers FlatBuffers mediante esquemas `.fbs` | Evita parsear JSON durante la ejecución distribuida y permite definir un formato binario versionado | Parsear JSON en el runtime; formato binario propio | Decidido |
| Interfaz de usuario | API Builder propia (sin macros) con los componentes mínimos para el editor de escritorio; la adaptación responsive a dispositivos y las interfaces de juego quedan para fases posteriores | Permite validar la API Builder en Linux sin implementar aún un sistema de UI multiplataforma completo | Layouts escritos a mano en C++; sistema inspirado en Clay sin generación de código | Builder mínimo para el MVP; UI responsive de juego posterior |
| Diálogos y lógica del juego | Se implementan directamente en C++ | No se editarán visualmente en el MVP | Editor visual de diálogos; archivos de datos interpretados en runtime | Decidido para el MVP |
| Contenedor de recursos | `.pak` v1 con cabecera `KPAK`, TOC de entradas fijas de 40 bytes, enteros little-endian, offsets absolutos y datos alineados a 16 bytes; sin compresión ni cifrado; CRC-32/ISO-HDLC; lectura de recursos bajo demanda | Evita cargar el paquete completo en memoria y mantiene verificables sus entradas | Cifrar el índice o todo el paquete; comprimir entradas | Estructura v1 definida; fijar límite de memoria del índice y límites por tipo de recurso |

---

## 10. Requisitos de calidad


| Requisito | Prioridad | Cómo comprobarlo |
|---|---|---|
| Rendimiento | Imprescindible | Ejecutar la escena de referencia en Linux Mint 22.2 a 1366 × 768 durante 5 minutos. Objetivo inicial: percentil 95 del tiempo de fotograma ≤ 16,67 ms (referencia de 60 Hz). Registrar los tiempos de fotograma y conservar los resultados de la prueba. |
| Tiempo de carga | Deseable | Medir por separado el arranque en frío y la carga del nivel de prueba en Linux. Como objetivo provisional, comprobar si el nivel se carga en menos de 3 segundos y ajustar el umbral con los resultados reales. |
| Estabilidad | Imprescindible | Ejecutar la escena de referencia durante 10 minutos y repetir 10 veces la secuencia de abrir, cargar, renderizar y cerrar. No debe haber cierres inesperados, fugas detectables ni errores o advertencias de Vulkan atribuibles al engine. |
| Tamaño de la aplicación | Deseable | Registrar el tamaño de los ejecutables Linux y del `.pak` de prueba. Fijar límites cuando exista una muestra representativa de recursos. |
| Facilidad para añadir contenido | Imprescindible | Crear un nivel y una animación básicos desde el editor, empaquetarlos y cargarlos sin modificar el código del núcleo. |
| Compatibilidad del MVP | Imprescindible | Compilar y ejecutar runtime, editor y empaquetador en Linux Mint 22.2 x86_64. La validación Android pertenece a la etapa posterior de esa plataforma. |
| Facilidad de depuración | Imprescindible | En una compilación de desarrollo, registrar los errores de Vulkan causados por el núcleo. Simular la ausencia de un recurso y comprobar que el mensaje identifique el recurso y la operación fallida. |

---

## 11. Pruebas y validación

**Dispositivos y sistemas disponibles para probar:**
- Linux Mint 22.2 Xfce (Zara), x86_64. GPU AMD Radeon Graphics (`1002:15e7`), pantalla de 1366 × 768 a 60 Hz y sesión X11.
- Para la etapa posterior de Android: Motorola Moto G34 5G, Android 16 (API 36), arquitectura arm64-v8a.

**Qué debe probarse en Linux:**
- Compilar y ejecutar el runtime y el editor, y ejecutar el empaquetador, en Linux Mint 22.2.
- Verificar que Vulkan detecte la GPU AMD y pueda crear una superficie de presentación y renderizar.
- Abrir un nivel y cargar desde el `.pak` una textura, una animación y datos FlatBuffers.
- Probar el redimensionamiento de la ventana y el cierre limpio.
- Comprobar el renderizado y el layout del editor en la resolución nativa de Linux (1366 × 768).
- Medir el rendimiento del runtime con la escena de referencia a 1366 × 768.

**Validación posterior: Android 16+:**
- Instalar y ejecutar el APK arm64-v8a en el Moto G34 5G.
- Consultar en tiempo de ejecución la versión de Vulkan y las capacidades que expone el driver.
- Cargar y renderizar los recursos del `.pak` y probar la entrada táctil.
- Probar pausa, reanudación y recreación de la superficie al cambiar de aplicación o volver a ella.
- Medir el rendimiento y el tiempo de carga de la escena de referencia en el dispositivo.

**Pruebas manuales necesarias:**
- Crear y guardar un nivel y una animación en el editor; volver a abrirlos y comprobar que conservan sus datos.
- (Post-MVP) Generar el código C++ de una interfaz de juego, compilarlo e inspeccionar su layout en distintos tamaños de ventana.
- Empaquetar el contenido, cargarlo desde el runtime y confirmar que el índice permite localizar los recursos.
- Comprobar que el CRC-32/ISO-HDLC de la tabla y los recursos, los tamaños y la alineación de 16 bytes se validen correctamente.
- Comprobar los mensajes del núcleo cuando Vulkan no está disponible o falta un recurso.

**Pruebas automatizadas necesarias:**
- Verificar que el empaquetador rechace recursos inexistentes, referencias inválidas y archivos de entrada corruptos.
- Verificar la serialización y lectura de niveles y animaciones FlatBuffers, incluidos buffers truncados o inválidos.
- Probar el lector del `.pak` con índices y offsets válidos e inválidos, IDs duplicados, offsets no alineados, tamaños fuera de límites y CRC incorrectos.
- Verificar que abrir el `.pak` no lea todos sus datos y que solicitar un recurso lea solo su rango; validar también la comprobación CRC por bloques.
- Comprobar que el núcleo pueda inicializarse, cargar una escena básica y cerrarse sin errores.

**Problemas conocidos o riesgos:**
- `vulkaninfo --summary` confirma la GPU AMD Radeon Graphics (Renoir) con RADV de Mesa 25.2.8, API Vulkan de dispositivo 1.4.318 y versión de instancia/loader 1.3.275. También enumera `llvmpipe` como dispositivo de software, que no será el objetivo de rendimiento. Se enumeran extensiones de superficie Xlib y XCB, pero aún falta comprobar que el runtime pueda crear una superficie X11 y presentar mediante swapchain.
- La salida de `vulkaninfo` advierte que el ICD no exporta `vkGetPhysicalDeviceDisplayPropertiesKHR` ni `vkGetPhysicalDeviceDisplayPlanePropertiesKHR`. Esas funciones corresponden a presentación directa; la advertencia no demuestra que X11 falle, por lo que debe verificarse con el prototipo de ventana.
- La lista de capas de `vulkaninfo` no incluye `VK_LAYER_KHRONOS_validation`. Instalarla y comprobar que el runtime la active antes de exigir pruebas con Validation Layers.
- El Moto G34 5G es el único dispositivo Android disponible para la etapa posterior; probarlo no demostrará compatibilidad con todos los dispositivos Android 16 o superiores.
- El runtime leerá los recursos del `.pak` bajo demanda, sin cargar todo el archivo en memoria. Antes de implementarlo se fijará un límite de memoria para la tabla de contenidos y límites por tipo de recurso; no se impondrá un límite global del `.pak` basado en RAM. El MVP no incluirá cifrado ni compresión adicional.
- La disponibilidad hasta el 1 de diciembre de 2026 es de 25 horas semanales más ocho jornadas completas adicionales en noviembre. La estimación de horas productivas y la reserva para imprevistos se detallan en el plan.

---

## 12. Plan de desarrollo

| Etapa | Resultado esperado | Criterio para darla por terminada |
|---|---|---|
| 1. Verificación técnica Linux | Confirmar que la GPU y el controlador de Linux permiten crear una superficie Vulkan y presentar una imagen. Instalar Validation Layers y definir el contrato del `.pak`. | Un prototipo mínimo crea una superficie X11 y presenta una imagen en la GPU AMD; el runtime activa `VK_LAYER_KHRONOS_validation`. El formato del `.pak` queda documentado antes de implementar lector y escritor. |
| 2. Runtime Linux mínimo | Ventana, entrada, renderizado 2D, carga del `.pak` y cierre limpio; sin física ni audio como requisitos del MVP. | El runtime carga un paquete de prueba, muestra un sprite y responde a una entrada básica en Linux Mint 22.2. |
| 3. Editor de niveles y animaciones | Editor de escritorio con cuadrícula, capas básicas, clips por fotogramas y persistencia JSON. | Se puede crear, guardar, cerrar, reabrir y verificar un nivel y una animación sin pérdida de datos. |
| 4. Empaquetador y formatos de distribución | CLI de escritorio que valida los JSON, los convierte a FlatBuffers y crea un `.pak` v1 con índice legible, sin compresión adicional ni cifrado. | El runtime lee el paquete generado; la CLI rechaza referencias ausentes y datos inválidos; las pruebas cubren offsets, alineación de 16 bytes, tamaños y CRC-32/ISO-HDLC. |
| 5. Integración de extremo a extremo | Nivel técnico pequeño creado con las herramientas previstas y cargado por el runtime Linux. | Se completa el flujo editor → JSON → FlatBuffers/`.pak` → runtime, con pruebas de rendimiento, estabilidad y cierre limpio. |
| 6. Validación y entrega del MVP Linux | Runtime, editor y empaquetador ejecutables en Linux, más instrucciones de prueba. | Se completan los criterios de aceptación de la sección 10 en el equipo Linux objetivo. |
| 7. Runtime Android | Portar el runtime y validar Vulkan, entrada táctil y ciclo de vida en Android 16/API 36 o superior. | Se instala y ejecuta en el Moto G34 5G, carga el mismo formato `.pak` y supera las pruebas de pausa, reanudación y recreación de superficie. |
| 8. Otras plataformas | Evaluar Windows, WebAssembly u otros sistemas/dispositivos tras validar las dos plataformas anteriores. | Cada plataforma tendrá backend, formato de distribución y pruebas de compatibilidad definidos antes de declararse soportada. |


**Horas disponibles por semana:**  
25 horas a la semana (martes, miércoles, jueves y viernes), más ocho jornadas completas adicionales en noviembre. Como estimación inicial, contar 6–8 horas productivas por jornada adicional (48–64 horas); confirmar la disponibilidad real al calendarizar.

**Estimación de disponibilidad hasta la fecha objetivo:**
Desde el 22 de octubre hasta el 1 de diciembre hay aproximadamente 5,7 semanas: unas 143 horas al ritmo habitual, más 48–64 horas de las jornadas extra, para un total aproximado de 191–207 horas. No comprometer todo ese tiempo en funcionalidades: reservar unas 30–40 horas para integración, bloqueos y correcciones, especialmente por la experiencia limitada con Vulkan.

**Fecha objetivo:**  
1 de diciembre de 2026

**Qué significa “terminado” en esa fecha:**  
El runtime, el editor y el empaquetador funcionan en Linux Mint 22.2 x86_64; el editor guarda un nivel y una animación en JSON; el empaquetador genera un `.pak` con datos FlatBuffers; y el runtime carga y muestra ese contenido. Android y el resto de plataformas no forman parte de esta fecha de entrega.


---

## 13. Suposiciones y preguntas abiertas

### Suposiciones actuales
- El PC de pruebas usa Linux Mint 22.2 Xfce x86_64 y un AMD Ryzen 3 7330U con Radeon Graphics integrada (dispositivo PCI `1002:15e7`). `vulkaninfo --summary` identifica RADV de Mesa 25.2.8; la presentación en una superficie X11 sigue pendiente de prueba.
- El runtime Linux usará GLFW detrás de una API del engine. La lógica del juego no dependerá directamente de GLFW; el backend Android nativo se implementará después del MVP Linux.
- El editor guardará niveles y animaciones en JSON durante el desarrollo; al preparar una distribución, el empaquetador los convertirá a FlatBuffers. Los layouts de interfaz del juego y su generador de C++ quedan para una fase posterior.
- El empaquetador incluirá recursos y datos FlatBuffers en un `.pak` v1 con índice legible, offsets de datos alineados a 16 bytes, sin cifrado ni compresión adicional.
- Los diálogos y la lógica del juego se implementarán directamente en C++ durante el MVP.

### Preguntas que necesito resolver
- ¿Puede el runtime crear una superficie X11 y presentar mediante swapchain en la GPU AMD RADV? `vulkaninfo` ya confirma la versión del dispositivo y el controlador.
- ¿Qué límite de memoria se impondrá a la tabla de contenidos, y qué tamaño máximo se permitirá para cada tipo de recurso según su forma de carga (por ejemplo, niveles FlatBuffers completos frente a audio transmitido por bloques en una etapa futura)? No se necesita un máximo global del `.pak` por memoria porque el runtime lee los recursos bajo demanda.
- ¿Qué backend y estrategia de ciclo de vida se usarán para Android? Se resolverá durante la etapa posterior de esa plataforma.
### Decisiones que no necesito tomar todavía
- Soporte para Windows, WebAssembly y otras distribuciones o dispositivos distintos de los objetivos de prueba del MVP.
- Android, incluido su runtime, backend de plataforma, entrada táctil, gestión de superficies y ciclo de pausa/reanudación; se abordará después de validar el MVP Linux.
- La implementación completa del multijugador competitivo por LAN, incluidos sesiones, sincronización y desconexiones. La lógica del juego y la entrada se mantendrán separadas para facilitar su incorporación posterior.
- Prototipos de comunicación entre instancias; no son necesarios para la preparación arquitectónica mínima del MVP.
- Física de juego, Box2D, audio, combate, carriles de profundidad y raycasting hasta que se defina una prueba de juego que los necesite.
- Cifrado del `.pak` y compresión adicional de sus entradas.
- El editor visual de diálogos y la implementación completa de sus ramas y consecuencias.
- Las funciones avanzadas de audio y la animación esquelética completa con huesos, slots y attachments.
- La distribución comercial, la publicación en tiendas y la contratación de servidores.
- Cuándo podrá un raycast alcanzar objetos de otro carril de profundidad (se decidirá al implementar el raycasting, después del MVP).
- Cómo se generará y compilará el C++ producido por el editor para los layouts de interfaz del juego (post-MVP).

---

## 14. Contexto del desarrollador

**Experiencia relevante:**  
Tengo más de 3 años con C++ y más de 2 años con Python. He creado prototipos con raylib en C++ y con Godot usando GDScript. Tengo poca experiencia en programación gráfica: he realizado algunas pruebas superficiales con OpenGL, pero todavía no tengo una base sólida en Vulkan.

**Herramientas y sistema de desarrollo:**
- Sistema principal: Linux Mint 22.2 Xfce (Zara).
- Compilador: GCC 13.3.0.
- Sistema de compilación: CMake 3.28.3.
- Editor de código: VS Code; también utilizo Helix.
- Harness principal: OpenCode.

**Limitaciones importantes de tiempo, equipo o conocimientos:**

### Equipo y disponibilidad
Utilizo habitualmente tanto el ordenador como el teléfono, y a veces tengo que interrumpir el trabajo o las pruebas para cargarlos. La disponibilidad de ambos dispositivos puede ser intermitente.

### Conocimientos
- Tengo poca experiencia con programación gráfica y Vulkan; mis pruebas con OpenGL han sido superficiales.
- No domino la gestión manual de hilos.

**Preferencias de colaboración o revisión:**  
Usaré OpenCode y agentes de IA con modelos de DeepSeek como apoyo al desarrollo. Revisaré e integraré las propuestas en el proyecto. Un amigo participará como beta tester de los productos finales; no es programador y no probará habitualmente prototipos técnicos o herramientas internas.

**Desarrollo asistido por IA:**  
OpenCode será el harness principal para planificar tareas, explorar el código y coordinar la generación de cambios. La mayor parte del código se generará con modelos DeepSeek a través de OpenCode. Yo definiré los requisitos, revisaré e integraré los cambios generados y ejecutaré las pruebas necesarias para validar el resultado.
