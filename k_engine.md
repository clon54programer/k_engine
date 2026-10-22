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

Anota qué debe poder hacer el engine. Marca cada punto como **necesario**, **deseable** o **futuro**.

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

Completa cada decisión con su motivo, alternativas y nivel de certeza. No hace falta cerrar ahora las que todavía no estén investigadas.

| Tema | Decisión actual | Motivo | Alternativas consideradas | Estado |
|---|---|---|---|---|
| Lenguaje y estándar | C++17 para el núcleo, el editor y el runtime | Es el estándar definido para el proyecto y permite compartir código entre Linux y Android | Otro estándar de C++ | Decidido |
| API gráfica en Linux | Vulkan | Es la API gráfica elegida para el engine, porque es mulitplataforma y no existe peligro deser deprecada como OpenGL | OpenGL | Decidido; verificar compatibilidad con la GPU y el driver del equipo |
| API gráfica en Android | Vulkan 1.1 o superior en arm64-v8a; comprobar las capacidades disponibles en ejecución | El Moto G34 5G con Snapdragon 695/Adreno 619 admite Vulkan 1.1 | Vulkan 1.0; otro backend gráfico | Decidido para el dispositivo de prueba |
| Android SDK | `minSdkVersion` 35, `targetSdkVersion` 36 y `compileSdkVersion` 36 o superior | Permite instalar en Android 15+ y apunta a Android 16 para distribución en Google Play | Revisar los valores cuando cambien los requisitos de Play | Decidido para el MVP |
| Ventanas y entrada | GLFW3 está contemplado para Linux; Para android, la api nativa | El engine debe ofrecer ventana, superficie Vulkan y entrada en ambos sistemas | SDL3 o una capa de plataforma con integración nativa en Android | Investigar antes de fijar la API común |
| Audio | Reproducción básica de WAV, efectos y música; miniaudio | El audio es necesario, pero el alcance inicial es acotado | OpenAl o el componte de audio SDL3 | Investigar |
| Física y colisiones | Movimiento 2D, colisiones y raycasting básicos; se usara Box2D, pero mediante una interfaz pura para evitar acomplamiento a la dependencia | El juego requiere combate, armas a distancia, línea de visión y carriles de profundidad | Implementación propia acotada; biblioteca de física 2D | Investigar; separar las reglas del juego del núcleo |
| Datos de niveles y animaciones | JSON editable en el editor, convertido a buffers FlatBuffers mediante esquemas `.fbs` para el runtime | Evita parsear JSON durante la ejecución y establece un formato binario definido | Cargar JSON directamente; formato binario propio | Decidido |
| Interfaz de usuario | Layouts responsive creados visualmente en el editor y convertidos a C++ que usa una API Builder propia, sin macros | Permite diseñar visualmente y mantener la UI adaptable | Layouts escritos a mano en C++; sistema inspirado en Clay sin generación de código | Decidido; falta definir los detalles del generador |
| Diálogos y lógica del juego | Se implementan directamente en C++ | No se editarán visualmente en el MVP | Editor visual de diálogos; archivos de datos interpretados en runtime | Decidido para el MVP |
| Contenedor de recursos | Un archivo `.pak` versionado con índice y entradas para recursos y buffers FlatBuffers | Agrupa los datos que consume el runtime | Archivos sueltos; otro contenedor | Decidido |
| Cifrado del índice | Cifrar el índice del `.pak` con AES-128 para ocultar su estructura; el contenido de los recursos no se cifra | El objetivo es ocultar nombres, offsets y organización de las entradas | Índice en texto claro; cifrar todo el paquete | Decidido el alcance; investigar modo, nonces y gestión de clave. AES-GCM es una opción por evaluar |
| Empaquetador | CLI en Python para validar datos, convertir JSON a FlatBuffers y generar `.pak` | Es un lenguaje conocido por el desarrollador y adecuado para la herramienta de desarrollo | Implementación en C++, Go u otro lenguaje | Decidido |
| Sistema de compilación |Cmake | Necesita soportar Linux Mint y Android mediante el NDK | Uno propio o Meson| Investigar |
| Red | La arquitectura mantendrá separadas la simulación, la presentación y la entrada; las partidas LAN completas quedan para después del MVP | Reduce el riesgo de acoplar el juego a entrada local y dificulta menos añadir multijugador | Implementar red completa desde el MVP; no considerar red hasta una fase posterior | Decidido para la preparación; modelo de autoridad y transporte por investigar |
| Dependencias externas | Vulkan SDK/capas para desarrollo, FlatBuffers, librerías de ventana y audio, y dependencia criptográfica por elegir. Todas estas librerias deben ser expuesta por interfaces puras, para evitar acomplamiento a las libreris externas | El runtime debe funcionar en Linux y Android, y el empaquetador en Python | Reducir dependencias; implementar componentes propios | Investigar y validar cada dependencia en ambos destinos |

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
