# Find Your Sens (Unreal 3D)

First-person sister of the Vue prototype. Blind finder: camera stays still during measurement. Felt 90°, flick, casual, and micro all feed the quoted CS2 pack. After the pack you can test a chosen sensitivity with live look. Capture is raw, like CS2.

## Requirements

- Unreal Engine **5.6** (this machine: `C:\Program Files\Epic Games\UE_5.6`)
- Visual Studio 2022 with the Game Development with C++ workload

## Open

1. Double-click [`FindYourSens.uproject`](FindYourSens.uproject) (or right-click, Generate Visual Studio project files, then open).
2. Play In Editor. Setup, prep, test, and results are C++ Slate UI (no UMG assets required).
3. The range is a sealed dark box spawned in code around the pawn (engine basic shapes). Prep uses Start test. Setup still uses Continue.

## Product rules

- Blind measurement never yaws the camera (we do not have a sensitivity yet).
- Felt 90° demo is a scripted 90° camera turn, then the view snaps back. Left click starts replicate. R replays the demo.
- Replicate and pointing stages: centered crosshair, pad travel in inches + cm, arm then commit.
- Quoted pack is a confidence-weighted blend: Felt 90° ~40%, flick ~22%, casual ~22%, micro ~16%. Windows Enhance pointer precision is not part of the quote.
- After results, **Test your sensitivity** replays the same stages with live FPS look at a chosen sens (default: pack center). That report is diagnostic. It does not rewrite the pack.

## Layout

- `Source/FindYourSens/` game module (session, solver, pawn, HUD, range)
- `Config/` mouse smoothing off, mouse axis sensitivity 1.0 (no look-scale)
- UI is C++ Slate (no UMG `.uasset` widgets), so a blank project can play without Content assets
- The range is spawned in C++ from engine basic shapes

## Compile

Needs Visual Studio 2022 with the **Desktop development with C++** workload plus Unreal's game development components. Then generate project files from `FindYourSens.uproject` and build `FindYourSensEditor`.
