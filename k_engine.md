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
Para el runtime, abrir una ventana o superficie, inicializar el pipeline básico de Vulkan para renderizar un sprite/textura 2D desde un contenedor binario personalizado y permitir cerrar la aplicación de forma limpia en todas las plataformas objetivo. Para las herramientas de escritorio, crear un editor GUI que permita colocar baldosas (tiles) en una cuadrícula 2D y definir una secuencia de fotogramas (spritesheet) para una animación básica. Durante el desarrollo, los proyectos y sus niveles y animaciones se guardarán en JSON. Cuando se prepare un juego para distribución, una CLI en Python validará esos JSON y los convertirá a buffers FlatBuffers; después empaquetará esos buffers y los recursos multimedia (por ejemplo, PNG y WAV) en un archivo binario (.pak) que el runtime C++ pueda leer de forma eficiente.

### Cómo sabré que el MVP está terminado

#### Núcleo
- [ ] Pila de validación limpia: La aplicación ejecuta, renderiza y se cierra sin arrojar advertencias ni errores en las Vulkan Validation Layers.

- [ ] Criterio de memoria y cierre: El engine debe liberar los recursos propiedad de la aplicación al cerrar. Se validará mediante pruebas repetidas de apertura, carga, renderizado y cierre, análisis de memoria y capas de validación de Vulkan en la configuración de desarrollo. Android deberá poder pausar y reanudar el renderizado cuando cambie la disponibilidad de la superficie.

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
- [ ] Ejecutarse en Linux Mint Xfce y x86_64.
- [ ] Instalarse y ejecutarse en un dispositivo Android 16 (API 36) o superior
      con arquitectura arm64-v8a y soporte Vulkan requerido.
- [ ] Ejecutar el runtime en todas las plataformas objetivo del MVP (Linux y Android). El editor visual y el empaquetador serán herramientas de escritorio; en el MVP, se ejecutarán en Linux.
- [ ] Renderizar un sprite cargado desde un contenedor creado por el empaquetador.
- [ ] Crear, guardar y cargar un nivel 2D básico desde el editor.
- [ ] Crear y previsualizar una animación básica a partir de una spritesheet.
- [ ] Guardar los datos del editor en JSON durante el desarrollo; al preparar la distribución, convertirlos a FlatBuffers y empaquetarlos con los recursos para cargarlos correctamente en el runtime.
- [ ] Completar una prueba de extremo a extremo con un nivel pequeño.
- [ ] Proporcionar una versión ejecutable para Linux y un paquete instalable
      para Android, junto con instrucciones básicas de prueba.
- [ ] Preparación arquitectónica para red

### No incluirá en esta primera versión
- [ ] Renderizado 3D ni herramientas de edición 3D.
- [ ] Compilación o distribución para Windows y WebAssembly.
- [ ] Multijugador en línea.
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
- **Forma de distribución para el MVP:** Runtime y herramientas de escritorio ejecutables de forma nativa para probar en el sistema objetivo; no se requiere instalador.
- **Versión mínima del sistema:** Linux Mint 22.2.
- **Dispositivos de prueba disponibles:** El equipo de desarrollo con Linux Mint 22.2 Xfce.
- **Métodos de entrada:** Teclado y ratón.

### Android
- **Versión mínima:** Android 16 (API 36).
- **SDK objetivo para Google Play:** `targetSdkVersion` 36 o superior para nuevas apps y actualizaciones, según la política vigente.
- **SDK de compilación:** `compileSdkVersion` 36 o superior.
- **Arquitectura objetivo:** arm64-v8a.
- **Dispositivo de prueba:** Moto G34 5G con Android 16 y arquitectura arm64-v8a.
- **Forma de instalación para el MVP:** APK instalable manualmente en el dispositivo de prueba; contiene el runtime, no el editor ni el empaquetador.
- **Orientación de pantalla:** Horizontal, recomendada para el juego de plataformas.
- **Métodos de entrada:** Pantalla táctil.
- **Requisito gráfico en Android:** Vulkan 1.1 o superior. Verificar las capacidades expuestas por el driver en el Moto G34 5G.


