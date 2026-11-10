import { Plugin } from "@opencode/plugin"
import { spawn } from "node:child_process"
import { rmSync, writeFileSync } from "node:fs"
import { tmpdir } from "node:os"
import { join } from "node:path"
import { fileURLToPath } from "node:url"

const script =
  process.env.OPENCODE_INTERACTION_NOTIFY_SCRIPT ??
  fileURLToPath(
    new URL("../skills/interaction-notify/scripts/notify.sh", import.meta.url),
  )

const marker = join(
  process.env.XDG_RUNTIME_DIR ?? tmpdir(),
  "opencode-interaction-notify.active",
)

function notify(reason: string) {
  const child = spawn("bash", [script, "attention", reason, "--quiet", "--log"], {
    detached: true,
    stdio: "ignore",
  })

  child.on("error", (error) => {
    console.error("[interaction-notify] no se pudo lanzar el notificador:", error)
  })

  child.on("exit", (code) => {
    if (code !== 0) {
      console.error(
        `[interaction-notify] el notificador terminó con código ${code}; revisa $XDG_RUNTIME_DIR/opencode-interaction-notify.log`,
      )
    }
  })

  child.unref()
}

function summarize(input: unknown): string {
  try {
    const text = typeof input === "string" ? input : JSON.stringify(input)

    if (!text) {
      return "pregunta sin resumen"
    }

    return text.length > 120 ? `${text.slice(0, 117)}...` : text
  } catch {
    return "pregunta sin resumen"
  }
}

export default Plugin.define({
  id: "interaction-notify",
  async setup(ctx) {
    try {
      writeFileSync(marker, String(process.pid), { mode: 0o600 })
    } catch (error) {
      console.error("[interaction-notify] no se pudo escribir el marcador:", error)
    }

    await ctx.permission.hook("evaluate", (event) => {
      if (event.effect !== "ask") {
        return
      }

      // La tool question tiene su propio hook; se evita duplicar el aviso.
      if (event.action === "question") {
        return
      }

      const resource = event.resources[0] ?? "*"
      notify(`Permiso pendiente: ${event.action} ${resource}`)
    })

    await ctx.tool.hook("execute.before", (event) => {
      if (event.tool !== "question") {
        return
      }

      notify(`Pregunta pendiente: ${summarize(event.input)}`)
    })

    return () => {
      try {
        rmSync(marker, { force: true })
      } catch (error) {
        console.error("[interaction-notify] no se pudo eliminar el marcador:", error)
      }
    }
  },
})
