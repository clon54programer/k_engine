# PLANTILLA DE PLANIFICACIÓN GAME ENGINE 2D MULTIJUGADOR

## PLANTILLA PARA EL ESBOZO DE UN GAME ENGINE MVP 2D multijugador C++ Vulkan Windows, Linux y Android

Una estructura rellenable para ordenar ideas, detectar incongruencias y preparar tareas verificables para OpenCode y otros agentes de IA.

**CÓMO USARLA:** Completa primero las secciones 1 a 6. No intentes llenar todo en una sola sesión. Las notas azules son recomendaciones, las naranjas señalan riesgos o incongruencias y las amarillas indican decisiones pendientes.

| Campo | Valor |
| :--- | :--- |
| **Nombre del proyecto** | |
| **Autor** | Juan Danilo Chamorro Pérez |
| **Versión del documento** | 0.1 |
| **Fecha objetivo del MVP** | 1 de diciembre de 2026 |
| **Estado** | Borrador |
| **Repositorio** | |

*Documento de trabajo Completar, revisar y versionar*

---

## Guía rápida y leyenda

*   **RECOMENDACIÓN:** Una práctica o estructura aconsejada para reducir complejidad, retrabajo o ambigüedad.
*   **INCONGRUENCIA O RIESGO:** Una contradicción, dependencia dudosa o decisión que puede impedir llegar al MVP.
*   **DECISIÓN PENDIENTE:** Un punto que debe resolverse y registrarse antes de delegar una implementación a un agente.
*   **CRITERIO VALIDADO:** Una definición suficientemente concreta para implementarse y probarse.

### Orden sugerido de llenado

*   [ ] Visión y restricciones
*   [ ] Demostración vertical del MVP
*   [ ] Alcance incluido y excluido
*   [ ] Arquitectura y dependencias
*   [ ] Plataformas y Android
*   [ ] Networking
*   [ ] Roadmap
*   [ ] Reglas para agentes de IA
*   [ ] Editor, empaquetador y funciones post-MVP

> **INCONGRUENCIA INICIAL:** La meta de diciembre debe describir una demostración ejecutable, no "el motor completo". Editor avanzado, scripting visual, cifrado, animación avanzada y herramientas generales deben quedar fuera del MVP salvo que sean indispensables para la demo.

---

## 1. Visión y propósito

### Nombre del motor
`K_engine`

### Descripción en cinco líneas
*Qué permite hacer, para quién y con qué límites.*
> K_engine es un motor 2D de uso interno, diseñado para desarrollar dos juegos ambientados en un mismo mundo ficticio. Combinará programación directa de la lógica en C++ estándar con herramientas visuales para crear niveles, interfaces y animaciones. Los juegos integraran narrativa de novela visual con mecánicas de plataformas y acción. El motor no ofrecerá renderizado ni herramientas de producción 3D.

### Problema específico que resuelve para el juego
> El proyecto busca evitar la fricción entre el código del juego y las herramientas de contenido. La lógica se escribirá en C++ estándar mediante una API directa, sin exigir macros invasivas, generación manual de enlaces ni una variante del lenguaje específica del motor. Los niveles, interfaces y animaciones podrán crearse visualmente y cargarse desde formatos definidos por el proyecto. La integración con CMake deberá ser reproducible y requerir la menor configuración específica posible.

### Razón para construir un motor propio en lugar de usar uno existente
> Se construirá un motor propio para conservar control directo sobre la lógica en C++, adaptar las herramientas a las necesidades de los dos juegos y cumplir el objetivo personal de desarrollar un game engine. El proyecto se tratará como tecnología interna reutilizable entre esos juegos, no como un motor general destinado a terceros.

### Usuario principal y flujo de trabajo
> El usuario principal será el autor de los juegos, con experiencia en C++, Python y Rust. OpenCode y otros agentes de IA se utilizarán principalmente para implementar tareas delimitadas del motor, generar pruebas y apoyar la documentación. La lógica del juego se escribirá y revisará directamente en C++; la IA apoyará la planificación y la descomposición de tareas, pero no decidirá cambios de arquitectura sin aprobación.

