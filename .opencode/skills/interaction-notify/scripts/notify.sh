#!/usr/bin/env bash

# Notificador de interacciones de OpenCode para K_engine.
# Adaptado de Project_k/k_wiki_notify.sh.
#
# Uso:
#   bash notify.sh attention "motivo" [opciones]
#   bash notify.sh completed [opciones]
#   bash notify.sh blocked [opciones]
#
# Opciones de informe:
#   --component "componente"
#   --files "archivos modificados"
#   --tests-run "pruebas ejecutadas"
#   --tests-unavailable "pruebas no ejecutadas"
#   --errors "errores pendientes"
#   --manual-action "acción manual requerida"
#
# Opciones de comportamiento:
#   --key "clave"        Evita repetir el mismo aviso durante 5 segundos.
#   --no-sound           Desactiva el sonido para este aviso.
#   --quiet              No imprime el informe por stdout.
#   --respect-plugin     No notifica si el plugin interaction-notify está activo.
#   --log                Registra fallos en el log de runtime del plugin.

set -u

usage() {
    printf '%s\n' \
        'Uso:' \
        '  bash notify.sh attention "motivo" [opciones]' \
        '  bash notify.sh completed [opciones]' \
        '  bash notify.sh blocked [opciones]' \
        '' \
        'Opciones de informe:' \
        '  --component "componente"' \
        '  --files "archivos modificados"' \
        '  --tests-run "pruebas ejecutadas"' \
        '  --tests-unavailable "pruebas no ejecutadas"' \
        '  --errors "errores pendientes"' \
        '  --manual-action "acción manual requerida"' \
        '' \
        'Opciones de comportamiento:' \
        '  --key "clave"' \
        '  --no-sound' \
        '  --quiet' \
        '  --respect-plugin' \
        '  --log' \
        '' \
        'Usa "ninguna" cuando no haya elementos que informar.'
}

if [[ $# -lt 1 ]]; then
    usage >&2
    exit 2
fi

action=$1
shift
reason=''

case "$action" in
    attention)
        if [[ $# -lt 1 || "$1" == --* ]]; then
            printf '%s\n' 'Error: indica el motivo de la solicitud de atención.' >&2
            usage >&2
            exit 2
        fi
        reason=$1
        shift
        notification_message="Se requiere tu atención: $reason"
        ;;
    completed)
        notification_message='Componente terminado'
        ;;
    blocked)
        notification_message='Tarea bloqueada después de 3 intentos'
        ;;
    *)
        printf 'Error: acción desconocida: %s\n' "$action" >&2
        usage >&2
        exit 2
        ;;
esac

component='No indicado'
files='No indicado'
tests_run='No indicado'
tests_unavailable='No indicado'
errors='No indicado'
manual_action='No indicada'
quiet=0
sound_enabled=1
respect_plugin=0
log_enabled=0
dedupe_key=''
state_dir="${XDG_RUNTIME_DIR:-/tmp}"
log_file="$state_dir/opencode-interaction-notify.log"

while [[ $# -gt 0 ]]; do
    option=$1
    shift

    case "$option" in
        --quiet)
            quiet=1
            ;;
        --no-sound)
            sound_enabled=0
            ;;
        --respect-plugin)
            respect_plugin=1
            ;;
        --log)
            log_enabled=1
            ;;
        --component|--files|--tests-run|--tests-unavailable|--errors|--manual-action|--key)
            if [[ $# -eq 0 ]]; then
                printf 'Error: falta el valor para %s.\n' "$option" >&2
                usage >&2
                exit 2
            fi

            value=$1
            shift

            case "$option" in
                --component) component=$value ;;
                --files) files=$value ;;
                --tests-run) tests_run=$value ;;
                --tests-unavailable) tests_unavailable=$value ;;
                --errors) errors=$value ;;
                --manual-action) manual_action=$value ;;
                --key) dedupe_key=$value ;;
            esac
            ;;
        *)
            printf 'Error: opción desconocida: %s\n' "$option" >&2
            usage >&2
            exit 2
            ;;
    esac