### Plataformas futuras
Windows y WebAssembly quedan fuera del MVP. Se podrán considerar después de validar el engine en Linux Mint y Android. También se podrá ampliar la compatibilidad a otras distribuciones de Linux y dispositivos Android. El mínimo del MVP es Android 16 (API 36); en una fase posterior se podrá evaluar reducir el mínimo hasta Android 10 (API 29), manteniendo Vulkan 1.1 como requisito gráfico. Este cambio solo se considerará después de probar el engine en dispositivos reales con distintas GPU y drivers; si no se pueden realizar esas pruebas, no se declarará compatibilidad con Android 10.
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
En este juego, un carril es un plano de profundidad discreto dentro de la vista lateral 2D (típicamente 2 o 3: fondo, medio y primer plano). Cada personaje o enemigo ocupa un carril: se mueve lateralmente y salta dentro de él, y cambia de carril solo en puntos de conexión definidos. El carril condiciona el combate, las colisiones y el orden de dibujado (los del carril de fondo se dibujan detrás). Para el MVP, la lógica es discreta (índice de carril) y la transición visual entre carriles se interpola suavemente.

**Modo de juego:**  
Individual offline y multijugador competitivo mediante una red local, con modos 1 vs. 1 y 3 vs. 3. En el multijugador cada jugador elige un personaje disponible del modo offline.

**¿Qué parte del juego se desarrollará para validar el MVP del engine?**   
Se desarrollará un nivel corto, individual y sin conexión a internet, con tres personajes provisionales entre los que el jugador pueda cambiar. El nivel tendrá carriles de profundidad con conexiones definidas, un enemigo, un ataque básico y un objetivo sencillo. Se usarán una cuadrícula, animaciones y recursos creados para la prueba; estos se empaquetarán y cargarán desde el runtime.

La selección completa de armas y hechizos, el inventario durante el nivel, el sistema de sigilo y el multijugador competitivo en red local quedarán fuera de esta prueba inicial.

---

## 6. Requisitos funcionales del engine


