El game engine sera usando para  proyectos personals, estara enfocando en el 2d,
con caracteristicas de multijugador.
Ademas, estara disponible para webassembly, android, linux y windows, 
usando como api de graficos, vulkan y C++ como lenguaje de programcion
y la version 17 como estandar.
Estara dividido en tres partes: El nucleo, el editor y empaquetador.


El nucleo sera como una libreria de juegos (como raylib o smfl),
esta  contendra componentes generales como renderizador,
gestion de ventanas, input, audio, fisicas y red. 
Ademas de componentes especificos, 
como gui 
(responsive como https://github.com/nicbarker/clay,
pero usando el patron builder o el editor para crearla, que macros),
administrador de recursos, componentes genericos de fisicas
(dinamico y estaticos), animaciones y arbol de dialagos.
Este nucleo se usara para escribir la logica de todo los componentes del juego.

El editor sera usado para todo que tiene que ver como diseño,
diseño de niveles, diseño de ui, diseñar y prueba de animaciones, etc. 
todo que se realize en este editor, sera guardado en varios jsons (en desarrollo)
que el codigo de c++, puede acceder para que coloque cada componente en su lugar.
Para que la logica del juego,
accedera a los json, mediante el administrador de recursos,
que mediante un hash (almacenado en el json) de cada componente para identificar y
 acceder de forma individual,
aunque los json, seran convertidos a un formato binario por el empaquetador.

El empaquetador su funcion principal sera almacenar todos los recursos multimedia
(imagenes, texturas, audio o objetos 3D)
y cifrar en un solo archivo para facilitar su exportacion a otras plataformas.
Ademas, de convertir los json del editor a un formato binario.
Este compenente sera el ulitmo en entigrarse al resto de la arquitectura del game engine.
Otra cosa, este componente, puede estar escrito en otro lenguaje que no sea C++.


### Decisiones tecnicas

La decision mas importante es la api de graficos, que sera vulkan,
ya que el juego que estoy haciendo esta dirigido principalmente a android,
y he leido rumores que opengl, es posible que sea declarado como heredado,
y por eso, usare vulkan, ademas el tiempo desarrollo que tengo estimando
es de 4 a 5 años[^1], por eso mismo, evitare usar cosas que en el futuro,
queden sin nuevas actualizaciones. Pero, usare a Vulkan con alguna libreria
que me abstraiga toda la logica que se necesita para usar vulkan,
ya que vulkan para alguien con poco experiencia en pogramacion de graficos
y con multiples intentos de aprender, se le acomplicado, principalmente por
lo complicado de depurar errores.

Otra decision importante, es que los componentes generales descriptos,
seran en su mayoria abstraciones de librerias externas por ejemplo,
para la gestion de ventanas y inputs, usare glfw3.
Lo anterior tiene una excepcion, que son las interfaces para los componentes de
Red, principalmente por seguridad y poder sacar el mayor rendimiento posible.

El empaquetador sera escrito en python, porque es un lenguaje que conozco y puedo 
depurar. Opciones mas rapida en rendimiento, como Go o Rust, no la usare, el primero 
porque no lo conozco y rust por razones personales y por los binarios pesados en desarrollo.
Tambien pense en Zig, que lo he usado, pero que en cada actualizacion cambian muchas cosas 
y que no lo use mucho, lo descarto.

## Descripcion del juego
En la primera parte, mencione el uso y aqui los describire. Tengo dos juegos
habientados en un mundo ficitio, uno en planeacion y uno en TODO. El primero y el
cual se centrara este primer MVP, sin dejar de ser reutilizable para futuros proyectos,
es un juego de plataformas con accion, al estilo de Contra, pero mas moderno y fanstastico,
ademas, se centrara en la narrativa, ya que su historia se contara en el formato de novela visual,
es decir, el juego sera en su mayoria offline. Los componentes de red seran un extra para 
personas que quieran retarse entre si en una misma red local, no obstante, si tiene exito suficiente
tengo planeado contractar servidores para que esta caracteristica tenga mas importancia.

### Mi Perfil del Desarrollador
1. ¿Cuál es tu experiencia y contexto técnico actual?
- **Formación y Enfoque**: Estudiante de sexto semestre de Gestión de 
Redes de Computadores y Seguridad Informática.
- **Áreas de Desarrollo**: Programación de sistemas de alto rendimiento en C++ (C++17), automatización
 y scripts en Python, y desarrollo de herramientas en Rust .

- **Entorno de Trabajo**: Entornos Linux como Linux Mint como SO principal y editor principal
es VSCODE y auxliar es Helix.


3. ¿Cómo prefieres que la IA interactúe contigo y planifique las tareas?
- **Precisión técnica**: Se prefieren respuestas directas, código idiomático y limpio,
 sin introducciones teóricas redundantes ni explicaciones genéricas de conceptos básicos (Sino se 
 lo pido de forma explicita).

- **Modo Plan Estructurado**: 
Las tareas complejas deben desglosarse en fases lógicas y ejecutables
 (por ejemplo: diseño de la API del subsistema, implementación del núcleo en C++, 
 pruebas de rendimiento y automatización con scripts).

Compatibilidad Multiplataforma: Toda propuesta de arquitectura debe tener en cuenta
 la compilación cruzada y la integración limpia con sistemas de compilación modernos.

## Meta principal

El game engine debe estar listo para el 1 de diciembre. Ya que despues,
no tendre menos cantidad de horas para poder trabajar en engine y el juego a al vez.


[^1]: Este tiempo estimando fue calculando antes de aprender a usar agentes IA,
pero aun es un proyecto de largo plazo, porque para crear un lore coherente y interantes,
ademas del arte de los juegos de mi parte.