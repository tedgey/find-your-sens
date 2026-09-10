# Find Your Sens (Unreal 3D)

First-person sister of the Vue prototype. Camera stays still during measurement. Felt 90° sets the quoted CS2 sens. Flick / Casual / Micro are consistency checks.

## Requirements

- Unreal Engine **5.6** (this machine: `C:\Program Files\Epic Games\UE_5.6`)
- Visual Studio 2022 with the Game Development with C++ workload

## Open

1. Double-click [`FindYourSens.uproject`](FindYourSens.uproject) (or right-click, Generate Visual Studio project files, then open).
2. Play In Editor. Setup, prep, test, and results are C++ Slate UI (no UMG assets required).
3. The range is spawned in code around the pawn. Geometry uses engine basic shapes.

## Product rules

- Measurement never yaws the camera (we do not have a sensitivity yet).
- Felt 90° demo is a scripted 90° camera turn, then the view snaps back.
- Replicate and pointing stages: centered crosshair, pad travel in inches + cm, arm then commit.
- Quoted pack comes from Felt 90° only. See `2d/src/math/` for the original formulas this module ports.

## Layout

- `Source/FindYourSens/` game module (session, solver, pawn, HUD, range)
- `Config/` mouse smoothing off, mouse axis sensitivity 1.0 (no look-scale)
- UI is C++ Slate (no UMG `.uasset` widgets), so a blank project can play without Content assets
- The range is spawned in C++ from engine basic shapes

## Compile

Needs Visual Studio 2022 with the **Desktop development with C++** workload plus Unreal's game development components. Then generate project files from `FindYourSens.uproject` and build `FindYourSensEditor`.
