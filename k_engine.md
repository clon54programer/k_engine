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
- [ ] preparación arquitectónica para red 

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
Un juego de plataformas de acción 2D con combate hack and slash. El jugador elige 3 personajes y cambia de personaje durante el nivel. Cada personaje
puede usar hechizos comunes, hechizos especificos por naturaleza o faccion, hechizos personales,
armas de fuego y armas blancas, antes de cada nivel, puede elegir 3 tipos de armas y 12 hechizos de cualquier tipo o clasificacion, pero en un nivel, solo puede elegir dos tipos de amras y 4 hechizos, el resto se va al inventario, que sera un menu que no pausa el nivel, que puedem reemplazar a un hechizo o arma durante el nivel, al menos que el nivel exiga un arma o hechizo para su realizacion. 

En cada nivel el jugador va por un objetivo (robar o alcanzar un objecto, o eliminar a alguien), normalmente se lucha de forma lineal (con profundidad, es decir, puede haber carriles en el eje de profundidad) hasta llegar a ese objetivo, y en niveles muy especificos son sigilos, en donde
si un npc de baja rango (dependiendo de la faccion) te visualiza y logra alertar al resto falla, pero si es de alto rango, falla de forma automatica. no se puede especificar un tiempo, ya que dependera del entorno (abirto o cerrado) y la forma de comunicacion sea a voz o con un sistema de comunicaciones.

La narrativa sera importante de forma indirecta al gameplay, ya que segun las interaciones o decisiones que tengas antes de cada nivel, podras tener mas o menos recursos, o bajar o aumentar la dificultad al interactuar con el que no debias, o haber tratado mal en una ocasion anterior.

**Características centrales del juego:**
- El jugador controla un equipo de tres personajes y puede cambiar entre ellos durante el nivel. Los personajes pueden usar hechizos comunes, hechizos asociados a su naturaleza o facción, y hechizos personales.
- Antes de cada nivel, el jugador prepara hasta tres tipos de armas y doce hechizos. Durante el nivel puede seleccionar hasta dos tipos de armas y cuatro hechizos; el resto queda en el inventario y puede sustituirse desde un menú sin pausar el juego.
- Los niveles combinan plataformas de acción 2D y combate hack and slash. Algunos incluyen carriles de profundidad que afectan el movimiento y los combates, pero que no están conectados de forma continua por todo el nivel.
- Algunos niveles incluyen sigilo. Si un NPC de rango bajo detecta al jugador y consigue alertar a los demás, el nivel falla. Si lo detecta un NPC de rango alto, el nivel falla automáticamente. El tiempo para transmitir una alerta depende del entorno y del medio de comunicación, como la voz o un sistema de comunicaciones.
- Las decisiones e interacciones narrativas anteriores a un nivel pueden afectar los recursos disponibles o la dificultad de niveles posteriores

**Modo de juego:**  
Individual offline y multijugador competitivo mediante una red local, con modos 1 vs. 1 y 3 vs. 3. En el multijugador cada jugador elegi un personaje disponible del modo offline.

**¿Qué parte del juego se desarrollará para validar el MVP del engine?**   
Se desarrollará un nivel corto, individual y sin conexión a internet, con tres personajes provisionales entre los que el jugador pueda cambiar. El nivel tendrá carriles de profundidad con conexiones definidas, un enemigo, un ataque básico y un objetivo sencillo. Se usarán una cuadrícula, animaciones y recursos creados para la prueba; estos se empaquetarán y cargarán desde el runtime.

La selección completa de armas y hechizos, el inventario durante el nivel, el sistema de sigilo y el multijugador competitivo en red local quedarán fuera de esta prueba inicial.

---

## 6. Requisitos funcionales del engine


