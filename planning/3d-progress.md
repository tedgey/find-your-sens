# 3D progress

Log of finished Unreal / repo-split features on the `3d-init` branch. Add an entry when a real capability ships, not after every commit.

## How to use this file

- One section per feature (often several atomic commits).
- Record what landed, where it lives, and what is still not true.
- No em dashes. Keep claims honest.

---

## 2026-09-10: Vue prototype lives in `2d/`

The web app is no longer at the repo root. Source, Vite config, and npm lockfile sit in [`2d/`](../2d/). GitHub Pages still deploys that app: the workflow installs and builds from `2d/` and uploads `2d/dist`. Root [`README.md`](../README.md) points at the 2D scripts (`cd 2d`) and at the Unreal sister project.

`npm test` from `2d/` still passes (8 tests).

---

## 2026-09-10: Unreal 5.6 C++ project in `3d/`

Blank C++ game module [`FindYourSens.uproject`](../3d/FindYourSens.uproject) targeting Engine 5.6. Frozen pawn (no mouse-look). Mouse smoothing off, FOV mouse scaling off, MouseX/Y/2D axis sensitivity 1.0 so capture is raw-leaning. Game mode, player controller, HUD, and pawn classes exist. Range geometry is spawned in code from engine basic shapes. Root `.gitignore` ignores Unreal build products.

Open from [`3d/README.md`](../3d/README.md). Needs Visual Studio 2022 with C++ game workloads; this machine has UE 5.6 but UBT reported Win64 as an invalid platform (no MSVC SDK in PATH), so PIE has not been run here yet.

---

## 2026-09-10: Solver port (Felt 90° still quotes the pack)

C++ math under `3d/Source/FindYourSens/` ports `2d/src/math/*`: games, path, angles, targets, solver. Quoted center is Felt 90° only (`sens = 90 / (|dx| * m_yaw)`). Flick / Casual / Micro feed ~30% of confidence. Unreal sets `bCaptureIsRaw`, so the browser ×0.78 EPP haircut is skipped. Automation tests in `Private/Tests/SensSolverTests.cpp` include the 1.13 vs ~2.4 pointing-baseline guardrail.

Formulas are duplicated on purpose. The TypeScript tests remain the 2D source of truth.

---

## 2026-09-10: Still-camera 3D session (setup through results)

Playable loop in PIE once the editor target compiles:

1. **Setup / How it works / Prep** as C++ Slate (no UMG assets).
2. **Felt 90°**: scripted 90° camera demo (1600 ms ease-out), then camera snaps back. Replicate is still. Arm (lime crosshair + ring) → move → commit. Pad travel is inches + cm at ~1/3 viewport height.
3. **Flick / Casual / Micro**: lime world markers at real yaw/pitch in the 2D angle bands. Camera stays still. Same arm/commit protocol.
4. **Results**: Felt-only sens range, consistency tags, confidence, notes.

Session plan matches 2D: 5 + 10 + 10 + 10. Escape during a round disarms and returns Felt 90° to the demo (same idea as losing pointer lock).

Not done: editor compile/PIE on this machine, Valorant and other titles, share/copy pack, in-game check-in.

---

## 2026-09-10: Click-through test, AimLabs box, visible markers

Prep and the in-test flow advance on left click. Setup still uses Continue because of the form. Felt 90° demo no longer needs Replay / Start replicate buttons: after the turn finishes, left click starts replicate, R replays. The test HUD is hit-test invisible so Slate does not eat those clicks.

Default map is `/Engine/Maps/Templates/Template_Default` instead of OpenWorld. [`SensRangeBuilder`](../3d/Source/FindYourSens/Private/SensRangeBuilder.cpp) now spawns a sealed dark box (floor, walls, ceiling, one dim light). Posts, props, fog, and skylight are gone.

Flick / Casual / Micro markers were spawning but never drawing: the sphere component stayed `HiddenInGame`. [`SensTargetActor`](../3d/Source/FindYourSens/Private/SensTargetActor.cpp) now toggles actor and mesh visibility, uses a larger (~45 cm) lime basic-shape sphere.

Not done: PIE on this machine. Results still uses New setup / Retest buttons (those are choices).
