---
name: Interaction Notify
description: Notifica al usuario por escritorio cuando OpenCode necesite una interacción (question, permisos ask, bloqueos o acciones manuales). Úsala siempre antes de question o de cualquier tool que pueda abrir un permiso.
---

# Interaction Notify

## Cuándo cargar esta skill

Carga esta skill antes de:

- Invocar la tool `question`.
- Invocar una tool que pueda resolver a `ask`: editar archivos protegidos,
  acceder a `external_directory`, lanzar un `subagent`, cargar otra `skill` o
  usar una tool MCP.
- Informar de un bloqueo tras tres intentos.
- Pedir una acción manual al usuario.

No la cargues para acciones `allow` ni `deny`: no requieren interacción.

## Cómo notificar

Para una interacción pendiente:

```bash
bash .opencode/skills/interaction-notify/scripts/notify.sh attention "motivo breve" --respect-plugin --quiet
```

`--respect-plugin` evita duplicar el aviso si el plugin `interaction-notify`
está activo. El plugin es la vía principal para `question` y permisos; la skill
es el respaldo cuando el plugin no está disponible.

Para un bloqueo:

```bash
bash .opencode/skills/interaction-notify/scripts/notify.sh blocked --quiet
```

Para un componente terminado:

```bash
bash .opencode/skills/interaction-notify/scripts/notify.sh completed --quiet \
  --component "engine" \
  --files "archivos modificados" \
  --tests-run "pruebas ejecutadas" \
  --tests-unavailable "pruebas no ejecutadas" \
  --errors "errores pendientes" \
  --manual-action "acción manual requerida"
```

## Reglas

- No notifiques acciones `allow` ni `deny`.
- No incluyas secretos, tokens, contraseñas, valores de variables de entorno ni
  comandos completos.
- Trunca el motivo a una frase corta, de unos 120 caracteres como máximo.
- El sonido está activado por defecto. No uses `--no-sound` salvo que el
  usuario lo pida explícitamente.
- Si el script falla, continúa con la interacción e informa del error; no
  bloquees la tarea.
- No ejecutes el script dos veces para la misma interacción.

## Comprobación del plugin

El plugin escribe este marcador mientras está activo:

```text
$XDG_RUNTIME_DIR/opencode-interaction-notify.active
```

El script comprueba el marcador y el PID con `--respect-plugin`. Si el marcador
está obsoleto, el script ignora el PID y notifica igualmente.