| Capacidad | Prioridad | Descripción o ejemplo |
|---|---|---|
| Renderizado 2D | Necesario | Renderizar sprites y texturas con Vulkan desde los recursos empaquetados. En Android, exigir Vulkan 1.1 o superior; verificar también los requisitos de GPU y driver en Linux. Permitir ordenar los personajes según su posición en los carriles de profundidad. |
| Ventanas y superficies | Necesario | Crear la ventana o superficie de renderizado en Linux y Android. Gestionar redimensionamiento, recreación de la superficie y ciclo de pausa/reanudación en Android. |
| Entrada | Necesario | Ofrecer acciones independientes del dispositivo. Para el MVP, admitir teclado y ratón en Linux y pantalla táctil en Android. El mando puede añadirse después. |
| Física y colisiones | Necesario para el juego; raycasting deseable después del MVP inicial | Incluir movimiento 2D básico, colisiones y detección de impactos. Añadir consultas de raycasting para comprobar impactos de armas a distancia y línea de visión. El raycast de visión solo comprueba si hay obstáculos: el campo visual, la detección del jugador y las reglas de alerta pertenecen a la lógica del juego. Definir si un raycast puede atravesar carriles de profundidad o si solo afecta al carril correspondiente. |
| Animaciones | Necesario: animación por fotogramas en el MVP; Deseable: animación esquelética en una fase posterior | El MVP permite definir secuencias desde spritesheets y previsualizarlas. En una fase posterior, añadir un sistema 2D de huesos jerárquicos, slots asociados a huesos y attachments —por ejemplo, sprites que se puedan cambiar—, con clips que animen transformaciones y cambios de attachment. |
| Interfaz de usuario | Necesario | La interfaz debe adaptarse al tamaño de ventana, resolución, orientación y escala de pantalla en Linux y Android. El sistema de layout estará inspirado en el enfoque responsive y declarativo de Clay, pero se implementará como una biblioteca C++ propia. La UI se construirá mediante una API de bajo nivel basada en el patrón Builder, sin macros, separando la descripción del layout de su renderizado. El MVP incluirá los componentes necesarios para el editor —paneles, botones, paleta de tiles, cuadrícula y controles de animación—. Más adelante, permitirá crear interfaces de juego, como el inventario que se abre sin pausar el nivel. |
| Carga de recursos | Necesario | Leer y validar recursos desde un contenedor versionado, incluyendo imágenes y datos de niveles. Informar errores descriptivos si faltan recursos o el contenedor no es compatible. |
| Carga de  niveles | Necesario | Cargar un nivel 2D con tiles, capas, objetos y carriles de profundidad. El nivel debe poder representar dónde existen carriles y dónde se permite cambiar entre ellos; las reglas de combate y sigilo pertenecen al juego. |
| Guardado de datos | Necesario | Guardar y cargar los proyectos editables del editor. El guardado de progreso, decisiones narrativas y recursos de campaña queda para una fase posterior. |
| Red | Necesario: preparación arquitectónica; Futuro: partidas multijugador completas | Mantener la simulación y el estado del juego separados del renderizado y de la entrada física. Representar las acciones como comandos que puedan proceder del jugador local o de la red, usar un paso de simulación independiente de la tasa de renderizado y asignar identificadores estables a las entidades. Antes de implementar el combate completo, realizar un prototipo que intercambie acciones o estado entre dos instancias. Las partidas LAN 1 vs. 1 y 3 vs. 3, las salas y la gestión de desconexiones quedan para una fase posterior. |
| Audio | Necesario | Cargar audio desde el contenedor y reproducir efectos de sonido y música 2D. Para el MVP, admitir archivos WAV, reproducción, pausa, reanudación, detención, repetición y control básico de volumen. Gestionar la pausa y reanudación de la aplicación en Android. No incluir audio espacial, efectos DSP ni mezcla avanzada. |
| Editor de niveles y animaciones | Necesario | Permitir crear un nivel en cuadrícula, seleccionar y colocar tiles, editar capas y crear una animación básica. Guardar el proyecto y exportar sus datos. |
| Empaquetador | Necesario | Incluir una CLI en Python que valide los recursos y niveles, detecte referencias ausentes y genere un contenedor que el runtime C++ pueda leer. |
| Otras | Deseable | Facilitar mensajes de diagnóstico útiles para errores de carga, inicialización de Vulkan y recursos incompatibles. |

---

## 7. Editor y herramientas

**¿El MVP necesita un editor visual?**  
Sí. Se usará para crear y guardar niveles, animaciones básicas e interfaces de usuario.

**¿Qué se podrá crear o modificar con el editor?**
- [x] Niveles: tilemaps en cuadrícula, capas básicas y datos necesarios para representar los carriles de profundidad.
- [x] Interfaz de usuario: diseñar interfaces responsive para el juego. El editor guardará los layouts en un formato editable; una herramienta de generación los convertirá a código C++ que construye la interfaz mediante la API Builder del engine. La lógica y los eventos del juego se implementarán en C++.
- [x] Animaciones: clips básicos creados a partir de fotogramas de una spritesheet.
- [ ] Diálogos: no habrá un editor visual de diálogos en el MVP. Si se necesitan para una prueba, su contenido se escribirá manualmente en archivos JSON.
- [x] Otros: explorador de recursos para importar, organizar y previsualizar imágenes, spritesheets y audio, y detectar recursos faltantes o referencias rotas.