### Usuarios secundarios (por ejemplo, betatesters)
> Los usuarios secundarios serán betatesters o colaboradores sin experiencia en C++ ni en gestión manual de memoria. Utilizarán únicamente herramientas limitadas del editor para configurar escenas, ejecutar pruebas y modificar parámetros expuestos. No tendrán acceso directo a la arquitectura interna, la gestión de memoria ni las APIs de bajo nivel.

> **RECOMENDACIÓN:** Formula la visión alrededor del juego que quieres terminar. Evita promesas como "servir para cualquier juego 2D", porque obligan a generalizar sistemas antes de conocer una necesidad real.
>
> **DECISIÓN PENDIENTE:** Define si el proyecto es un motor reutilizable o una tecnología interna para un solo juego. Esa decisión cambia el nivel de abstracción, documentación y estabilidad esperado.

---

## 2. Restricciones reales

| Restricción | Respuesta |
| :--- | :--- |
| **Horas disponibles por semana** | 4 días desde las 2:30 pm hasta 8:00 pm |
| **Nivel actual de C++** | Intermedio, tengo experiencia principal en $C++17$ |
| **Experiencia con Vulkan** | No |
| **Experiencia con Android NDK** | NO |
| **Dispositivos Android de prueba** | 1, Mi teléfono personal |
| **Presupuesto para servicios/servidores** | 20 dólares, pero se debe compartir con las api de los llms que voy a usar, principalmente los modelos de DeepSeek en su platform. |
| **Fecha de congelación del alcance** | Hasta junio de 2027 |
| **Fecha de demostración** | 1 de diciembre de 2026 |

> **INCONGRUENCIA O RIESGO:** Usar IA acelera la escritura, pero no elimina integración, depuración, pruebas multiplataforma ni decisiones arquitectónicas. El roadmap debe reservar capacidad para validar código generado y revertir cambios defectuosos.

---

## 3. Definición verificable del MVP

### 3.1 Frase de éxito
*El MVP estará terminado cuando sea posible... Describe una acción observable de principio a fin.*

> El MVP de K_engine estará terminado cuando permita crear, compilar y ejecutar una experiencia narrativa 2D offline en la que el jugador pueda recorrer una escena, interactuar con personajes, avanzar por diálogos, tomar una decisión y producir un cambio verificable en la historia. La demostración deberá funcionar en Windows, Linux y Android utilizando contenido creado con las herramientas del proyecto.
> 
> La capa de red será un módulo adicional del motor para las funciones específicas que requieran comunicación entre jugadores. La experiencia narrativa principal deberá funcionar completamente offline y no dependerá de servidores, cuentas ni conexión a Internet.

### 3.2 Demostración vertical

#### Nombre de la demo
`Primera demostración`

#### Flujo de la demo
*Ejemplo: abrir, conectarse, cargar escena, mover dos jugadores, reproducir audio y cerrar.*

1. El jugador inicia el juego sin conexión a Internet.
2. El juego carga un menú principal.
3. El jugador selecciona una partida nueva.
4. Se carga una escena 2D desde los recursos procesados por el motor.
5. El personaje puede desplazarse por la escena.
6. La cámara sigue al personaje dentro de los límites definidos.
7. El jugador colisiona correctamente con el escenario.
8. El jugador interactúa con un personaje.
9. Se abre una interfaz de diálogo con:
   * a. nombre del personaje;
   * b. texto;
   * c. retrato o imagen;
   * d. controles para avanzar.
10. El diálogo presenta al menos dos opciones.
11. El jugador selecciona una opción.
12. La elección modifica una variable narrativa.
13. El árbol de diálogo continúa por una rama diferente según la opción seleccionada.
14. La decisión produce una consecuencia observable, como:
    * cambiar el siguiente diálogo;
    * activar un evento;
    * habilitar o bloquear una interacción;
    * modificar el estado de un personaje;
    * cambiar de escena.
15. El juego reproduce música y efectos de sonido durante la secuencia.
16. El jugador guarda la partida.
17. El jugador cierra y vuelve a abrir el juego.
18. Al cargar la partida, la escena y el estado narrativo se restauran correctamente.
19. La demostración finaliza sin errores críticos.

#### Networking
> Dos clientes cargan una escena de prueba, se conectan a un servidor y sincronizan la posición de dos entidades. Esta prueba valida exclusivamente la integración de red y no forma parte del recorrido narrativo principal del MVP.

