# Find Your Sens

Blind intent-mapping tool that turns how you *think* you should move your mouse into a game settings pack (DPI tier + sensitivity range).

## Prototype scope

- Web app (Vue 3 + Vite + TypeScript)
- CS2 first
- Scenarios: flick · casual · micro (10 blind rounds each)
- Path-aware, confidence-weighted recommendation

## Scripts

```bash
npm install
npm run dev
npm test
npm run build
```

## Deploy

Push to `main` and GitHub Actions will deploy the app to GitHub Pages at:

`https://tedgey.github.io/find-your-sens/`

In the repository settings, set **Pages** to use **GitHub Actions** as the source.
