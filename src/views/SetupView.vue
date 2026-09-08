<script setup lang="ts">
import { reactive } from 'vue'
import { getGame, PROTOTYPE_GAME_ORDER } from '../math'
import { useSession } from '../composables/useSession'

const { state, saveSetup, startPrep, openInfo } = useSession()

const form = reactive({
  gameId: state.setup.gameId,
  dpi: state.setup.dpi,
  fovSetting: state.setup.fovSetting ?? '',
  resolutionWidth: state.setup.resolutionWidth,
  resolutionHeight: state.setup.resolutionHeight,
  currentSens: state.setup.currentSens ?? '',
})

function submit() {
  const fovRaw = String(form.fovSetting).trim()
  const sensRaw = String(form.currentSens).trim()
  saveSetup({
    gameId: form.gameId,
    dpi: Number(form.dpi) || 400,
    fovSetting: fovRaw === '' ? null : Number(fovRaw),
    resolutionWidth: Number(form.resolutionWidth) || 1920,
    resolutionHeight: Number(form.resolutionHeight) || 1080,
    currentSens: sensRaw === '' ? null : Number(sensRaw),
  })
  startPrep()
}
</script>

<template>
  <section class="panel">
    <div class="intro">
      <p class="eyebrow">Prototype · CS2 first</p>
      <h1>Find Your Sens</h1>
      <p class="lede">
        We map what your hand thinks a flick, casual move, and micro-adjustment
        should feel like, then turn that into a settings pack. The view stays
        still; you move as if it didn’t.
      </p>
      <button type="button" class="info-link" @click="openInfo">
        How it works
      </button>
    </div>

    <form class="form" @submit.prevent="submit">
      <label class="span-2">
        <span>Game</span>
        <select v-model="form.gameId">
          <option v-for="id in PROTOTYPE_GAME_ORDER" :key="id" :value="id">
            {{ getGame(id).name }}
          </option>
        </select>
      </label>

      <label>
        <span>Mouse DPI</span>
        <input
          v-model.number="form.dpi"
          type="number"
          min="100"
          max="20000"
          step="50"
          required
        />
      </label>

      <label>
        <span>Current sensitivity <em>(optional)</em></span>
        <input
          v-model="form.currentSens"
          type="number"
          min="0.001"
          max="20"
          step="0.001"
          placeholder="e.g. 1.13"
        />
      </label>

      <label>
        <span>Resolution width</span>
        <input
          v-model.number="form.resolutionWidth"
          type="number"
          min="800"
          step="1"
          required
        />
      </label>

      <label>
        <span>Resolution height</span>
        <input
          v-model.number="form.resolutionHeight"
          type="number"
          min="600"
          step="1"
          required
        />
      </label>

      <label class="span-2">
        <span>FOV <em>(optional, CS2 default 90)</em></span>
        <input
          v-model="form.fovSetting"
          type="number"
          min="60"
          max="120"
          step="1"
          placeholder="90"
        />
      </label>

      <button type="submit" class="btn span-2">Continue</button>
    </form>
  </section>
</template>

<style scoped>
.panel {
  width: min(1400px, 100%);
  height: 100%;
  min-height: 0;
  display: grid;
  grid-template-columns: minmax(22rem, 0.95fr) minmax(28rem, 1.05fr);
  gap: 2.5rem 4rem;
  align-items: center;
}

.eyebrow {
  margin: 0 0 0.35rem;
  font-family: var(--font-mono);
  font-size: 0.75rem;
  letter-spacing: 0.12em;
  text-transform: uppercase;
  color: var(--accent);
}

h1 {
  margin: 0;
  font-family: var(--font-display);
  font-size: clamp(2.4rem, 5vw, 3.6rem);
  font-weight: 700;
  letter-spacing: 0.02em;
  text-transform: uppercase;
  line-height: 0.95;
  color: var(--ink);
}

.lede {
  margin: 0.85rem 0 0;
  color: var(--muted);
  font-size: 1rem;
  line-height: 1.45;
}

.info-link {
  appearance: none;
  margin: 1rem 0 0;
  padding: 0;
  border: none;
  background: transparent;
  font-family: var(--font-mono);
  font-size: 0.82rem;
  letter-spacing: 0.1em;
  text-transform: uppercase;
  color: var(--accent);
  cursor: pointer;
  text-decoration: underline;
  text-underline-offset: 0.2em;
}

.info-link:hover {
  filter: brightness(1.12);
}

.form {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 0.7rem 0.85rem;
  align-content: center;
}

.span-2 {
  grid-column: 1 / -1;
}

label {
  display: flex;
  flex-direction: column;
  gap: 0.3rem;
  font-size: 0.88rem;
  color: var(--ink);
}

label span em {
  font-style: normal;
  color: var(--muted);
  font-weight: 400;
}

input,
select {
  font: inherit;
  font-family: var(--font-mono);
  padding: 0.65rem 0.75rem;
  border: 1px solid var(--line);
  border-radius: 2px;
  background: var(--surface);
  color: var(--ink);
  outline: none;
}

input:focus,
select:focus {
  border-color: rgba(184, 255, 60, 0.55);
}

@media (max-width: 900px) {
  .panel {
    grid-template-columns: 1fr;
    align-content: center;
    gap: 1.25rem;
  }
}
</style>