#### Criterios de éxito medibles

##### Compilación y plataformas
* [ ] El proyecto genera una compilación Release documentada para Windows.
* [ ] El proyecto genera una compilación Release documentada para Linux.
* [ ] El proyecto genera una compilación instalable para Android.
* [ ] La demostración puede iniciarse sin conexión a Internet.
* [ ] El juego no requiere componentes del editor para ejecutarse.

##### Escenas y jugabilidad
* [ ] El menú principal permite iniciar y cargar una partida.
* [ ] La escena narrativa se carga desde un archivo creado por las herramientas del proyecto.
* [ ] El jugador puede desplazarse mediante teclado en escritorio.
* [ ] El jugador puede desplazarse mediante controles táctiles en Android.
* [ ] La cámara 2D sigue correctamente al personaje.
* [ ] Los colliders impiden atravesar los obstáculos incluidos en la demostración.
* [ ] El jugador puede activar una interacción cercana.

##### Sistema narrativo
* [ ] El sistema muestra el nombre, retrato y texto del personaje.
* [ ] El jugador puede avanzar manualmente por el diálogo.
* [ ] El diálogo puede presentar al menos dos opciones.
* [ ] La opción seleccionada se almacena como estado narrativo.
* [ ] El sistema selecciona una rama distinta según el estado de la historia.
* [ ] Una decisión produce al menos una consecuencia observable en la escena.
* [ ] Los diálogos y sus condiciones se cargan desde datos externos, sin recompilar el motor.
* [ ] Una referencia inválida dentro del diálogo genera un error comprensible en el logging.

##### Recursos y presentación
* [ ] La escena carga sprites, retratos, fuentes, música y efectos desde el administrador de recursos.
* [ ] Los recursos faltantes generan un error identificable.
* [ ] La interfaz narrativa se adapta a las resoluciones contempladas por la demostración.
* [ ] El texto sigue siendo legible en escritorio y Android.

##### Guardado y persistencia
* [ ] Se puede guardar el estado de la partida.
* [ ] El archivo incluye la escena actual y las variables narrativas necesarias.
* [ ] Se puede cerrar y volver a abrir el juego.
* [ ] La carga restaura la escena y la rama narrativa correcta.
* [ ] Un archivo de guardado inválido no provoca el cierre inesperado del juego.

##### Estabilidad
* [ ] La demostración se puede completar de principio a fin.
* [ ] No se producen errores críticos durante el recorrido principal.
* [ ] El cierre libera correctamente los subsistemas utilizados.
* [ ] Los fallos relevantes aparecen en el sistema de logging.
* [ ] La prueba puede repetirse desde una compilación limpia siguiendo instrucciones documentadas.

> **RECOMENDACIÓN:** La demo mínima recomendada es: dos procesos se conectan a un servidor, cargan una escena 2D, muestran sprites y sincronizan movimiento básico. Android puede validarse con un cliente físico conectado al mismo servidor.

### 3.3 Alcance del MVP

### 3.3 Alcance del MVP

