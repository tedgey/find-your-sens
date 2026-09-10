# Find Your Sens: prototype decisions

Product thesis, interview decisions, math lessons, UX rules, and current state from the Sep 2026 build session. Source chat: [Mouse sensitivity tool design](a0f347cc-a9c8-420e-aba3-83a68b36cb06).

---

## 1. Origin

User wanted a mouse-sensitivity tool that analyses movement across scenarios and delivers ideal DPI + in-game sensitivity for popular FPS games. Before planning, we interviewed for product, scenarios, math, UX, and non-goals.

Working name: **Find Your Sens**.

---

## 2. Product thesis (locked)

- **Not** an aim trainer. No tracking, recoil, Kovaaks-style drills, or “beat this score.”
- **Not** a pro-sens copy tool or a simple cm/360 converter.
- **Is** a blind, open-loop **intent mapper**: extract what the user’s brain thinks a turn or aim move should feel like, then turn that into a per-game **settings pack** (DPI + sens range + confidence).

Differentiator: bespoke sens from *their* intended travel, not someone else’s settings.

### Games priority

1. CS2 (prototype focus)
2. Valorant
3. PUBG
4. Apex
5. Fortnite

### Platform

- Web-first (Vue 3 + Vite + TypeScript). Comfort zone + ship now.
- Desktop / Steam later if raw input becomes the accuracy differentiator.
- Free to use; profitability deferred until results feel stable and believable.

### Accounts

- v1: one-shot anonymous.
- v2: history / accounts / progress.

---

## 3. Interview decisions (early rounds)

| Topic | Decision |
| --- | --- |
| Deliverable | Full settings pack per game |
| Success metric | Map brain → mouse travel → game sens; consistency + less overshoot/undershoot |
| Core mechanic | Screen stays still; user moves as if it didn’t; click commits; path is recorded |
| Blindness | Full blind until end |
| Rounds | ~10 per target scenario with variance; stddev informs confidence |
| Targets | Fixed positions first |
| DPI | User types it |
| FOV / res | Ask; FOV optional with game defaults |
| Path | Path quality matters (not net displacement alone) |
| Output | Confidence-weighted tight range; DPI tiers for optional conversion |
| Windows accel | Consider Enhance pointer precision; detect if possible, else instruct |

### Explicit non-goals (v1)

- Aim training, tracking scenarios, recoil practice
- Telemetry productization (undecided; not a v1 focus)
- Steam publish path (unknown; build foundation first)

---

## 4. Prototype flow (as built)

1. **Setup** (home): game, DPI, resolution, optional FOV, optional current sens. Link to **How it works**.
2. **Prep**: checklist, Felt 90° explanation, EPP on/off.
3. **Test** (pointer lock, fullscreen-friendly arena):
   - Felt 90° × 5 (absolute scale)
   - Flick × 10 (consistency)
   - Casual × 10 (consistency)
   - Micro × 10 (consistency)
4. **Results**: settings pack + per-scenario table + notes.

Session machine lives in `src/composables/useSession.ts` (steps: `setup` | `info` | `prep` | `test` | `results`). No Vue Router yet.

---

## 5. Math evolution (hard-won)

### Problem 1: early quotes were ~2× too high

Static-target angle mapping used wrong size (canvas vs arena / res). Fixed to use arena size for angles. Still not enough: target stages alone overstated CS2 sens vs user reality (~1.13).

### Problem 2: screen pointing ≠ FPS yaw feel

Pointing at a static on-screen mark tends toward **geometric ~1:1 / ~2.4–2.5** desktop-style aiming. That is **not** what we should quote for CS2.

### Fix: Felt 90° as absolute scale

- Demo animates a real 90° world turn.
- User replicates; bars yaw with mouse during replicate.
- Live pad travel shown as **inches + cm** (`|net.x| / DPI`).
- Absolute sens: `sens ≈ 90 / (|counts| × m_yaw)` (CS2 `m_yaw = 0.022`).
- **Quoted pack comes from Felt 90° only.**

### Absolute vs consistency (results tags)

- **Absolute** (Felt 90°): sets the number we quote.
- **Consistency** (Flick / Casual / Micro): does **not** set quoted sens. Path quality feeds overall confidence (~30%). Stage medians are a diagnostic “pointing baseline,” often near ~1:1 and expected to differ from Felt 90°.