| Capacidad | Prioridad | Descripción o ejemplo |
|---|---|---|
| Renderizado 2D | Necesario | Renderizar sprites y texturas con Vulkan desde los recursos empaquetados. En Android, exigir Vulkan 1.1 o superior; verificar también los requisitos de GPU y driver en Linux. Permitir ordenar los personajes según su posición en los carriles de profundidad (ordenar por carril y, dentro de cada carril, por la posición vertical). |
| Ventanas y superficies | Necesario | Crear la ventana o superficie de renderizado en Linux y Android. Gestionar redimensionamiento, recreación de la superficie y ciclo de pausa/reanudación en Android. |
| Entrada | Necesario | Ofrecer acciones independientes del dispositivo. Para el MVP, admitir teclado y ratón en Linux y pantalla táctil en Android. El mando puede añadirse después. |
| Física y colisiones | Necesario para el juego; raycasting deseable después del MVP inicial | Incluir movimiento 2D básico, colisiones y detección de impactos para el ataque básico. Usar un único mundo Box2D para todo el nivel, con X = horizontal, Y = vertical (salto y gravedad); el carril se representa como metadato del cuerpo y se filtra con una máscara de carril (lane mask), de modo que por defecto las entidades solo interactúan con las de su propio carril. Las consultas de raycasting (impactos de armas a distancia y línea de visión) se añadirán después del MVP; en esa fase se definirá si un raycast puede atravesar carriles de profundidad o si solo afecta al carril correspondiente. El campo visual, la detección del jugador y las reglas de alerta pertenecen a la lógica del juego. |
| Animaciones | Necesario: animación por fotogramas en el MVP; Deseable: animación esquelética en una fase posterior | El MVP permite definir secuencias desde spritesheets y previsualizarlas. En una fase posterior, añadir un sistema 2D de huesos jerárquicos, slots asociados a huesos y attachments —por ejemplo, sprites que se puedan cambiar—, con clips que animen transformaciones y cambios de attachment. |
| Interfaz de usuario | Necesario | La UI del runtime debe adaptarse al tamaño de ventana, resolución, orientación y escala de pantalla en Linux y Android. El editor es una herramienta de escritorio y su UI debe adaptarse a los tamaños de ventana de escritorio. El sistema de layout estará inspirado en el enfoque responsive y declarativo de Clay, pero se implementará como una biblioteca C++ propia. La UI se construirá mediante una API de bajo nivel basada en el patrón Builder, sin macros, separando la descripción del layout de su renderizado. El MVP incluirá los componentes necesarios para el editor —paneles, botones, paleta de tiles, cuadrícula y controles de animación—. Más adelante, permitirá crear interfaces de juego, como el inventario que se abre sin pausar el nivel. |
| Carga de recursos | Necesario | Leer y validar recursos desde un contenedor versionado, incluyendo imágenes y datos de niveles. Informar errores descriptivos si faltan recursos o el contenedor no es compatible. |
| Carga de niveles | Necesario | Cargar un nivel 2D con tiles, capas, objetos y carriles de profundidad. El nivel debe poder representar dónde existen carriles, en qué tramos, y las zonas de conexión donde se permite cambiar entre ellos; las reglas de combate y sigilo pertenecen al juego. |
| Guardado de datos | Necesario | Guardar y cargar los proyectos editables del editor. El guardado de progreso, decisiones narrativas y recursos de campaña queda para una fase posterior. |
| Red | Necesario: preparación arquitectónica; Futuro: partidas multijugador completas | Mantener la simulación y el estado del juego separados del renderizado y de la entrada física. Representar las acciones como comandos que puedan proceder del jugador local o de la red, usar un paso de simulación independiente de la tasa de renderizado y asignar identificadores estables a las entidades. Antes de implementar el combate completo, realizar un prototipo que intercambie acciones o estado entre dos instancias. Las partidas LAN 1 vs. 1 y 3 vs. 3, las salas y la gestión de desconexiones quedan para una fase posterior. |
| Audio | Necesario | Cargar audio desde el contenedor y reproducir efectos de sonido y música 2D. Para el MVP, admitir archivos WAV, reproducción, pausa, reanudación, detención, repetición y control básico de volumen. Gestionar la pausa y reanudación de la aplicación en Android. No incluir audio espacial, efectos DSP ni mezcla avanzada. |
| Editor de niveles y animaciones | Necesario; herramienta de escritorio | Permitir crear un nivel en cuadrícula, seleccionar y colocar tiles, editar capas y crear una animación básica. Guardar el proyecto y sus datos de desarrollo en JSON. |
| Empaquetador | Necesario; herramienta de escritorio | Incluir una CLI en Python que valide los recursos y niveles, detecte referencias ausentes, convierta los JSON de desarrollo a FlatBuffers para distribución y genere un contenedor que el runtime C++ pueda leer. |
| Otras | Deseable | Facilitar mensajes de diagnóstico útiles para errores de carga, inicialización de Vulkan y recursos incompatibles. |

---

## 7. Editor y herramientas

**¿El MVP necesita un editor visual?**  
Sí. Será una herramienta de escritorio (en el MVP, para Linux) para crear y guardar niveles y animaciones básicas en JSON durante el desarrollo. No se instalará ni ejecutará en Android; el runtime sí se ejecutará tanto en Linux como en Android.

**¿Qué se podrá crear o modificar con el editor?**
- [x] Niveles: tilemaps en cuadrícula, capas básicas y datos necesarios para representar los carriles de profundidad y sus zonas de conexión.
- [x] Interfaz de usuario (del editor): el editor de escritorio construye su propia interfaz (paneles, botones, paleta de tiles, cuadrícula y controles de animación) con la API Builder del engine, lo que valida el sistema de UI. La lógica y los eventos del juego se implementarán en C++.
- [ ] Interfaz de usuario (del juego) y generador de C++: diseñar visualmente layouts responsive para el juego (por ejemplo, el inventario que no pausa el nivel) y generarlos como código C++ mediante la API Builder quedará para una fase posterior.
- [x] Animaciones: clips básicos creados a partir de fotogramas de una spritesheet.
- [ ] Diálogos: no habrá un editor visual de diálogos en el MVP. Si se necesitan para una prueba, su contenido se escribirá manualmente en archivos JSON.
- [x] Otros: explorador de recursos para importar, organizar y previsualizar imágenes, spritesheets y audio, y detectar recursos faltantes o referencias rotas.


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
- No se aplicará compresión adicional a PNG, JPG ni a otros formatos que ya estén comprimidos.
- Se podrá comprimir otros datos si las pruebas muestran una reducción útil del tamaño. El índice registrará el método de compresión y los tamaños comprimido y original de cada entrada.
- El empaquetador validará las referencias a recursos, los offsets y los tamaños antes de generar el paquete.

