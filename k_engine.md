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
Yo, un usuario intermedio de C++ y estuxiasta de la programacion
, con 3 años escribiendo a diario C++.

---

## 2. Objetivos del proyecto

### Objetivo a largo plazo
Quiero que permita realizar juegos comerciales o para mi, usando C++ como
lenguaje principal.

### Objetivo del primer MVP
Para el editor, Abrir una ventana con GLFW3, inicializar el pipeline básico de Vulkan para renderizar un sprite/textura 2D en pantalla desde un contenedor binario personalizado, y permitir cerrar la aplicación de forma limpia. Y, para el editor, Crear una herramienta GUI accesible  que permita colocar baldosas (tiles) en una cuadrícula 2D, definir una secuencia de fotogramas (spritesheet) para una animación básica, y exportar/guardar todo en el formato JSON que el motor C++ pueda cargar e interpretar. Desarrollar una CLI en Python que tome los archivos fuente del editor (niveles JSON y recursos multimedia raw como imágenes PNG y audios WAV), los valide, aplique compresión/estructuración básica y los empaquete en un único archivo binario contenedor (ej. .kbin / .pak) con una cabecera (header) e índice de recursos que el motor en C++ pueda leer de forma eficiente.

### Cómo sabré que el MVP está terminado

#### Nucleo
- [ ] Pila de validación limpia: La aplicación ejecuta, renderiza y se cierra sin arrojar advertencias ni errores en las Vulkan Validation Layers.

- [ ] Gestión de memoria estable: Cero fugas de memoria (memory leaks) ni colgados al cerrar el motor, validado mediante la gestión determinista de recursos (RAII).

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

- [ ] Cabecera e Índice (Header & Table of Contents): El archivo generado incluye una firma mágica (ej. KBIN), versión del formato, número de recursos y una tabla de desplazamiento (offsets y tamaños en bytes) para cada archivo empaquetado.

- [ ] Conversión de Niveles: Convierte la estructura lógica del editor (JSON o YAML) a un formato binario compacto (evitando procesar cadenas de texto en C++ en tiempo de ejecución).

- [ ] Inclusión Multimedia: Empaqueta imágenes (PNG/JPG) y otros assets (audio, fuentes, etc.) sin corromper sus bytes ni modificar su alineación necesaria para la carga.

- [ ] Ruta de entrada y salida: Acepta argumentos por línea de comandos para especificar el directorio de assets/proyecto del editor y la ruta del archivo .pak resultante.

- [ ] Validación de integridad: Si falta un recurso referenciado en el nivel o un archivo está corrupto, la herramienta lanza un error descriptivo en la terminal antes de empaquetar.

---

## 3. Alcance del MVP

### Debe incluir
- [ ] [Función o capacidad necesaria]
- [ ] [Función o capacidad necesaria]
- [ ] [Función o capacidad necesaria]

### No incluirá en esta primera versión
- [ ] [Función futura]
- [ ] [Función futura]
- [ ] [Plataforma o característica fuera del MVP]

### Motivo de estas prioridades
[¿Por qué estas funciones son más importantes que las excluidas?]

---

## 4. Plataformas objetivo

### Linux
- **Distribución o distribuciones objetivo:** [Por decidir]
- **Arquitectura:** [x86_64, ARM64 u otra]
- **Forma de distribución:** [Por decidir]
- **Versión mínima del sistema:** [Por decidir]
- **Dispositivos de prueba disponibles:** [Por completar]

### Android
- **Versión mínima:** Android 15 (API 35)
- **Dispositivo de prueba:** [Modelo y características, si se conocen]
- **Forma de instalación o distribución:** [APK, otra, por decidir]
- **Orientación de pantalla:** [Horizontal / vertical / ambas]
- **Métodos de entrada necesarios:** [Táctil, mando, teclado u otros]

### Plataformas futuras
[¿Hay plataformas que se quieran considerar después del MVP?]

---

## 5. El juego que utilizará el engine

**Género:**  
[Ej.: plataformas de acción]

**Descripción breve:**  
[¿Qué hace el jugador y cuál es el objetivo del juego?]

**Características centrales del juego:**
- [ ]
- [ ]
- [ ]

**Modo de juego:**  
[Individual, multijugador local, en línea u otro.]

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