**Herramientas adicionales necesarias:**  
- Generador de código C++ para convertir los layouts de interfaz del editor en código que use la API Builder.
- CLI en Python para validar y empaquetar niveles y recursos.
- Validación de referencias a recursos y errores descriptivos antes de generar código o paquetes.
---

## 8. Datos y recursos

## 8. Datos y recursos

**Tipos de recursos necesarios:**
- Imágenes y spritesheets, principalmente PNG.
- Efectos de sonido y música, inicialmente WAV.
- Fuentes y shaders compilados para Vulkan.
- Niveles y datos de animación creados visualmente en el editor.
- Layouts de interfaz creados visualmente en el editor y convertidos a código C++.
- Diálogos y lógica del juego escritos directamente en C++.

**Formato editable durante el desarrollo:**
- Los niveles, las animaciones y los layouts de interfaz se guardarán en archivos JSON, con una versión de esquema definida.
- Los archivos multimedia se conservarán en su formato original dentro del proyecto.
- Los esquemas FlatBuffers (`.fbs`) definirán el formato binario de los niveles y las animaciones.
- El editor será la fuente editable de los layouts de interfaz. El generador de código los convertirá en C++ que use la API Builder del engine.
- Los diálogos y la lógica del juego se mantendrán en archivos C++.

**Formato utilizado al ejecutar el juego:**
- El empaquetador validará los JSON de niveles y animaciones y los convertirá a buffers binarios FlatBuffers usando los esquemas `.fbs`.
- El runtime C++ utilizará el código de acceso generado por `flatc` y verificará cada buffer FlatBuffers antes de leerlo.
- Los buffers de niveles y animaciones, junto con los recursos multimedia, se incluirán como entradas de un archivo `.pak` versionado.
- El código C++ generado a partir de los layouts de interfaz se compilará con el juego; no se incluirá como layout dentro del `.pak`.

**¿Se necesita agrupar o comprimir recursos?**
- Sí. El empaquetador agrupará los recursos y los buffers FlatBuffers en un archivo `.pak`.
- No se aplicará compresión adicional a PNG, JPG ni a otros formatos que ya estén comprimidos.
- Se podrá comprimir otros datos si las pruebas muestran una reducción útil del tamaño. El índice registrará el método de compresión y los tamaños comprimido y original de cada entrada.
- El empaquetador validará las referencias a recursos, los offsets y los tamaños antes de generar el paquete.

