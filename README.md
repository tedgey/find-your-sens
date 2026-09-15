# Find Your Sens

Blind intent-mapping tool that turns how you _think_ you should move your mouse into a game settings pack (DPI tier + sensitivity range).

This repo holds two prototypes:

| Folder | What it is |
| --- | --- |
| [`2d/`](2d/) | Vue 3 + Vite web app (CS2-first). This is what GitHub Pages deploys. |
| [`3d/`](3d/) | Unreal Engine 5.6 first-person sister project. |

Product notes live in [`planning/`](planning/).

## 2D web prototype

CS2 first. Felt 90° sets absolute scale. Flick, casual, and micro are consistency checks.

```bash
cd 2d
npm install
npm run dev
npm test
npm run build
```

Push to `main` and GitHub Actions deploys the `2d/` app to GitHub Pages at:

`https://tedgey.github.io/find-your-sens/`

In the repository settings, set **Pages** to use **GitHub Actions** as the source.

## 3D Unreal prototype

Requires Unreal Engine 5.6. See [`3d/README.md`](3d/README.md).
