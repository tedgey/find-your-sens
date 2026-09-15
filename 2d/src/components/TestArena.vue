<script setup lang="ts">
import { computed, onMounted, onUnmounted, ref, watch } from 'vue'
import {
  spawnTarget,
  type PathSample,
  type ScenarioId,
  type SessionSetup,
  type TargetSpec,
  buildDisplayConfig,
  getGame,
  cmPer360,
} from '../math'

const props = defineProps<{
  scenario: ScenarioId
  roundIndex: number
  roundsPerScenario: number
  setup: SessionSetup
}>()

const emit = defineEmits<{
  complete: [
    payload: {
      target: TargetSpec | null
      samples: PathSample[]
      net: { x: number; y: number }
      durationMs: number
      arenaWidth: number
      arenaHeight: number
    },
  ]
}>()

const rootRef = ref<HTMLElement | null>(null)
const canvasRef = ref<HTMLCanvasElement | null>(null)
const locked = ref(false)
const armed = ref(false)
const target = ref<TargetSpec | null>(null)
const samples = ref<PathSample[]>([])
const net = ref({ x: 0, y: 0 })
const startedAt = ref(0)
const isFullscreen = ref(false)

/** Felt-90 helper: watch a real 90° turn, then replicate with moving bars + pad feedback. */
const feelPhase = ref<'demo' | 'replicate'>('demo')
const demoYaw = ref(0)
const replicateYaw = ref(0)
const demoPlaying = ref(false)
let demoRaf = 0
let drawRaf = 0

const isFeel = computed(() => props.scenario === 'feel90')

/** Horizontal pad travel used for felt-turn measurement (matches solver). */
const inchesTraveled = computed(() => {
  if (props.setup.dpi <= 0) return 0
  return Math.abs(net.value.x) / props.setup.dpi
})

const cmTraveled = computed(() => inchesTraveled.value * 2.54)

/** Soft reference only, never a pass/fail target. */
const referenceTravelFor90 = computed(() => {
  const game = getGame(props.setup.gameId)
  const sens = props.setup.currentSens
  if (sens == null || sens <= 0) return null
  const cm = cmPer360(sens, props.setup.dpi, game.yawConstant) / 4
  return { cm, inches: cm / 2.54 }
})

const scenarioMeta: Record<ScenarioId, { title: string; hint: string }> = {
  feel90: {
    title: 'Felt 90°',
    hint: "First watch a real 90° turn. Then copy that same mouse travel. The bars move with your mouse; stop when it feels like you've turned 90°.",
  },
  flick: {
    title: 'Flick',
    hint: 'Consistency check: move as if snapping onto the target in one commit, then click.',
  },
  casual: {
    title: 'Casual',
    hint: 'Consistency check: move as if smoothly placing your crosshair on the target, then click.',
  },
  micro: {
    title: 'Micro',
    hint: 'Consistency check: move as if making a small correction onto the target, then click.',
  },
}

const scenarioTitle = computed(() => scenarioMeta[props.scenario].title)
const scenarioHint = computed(() => scenarioMeta[props.scenario].hint)

const hudLine = computed(() => {
  if (isFeel.value && feelPhase.value === 'demo') {
    return demoPlaying.value
      ? 'Watch the turn. This is what 90° of yaw looks like.'
      : 'Demo ready. Press Replay if you want another look, then Start replicate.'
  }
  if (!locked.value) return 'Click the arena to lock the pointer and begin.'
  if (!armed.value) {
    return isFeel.value
      ? 'Click once to arm (crosshair turns green), then sweep like that turn, then click to commit.'
      : 'Click once to arm (crosshair turns green), then move, then click to commit.'
  }
  return isFeel.value
    ? 'Armed. Turn until it feels like 90°. Bars move with your mouse; watch inches and cm on the pad.'
    : 'Armed. Move as you would in-game. Click when you believe you’d be on target.'
})

function previewSens(): number {
  const s = props.setup.currentSens
  return s != null && s > 0 ? s : 1
}

function yawFromMouseX(mouseDx: number): number {
  const game = getGame(props.setup.gameId)
  return mouseDx * game.yawConstant * previewSens()
}

function displayConfig() {
  const game = getGame(props.setup.gameId)
  const fov = props.setup.fovSetting ?? game.defaultFovSetting
  const canvas = canvasRef.value
  const width = Math.max(1, canvas?.clientWidth || props.setup.resolutionWidth)
  const height = Math.max(
    1,
    canvas?.clientHeight || props.setup.resolutionHeight,
  )
  return buildDisplayConfig(game, fov, width, height)
}

