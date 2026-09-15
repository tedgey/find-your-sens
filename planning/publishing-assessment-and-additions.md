# Publishing assessment and additions

Honest product take from a Sep 2026 review of `InfoView.vue` and the surrounding prototype: is Find Your Sens unique and worth publishing, and what is worth adding next.

---

## 1. Verdict

**Publish it.** The method is meaningfully different from most sens tools. It will not invent a category overnight, but it is worth putting in front of people, especially as a free CS2-first prototype.

Frame it as an experiment with a sharp thesis: natural turn travel should drive yaw scale; screen pointing should not. Do not frame it yet as definitive sens science.

---

## 2. What is unique

Most of the market does one of three things:

1. Copy a pro’s settings
2. Convert cm/360 / eDPI between games
3. Optimize closed-loop aim (hit targets, then nudge sens)

PSA-style methods are feel-based, but they still rely on long in-game A/B loops.

Find Your Sens asks a different question: when you intend a turn, how far does your hand actually move?

### Method edges

- **Open-loop intent mapping**: view stays still on purpose so live yaw feedback does not train the user toward a geometric target.
- **Felt 90° sets absolute scale** from intended horizontal travel, mapped with the game’s yaw constant (`sens ≈ 90 / (|counts| × m_yaw)` for CS2).
- **Flick / Casual / Micro are consistency diagnostics**, not the quoted number. Pointing at static marks tends to measure desktop-style aiming (~1:1), not FPS turn feel.
- **Path quality and confidence**: weighted Felt 90° estimates, range width from agreement, overall confidence ~70% Felt stability + ~30% target-stage path confidence, plus an EPP-on haircut when the game uses raw input.

The Info copy already states clear non-claims (not aim training, not pro-sens, not “screen pointing equals yaw feel”). That honesty helps trust.

---

## 3. What is not unique enough alone

“Find your sens” as a category already exists (aim trainers, MouseTester PSA, converters, pro databases). The edge is the **method**, not the category name.

People will still ask why not use a converter. The answer has to stay specific: measure intended travel without live yaw feedback.

**Main product risk: validity.** Does Felt 90° without camera motion predict good in-game feel? If browser travel maps poorly to in-game feel for many players, uniqueness will not save retention. Treat large gaps vs current sens as experimental.

---

## 4. Worth publishing?

Yes, especially as a free / public prototype for FPS players who already care about sens:

- Clear problem, clear non-claims, credible math story
- Niche but passionate audience
- Easy to share (“do this short blind test, get a range”)
- Room to grow (more games, better raw-input story, validation against in-game feel)

Publish bar for a free web tool is low enough to clear. Monetization / must-use default is a higher bar and needs evidence that packs feel right after real play.

---

## 5. Suggested additions (priority order)

### Highest leverage

1. **In-app closed-loop check-in** (3D first)  
   After the pack: pick a sens (default center), replay the same stages with live FPS look, then show how close they got to 90° and how Flick / Casual / Micro landed (on-target / close / miss). Diagnostic only; do not rewrite the pack. Real-match feedback (too slow / too fast / right after a game) is later, once this in-app loop exists.

2. **Bridge when far from current**  
   Already flag large ratios. Add a practical intermediate (e.g. halfway toward the pack) so users are less likely to discard the result after one weird session.

3. **One-click copy pack**  
   Pasteable `sensitivity X` (and later game-specific lines). Last mile for publishing.

### Sharpen the method

4. **Short mode**  
   Full plan is 35 rounds. Offer a shorter Felt-heavy path for first publish; keep full mode as high confidence. Friction hurts shareability more than missing another title.

5. **Second absolute check**  
   Felt 180° or reverse 90° as a cross-check. If absolute stages disagree hard, widen the range / lower confidence.

6. **Retest delta**  
   Same setup, second run: show prior center vs new center. Builds trust that the tool is not random.

### Nice later (not blocking publish)

- Ship **Valorant** next (profile already exists); leave Fortnite until the yaw constant is real.
- Persist last pack in localStorage so refresh does not erase it.
- Optional share card: cm/360, eDPI, confidence, “from Felt 90°.”

### Explicitly deprioritize

Aim drills, pro databases, and a big multi-game converter. Those dilute the differentiator.

---

## 6. Bottom line

Unique enough to publish. Not unique enough that polish alone wins. The win path is tighter validation of Felt 90°, easier adoption of the number, and less commitment shock when the pack disagrees with habit.