| Sistema | Clasificación | Criterio mínimo |
| :--- | :---: | :--- |
| **Aplicación y logging** | Obligatorio | Iniciar y cerrar correctamente; registrar errores relacionados con recursos, escenas, diálogos y archivos de guardado. |
| **Render 2D** | Obligatorio | Renderizar sprites con transparencia, orden por capas, cámara 2D y viewport adaptable a las resoluciones admitidas. |
| **Input** | Obligatorio | Procesar teclado en Linux y controles táctiles básicos en Android mediante una API unificada. |
| **Gestión de recursos** | Obligatorio | Cargar texturas, retratos, fuentes, música, efectos, escenas y datos narrativos; detectar referencias ausentes. |
| **Escenas** | Obligatorio | Cargar y cambiar escenas; restaurar la escena activa, el punto de aparición y los cambios persistentes. |
| **Interacción** | Obligatorio | Detectar una entidad interactuable cercana, mostrar una indicación e iniciar una acción o diálogo al recibir input. |
| **UI narrativa** | Obligatorio | Mostrar nombre, retrato, texto, avance y opciones de respuesta de forma legible en Linux y Android. |
| **Diálogos** | Obligatorio | Cargar diálogos desde archivos externos, avanzar por sus nodos y detectar referencias inválidas. |
| **Decisiones narrativas** | Obligatorio | Presentar al menos dos opciones, registrar la selección y continuar por la rama correspondiente. |
| **Estado narrativo** | Obligatorio | Leer y modificar variables booleanas, enteras y de texto utilizadas por condiciones, diálogos y eventos. |
| **Consecuencias narrativas** | Obligatorio | Permitir que una decisión modifique al menos un diálogo posterior, evento, interacción, entidad o cambio de escena. |
| **Guardado y carga** | Obligatorio | Guardar la escena activa, el punto de aparición y las variables narrativas; restaurar el estado después de reiniciar el juego. |
| **Física y colisiones 2D** | Obligatorio | Gestionar obstáculos estáticos, collider del jugador, triggers y detección de contactos mediante una biblioteca externa. |
| **Física dinámica** | Condicional | Permitir que al menos un cuerpo dinámico interactúe con el escenario y el jugador, únicamente si la demostración lo necesita. |
| **Audio** | Obligatorio | Reproducir música y al menos un efecto de sonido; proporcionar control básico de volumen. |
| **Animación 2D** | Condicional | Reproducir animaciones básicas mediante sprites para reposo, movimiento o interacción, si la demostración las utiliza. |
| **Linux** | Obligatorio | Generar un build Release reproducible y ejecutar la demostración narrativa completa. |
| **Android** | Obligatorio | Generar un build instalable; ejecutar la demostración offline, procesar controles táctiles y restaurar una partida guardada en el dispositivo de prueba. |
| **Windows** | No bloqueante | Generar un build Release ejecutable; los problemas exclusivos de Windows no impedirán aprobar el MVP principal. |
| **Editor mínimo** | Obligatorio | Crear o abrir una escena; posicionar entidades; modificar transformaciones; asignar sprites, colliders y diálogos; guardar y validar referencias. |
| **Empaquetado** | Obligatorio | Validar referencias, generar un manifiesto versionado y copiar o agrupar los recursos necesarios para distribuir la demostración. |
| **Networking** | Post-MVP | Implementar un único modelo inicial para conectar dos clientes y replicar dos entidades en una escena de prueba independiente. |

#### Decisiones técnicas relacionadas con el alcance

- **Input de escritorio:** GLFW será la implementación inicial para Linux y Windows.
- **Input en Android:** se implementará mediante la capa de plataforma correspondiente, conservando una API de input común.
- **Física y colisiones:** se utilizará Box2D u otra biblioteca externa compatible con las plataformas objetivo.
- **Audio:** se utilizará una biblioteca externa con soporte para Linux, Android y, posteriormente, Windows.
- **Networking:** no será necesario para aprobar el MVP narrativo. Su diseño e implementación se realizarán después de validar la experiencia offline.
- **Empaquetado:** el MVP no incluirá cifrado, compresión avanzada, actualizaciones incrementales ni protección contra extracción.
- **Plataforma de referencia:** Linux será la plataforma principal de desarrollo y validación.
- **Plataforma móvil obligatoria:** Android deberá ejecutar la demostración narrativa en el dispositivo físico disponible.
- **Compatibilidad adicional:** Windows será importante, pero no bloqueará la aprobación del MVP principal.

#### Incluido en el editor mínimo

- Crear, abrir y guardar escenas.
- Crear y eliminar entidades.
- Modificar posición, rotación y escala.
- Asignar sprites.
- Configurar colliders y triggers.
- Asignar un diálogo a una entidad.
- Configurar el punto de aparición del jugador.
- Validar referencias a recursos y diálogos.
- Mostrar errores de validación comprensibles.

#### Fuera del editor mínimo

- Editor gráfico completo de árboles de diálogo.
- Editor avanzado de animaciones.
- Editor visual completo de UI.
- Scripting visual.
- Sistema avanzado de prefabs.
- Editor de partículas.
- Editor de shaders.
- Sistema público de plugins.


#### Regla de validación del alcance
El alcance del MVP se considerará válido cuando cada sistema marcado como obligatorio sea utilizado por la demostración narrativa y cada paso obligatorio de la demostración esté respaldado por uno o más sistemas incluidos en esta sección.