function resizeCanvas() {
  const canvas = canvasRef.value
  if (!canvas) return
  const dpr = window.devicePixelRatio || 1
  const rect = canvas.getBoundingClientRect()
  canvas.width = Math.floor(rect.width * dpr)
  canvas.height = Math.floor(rect.height * dpr)
  const ctx = canvas.getContext('2d')
  if (ctx) ctx.setTransform(dpr, 0, 0, dpr, 0, 0)
  scheduleDraw()
}

function resetRound() {
  samples.value = []
  net.value = { x: 0, y: 0 }
  replicateYaw.value = 0
  startedAt.value = 0
  armed.value = false
  if (props.scenario === 'feel90') {
    target.value = null
  } else {
    target.value = spawnTarget(props.scenario, displayConfig())
  }
  scheduleDraw()
}

function drawWorld(
  ctx: CanvasRenderingContext2D,
  w: number,
  h: number,
  yawDeg: number,
  opts: { demo: boolean; showTravel: boolean },
) {
  const cx = w / 2
  const cy = h / 2
  const display = displayConfig()
  const halfFov = display.horizontalFovDeg / 2

  const grad = ctx.createRadialGradient(
    cx,
    cy,
    40,
    cx,
    cy,
    Math.max(w, h) * 0.7,
  )
  grad.addColorStop(0, '#121820')
  grad.addColorStop(1, '#06080c')
  ctx.fillStyle = grad
  ctx.fillRect(0, 0, w, h)

  // Sky / ground split (horizon)
  ctx.fillStyle = 'rgba(40, 70, 100, 0.18)'
  ctx.fillRect(0, 0, w, cy)
  ctx.fillStyle = 'rgba(20, 28, 24, 0.45)'
  ctx.fillRect(0, cy, w, h - cy)

  ctx.strokeStyle = 'rgba(160, 170, 185, 0.45)'
  ctx.lineWidth = 2
  ctx.beginPath()
  ctx.moveTo(0, cy)
  ctx.lineTo(w, cy)
  ctx.stroke()

  // Vertical posts at fixed world yaw angles; rotate with demo yaw
  const posts = [-135, -90, -45, 0, 45, 90, 135, 180]
  for (const worldYaw of posts) {
    let rel = ((worldYaw - yawDeg + 540) % 360) - 180
    if (Math.abs(rel) > halfFov * 1.15) continue
    const x =
      cx +
      (Math.tan((rel * Math.PI) / 180) / Math.tan((halfFov * Math.PI) / 180)) *
        (w / 2)
    const isCardinal = worldYaw % 90 === 0
    ctx.strokeStyle = isCardinal
      ? 'rgba(190, 200, 215, 0.85)'
      : 'rgba(140, 150, 165, 0.28)'
    ctx.lineWidth = isCardinal ? 3 : 1
    ctx.beginPath()
    ctx.moveTo(x, cy - (isCardinal ? 70 : 40))
    ctx.lineTo(x, cy + (isCardinal ? 70 : 40))
    ctx.stroke()
  }

  // Crosshair: white idle, lime when armed
  ctx.strokeStyle = armed.value ? '#b8ff3c' : '#e8eef5'
  ctx.lineWidth = armed.value ? 3 : 2
  ctx.beginPath()
  ctx.moveTo(cx - 16, cy)
  ctx.lineTo(cx - 5, cy)
  ctx.moveTo(cx + 5, cy)
  ctx.lineTo(cx + 16, cy)
  ctx.moveTo(cx, cy - 16)
  ctx.lineTo(cx, cy - 5)
  ctx.moveTo(cx, cy + 5)
  ctx.lineTo(cx, cy + 16)
  ctx.stroke()

  if (armed.value) {
    ctx.beginPath()
    ctx.arc(cx, cy, 7, 0, Math.PI * 2)
    ctx.strokeStyle = 'rgba(184, 255, 60, 0.9)'
    ctx.lineWidth = 2
    ctx.stroke()
  }

  if (opts.demo) {
    ctx.fillStyle = 'rgba(232, 238, 245, 0.9)'
    ctx.font = '700 15px Oxanium, sans-serif'
    ctx.fillText('DEMO · 90° TURN', 20, 32)
  }

  if (opts.showTravel && armed.value) {
    const inches = inchesTraveled.value
    const cm = cmTraveled.value
    const travelY = h / 3
    ctx.save()
    ctx.textAlign = 'center'
    ctx.textBaseline = 'middle'
    ctx.fillStyle = 'rgba(184, 255, 60, 0.95)'
    ctx.font = '700 22px "Share Tech Mono", monospace'
    ctx.fillText(`${inches.toFixed(2)} in  ·  ${cm.toFixed(1)} cm`, cx, travelY)
    ctx.font = '12px "Share Tech Mono", monospace'
    if (referenceTravelFor90.value != null) {
      ctx.fillStyle = 'rgba(232, 238, 245, 0.55)'
      ctx.fillText(
        `current sens ≈ ${referenceTravelFor90.value.inches.toFixed(2)} in / ${referenceTravelFor90.value.cm.toFixed(1)} cm for 90°`,
        cx,
        travelY + 22,
      )
    } else {
      ctx.fillStyle = 'rgba(232, 238, 245, 0.4)'
      ctx.fillText('horizontal pad travel', cx, travelY + 22)
    }
    ctx.restore()
  }
}