done

if [[ $respect_plugin -eq 1 ]]; then
    marker="${XDG_RUNTIME_DIR:-/tmp}/opencode-interaction-notify.active"
    if [[ -f "$marker" ]]; then
        plugin_pid=$(sed -n '1p' "$marker" 2>/dev/null || true)
        if [[ "$plugin_pid" =~ ^[0-9]+$ ]] && kill -0 "$plugin_pid" 2>/dev/null; then
            exit 0
        fi
    fi
fi

if [[ -n "$dedupe_key" ]]; then
    dedupe_file="$state_dir/k_engine_notify_${UID}.state"
    now=$(date +%s)

    if [[ -f "$dedupe_file" ]]; then
        last_key=$(sed -n '1p' "$dedupe_file" 2>/dev/null || true)
        last_time=$(sed -n '2p' "$dedupe_file" 2>/dev/null || true)

        if [[ "$last_key" == "$dedupe_key" && "$last_time" =~ ^[0-9]+$ ]]; then
            if (( now - last_time < 5 )); then
                exit 0
            fi
        fi
    fi

    printf '%s\n%s\n' "$dedupe_key" "$now" > "$dedupe_file" 2>/dev/null || true
fi

run_with_timeout() {
    if command -v timeout >/dev/null 2>&1; then
        timeout 5s "$@"
    else
        "$@"
    fi
}

visual_status=0
sound_status=0

if command -v notify-send >/dev/null 2>&1; then
    run_with_timeout notify-send --app-name=OpenCode --urgency=normal 'K_engine' "$notification_message"
    visual_status=$?
else
    printf '%s\n' 'Error: no se encontró notify-send.' >&2
    visual_status=127
fi

if [[ $sound_enabled -eq 1 ]]; then
    sound_file='/usr/share/sounds/freedesktop/stereo/complete.oga'

    if [[ ! -f "$sound_file" ]]; then
        sound_file='/usr/share/sounds/freedesktop/stereo/bell.oga'
    fi

    if command -v paplay >/dev/null 2>&1 && [[ -f "$sound_file" ]]; then
        run_with_timeout paplay "$sound_file"
        sound_status=$?
    else
        printf '%s\n' 'Error: no se encontró paplay o el archivo de sonido.' >&2
        sound_status=127
    fi
fi

if [[ $visual_status -eq 0 ]]; then
    visual_result='completada'
else
    visual_result="fallida (código $visual_status)"
fi

if [[ $sound_enabled -eq 0 ]]; then
    sound_result='desactivado'
elif [[ $sound_status -eq 0 ]]; then
    sound_result='completado'
else
    sound_result="fallido (código $sound_status)"
fi

if [[ $quiet -eq 0 ]]; then
    printf '\n%s\n' 'Informe:'
    printf 'Componente: %s\n' "$component"
    printf 'Archivos modificados: %s\n' "$files"
    printf 'Pruebas ejecutadas: %s\n' "$tests_run"
    printf 'Pruebas no ejecutadas: %s\n' "$tests_unavailable"
    printf 'Errores pendientes: %s\n' "$errors"
    printf 'Acción manual requerida: %s\n' "$manual_action"
    printf 'Notificación visual: %s\n' "$visual_result"
    printf 'Sonido: %s\n' "$sound_result"
fi

if [[ $visual_status -ne 0 || $sound_status -ne 0 ]]; then
    if [[ $log_enabled -eq 1 ]]; then
        printf '%s fallo de notificación: visual=%s sonido=%s mensaje=%s\n' \
            "$(printf '%(%Y-%m-%d %H:%M:%S)T' -1)" \
            "$visual_result" \
            "$sound_result" \
            "$notification_message" \
            >> "$log_file" 2>/dev/null || true
    fi

    exit 1
fi