Los sistemas condicionales solo pasarán a ser obligatorios cuando exista una acción concreta de la demostración que dependa de ellos.

Las funciones clasificadas como no bloqueantes o post-MVP no impedirán aprobar el MVP narrativo.

### 3.4 Fuera del MVP

* [x ] Scripting visual completo
* [ ] Cifrado de recursos
* [ ] Editor avanzado de UI
* [ ] Editor avanzado de animaciones
* [x ] Árboles de diálogo generales
* [x ] Partículas y postprocesado
* [x ] Rollback, predicción compleja y lag compensation
* [ ] Sistema público de plugins
* [x ] Física propia
* [x ] Soporte 3D completo
LA **x** significa que nunca se implementaran despues del MVP.

> **INCONGRUENCIA O RIESGO:** El scripting visual destinado solo a betatesters no justifica construir un Blueprint completo antes de diciembre. Una consola de comandos, archivo de acciones o panel de pruebas puede cumplir temporalmente la misma función.

---

## 4. Arquitectura y límites

### 4.1 División principal

| Parte | Responsabilidad | No debe hacer |
| :--- | :--- | :--- |
| **Núcleo/runtime** | Ejecutar juego, escenas y subsistemas | Depender del editor |
| **Editor** | Crear y validar contenido | Formar parte obligatoria del runtime |
| **Empaquetador** | Procesar, validar y agrupar recursos | Editar escenas o compilar lógica |
| **Servidor** | Mantener autoridad de partida | Depender de render o editor |
| **Juego de prueba** | Validar capacidades del motor | Definir APIs internas del motor |

### 4.2 Reglas de dependencia

*  El juego depende de la API pública del núcleo.
*  El núcleo no depende del editor ni del empaquetador.
*  Los tipos de Vulkan no salen del módulo de render.
*  Las diferencias de plataforma se aíslan detrás de interfaces.
* Los formatos de escena y red tienen versión.
*  Cada módulo declara dependencias permitidas y prohibidas.

### Matriz inicial de dependencias entre módulos

| Módulo | Dependencias permitidas | Dependencias prohibidas |
| :--- | :--- | :--- |
| **Core** | Biblioteca estándar de C++ | Platform, Renderer, Input, Resources, Scene, Narrative, SaveSystem, Audio, Physics, Editor, Packer y Networking |
| **Platform** | Core y APIs específicas del sistema operativo | Renderer, Scene, Narrative, SaveSystem, Editor y Packer |
| **Renderer** | Core, Platform y Resources | Narrative, SaveSystem, Networking y Editor |
| **Input** | Core y Platform | Renderer, Scene, Narrative, SaveSystem y Editor |
| **Resources** | Core, Platform y Serialization | Renderer, Scene, Narrative, Editor y Networking |
| **Serialization** | Core | Renderer, Input, Scene, Narrative, Editor y APIs específicas de plataforma |
| **Scene** | Core, Resources, Serialization y componentes compartidos | Editor, Packer y APIs específicas de plataforma |
| **Narrative** | Core, Resources y Serialization | Renderer, Editor, Vulkan, GLFW y Networking |
| **SaveSystem** | Core, Scene, Narrative y Serialization | Renderer, Editor, Vulkan, GLFW y Packer |
| **Audio** | Core, Platform y Resources | Narrative, Scene, Editor y Networking |
| **Physics** | Core | Narrative, Resources, Editor y APIs específicas de plataforma |
| **Editor** | Core, Renderer, Input, Resources, Scene, Narrative y Serialization | Detalles internos de Vulkan, Packer y ejecutable final del juego |
| **Packer** | Core, Serialization, formatos compartidos y validadores | Renderer, Input, Audio, Physics, Narrative y Editor |
| **Networking** | Core, Platform y Serialization | Renderer, Editor y Narrative |

> **RECOMENDACIÓN:** Evita un EventBus global para toda comunicación. Úsalo solo para eventos claramente definidos; para dependencias esenciales, prefiere interfaces explícitas.

### 4.3 Ficha de módulo (copiar por cada módulo)