function draw() {
  const canvas = canvasRef.value
  if (!canvas) return
  const ctx = canvas.getContext('2d')
  if (!ctx) return

  const w = canvas.clientWidth
  const h = canvas.clientHeight
  ctx.clearRect(0, 0, w, h)

  if (isFeel.value && feelPhase.value === 'demo') {
    drawWorld(ctx, w, h, demoYaw.value, { demo: true, showTravel: false })
    return
  }

  if (isFeel.value) {
    drawWorld(ctx, w, h, replicateYaw.value, { demo: false, showTravel: true })
    const cx = w / 2
    ctx.fillStyle = 'rgba(190, 200, 215, 0.55)'
    ctx.font = '12px "Share Tech Mono", monospace'
    ctx.fillText('MOVE TO TURN · STOP AT A FELT 90°', cx - 130, 28)
    return
  }

  // Target scenarios
  drawWorld(ctx, w, h, 0, { demo: false, showTravel: false })
  const cx2 = w / 2
  const cy2 = h / 2
  if (target.value) {
    const tx = cx2 + target.value.offsetPx.x
    const ty = cy2 + target.value.offsetPx.y
    ctx.beginPath()
    ctx.arc(tx, ty, 14, 0, Math.PI * 2)
    ctx.fillStyle = '#b8ff3c'
    ctx.fill()
    ctx.beginPath()
    ctx.arc(tx, ty, 22, 0, Math.PI * 2)
    ctx.strokeStyle = 'rgba(184, 255, 60, 0.45)'
    ctx.lineWidth = 2
    ctx.stroke()
  }
}

function scheduleDraw() {
  cancelAnimationFrame(drawRaf)
  drawRaf = requestAnimationFrame(() => draw())
}

function stopDemo() {
  cancelAnimationFrame(demoRaf)
  demoPlaying.value = false
}

function playDemo() {
  stopDemo()
  feelPhase.value = 'demo'
  armed.value = false
  demoPlaying.value = true
  demoYaw.value = 0
  const durationMs = 1600
  const start = performance.now()

  const tick = (now: number) => {
    const t = Math.min(1, (now - start) / durationMs)
    // Ease out; turns aren't linear
    const eased = 1 - (1 - t) ** 2
    demoYaw.value = 90 * eased
    scheduleDraw()
    if (t < 1) {
      demoRaf = requestAnimationFrame(tick)
    } else {
      demoPlaying.value = false
      demoYaw.value = 90
      scheduleDraw()
    }
  }
  demoRaf = requestAnimationFrame(tick)
}

function startReplicate() {
  stopDemo()
  feelPhase.value = 'replicate'
  demoYaw.value = 0
  replicateYaw.value = 0
  resetRound()
}

function onMouseMove(e: MouseEvent) {
  if (!locked.value || !armed.value) return
  if (isFeel.value && feelPhase.value !== 'replicate') return
  if (!isFeel.value && !target.value) return
  const dx = e.movementX
  const dy = e.movementY
  if (dx === 0 && dy === 0) return

  const now = performance.now()
  if (startedAt.value === 0) startedAt.value = now

  samples.value.push({ t: now, dx, dy })
  net.value = { x: net.value.x + dx, y: net.value.y + dy }
  if (isFeel.value) {
    replicateYaw.value = yawFromMouseX(net.value.x)
    scheduleDraw()
  }
}