**¿Se necesita cifrar recursos?**
El índice del `.pak` se cifrará con AES-128-GCM para ocultar su estructura, incluidas las entradas y sus ubicaciones. El encabezado inicial permanecerá sin cifrar y contendrá únicamente la información necesaria para identificar el formato y localizar el índice cifrado. El encabezado se autenticará junto con el índice (se usará como datos autenticados adicionales de GCM, sin cifrarlo).

Los datos de los recursos no se cifrarán: esta medida oculta el índice, pero no impide extraer o inspeccionar recursos que se almacenen en formatos reconocibles. Cada paquete usará un nonce único para AES-GCM. La clave se almacenará en el juego; por ello, este cifrado busca dificultar la inspección casual del paquete, no ofrecer protección absoluta frente a la ingeniería inversa.

**Identificación de recursos:**
- Cada recurso tendrá un ID estable asignado y guardado en los datos del proyecto.
- La ruta relativa del archivo servirá para localizarlo durante el desarrollo.
- El índice cifrado asociará cada ID con el tipo, offset, tamaño, método de compresión y suma de comprobación de la entrada correspondiente.
- Los hashes de contenido no se usarán como identificadores principales, porque cambian cuando cambia el archivo. Una suma de comprobación podrá usarse aparte para detectar corrupción accidental.
---

## 9. Decisiones técnicas



| Tema | Decisión actual | Motivo | Alternativas consideradas | Estado |
|---|---|---|---|---|
| Lenguaje y estándar | C++17 para el núcleo (runtime) y el editor | Es el estándar elegido para compartir código entre Linux y Android | Otros estándares de C++ | Decidido |
| GPU Linux de referencia | AMD Radeon Graphics, dispositivo `1002:15e7`, con Linux Mint 22.2 Xfce; pantalla de 1366 × 768 a 60 Hz bajo X11 | Define el hardware y el entorno principal para probar el runtime Linux | Otras GPU y entornos de pantalla | Hardware de prueba identificado; capacidad Vulkan por verificar |
| API gráfica en Linux | Vulkan; comprobar la versión y el controlador disponibles en la GPU de referencia | Se busca compartir la API gráfica con Android y usar un backend explícito | OpenGL; otro backend | Decidido; comprobar soporte y presentación de superficies en Linux |
| API gráfica en Android | Vulkan 1.1 o superior en arm64-v8a; consultar las capacidades del dispositivo en tiempo de ejecución | El Moto G34 5G con Snapdragon 695/Adreno 619 admite Vulkan 1.1 | Vulkan 1.0; otro backend | Decidido para el dispositivo de prueba |
| Android SDK | `minSdkVersion` 36, `targetSdkVersion` 36 y `compileSdkVersion` 36 o superior | Permite instalar en Android 16 o superior y cumplir el requisito actual de Google Play para nuevas apps y actualizaciones | Revisar cuando cambien los requisitos de Google Play | Decidido para el MVP |
| Ventanas y entrada | La lógica del juego y el editor usan únicamente la API pública del núcleo para ventanas y entrada. El runtime selecciona el backend de plataforma: GLFW3 en Linux y las APIs nativas de Android en Android. El editor se ejecutará como herramienta de escritorio en Linux para el MVP. | Evita que el juego y el editor dependan directamente de una librería o sistema operativo, y permite cambiar los backends sin modificar su lógica. | SDL3 como backend común; otros backends por plataforma | Decidido como enfoque; validar la implementación de ambos backends |
| Audio | miniaudio para reproducción básica de WAV, efectos y música | Mantiene acotado el alcance de audio del MVP | OpenAL; audio de SDL3 | Investigar mediante un prototipo en Linux y Android |
| Física y colisiones | Box2D detrás de una interfaz propia del engine: un único mundo por nivel (X = horizontal, Y = vertical para salto y gravedad), con el carril como metadato del cuerpo y filtrado por máscara de carril (lane mask); el raycasting se añadirá después del MVP | Evita que la lógica del juego dependa directamente de Box2D y permite implementar las consultas necesarias para combate y visión | Un mundo Box2D por carril; física propia acotada; otra biblioteca 2D | Decidido: un solo mundo + filtros de carril; validar la integración de los carriles de profundidad |
| Datos de niveles y animaciones | JSON editable durante el desarrollo; al preparar una distribución, conversión a buffers FlatBuffers mediante esquemas `.fbs` | Evita parsear JSON durante la ejecución distribuida y permite definir un formato binario versionado | Parsear JSON en el runtime; formato binario propio | Decidido |
| Interfaz de usuario | API Builder propia (sin macros) para construir la interfaz responsive del editor; el diseño visual de interfaces de juego y su generador de C++ quedan para una fase posterior | Permite construir la interfaz del editor y adaptar su distribución a distintas pantallas; el generador de layouts de juego se añadirá después del MVP | Layouts escritos a mano en C++; sistema inspirado en Clay sin generación de código | Decidido para el MVP; generador de C++ para layouts de juego en fase posterior |
| Diálogos y lógica del juego | Se implementan directamente en C++ | No se editarán visualmente en el MVP | Editor visual de diálogos; archivos de datos interpretados en runtime | Decidido para el MVP |
| Contenedor de recursos | Archivo `.pak` versionado con recursos y buffers FlatBuffers; índice cifrado con AES-128 para ocultar la estructura del paquete | Agrupa los datos del runtime y oculta la información del índice | Archivos sueltos; cifrar todo el paquete | Decidido el objetivo (AES-128-GCM); gestión de clave y nonce por investigar |