* **Nombre del módulo:** 
* **Responsabilidad única:** 
* **Entradas:** 
* **Salidas:** 
* **Dependencias permitidas:** 
* **Dependencias prohibidas:** 
* **API pública:** 
* **Errores esperados:** 
* **Pruebas requeridas:** 
* **Incluido en el MVP:** Sí / No

---

## 5. Decisiones técnicas

| Área | Decisión actual | Alternativas | Justificación | Riesgo |
| :--- | :--- | :--- | :--- | :--- |
| **Lenguaje** | C++17 | | | |
| **Gráficos** | Vulkan mediante abstracción | | | |
| **Ventana/input** | GLFW | | | |
| **Física** | Box2D u otra librería | | | |
| **Audio** | | | | |
| **Red** | | | | |
| **Serialización** | JSON editable + binario runtime | | | |
| **Build** | CMake | | | |
| **Editor UI** | | | | |
| **Empaquetador** | C++ u otro lenguaje | | | |

> **DECISIÓN PENDIENTE:** $C++17$ frente a $C++20$ debe resolverse con criterios de compatibilidad de compiladores, Android NDK, librerías y necesidades reales. No cambies de estándar solo porque un agente produzca código más moderno.
>
> **INCONGRUENCIA O RIESGO:** GLFW debe verificarse dentro de la estrategia Android. Diseña una capa de plataforma para no asumir que la misma ruta de ventana, ciclo de vida e input será idéntica en escritorio y Android.
>
> **RECOMENDACIÓN:** Registra cada decisión importante como ADR: contexto, opciones, decisión, consecuencias y condición para reconsiderarla.

### 5.1 Registro ADR

* **ID y título:** 
* **Estado y fecha:** 
* **Contexto:** 
* **Opciones evaluadas:** 
* **Decisión:** 
* **Consecuencias positivas:** 
* **Consecuencias negativas:** 
* **Condición para reconsiderar:** 

---

## 6. Escenas, recursos y formatos

### 6.1 Flujo de contenido

`Archivo fuente` $\rightarrow$ `Importador/validador` $\rightarrow$ `Metadatos` $\rightarrow$ `Formato procesado` $\rightarrow$ `Paquete` $\rightarrow$ `Resource Manager` $\rightarrow$ `Runtime`

* **Formato editable y versión:** 
* **Identificador humano del recurso:** 
* **Identificador interno estable:** 
* **Política al renombrar o mover recursos:** 
* **Manejo de recursos faltantes:** 

> **RECOMENDACIÓN:** Mantén nombres y rutas legibles en el editor. Genera hashes o IDs internos durante importación/empaquetado. No obligues a editar hashes manualmente en JSON.
>
> **INCONGRUENCIA O RIESGO:** Cifrar todos los recursos no evita realmente la extracción y añade claves, compatibilidad y depuración. Para el MVP, prioriza integridad, versión y manifiesto.

### 6.2 Contrato de escena

| Campo | Obligatorio | Descripción |
| :--- | :--- | :--- |
| **formatVersion** | Sí | Versión del esquema |
| **sceneId** | Sí | Identificador estable |
| **entities** | Sí | Lista de entidades |
| **entity.id** | Sí | ID único en la escena |
| **components** | Sí | Datos por componente |
| **asset references** | Según uso | Referencias legibles y validables |

---

## 7. Networking

* **Modelo de autoridad:** Servidor autoritativo, listen server o P2P.
* **Número máximo de jugadores:** 
* **Datos controlados por el cliente:** 
* **Datos validados por el servidor:** 
* **Datos replicados:** 
* **Frecuencia de actualización objetivo:** 
* **Comportamiento ante desconexión:** 
* **Versionado del protocolo:** 

| Dato | Autoridad | Fiabilidad | Frecuencia | Interpolación |
| :--- | :--- | :--- | :--- | :--- |
| **Input** | | | | |
| **Posición** | | | | |
| **Rotación** | | | | |
| **Estado de partida** | | | | |
| **Creación/destrucción** | | | | |

> **INCONGRUENCIA O RIESGO:** "Multijugador" no es un requisito suficiente. Sin modelo de autoridad, transporte, fiabilidad y propiedad de objetos, el agente tenderá a inventar protocolos incompatibles.

---

## 8. Plataformas y portabilidad

