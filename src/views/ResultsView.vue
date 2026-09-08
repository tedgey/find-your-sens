<script setup lang="ts">
import { computed } from 'vue'
import { getGame } from '../math'
import { useSession } from '../composables/useSession'

const { state, retest, backToSetup } = useSession()

const rec = computed(() => state.recommendation)
const game = computed(() => (rec.value ? getGame(rec.value.gameId) : null))

function pct(n: number) {
  return `${Math.round(n * 100)}%`
}

function scenarioLabel(id: string) {
  if (id === 'feel90') return 'Felt 90°'
  return id.charAt(0).toUpperCase() + id.slice(1)
}
</script>

<template>
  <section v-if="rec && game" class="panel">
    <header class="head">
      <div>
        <p class="eyebrow">Your settings pack</p>
        <h1>{{ game.name }}</h1>
        <p class="lede">
          Absolute scale from <strong>Felt 90°</strong>. Target stages are consistency checks only.
        </p>
      </div>
      <div class="actions">
        <button type="button" class="btn btn--ghost" @click="backToSetup">New setup</button>
        <button type="button" class="btn" @click="retest">Retest</button>
      </div>
    </header>

    <div class="hero-stats">
      <div>
        <span class="label">DPI</span>
        <strong>{{ rec.dpi }}</strong>
        <small v-if="rec.sensRangeAtNearestTier">
          ≈ {{ rec.sensRangeAtNearestTier.center }} @ {{ rec.nearestDpiTier }} DPI
        </small>
      </div>
      <div>
        <span class="label">{{ game.settingsLabels.sensitivity }}</span>
        <strong>{{ rec.sensRange.low }} - {{ rec.sensRange.high }}</strong>
        <small>center {{ rec.sensRange.center }}</small>
      </div>
      <div>
        <span class="label">Confidence</span>
        <strong>{{ pct(rec.confidence) }}</strong>
        <small>
          {{ rec.cmPer360.toFixed(1) }} cm / 360°
          <template v-if="rec.confidenceBreakdown.eppPenaltyApplied">
            · stages {{ pct(rec.confidenceBreakdown.fromStages) }}, then ×{{
              rec.confidenceBreakdown.eppFactor
            }} for EPP-on
          </template>
        </small>
      </div>
    </div>

    <p v-if="rec.currentSens != null" class="compare">
      You run <strong>{{ rec.currentSens }}</strong>
      <template v-if="rec.vsCurrentRatio != null">
        · measured is
        <strong>{{ rec.vsCurrentRatio.toFixed(2) }}×</strong>
        that
      </template>
    </p>

    <div class="body">
      <div class="stages">
        <h2>Per scenario</h2>
        <ul>
          <li v-for="stage in rec.stageResults" :key="stage.scenario">
            <span>
              {{ scenarioLabel(stage.scenario) }}
              <em v-if="stage.isAbsolute" class="tag">absolute</em>
              <em v-else class="tag tag--muted">consistency</em>
            </span>
            <span class="mono">
              med {{ stage.medianSens.toFixed(game.sensDecimals) }}
              · σ {{ stage.stdDev.toFixed(game.sensDecimals) }}
              · {{ pct(stage.confidence) }}
            </span>
          </li>
        </ul>
      </div>

      <div class="meta">
        <p class="assumptions">
          Assumed FOV setting {{ rec.assumptions.fovSetting }}
          (≈ {{ rec.assumptions.horizontalFovDeg.toFixed(1) }}° horizontal on the
          {{ rec.assumptions.arenaWidth }}×{{ rec.assumptions.arenaHeight }} arena).
          Game res {{ rec.assumptions.gameResolutionWidth }}×{{ rec.assumptions.gameResolutionHeight }}.
          eDPI {{ rec.eDpiRange.low }}-{{ rec.eDpiRange.high }}.
          Enhance pointer precision:
          {{ rec.assumptions.enhancePointerPrecision === 'on' ? 'on' : 'off' }}.
          <template v-if="rec.pointingBaselineSens != null">
            Pointing baseline ~{{ rec.pointingBaselineSens.toFixed(game.sensDecimals) }}.
          </template>
        </p>

        <div v-if="rec.inputNotes.length" class="notes">
          <p v-for="(note, i) in rec.inputNotes" :key="i">{{ note }}</p>
        </div>
      </div>
    </div>
  </section>
