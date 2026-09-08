<script setup lang="ts">
import { ref } from 'vue'
import { getGame, type EnhancePointerPrecision } from '../math'
import { useSession } from '../composables/useSession'

const { state, beginTest, backToSetup, saveSetup, totalRounds } = useSession()
const game = getGame(state.setup.gameId)
const epp = ref<EnhancePointerPrecision>(state.setup.enhancePointerPrecision)

function start() {
  saveSetup({ enhancePointerPrecision: epp.value })
  beginTest()
}
</script>

<template>
  <section class="panel">
    <div class="stack">
      <div class="head">
        <p class="eyebrow">Before you start</p>
        <h1>Input prep</h1>
        <p class="lede">
          You’re testing for <strong>{{ game.name }}</strong> at
          <strong>{{ state.setup.dpi }} DPI</strong>
          · {{ totalRounds }} blind rounds · no feedback until the end.
        </p>
      </div>

      <div class="grid">
        <article class="card">
          <h2>Checklist</h2>
          <ol>
            <li>Windows pointer speed at the middle notch (6/11).</li>
            <li>Same mousepad posture you use in-game.</li>
            <li>Use fullscreen when prompted.</li>
            <li>Click to commit each round. Don’t “fix” after you stop.</li>
          </ol>
        </article>

        <article class="card">
          <h2>What is Felt 90°?</h2>
          <p>
            Watch a short demo of a real 90° turn, then copy that mouse travel.
            The bars move with your mouse like the demo. You’ll also see
            <strong>inches and cm</strong> for horizontal pad travel. Stop when
            it feels like you\'ve turned 90°; that distance sets your absolute
            sensitivity. Later stages only check consistency.
          </p>
        </article>

        <article class="card">
          <h2>Enhance pointer precision</h2>
          <p>
            {{ game.name }} ignores Windows acceleration (raw input). The
            browser may not. Off for this test is cleanest; on is supported with
            a confidence haircut.
          </p>
          <div
            class="choices"
            role="radiogroup"
            aria-label="Enhance pointer precision"
          >
            <label class="choice">
              <input v-model="epp" type="radio" value="on" />
              <span>
                <strong>Leave it on</strong>
                <small>Typical default · slightly wider range</small>
              </span>
            </label>
            <label class="choice">
              <input v-model="epp" type="radio" value="off" />
              <span>
                <strong>Off for this test</strong>
                <small>Best match for raw-input games</small>
              </span>
            </label>
          </div>
        </article>
      </div>

      <div class="actions">
        <button type="button" class="btn btn--ghost" @click="backToSetup">
          Back
        </button>
        <button type="button" class="btn" @click="start">Start test</button>
      </div>
    </div>
  </section>
</template>

<style scoped>
.panel {
  width: min(1400px, 100%);
  height: 100%;
  min-height: 0;
  display: flex;
  align-items: center;
}

.stack {
  width: 100%;
  display: flex;
  flex-direction: column;
  gap: 1.25rem;
}

.head {
  max-width: 42rem;
}

.eyebrow {
  margin: 0 0 0.25rem;
  font-family: var(--font-mono);
  font-size: 0.75rem;
  letter-spacing: 0.08em;
  text-transform: uppercase;
  color: var(--accent);
}

h1 {
  margin: 0;
  font-family: var(--font-display);
  font-size: clamp(1.8rem, 3.5vw, 2.6rem);
  font-weight: 700;
  letter-spacing: 0.04em;
  text-transform: uppercase;
  color: var(--ink);
}

.lede {
  margin: 0.45rem 0 0;
  color: var(--muted);
  line-height: 1.4;
  font-size: 0.95rem;
}

.grid {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 1rem;
  align-items: start;
}

.card {
  margin: 0;
  padding: 1rem 1.1rem 1.1rem;
  border: 1px solid var(--line);
  border-radius: 2px;
  background: var(--surface);
}

.card h2 {
  margin: 0 0 0.55rem;
  font-family: var(--font-display);
  font-size: 1rem;
  font-weight: 700;
  letter-spacing: 0.06em;
  text-transform: uppercase;
  color: var(--accent);
}

.card p,
.card ol {
  margin: 0;
  color: var(--muted);
  font-size: 0.9rem;
  line-height: 1.45;
}

.card ol {
  padding-left: 1.15rem;
  color: var(--ink);
}

.card li + li {
  margin-top: 0.35rem;
}

.choices {
  display: flex;
  flex-direction: column;
  gap: 0.55rem;
  margin-top: 0.75rem;
}

.choice {
  display: flex;
  gap: 0.55rem;
  align-items: flex-start;
  cursor: pointer;
  color: var(--ink);
}

.choice input {
  margin-top: 0.25rem;
}

.choice strong {
  display: block;
  font-size: 0.9rem;
}

.choice small {
  display: block;
  margin-top: 0.1rem;
  color: var(--muted);
  font-size: 0.8rem;
}

.actions {
  display: flex;
  gap: 0.65rem;
}

@media (max-width: 960px) {
  .grid {
    grid-template-columns: 1fr 1fr;
  }
}

@media (max-width: 720px) {
  .panel {
    align-items: stretch;
  }

  .grid {
    grid-template-columns: 1fr;
  }
}
</style>