| Plataforma | Objetivo mínimo | Toolchain | Prueba física/real | Estado |
| :--- | :--- | :--- | :--- | :--- |
| **Windows** | | | | |
| **Linux** | | | | |
| **Android** | | | | |

### 8.1 Android

* **Versión mínima de Android:** 
* **ABI objetivo:** 
* **NDK/toolchain:** 
* **Ciclo de vida:** pausa, reanudación y cierre
* **Entrada táctil:** 
* **Carga de assets:** 
* **Cambio o pérdida de red:** 
* **Permisos necesarios:** 
* **Dispositivos reales disponibles:** 

> **RECOMENDACIÓN:** Crea una compilación Android temprana, aunque solo muestre una pantalla y un sprite. Integrar Android al final aumenta el riesgo de descubrir tarde incompatibilidades de ciclo de vida, assets o librerías.

---

## 9. Editor, empaquetador y scripting visual

### 9.1 Editor mínimo

* [ ] Viewport 2D
* [ ] Jerarquía
* [ ] Inspector de Transform/Sprite/Collider
* [ ] Explorador de recursos
* [ ] Guardar y cargar escena
* [ ] Play/Stop
* [ ] Consola de errores
* [ ] Deshacer/rehacer básico

* **Qué problema del MVP resuelve el editor:** 
* **Alternativa temporal si el editor no llega:** Ejemplo: JSON validado o herramienta mínima.

> **INCONGRUENCIA O RIESGO:** Un editor puede consumir más esfuerzo que el runtime. Si editar JSON sigue siendo tolerable para la demo, el editor completo no debe bloquear el MVP.

### 9.2 Empaquetador mínimo

* [ ] Descubrir recursos
* [ ] Validar referencias
* [ ] Generar manifiesto versionado
* [ ] Convertir escenas si es necesario
* [ ] Agrupar o copiar assets
* [ ] Emitir errores claros
* [ ] Build reproducible

* **Formato de paquete propuesto:** 
* **Razón para usar otro lenguaje:** 

> **RECOMENDACIÓN:** Si eliges Rust u otro lenguaje para el empaquetador, define un contrato de archivos estable. Evita una integración profunda entre runtimes antes de validar el paquete mínimo.

### 9.3 Scripting visual post-MVP

* **Problema exacto del betatester:** 
* **Alternativa más simple:** Consola, panel de acciones, archivos de escenarios o comandos predefinidos.
* **Nodos mínimos futuros:** 

> **DECISIÓN PENDIENTE:** Incluye scripting visual en el MVP solo si existe una prueba que no pueda realizarse de otra manera.

---

## 10. Calidad y desarrollo con agentes de IA

### 10.1 Definición de terminado

* [ ] Compila en las plataformas correspondientes.
* [ ] No agrega advertencias nuevas.
* [ ] Incluye pruebas cuando la lógica es comprobable.
* [ ] Registra errores útiles.
* [ ] Documenta la API pública afectada.
* [ ] No introduce dependencias sin ADR.
* [ ] Respeta límites de módulos.
* [ ] Se valida dentro de la demo.
* [ ] Pasa criterios de aceptación.
* [ ] Tiene evidencia: logs, captura, prueba o artefacto.

### 10.2 Autoridad de OpenCode

| Puede hacer | No puede hacer sin aprobación |
| :--- | :--- |
| Implementar una tarea delimitada | Cambiar arquitectura o dependencias |
| Añadir pruebas relacionadas | Modificar formatos públicos |
| Corregir errores directamente relacionados | Reescribir módulos completos |
| Actualizar documentación afectada | Expandir alcance o agregar "mejoras" no solicitadas |
| Ejecutar comandos de validación permitidos | Cambiar protocolo de red o estándar de C++ |

### 10.3 Plantilla de tarea para OpenCode

* **Objetivo:** Un resultado concreto.
* **Contexto:** 
* **Archivos permitidos:** 
* **Archivos restringidos:** 
* **Contratos que debe respetar:** 
* **Requisitos funcionales:** 
* **Casos de error:** 
* **Pruebas requeridas:** 
* **Comandos de validación:** 
* **Criterios de aceptación:** 
* **Fuera de alcance:** 