function finishRound() {
  if (!armed.value) return
  if (isFeel.value && feelPhase.value !== 'replicate') return
  if (!isFeel.value && !target.value) return
  const durationMs =
    startedAt.value === 0 ? 0 : performance.now() - startedAt.value
  const canvas = canvasRef.value

  emit('complete', {
    target: target.value,
    samples: [...samples.value],
    net: { ...net.value },
    durationMs,
    arenaWidth: canvas?.clientWidth || props.setup.resolutionWidth,
    arenaHeight: canvas?.clientHeight || props.setup.resolutionHeight,
  })
}

function onMouseDown(e: MouseEvent) {
  if (e.button !== 0) return
  if (isFeel.value && feelPhase.value === 'demo') return

  if (!locked.value) {
    requestLock()
    return
  }
  if (!armed.value) {
    armed.value = true
    startedAt.value = 0
    samples.value = []
    net.value = { x: 0, y: 0 }
    replicateYaw.value = 0
    scheduleDraw()
    return
  }
  finishRound()
}

function requestLock() {
  const canvas = canvasRef.value
  if (!canvas) return
  canvas.requestPointerLock()
}

async function enterFullscreen() {
  const el = rootRef.value
  if (!el || document.fullscreenElement) return
  try {
    await el.requestFullscreen()
  } catch {
    // denied
  }
}

function onFullscreenChange() {
  isFullscreen.value = document.fullscreenElement === rootRef.value
  resizeCanvas()
}

function onLockChange() {
  locked.value = document.pointerLockElement === canvasRef.value
  if (locked.value) {
    armed.value = false
    if (isFeel.value) {
      if (feelPhase.value === 'replicate') resetRound()
    } else {
      resetRound()
    }
  } else {
    armed.value = false
    scheduleDraw()
  }
}

watch(
  () => props.scenario,
  (scenario) => {
    stopDemo()
    if (scenario === 'feel90') {
      feelPhase.value = 'demo'
      playDemo()
    } else {
      feelPhase.value = 'replicate'
      if (locked.value) resetRound()
      else scheduleDraw()
    }
  },
)

watch(
  () => props.roundIndex,
  () => {
    if (!isFeel.value && locked.value) resetRound()
    else if (isFeel.value && feelPhase.value === 'replicate' && locked.value)
      resetRound()
  },
)

onMounted(() => {
  resizeCanvas()
  window.addEventListener('resize', resizeCanvas)
  document.addEventListener('pointerlockchange', onLockChange)
  document.addEventListener('fullscreenchange', onFullscreenChange)
  document.addEventListener('mousemove', onMouseMove)
  canvasRef.value?.addEventListener('mousedown', onMouseDown)
  void enterFullscreen()
  if (isFeel.value) playDemo()
  else scheduleDraw()
})

onUnmounted(() => {
  stopDemo()
  cancelAnimationFrame(drawRaf)
  window.removeEventListener('resize', resizeCanvas)
  document.removeEventListener('pointerlockchange', onLockChange)
  document.removeEventListener('fullscreenchange', onFullscreenChange)
  document.removeEventListener('mousemove', onMouseMove)
  canvasRef.value?.removeEventListener('mousedown', onMouseDown)
  if (document.pointerLockElement) document.exitPointerLock()
  if (document.fullscreenElement === rootRef.value) {
    void document.exitFullscreen()
  }
})
</script>

<template>
  <div ref="rootRef" class="arena" :class="{ 'arena--fs': isFullscreen }">
    <div class="arena__chrome">
      <div class="arena__scenario">
        <h2 class="arena__title">{{ scenarioTitle }}</h2>
        <p class="arena__hint">{{ scenarioHint }}</p>
      </div>
      <div class="arena__aside">
        <p class="arena__progress">
          Round {{ roundIndex + 1 }} / {{ roundsPerScenario }}
        </p>
        <button
          v-if="!isFullscreen"
          type="button"
          class="arena__fs"
          @click="enterFullscreen"
        >
          Fullscreen
        </button>
      </div>
    </div>

    <div
      class="arena__hud"
      :class="{ 'arena__hud--feel': isFeel }"
      aria-live="polite"
    >
      <div v-if="isFeel" class="arena__feel-bar">
        <template v-if="feelPhase === 'demo'">
          <button
            type="button"
            class="arena__fs"
            :disabled="demoPlaying"
            @click="playDemo"
          >
            Replay demo
          </button>
          <button
            type="button"
            class="btn-mini"
            :disabled="demoPlaying"
            @click="startReplicate"
          >
            Start replicate
          </button>
        </template>
        <template v-else>
          <button type="button" class="arena__fs" @click="playDemo">
            Watch demo again
          </button>
          <span v-if="armed" class="arena__cm">
            {{ inchesTraveled.toFixed(2) }} in · {{ cmTraveled.toFixed(1) }} cm
          </span>
        </template>
      </div>
      <p class="arena__directive">{{ hudLine }}</p>
      <div v-if="isFeel" class="arena__feel-spacer" aria-hidden="true" />
    </div>

    <canvas ref="canvasRef" class="arena__canvas" tabindex="0" />
  </div>