</template>

<style scoped>
.panel {
  width: min(1100px, 100%);
  min-height: 0;
  display: flex;
  flex-direction: column;
  justify-content: center;
  gap: 0.85rem;
}

.head {
  display: flex;
  align-items: flex-start;
  justify-content: space-between;
  gap: 1.5rem;
  flex-shrink: 0;
}

.eyebrow {
  margin: 0 0 0.35rem;
  font-family: var(--font-mono);
  font-size: 0.75rem;
  letter-spacing: 0.08em;
  text-transform: uppercase;
  color: var(--accent);
}

h1 {
  margin: 0;
  font-family: var(--font-display);
  font-size: clamp(1.9rem, 3.5vw, 2.6rem);
  font-weight: 700;
  letter-spacing: 0.03em;
  text-transform: uppercase;
  color: var(--ink);
}

.lede {
  margin: 0.4rem 0 0;
  color: var(--muted);
  line-height: 1.4;
  font-size: 0.95rem;
  max-width: 36rem;
}

.hero-stats {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 1rem;
  flex-shrink: 0;
  padding: 0.85rem 1rem;
  border: 1px solid var(--line);
  background: var(--surface);
}

.hero-stats .label {
  display: block;
  font-family: var(--font-mono);
  font-size: 0.72rem;
  letter-spacing: 0.06em;
  text-transform: uppercase;
  color: var(--muted);
  margin-bottom: 0.3rem;
}

.hero-stats strong {
  display: block;
  font-family: var(--font-display);
  font-size: clamp(1.35rem, 2vw, 1.7rem);
  font-weight: 700;
  color: var(--ink);
  letter-spacing: 0.02em;
}

.hero-stats small {
  display: block;
  margin-top: 0.2rem;
  color: var(--muted);
  font-size: 0.82rem;
  line-height: 1.3;
}

.compare {
  margin: 0;
  padding: 0.55rem 0.85rem;
  background: var(--surface);
  border: 1px solid var(--line);
  color: var(--ink);
  font-size: 0.9rem;
  flex-shrink: 0;
}

.body {
  display: grid;
  grid-template-columns: 1.05fr 1fr;
  gap: 1rem 1.5rem;
  min-height: 0;
  align-items: start;
}

.stages h2 {
  margin: 0 0 0.45rem;
  font-family: var(--font-display);
  font-size: 0.85rem;
  font-weight: 700;
  letter-spacing: 0.08em;
  text-transform: uppercase;
  color: var(--accent);
}

.stages ul {
  list-style: none;
  margin: 0;
  padding: 0;
  border-top: 1px solid var(--line);
}

.stages li {
  display: flex;
  justify-content: space-between;
  gap: 1rem;
  padding: 0.5rem 0;
  border-bottom: 1px solid var(--line);
  color: var(--ink);
  font-size: 0.95rem;
}

.tag {
  margin-left: 0.4rem;
  font-style: normal;
  font-family: var(--font-mono);
  font-size: 0.65rem;
  letter-spacing: 0.04em;
  text-transform: uppercase;
  color: var(--accent);
}

.tag--muted {
  color: var(--muted);
}

.mono {
  font-family: var(--font-mono);
  font-size: 0.82rem;
  color: var(--muted);
  white-space: nowrap;
}

.meta {
  display: flex;
  flex-direction: column;
  gap: 0.65rem;
  min-width: 0;
}

.assumptions {
  margin: 0;
  font-size: 0.82rem;
  color: var(--muted);
  line-height: 1.4;
}

.notes {
  margin: 0;
  padding: 0.7rem 0.85rem;
  border: 1px solid rgba(184, 255, 60, 0.25);
  border-left: 3px solid var(--accent);
  background: rgba(184, 255, 60, 0.06);
}

.notes p {
  margin: 0;
  font-size: 0.84rem;
  color: var(--ink);
  line-height: 1.4;
}

.notes p + p {
  margin-top: 0.4rem;
}

.actions {
  display: flex;
  gap: 0.65rem;
  flex-shrink: 0;
}

@media (max-width: 900px) {
  .body {
    grid-template-columns: 1fr;
  }

  .head {
    flex-direction: column;
  }
}

@media (max-width: 640px) {
  .hero-stats {
    grid-template-columns: 1fr;
  }
}
</style>