**¿Se necesita cifrar recursos?**
El índice del `.pak` se cifrará con AES-128-GCM para ocultar su estructura, incluidas las entradas y sus ubicaciones. El encabezado inicial permanecerá sin cifrar y contendrá únicamente la información necesaria para identificar el formato y localizar el índice cifrado. El encabezado se autenticará junto con el índice.

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
| Lenguaje y estándar | C++17 para el núcleo, el editor y el runtime | Es el estándar elegido para compartir código entre Linux y Android | Otros estándares de C++ | Decidido |
| GPU Linux de referencia | AMD Radeon Graphics, dispositivo `1002:15e7`, con Linux Mint 22.2 Xfce; pantalla de 1366 × 768 a 60 Hz bajo X11 | Define el hardware y el entorno principal para probar el runtime Linux | Otras GPU y entornos de pantalla | Hardware de prueba identificado; capacidad Vulkan por verificar |
| API gráfica en Linux | Vulkan; comprobar la versión y el controlador disponibles en la GPU de referencia | Se busca compartir la API gráfica con Android y usar un backend explícito | OpenGL; otro backend | Decidido; comprobar soporte y presentación de superficies en Linux |
| API gráfica en Android | Vulkan 1.1 o superior en arm64-v8a; consultar las capacidades del dispositivo en tiempo de ejecución | El Moto G34 5G con Snapdragon 695/Adreno 619 admite Vulkan 1.1 | Vulkan 1.0; otro backend | Decidido para el dispositivo de prueba |
| Android SDK | `minSdkVersion` 35, `targetSdkVersion` 36 y `compileSdkVersion` 36 o superior | Permite instalar en Android 15 o superior y cumplir el requisito actual de Google Play para nuevas apps y actualizaciones | Revisar cuando cambien los requisitos de Google Play | Decidido para el MVP |
| Ventanas y entrada | La lógica del juego y el editor usan únicamente la API pública del núcleo para ventanas y entrada. El núcleo selecciona el backend de plataforma: GLFW3 en Linux y las APIs nativas de Android en Android. | Evita que el juego y el editor dependan directamente de una librería o sistema operativo, y permite cambiar los backends sin modificar su lógica. | SDL3 como backend común; otros backends por plataforma | Decidido como enfoque; validar la implementación de ambos backends |
| Audio | miniaudio para reproducción básica de WAV, efectos y música | Mantiene acotado el alcance de audio del MVP | OpenAL; audio de SDL3 | Investigar mediante un prototipo en Linux y Android |
| Física y colisiones | Box2D detrás de una interfaz propia del engine; incluir movimiento, colisiones y raycasting básicos | Evita que la lógica del juego dependa directamente de Box2D y permite implementar las consultas necesarias para combate y visión | Física propia acotada; otra biblioteca 2D | Decidido para evaluación; comprobar cómo se integra con los carriles de profundidad |
| Datos de niveles y animaciones | JSON editable convertido a buffers FlatBuffers mediante esquemas `.fbs` | Evita parsear JSON durante la ejecución y permite definir un formato binario versionado | Cargar JSON directamente; formato binario propio | Decidido |
| Interfaz de usuario | Layouts responsive creados visualmente en el editor y convertidos a C++ mediante una API Builder propia, sin macros | Permite diseñar interfaces en el editor y adaptar su distribución a distintas pantallas | Layouts escritos a mano en C++; sistema inspirado en Clay sin generación de código | Decidido; falta definir el generador |
| Diálogos y lógica del juego | Se implementan directamente en C++ | No se editarán visualmente en el MVP | Editor visual de diálogos; archivos de datos interpretados en runtime | Decidido para el MVP |
| Contenedor de recursos | Archivo `.pak` versionado con recursos y buffers FlatBuffers; índice cifrado con AES-128 para ocultar la estructura del paquete | Agrupa los datos del runtime y oculta la información del índice | Archivos sueltos; cifrar todo el paquete | Decidido el objetivo (AES-128-GCM); gestión de clave y nonce por investigar|

---

## 10. Requisitos de calidad


| Requisito | Prioridad | Cómo comprobarlo |
|---|---|---|
| Rendimiento | Imprescindible | Ejecutar la misma escena de prueba en Linux Mint 22.2 y en el Moto G34 5G. Objetivo inicial: 60 FPS; registrar los tiempos de fotograma y comprobar que no haya caídas sostenidas durante una prueba de 5 minutos. |
| Tiempo de carga | Deseable | Medir por separado el arranque en frío y la carga de un nivel de prueba en ambos destinos. Como objetivo provisional, comprobar si el nivel se carga en menos de 3 segundos y ajustar el umbral con los resultados reales. |
| Estabilidad | Imprescindible | Ejecutar la escena durante 30 minutos y repetir cargas, cierres, pausas y reanudaciones en Android. No debe haber cierres inesperados ni errores de Vulkan causados por el núcleo. |
| Tamaño de la aplicación | Deseable | Registrar el tamaño del APK y del `.pak` en una compilación de distribución. Fijar un límite cuando haya una muestra representativa de los recursos del juego. |
| Facilidad para añadir contenido | Imprescindible | Crear un nivel y una animación básicos desde el editor, empaquetarlos y cargarlos sin modificar el código del núcleo. |
| Compatibilidad entre Linux y Android | Imprescindible | Compilar y ejecutar la misma prueba funcional en Linux Mint 22.2 x86_64 y en el Moto G34 5G con Android 15 o superior, verificando Vulkan 1.1 en el dispositivo. |
| Facilidad de depuración | Imprescindible | En una compilación de desarrollo, registrar los errores de Vulkan causados por el núcleo. Simular la ausencia de un recurso y comprobar que el mensaje identifique el recurso y la operación fallida. |

---

## 11. Pruebas y validación

**Dispositivos y sistemas disponibles para probar:**
- Linux Mint 22.2 Xfce (Zara), x86_64. GPU AMD Radeon Graphics (`1002:15e7`), pantalla de 1366 × 768 a 60 Hz y sesión X11.
- Motorola Moto G34 5G, Android 16, arquitectura arm64-v8a. Versión mínima de Android prevista: 15 (API 35).