</template>

<style scoped>
.arena {
  display: flex;
  flex-direction: column;
  gap: 0.5rem;
  width: 100%;
  height: 100%;
  min-height: 0;
  overflow: hidden;
}

.arena--fs {
  height: 100%;
  max-height: 100dvh;
  padding: 0.75rem 1.25rem;
  background: var(--bg0);
  box-sizing: border-box;
}

.arena:fullscreen,
.arena:-webkit-full-screen {
  width: 100%;
  height: 100%;
  max-height: none;
  padding: 0.75rem 1.5rem 0.85rem;
  background: var(--bg0);
}

.arena__chrome {
  display: flex;
  justify-content: space-between;
  gap: 1rem;
  align-items: flex-start;
  flex-shrink: 0;
}

.arena__scenario {
  max-width: min(72rem, 100%);
  min-width: 0;
}

.arena__title {
  margin: 0;
  font-family: var(--font-display);
  font-size: clamp(1.25rem, 2vw, 1.45rem);
  font-weight: 700;
  letter-spacing: 0.06em;
  text-transform: uppercase;
  color: var(--accent);
  line-height: 1.1;
}

.arena__hint {
  margin: 0.25rem 0 0;
  color: var(--muted);
  font-size: 0.9rem;
  line-height: 1.35;
}

.arena__aside {
  display: flex;
  flex-direction: column;
  align-items: flex-end;
  gap: 0.35rem;
  flex-shrink: 0;
}

.arena__progress {
  margin: 0;
  font-family: var(--font-mono);
  font-size: 0.85rem;
  color: var(--muted);
  white-space: nowrap;
}

.arena__feel-bar {
  display: flex;
  align-items: center;
  gap: 0.6rem;
  flex-wrap: wrap;
  flex-shrink: 0;
  min-width: 11rem;
}

.arena__feel-spacer {
  min-width: 11rem;
  flex-shrink: 0;
}

.arena__cm {
  font-family: var(--font-mono);
  font-size: 0.95rem;
  color: var(--ink);
}

.arena__fs,
.btn-mini {
  appearance: none;
  border: 1px solid var(--line);
  background: var(--surface);
  color: var(--ink);
  font-family: var(--font-mono);
  font-size: 0.75rem;
  letter-spacing: 0.04em;
  text-transform: uppercase;
  padding: 0.4rem 0.65rem;
  border-radius: 2px;
  cursor: pointer;
}

.btn-mini {
  background: var(--accent);
  color: var(--accent-ink);
  border-color: var(--accent);
  font-family: var(--font-display);
  font-weight: 700;
}

.arena__fs:disabled,
.btn-mini:disabled {
  opacity: 0.5;
  cursor: default;
}

.arena__canvas {
  flex: 1 1 0;
  min-height: 0;
  width: 100%;
  border: 1px solid var(--line);
  border-radius: 2px;
  cursor: crosshair;
  background: #06080c;
}

.arena__hud {
  flex-shrink: 0;
  display: flex;
  justify-content: center;
  align-items: center;
  padding: 0.15rem 0 0.35rem;
}

.arena__hud--feel {
  display: grid;
  grid-template-columns: auto 1fr auto;
  gap: 0.75rem 1rem;
}

.arena__directive {
  margin: 0;
  justify-self: center;
  text-align: center;
  max-width: 52rem;
  font-family: var(--font-display);
  font-size: clamp(1.05rem, 2.2vw, 1.45rem);
  font-weight: 700;
  letter-spacing: 0.08em;
  text-transform: uppercase;
  line-height: 1.3;
  color: var(--accent);
  -webkit-text-stroke: 1px var(--accent);
  paint-order: stroke fill;
}

@media (max-width: 900px) {
  .arena__hud--feel {
    grid-template-columns: 1fr;
    justify-items: center;
  }

  .arena__feel-spacer {
    display: none;
  }
}
</style>