> **RECOMENDACIÓN:** Pide cambios pequeños y revisables. Un agente debe recibir contratos, archivos permitidos y comandos de validación; no una instrucción amplia como "crea el sistema de red".

---

## 11. Roadmap por fases

| Fase | Objetivo | Entregable de salida |
| :--- | :--- | :--- |
| **0. Fundaciones** | Build, estructura, logging y pruebas | Aplicación mínima en escritorio |
| **1. Vertical 2D** | Ventana, input, sprite, cámara y escena | Personaje moviéndose |
| **2. Sistemas** | Recursos, audio y física | Escena interactiva local |
| **3. Multijugador** | Servidor y replicación mínima | Dos clientes sincronizados |
| **4. Android temprano** | Ciclo de vida, touch y assets | Cliente en dispositivo real |
| **5. Herramientas** | Editor mínimo o alternativa | Escena creada sin tocar código |
| **6. Distribución** | Paquete/manifiesto y releases | Demo instalable y reproducible |
| **7. Post-MVP** | Herramientas avanzadas | Solo después de validar MVP |

### 11.1 Ficha de fase (copiar por fase)

* **Nombre y objetivo:** 
* **Valor para la demo:** 
* **Requisitos previos:** 
* **Entregables:** 
* **Tareas:** 
* **Riesgos:** 
* **Funciones excluidas:** 
* **Pruebas de integración:** 
* **Criterios de salida:** 
* **Evidencia:** 

> **INCONGRUENCIA O RIESGO:** No dejes Android, networking o integración de plataformas para el último tramo. Son riesgos estructurales y deben probarse con prototipos tempranos.

---

## 12. Registro de riesgos e incongruencias

| ID | Riesgo o incongruencia | Señal temprana | Mitigación | Plan alternativo |
| :--- | :--- | :--- | :--- | :--- |
| **R-01** | Vulkan retrasa el primer sprite | | Usar abstracción y limitar API | Backend/librería alternativa |
| **R-02** | Android se prueba demasiado tarde | | Build temprano | Reducir plataforma del primer hito |
| **R-03** | Código IA inconsistente | | Contratos y tareas pequeñas | Revertir y reimplementar |
| **R-04** | Networking bloquea el MVP | | Replicación mínima temprana | Demo LAN con menos datos |
| **R-05** | Editor absorbe el roadmap | | Editor mínimo | JSON validado temporalmente |
| **R-06** | Scripting visual expande el alcance | | Mover a post-MVP | Panel de pruebas |
| **R-07** | Hashes dificultan autoría | | IDs legibles en editor | Resolver hashes en empaquetado |
| **R-08** | Cifrado añade complejidad | | Posponer | Checksum/manifiesto |

### 12.1 Nuevos hallazgos

| ID | Hallazgo | Impacto | Decisión necesaria | Responsable/fecha |
| :--- | :--- | :--- | :--- | :--- |
| | | | | |

---

## 13. Lista de auditoría antes de implementar

* [ ] La demo del MVP está descrita de principio a fin.
* [ ] Cada función obligatoria aporta directamente a la demo.
* [ ] Las exclusiones están escritas.
* [ ] Cada módulo tiene responsabilidad y dependencias claras.
* [ ] Vulkan está aislado.
* [ ] Los formatos tienen versión.
* [ ] Android se prueba antes de la fase final.
* [ ] El modelo de autoridad de red está definido.
* [ ] Las tareas para IA tienen límites y pruebas.
* [ ] Existe una alternativa para editor y scripting visual.
* [ ] Las fases tienen criterios de salida verificables.
* [ ] La fecha de diciembre no depende de funciones post-MVP.

---

## 14. Resumen ejecutivo

* **Nombre del motor:** 
* **Objetivo:** 
* **Demo del MVP:** 
* **Plataformas obligatorias:** 
* **Arquitectura resumida:** 
* **Riesgo principal:** 
* **Plan alternativo:** 
* **Funciones excluidas:** 
* **Estado actual:** 
* **Próximo hito verificable:** 
* **Criterio final de éxito:** 

> **CIERRE:** Cuando completes las secciones 1 a 8 y el roadmap, el documento ya será suficiente para una primera auditoría de alcance, arquitectura y viabilidad.