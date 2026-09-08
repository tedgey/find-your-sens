<script setup lang="ts">
import { useSession } from '../composables/useSession'

const { backToSetup } = useSession()
</script>

<template>
  <section class="panel">
    <header class="head">
      <div>
        <p class="eyebrow">About this tool</p>
        <h1>How Find Your Sens works</h1>
        <p class="lede">
          A blind, open-loop test that turns how your hand wants to move into a
          DPI + in-game sensitivity pack. Not an aim trainer. Not a pro-sens
          converter.
        </p>
      </div>
      <button type="button" class="btn btn--ghost" @click="backToSetup">
        Back
      </button>
    </header>

    <div class="sections">
      <article>
        <h2>Purpose</h2>
        <p>
          Most sens tools copy someone else’s settings or convert cm/360 between
          games. This one asks a different question: when you intend a turn, how
          far does your hand actually move? We treat that intended travel as the
          source of truth, then quote a sensitivity that would make that travel
          feel like a real 90° yaw in CS2 (and later other FPS titles).
        </p>
        <p>
          The view stays still on purpose. You move as if the world were
          turning. That keeps the test open-loop: you commit without live
          feedback that would train you toward a geometric target.
        </p>
      </article>

      <article>
        <h2>What the test includes</h2>
        <ol>
          <li>
            <strong>Felt 90°</strong> (5 rounds): watch a real 90° demo, then
            replicate that mouse travel. Bars yaw with your mouse; inches and cm
            show pad distance. stop when it feels like you've turned 90°.
          </li>
          <li>
            <strong>Flick</strong> (10): commit as if snapping onto a target,
            then click.
          </li>
          <li>
            <strong>Casual</strong> (10): place the crosshair more smoothly,
            then click.
          </li>
          <li>
            <strong>Micro</strong> (10): small correction onto the mark, then
            click.
          </li>
        </ol>
        <p>
          Setup asks for game, DPI, resolution, optional FOV, and optional
          current sens. Prep covers Windows pointer speed, Enhance pointer
          precision, and fullscreen.
        </p>
      </article>

      <article>
        <h2>Absolute vs consistency</h2>
        <p>
          <strong>Absolute</strong> means the stage sets the number we quote.
          Only Felt 90° does that. Horizontal mouse counts map to yaw with the
          game’s <code>m_yaw</code> (CS2: 0.022):
          <code>sens ≈ 90 / (|counts| × m_yaw)</code>.
        </p>
        <p>
          <strong>Consistency</strong> means Flick, Casual, and Micro do not set
          the quoted sens. Pointing at a static on-screen mark mostly measures
          desktop-style aiming (often near geometric ~1:1), not FPS turn feel.
          We still score path quality and repeatability there. That feeds
          overall confidence (~30%), not the center of the range.
        </p>
      </article>

      <article>
        <h2>How the pack is built</h2>
        <ul>
          <li>
            Per Felt 90° round we estimate an implied sens from horizontal
            travel only (vertical drift is ignored for absolute scale).
          </li>
          <li>
            Rounds are weighted by path cleanliness (straighter, fewer late
            corrections count more).
          </li>
          <li>
            The quoted range is a weighted blend of Felt 90° estimates, with
            width from how tightly those estimates agree.
          </li>
          <li>
            Target-stage medians appear in the results table as a pointing
            baseline (diagnostic). They are expected to look different from Felt
            90° and are not what we recommend.
          </li>
          <li>
            Optional current sens is compared as a ratio so you can see how far
            the pack sits from what you already run.
          </li>
        </ul>
      </article>

      <article>
        <h2>Confidence</h2>
        <p>
          Overall confidence is mostly Felt 90° stability (~70%), plus average
          path confidence from the target stages (~30%). If Enhance pointer
          precision is on and the game uses raw input (CS2 does), we apply a
          silent ×0.78 haircut: the browser may still accelerate fast moves
          while the game would not. Off for the test is the cleanest match.
        </p>
      </article>

      <article>
        <h2>What you get</h2>
        <p>
          A settings pack at your typed DPI: sensitivity range, center, eDPI
          band, cm/360, confidence with a short breakdown, per-scenario medians
          and tags, and notes about assumptions (FOV, arena size, EPP).
        </p>
      </article>

      <article>
        <h2>What this is not</h2>
        <ul>
          <li>Not aim training, tracking drills, or recoil practice.</li>
          <li>Not “use this pro’s sens.”</li>
          <li>
            Not a claim that screen-pointing alone equals in-game yaw feel.
          </li>
          <li>
            Treat a large gap vs your current sens as experimental: try it
            in-game before committing.
          </li>
        </ul>
      </article>
    </div>
  </section>
</template>

<style scoped>
.panel {
  width: min(820px, 100%);
  display: flex;
  flex-direction: column;
  gap: 1rem;
  padding-bottom: 2rem;
}

.head {
  display: flex;
  align-items: flex-start;
  justify-content: space-between;
  gap: 1.25rem;
}

.eyebrow {
  margin: 0 0 0.3rem;
  font-family: var(--font-mono);
  font-size: 0.75rem;
  letter-spacing: 0.1em;
  text-transform: uppercase;
  color: var(--accent);
}

h1 {
  margin: 0;
  font-family: var(--font-display);
  font-size: clamp(1.7rem, 3.2vw, 2.4rem);
  font-weight: 700;
  letter-spacing: 0.03em;
  text-transform: uppercase;
  line-height: 1.05;
  color: var(--ink);
}

.lede {
  margin: 0.5rem 0 0;
  max-width: 36rem;
  color: var(--muted);
  font-size: 0.98rem;
  line-height: 1.45;
}

.sections {
  display: flex;
  flex-direction: column;
  gap: 1.15rem;
}

article {
  padding: 0.95rem 1.05rem;
  border: 1px solid var(--line);
  background: var(--surface);
}

h2 {
  margin: 0 0 0.45rem;
  font-family: var(--font-display);
  font-size: 0.95rem;
  font-weight: 700;
  letter-spacing: 0.08em;
  text-transform: uppercase;
  color: var(--accent);
}

p,
li {
  margin: 0;
  color: var(--muted);
  font-size: 0.95rem;
  line-height: 1.5;
}

p + p,
ul + p,
ol + p {
  margin-top: 0.55rem;
}

ul,
ol {
  margin: 0.35rem 0 0;
  padding-left: 1.15rem;
  display: flex;
  flex-direction: column;
  gap: 0.45rem;
}

strong {
  color: var(--ink);
  font-weight: 600;
}

code {
  font-family: var(--font-mono);
  font-size: 0.86em;
  color: var(--ink);
  background: rgba(184, 255, 60, 0.08);
  padding: 0.05rem 0.3rem;
  border-radius: 2px;
}

@media (max-width: 640px) {
  .head {
    flex-direction: column;
    align-items: stretch;
  }

  .head .btn {
    align-self: flex-start;
  }
}
</style>
