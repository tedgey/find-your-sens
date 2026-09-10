# 3D progress

Log of finished Unreal / repo-split features on the `3d-init` branch. Add an entry when a real capability ships, not after every commit.

## How to use this file

- One section per feature (often several atomic commits).
- Record what landed, where it lives, and what is still not true.
- No em dashes. Keep claims honest.

---

## 2026-09-10: Vue prototype lives in `2d/`

The web app is no longer at the repo root. Source, Vite config, and npm lockfile sit in [`2d/`](../2d/). GitHub Pages still deploys that app: the workflow installs and builds from `2d/` and uploads `2d/dist`. Root [`README.md`](../README.md) points at the 2D scripts (`cd 2d`) and leaves a slot for the Unreal sister project.

`npm test` from `2d/` still passes (8 tests).

Not done yet: Unreal project, 3D arena, math port.