### Confidence

- ~70% Felt 90° stage confidence + ~30% average target-stage path confidence.
- If EPP is **on** and game uses raw input: apply **×0.78** haircut (browser may accelerate; game won’t). Show this in the breakdown (explains stage % ~90+ while overall ~74%).
- Optional current sens → `vsCurrentRatio` warning when far off.

### Solver notes worth preserving

- Path weight: straighter + fewer late corrections → higher weight.
- Vertical drift ignored for Felt 90° absolute scale.
- Pointing baseline still computed/reported; never quoted as the pack center.

Key code: `src/math/solver.ts`, `angles.ts`, `path.ts`, `games.ts`.

---

## 6. Felt 90° UX arc

Open-loop “imagine 90°” was too mental. Iterations:

1. Explain Felt 90° on prep.
2. Add **demo animation** of a true 90° turn.
3. Show **cm** (then **inches + cm**) as pad reference.
4. During replicate, **bars move with the mouse** like the demo.
5. Remove `+90°` post labels and live degree readout; keep travel as the feedback.
6. Place travel readout centered at ~**1/3 canvas height**.
7. Copy: “peek” → “turn” everywhere.
8. Directive text: prominent lime, **no pulse** (pulse was called obnoxious).
9. Replay demo / Start replicate sit on the same row as the directive, just above the canvas.

---

## 7. Design / UX rules (user-enforced)

### Visual

- Lean into gamer tropes: dark palette, futuristic display type.
- Fonts: Oxanium / Rajdhani / Share Tech Mono; accent lime `#b8ff3c`.
- Armed crosshair: **lime** + ring. Idle: white. Measurement posts/horizon: **gray** (so armed state is obvious).
- Brand header: “Find Your Sens.” Wider panels; results ~1100px; avoid sparse “insane whitespace” cards.
- **Never use em dashes (`—`) or en dashes (`–`)** in copy, UI, comments, commits, or docs. Rule file: `.cursor/rules/no-em-dashes.mdc`.

### Layout / scroll (critical)

- Prefer **no scrolling** on setup / prep / results when content fits.
- **Never** interior/nested scrollbars on panels.
- If content must scroll (e.g. How it works), scroll the **page**, not an inner region.
- Test step may lock to viewport height (arena / pointer lock).

### Prep / results polish notes from session

- Prep cards: consistent titles; no weird left orange border accents.
- Results: widen delivery panel; two-column body; avoid panel scrollbars.
- One results layout was reverted after a bad redesign (“old settings pack looked better”).

---

## 8. Monetization (discussed, not built)

Ideas that fit the product:

1. Freemium: free one-game one-shot; paid multi-game packs, history, export/share.
2. Careful hardware affiliate on results.
3. B2B / creator / academy licenses later.
4. Paid raw-input desktop later.

**Do first:** ship until retests are stable and believable; trust > ads on results.

---

## 9. Current app surface

| Area | Role |
| --- | --- |
| `SetupView` | Home / setup form + How it works link |
| `InfoView` | Purpose, stages, absolute vs consistency, pack math, confidence, non-goals |
| `PrepView` | Checklist, Felt 90°, EPP |
| `TestView` + `TestArena` | Pointer lock, demo, canvas, HUD |
| `ResultsView` | Pack + stages + assumptions + notes |
| `src/math/*` | Games, angles, path, targets, solver + tests |

Deploy: GitHub Pages via Actions (`README.md`).

---

## 10. Open / next (from conversation, not finished)

- More games beyond CS2 (Valorant → …).
- Desktop raw-input path when browser accel remains a ceiling.
- Accounts / history (v2).
- Tighten Felt 90° UX further if users still find it mental.
- Keep validating quoted sens against real in-game feel (user’s ~1.13 was the calibration north star during debugging).
- README still slightly stale vs current scenario list (Felt 90° + consistency stages).

---

## 11. One-line status

**Working Vue prototype:** Felt 90° sets absolute CS2 sens; target stages score consistency; dark gamer UI; How it works page; page-level scroll only when needed; no nested panel scrollbars.
