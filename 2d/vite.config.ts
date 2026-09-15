/// <reference types="vitest/config" />
import vue from '@vitejs/plugin-vue'
import { defineConfig } from 'vite'

export default defineConfig({
  plugins: [vue()],
  base: process.env.GITHUB_ACTIONS ? '/find-your-sens/' : '/',
  test: {
    environment: 'node',
    include: ['src/**/*.test.ts'],
  },
})
