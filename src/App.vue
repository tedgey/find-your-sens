<script setup lang="ts">
import { useSession } from './composables/useSession'
import SetupView from './views/SetupView.vue'
import InfoView from './views/InfoView.vue'
import PrepView from './views/PrepView.vue'
import TestView from './views/TestView.vue'
import ResultsView from './views/ResultsView.vue'

const { state, backToSetup } = useSession()
</script>

<template>
  <div class="shell" :class="`shell--${state.step}`">
    <header v-if="state.step !== 'test'" class="top">
      <button
        v-if="state.step === 'info'"
        type="button"
        class="brand brand--link"
        @click="backToSetup"
      >
        Find Your Sens
      </button>
      <span v-else class="brand">Find Your Sens</span>
      <span
        v-if="state.step !== 'setup' && state.step !== 'info'"
        class="game-pill"
      >
        {{ state.setup.gameId.toUpperCase() }}
      </span>
    </header>

    <main>
      <SetupView v-if="state.step === 'setup'" />
      <InfoView v-else-if="state.step === 'info'" />
      <PrepView v-else-if="state.step === 'prep'" />
      <TestView v-else-if="state.step === 'test'" />
      <ResultsView v-else />
    </main>
  </div>
</template>

<style scoped>
.shell {
  min-height: 100dvh;
  display: flex;
  flex-direction: column;
  padding: 1rem clamp(1.5rem, 4vw, 3rem) 1rem;
}

.shell--test {
  height: 100dvh;
  max-height: 100dvh;
  overflow: hidden;
  padding: 0.65rem clamp(1.5rem, 4vw, 3rem);
}

.top {
  display: flex;
  align-items: center;
  justify-content: space-between;
  flex-shrink: 0;
  margin-bottom: 0.85rem;
}

.brand {
  font-family: var(--font-display);
  font-size: 1.05rem;
  font-weight: 700;
  letter-spacing: 0.08em;
  text-transform: uppercase;
  color: var(--ink);
}

.brand--link {
  appearance: none;
  border: none;
  background: transparent;
  padding: 0;
  cursor: pointer;
  text-align: left;
}

.brand--link:hover {
  color: var(--accent);
}

.game-pill {
  font-family: var(--font-mono);
  font-size: 0.78rem;
  letter-spacing: 0.14em;
  text-transform: uppercase;
  color: var(--accent);
  border: 1px solid rgba(184, 255, 60, 0.35);
  padding: 0.25rem 0.55rem;
}

main {
  flex: 1 1 auto;
  min-width: 0;
  display: flex;
  justify-content: center;
  align-items: stretch;
  width: 100%;
}

.shell--test main {
  flex: 1 1 0;
  min-height: 0;
  overflow: hidden;
}
</style>