---

## 10. Requisitos de calidad


| Requisito | Prioridad | Cómo comprobarlo |
|---|---|---|
| Rendimiento | Imprescindible | Ejecutar la misma escena de prueba en Linux Mint 22.2 y en el Moto G34 5G. Objetivo inicial: 60 FPS en Linux (pantalla de 60 Hz); en Android, fijar el objetivo según la frecuencia de la pantalla del Moto G34 (por confirmar). Registrar los tiempos de fotograma y comprobar que no haya caídas sostenidas durante una prueba de 5 minutos. |
| Tiempo de carga | Deseable | Medir por separado el arranque en frío y la carga de un nivel de prueba en ambos destinos. Como objetivo provisional, comprobar si el nivel se carga en menos de 3 segundos y ajustar el umbral con los resultados reales. |
| Estabilidad | Imprescindible | Ejecutar la escena durante 30 minutos y repetir cargas, cierres, pausas y reanudaciones en Android. No debe haber cierres inesperados ni errores de Vulkan causados por el núcleo. |
| Tamaño de la aplicación | Deseable | Registrar el tamaño del APK y del `.pak` en una compilación de distribución. Fijar un límite cuando haya una muestra representativa de los recursos del juego. |
| Facilidad para añadir contenido | Imprescindible | Crear un nivel y una animación básicos desde el editor, empaquetarlos y cargarlos sin modificar el código del núcleo. |
| Compatibilidad entre Linux y Android | Imprescindible | Compilar y ejecutar la misma prueba funcional del runtime en Linux Mint 22.2 x86_64 y en el Moto G34 5G con Android 16 (API 36) o superior, verificando Vulkan 1.1 en el dispositivo. El editor y el empaquetador se prueban como herramientas de escritorio en Linux. |
| Facilidad de depuración | Imprescindible | En una compilación de desarrollo, registrar los errores de Vulkan causados por el núcleo. Simular la ausencia de un recurso y comprobar que el mensaje identifique el recurso y la operación fallida. |

---

## 11. Pruebas y validación

**Dispositivos y sistemas disponibles para probar:**
- Linux Mint 22.2 Xfce (Zara), x86_64. GPU AMD Radeon Graphics (`1002:15e7`), pantalla de 1366 × 768 a 60 Hz y sesión X11.
- Motorola Moto G34 5G, Android 16 (API 36), arquitectura arm64-v8a. Coincide con la versión mínima prevista para el MVP.