**Qué debe probarse en Linux:**
- Compilar y ejecutar el editor y el runtime en Linux Mint 22.2.
- Verificar que Vulkan detecte la GPU AMD y pueda crear una superficie de presentación y renderizar.
- Abrir un nivel y cargar desde el `.pak` una textura, una animación y datos FlatBuffers.
- Probar el redimensionamiento de la ventana y el cierre limpio.
- Comprobar el renderizado y el layout del editor en la resolución nativa de Linux (1366 × 768).
- Medir el rendimiento con una escena de referencia y una resolución objetivo definida para cada plataforma.

**Qué debe probarse en Android 15+:**
- Instalar y ejecutar el APK arm64-v8a en el Moto G34 5G.
- Consultar en tiempo de ejecución la versión de Vulkan y las capacidades que expone el driver.
- Cargar y renderizar los recursos del `.pak`, reproducir audio WAV y probar la entrada táctil.
- Probar pausa, reanudación y recreación de la superficie al cambiar de aplicación o volver a ella.
- Medir el rendimiento y el tiempo de carga de la escena de referencia en el dispositivo.

**Pruebas manuales necesarias:**
- Crear y guardar un nivel y una animación en el editor; volver a abrirlos y comprobar que conservan sus datos.
- Generar el código C++ de una interfaz, compilarlo e inspeccionar su layout en distintos tamaños de ventana.
- Empaquetar el contenido, cargarlo desde el runtime y confirmar que el índice permite localizar los recursos.
- Probar cambios de carril, colisiones y raycasting en la escena de prueba.
- Comprobar los mensajes del núcleo cuando Vulkan no está disponible o falta un recurso.
- Tras definir el modo de cifrado, probar que el runtime pueda leer un índice válido y rechace uno alterado.

**Pruebas automatizadas necesarias:**
- Verificar que el empaquetador rechace recursos inexistentes, referencias inválidas y archivos de entrada corruptos.
- Verificar la serialización y lectura de niveles y animaciones FlatBuffers, incluidos buffers truncados o inválidos.
- Probar el lector del `.pak` con índices y offsets válidos e inválidos.
- Probar colisiones y raycasting a través de la interfaz del núcleo, sin exponer tipos de Box2D.
- Comprobar que el núcleo pueda inicializarse, cargar una escena básica y cerrarse sin errores.

**Problemas conocidos o riesgos:**
- El reporte actual de la GPU Linux muestra OpenGL, pero no confirma la versión ni la disponibilidad de Vulkan, aunque con el modelo de la cpu, la version maxima es de 1.3. Verificarlo con `vulkaninfo --summary` y probar la presentación en X11.
- El Moto G34 5G es el único dispositivo Android de prueba disponible; probarlo no demuestra compatibilidad con todos los dispositivos Android 15 o superiores.
- Falta definir la generación, el almacenamiento y la protección de la clave y del nonce para el índice del .pak (AES-128-GCM); las pruebas de cifrado e integridad dependerán de esa decisión.

---

## 12. Plan de desarrollo

| Etapa | Resultado esperado | Criterio para darla por terminada |
|---|---|---|
| 1. Investigación y decisiones técnicas | Decisiones documentadas sobre Vulkan en Linux y Android, capa de plataforma, audio, Box2D, FlatBuffers y cifrado del índice del `.pak`. | Hay pruebas o documentación para las decisiones principales. Se verifica Vulkan en Linux con `vulkaninfo --summary` y en el Moto G34 5G. |
| 2. Prototipo técnico multiplataforma | Aplicaciones de prueba para Linux y Android que crean la superficie Vulkan, reciben entrada y renderizan un sprite. | El prototipo funciona en Linux Mint 22.2 x86_64 y se instala y ejecuta en el Moto G34 5G con Android 16. En Android se prueba también la pausa y reanudación. |
| 3. Funciones esenciales del núcleo | API pública del núcleo, backend de plataforma, renderizado 2D, entrada, audio básico, colisiones y raycasting, animación por fotogramas y diagnóstico de Vulkan. | El runtime carga y muestra una escena de prueba; reproduce audio WAV; y puede cerrarse o reanudarse sin errores atribuibles al núcleo. |
| 4. Editor y generación de código | Editor visual para niveles, animaciones y layouts responsive de interfaz. Generador de C++ para los layouts. | Se pueden editar y guardar los datos, generar el C++, compilarlo con el juego y abrir el resultado. Los errores de generación indican qué dato debe corregirse. |
| 5. Empaquetador y formatos de ejecución | CLI en Python que valida los datos, convierte niveles y animaciones JSON a FlatBuffers y crea el `.pak` con el índice cifrado. | El runtime valida y abre el índice, carga los recursos y lee los buffers FlatBuffers. El empaquetador rechaza referencias ausentes y archivos inválidos. |
| 6. Prueba con contenido del juego | Nivel offline corto con tres personajes provisionales, cambio entre personajes, carriles de profundidad, un enemigo, un ataque y un objetivo sencillo. | El contenido se crea con las herramientas previstas, se empaqueta y se ejecuta en Linux y Android. Esta prueba no requiere implementar todo el inventario, el sigilo ni el multijugador. |
| 7. Validación y entrega del MVP | Versiones ejecutables para Linux y Android, junto con instrucciones para probarlas. | Se completan las pruebas definidas para ambos dispositivos, se revisan los errores de Vulkan y se verifica el cierre limpio del runtime. |


