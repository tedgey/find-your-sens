<script setup lang="ts">
import TestArena from '../components/TestArena.vue'
import { useSession } from '../composables/useSession'
import type { PathSample, TargetSpec } from '../math'

const { state, currentScenario, roundsThisScenario, recordRound } = useSession()

function onComplete(payload: {
  target: TargetSpec | null
  samples: PathSample[]
  net: { x: number; y: number }
  durationMs: number
  arenaWidth: number
  arenaHeight: number
}) {
  recordRound(payload)
}
</script>

<template>
  <section class="test">
    <TestArena
      :scenario="currentScenario"
      :round-index="state.roundIndex"
      :rounds-per-scenario="roundsThisScenario"
      :setup="state.setup"
      @complete="onComplete"
    />
  </section>
</template>

<style scoped>
.test {
  width: 100%;
  height: 100%;
  min-height: 0;
  display: flex;
}

:global(:fullscreen) .test,
:global(:-webkit-full-screen) .test {
  width: 100%;
  height: 100%;
}
</style>