**Qué debe probarse en Linux:**
- Compilar y ejecutar el editor y el runtime en Linux Mint 22.2.
- Verificar que Vulkan detecte la GPU AMD y pueda crear una superficie de presentación y renderizar.
- Abrir un nivel y cargar desde el `.pak` una textura, una animación y datos FlatBuffers.
- Probar el redimensionamiento de la ventana y el cierre limpio.
- Comprobar el renderizado y el layout del editor en la resolución nativa de Linux (1366 × 768).
- Medir el rendimiento con una escena de referencia y una resolución objetivo definida para cada plataforma.

**Qué debe probarse en Android 16+:**
- Instalar y ejecutar el APK arm64-v8a en el Moto G34 5G.
- Consultar en tiempo de ejecución la versión de Vulkan y las capacidades que expone el driver.
- Cargar y renderizar los recursos del `.pak`, reproducir audio WAV y probar la entrada táctil.
- Probar pausa, reanudación y recreación de la superficie al cambiar de aplicación o volver a ella.
- Medir el rendimiento y el tiempo de carga de la escena de referencia en el dispositivo.

**Pruebas manuales necesarias:**
- Crear y guardar un nivel y una animación en el editor; volver a abrirlos y comprobar que conservan sus datos.
- (Post-MVP) Generar el código C++ de una interfaz de juego, compilarlo e inspeccionar su layout en distintos tamaños de ventana.
- Empaquetar el contenido, cargarlo desde el runtime y confirmar que el índice permite localizar los recursos.
- Probar cambios de carril y colisiones en la escena de prueba.
- Comprobar los mensajes del núcleo cuando Vulkan no está disponible o falta un recurso.
- Tras definir la generación de la clave y del nonce, probar que el runtime pueda leer un índice válido y rechace uno alterado.

**Pruebas automatizadas necesarias:**
- Verificar que el empaquetador rechace recursos inexistentes, referencias inválidas y archivos de entrada corruptos.
- Verificar la serialización y lectura de niveles y animaciones FlatBuffers, incluidos buffers truncados o inválidos.
- Probar el lector del `.pak` con índices y offsets válidos e inválidos.
- Probar colisiones a través de la interfaz del núcleo, sin exponer tipos de Box2D.
- Comprobar que el núcleo pueda inicializarse, cargar una escena básica y cerrarse sin errores.

**Problemas conocidos o riesgos:**
- El reporte actual de la GPU Linux muestra OpenGL, pero no confirma la versión ni la disponibilidad de Vulkan, aunque, según el modelo de la CPU/GPU, la versión máxima esperada de Vulkan es 1.3. Verificarlo con `vulkaninfo --summary` y probar la presentación en X11.
- El Moto G34 5G es el único dispositivo Android de prueba disponible y ejecuta Android 16 (API 36), que coincide con el mínimo declarado. Probarlo no demuestra compatibilidad con otros dispositivos que ejecuten Android 16 o superior.
- Falta definir la generación, el almacenamiento y la protección de la clave y del nonce para el índice del `.pak` (AES-128-GCM); las pruebas de cifrado e integridad dependerán de esa decisión.
- El alcance del MVP (motor Vulkan multiplataforma, editor visual, empaquetador con FlatBuffers y cifrado) es amplio para unas 25 horas/semana hasta el 1 de diciembre de 2026; conviene definir un corte mínimo entregable por si el tiempo no alcanza.

---

## 12. Plan de desarrollo