**Horas disponibles por semana:**  
25 horas a al semana (martes, miercoles, jueves y viernes)

**Fecha objetivo:**  
1 de diciembre de 2026

**Qué significa “terminado” en esa fecha:**  
El editor permite crear el contenido básico de la prueba; el empaquetador produce un `.pak` válido; y el runtime carga ese paquete y ejecuta el nivel de prueba en Linux Mint 22.2 y en el Moto G34 5G. Las funciones excluidas del MVP se especificarán en la sección de alcance.


---

## 13. Suposiciones y preguntas abiertas

### Suposiciones actuales
- El PC de pruebas usa Linux Mint 22.2 Xfce x86_64 y un AMD Ryzen 3 7330U con Radeon Graphics integrada (dispositivo PCI `1002:15e7`). Vulkan en Linux está pendiente de verificación.
- La API pública del núcleo ocultará los detalles de plataforma. La lógica del juego y el editor no dependerán directamente de GLFW ni de las APIs nativas de Android.
- El editor guardará niveles, animaciones y layouts de interfaz en formatos editables. Los niveles y las animaciones se convertirán a FlatBuffers; los layouts de interfaz generarán código C++.
- El empaquetador incluirá los recursos y los datos FlatBuffers en un `.pak`. Se cifrará el índice para ocultar la estructura del paquete, pero no los recursos multimedia.
- Los diálogos y la lógica del juego se implementarán directamente en C++ durante el MVP.

### Preguntas que necesito resolver
- ¿Qué versión de Vulkan y qué controlador están disponibles realmente en la GPU Linux? El reporte actual confirma OpenGL, pero no Vulkan.
- ¿Cómo se generará, almacenará y (si aplica) rotará la clave AES-128 y el nonce único de cada paquete? El modo ya está fijado: AES-128-GCM.
- ¿Cómo se representarán en Box2D los carriles de profundidad y sus conexiones, y cuándo podrá un raycast alcanzar objetos de otro carril?
- ¿Funcionan GLFW3 en Linux, la integración nativa de Android, miniaudio y Box2D de forma fiable en los dispositivos objetivo?
- ¿Cómo se generará y compilará el C++ producido por el editor junto con el juego en Linux y Android?

### Decisiones que no necesito tomar todavía
- Soporte para Windows, WebAssembly y otras distribuciones o dispositivos distintos de los objetivos de prueba del MVP.
- La implementación completa del multijugador competitivo por LAN, incluidos sesiones, sincronización y desconexiones. La lógica del juego y la entrada se mantendrán separadas para facilitar su incorporación posterior.
- El editor visual de diálogos y la implementación completa de sus ramas y consecuencias.
- Las funciones avanzadas de audio y la animación esquelética completa con huesos, slots y attachments.
- La distribución comercial, la publicación en tiendas y la contratación de servidores.

---

## 14. Contexto del desarrollador

**Experiencia relevante:**  
Tengo mas de 3 años con C++ y más de 2 años con Python. He creado prototipos con raylib en C++ y con Godot usando GDScript. Tengo poca experiencia en programación gráfica: he realizado algunas pruebas superficiales con OpenGL, pero todavía no tengo una base sólida en Vulkan.

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
