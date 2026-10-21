# Descripción del proyecto

## 1. Resumen

**Nombre del proyecto:**  
[Nombre provisional]

**¿Qué estoy creando?**  
[Descríbelo en 2–4 frases.]

**¿Qué problema, necesidad o meta resuelve?**  
[Explica por qué quiero hacerlo.]

**Usuario principal:**  
[Quién usará el engine o jugará el juego.]

---

## 2. Objetivos del proyecto

### Objetivo a largo plazo
[¿Qué quiero que el proyecto pueda hacer en el futuro?]

### Objetivo del primer MVP
[¿Qué resultado mínimo útil quiero alcanzar primero?]

### Cómo sabré que el MVP está terminado
- [ ] [Criterio observable 1]
- [ ] [Criterio observable 2]
- [ ] [Criterio observable 3]

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