| Etapa | Resultado esperado | Criterio para darla por terminada |
|---|---|---|
| 1. Investigación y decisiones técnicas | Decisiones documentadas sobre Vulkan en Linux y Android, capa de plataforma, audio, Box2D, FlatBuffers y cifrado del índice del `.pak`. | Hay pruebas o documentación para las decisiones principales. Se verifica Vulkan en Linux con `vulkaninfo --summary` y en el Moto G34 5G. |
| 2. Prototipo técnico multiplataforma | Aplicaciones de prueba para Linux y Android que crean la superficie Vulkan, reciben entrada y renderizan un sprite. | El prototipo funciona en Linux Mint 22.2 x86_64 y se instala y ejecuta en el Moto G34 5G con Android 16. En Android se prueba también la pausa y reanudación. |
| 3. Funciones esenciales del núcleo | API pública del núcleo, backend de plataforma, renderizado 2D, entrada, audio básico, colisiones, animación por fotogramas y diagnóstico de Vulkan. | El runtime carga y muestra una escena de prueba; reproduce audio WAV; y puede cerrarse o reanudarse sin errores atribuibles al núcleo. |
| 4. Editor de niveles y animaciones | Editor visual de escritorio para crear, guardar y cargar niveles y animaciones en JSON durante el desarrollo. | Se pueden editar y guardar los niveles y las animaciones, y reabrirlos sin pérdida de datos. |
| 5. Empaquetador y formatos de ejecución | CLI de escritorio en Python que valida los datos, convierte niveles y animaciones JSON a FlatBuffers para distribución y crea el `.pak` con el índice cifrado. | El runtime valida y abre el índice, carga los recursos y lee los buffers FlatBuffers. El empaquetador rechaza referencias ausentes y archivos inválidos. |
| 6. Prueba con contenido del juego | Nivel offline corto con tres personajes provisionales, cambio entre personajes, carriles de profundidad con conexiones definidas, un enemigo, un ataque y un objetivo sencillo. | El contenido se crea con las herramientas previstas, se empaqueta y se ejecuta en Linux y Android. Esta prueba no requiere implementar todo el inventario, el sigilo ni el multijugador. |
| 7. Validación y entrega del MVP | Versiones ejecutables para Linux y Android, junto con instrucciones para probarlas. | Se completan las pruebas definidas para ambos dispositivos, se revisan los errores de Vulkan y se verifica el cierre limpio del runtime. |


**Horas disponibles por semana:**  
25 horas a la semana (martes, miércoles, jueves y viernes)

**Fecha objetivo:**  
1 de diciembre de 2026

**Qué significa “terminado” en esa fecha:**  
El editor permite crear el contenido básico de la prueba; el empaquetador produce un `.pak` válido; y el runtime carga ese paquete y ejecuta el nivel de prueba en Linux Mint 22.2 y en el Moto G34 5G. Las funciones excluidas del MVP se especificarán en la sección de alcance.


---

## 13. Suposiciones y preguntas abiertas

### Suposiciones actuales
- El PC de pruebas usa Linux Mint 22.2 Xfce x86_64 y un AMD Ryzen 3 7330U con Radeon Graphics integrada (dispositivo PCI `1002:15e7`). Vulkan en Linux está pendiente de verificación.
- La API pública del núcleo ocultará los detalles de plataforma. La lógica del juego y el editor no dependerán directamente de GLFW ni de las APIs nativas de Android.
- El editor guardará niveles y animaciones en JSON durante el desarrollo; al preparar una distribución, el empaquetador los convertirá a FlatBuffers. Los layouts de interfaz del juego y su generador de C++ quedan para una fase posterior.
- El empaquetador incluirá los recursos y los datos FlatBuffers en un `.pak`. Se cifrará el índice para ocultar la estructura del paquete, pero no los recursos multimedia.
- Los diálogos y la lógica del juego se implementarán directamente en C++ durante el MVP.

### Preguntas que necesito resolver
- ¿Qué versión de Vulkan y qué controlador están disponibles realmente en la GPU Linux? El reporte actual confirma OpenGL, pero no Vulkan.
- ¿Cómo se generará, almacenará y (si aplica) rotará la clave AES-128 y el nonce único de cada paquete? El modo ya está fijado: AES-128-GCM.
- ¿Funcionan GLFW3 en Linux, la integración nativa de Android, miniaudio y Box2D de forma fiable en los dispositivos objetivo?
### Decisiones que no necesito tomar todavía
- Soporte para Windows, WebAssembly y otras distribuciones o dispositivos distintos de los objetivos de prueba del MVP.
- La implementación completa del multijugador competitivo por LAN, incluidos sesiones, sincronización y desconexiones. La lógica del juego y la entrada se mantendrán separadas para facilitar su incorporación posterior.
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
