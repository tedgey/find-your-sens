import { describe, expect, it } from 'vitest'
import {
  buildDisplayConfig,
  getGame,
  impliedSensitivity,
  impliedSensFromFeelTurn,
  offsetPxToAngles,
  recommendFromRounds,
  scaleSensForDpi,
  cmPer360,
  eppStageWeight,
  FEEL_TURN_DEG,
} from './index'
import type { RoundRecording, SessionSetup } from './types'

describe('CS2 FOV + angles', () => {
  it('expands Source 90 FOV on 16:9', () => {
    const game = getGame('cs2')
    const display = buildDisplayConfig(game, 90, 1920, 1080)
    expect(display.horizontalFovDeg).toBeGreaterThan(100)
    expect(display.horizontalFovDeg).toBeLessThan(110)
  })

  it('maps center-right pixel offset to positive yaw', () => {
    const game = getGame('cs2')
    const display = buildDisplayConfig(game, 90, 1920, 1080)
    const { yawDeg, pitchDeg } = offsetPxToAngles({ x: 400, y: 0 }, display)
    expect(yawDeg).toBeGreaterThan(0)
    expect(Math.abs(pitchDeg)).toBeLessThan(1e-6)
  })
})

describe('implied sensitivity', () => {
  it('recovers known Source sens from matching mouse delta', () => {
    const yawConstant = 0.022
    const sens = 1.0
    const yawDeg = 30
    const mouseDx = yawDeg / (sens * yawConstant)
    const recovered = impliedSensitivity(yawDeg, 0, mouseDx, 0, yawConstant)
    expect(recovered).toBeCloseTo(1.0, 5)
  })

  it('recovers sens from a felt 90° turn', () => {
    const yawConstant = 0.022
    const sens = 1.13
    const dx = FEEL_TURN_DEG / (sens * yawConstant)
    expect(impliedSensFromFeelTurn(FEEL_TURN_DEG, dx, yawConstant)).toBeCloseTo(1.13, 5)
  })

  it('scales inversely with DPI tier', () => {
    expect(scaleSensForDpi(1.0, 800, 400)).toBeCloseTo(2.0)
    expect(scaleSensForDpi(1.0, 800, 1600)).toBeCloseTo(0.5)
  })

  it('computes cm/360', () => {
    const cm = cmPer360(1, 800, 0.022)
    expect(cm).toBeGreaterThan(50)
    expect(cm).toBeLessThan(53)
  })
})

describe('recommendFromRounds', () => {
  it('quotes absolute sens from felt turns, not target pointing', () => {
    const setup: SessionSetup = {
      gameId: 'cs2',
      dpi: 400,
      fovSetting: null,
      resolutionWidth: 1920,
      resolutionHeight: 1080,
      enhancePointerPrecision: 'off',
      currentSens: 1.13,
    }

    const feelSens = 1.13
    const feelDx = FEEL_TURN_DEG / (feelSens * 0.022)
    // Pointing-like target rounds around geometric ~2.4
    const pointSens = 2.4
    const pointYaw = 30
    const pointDx = pointYaw / (pointSens * 0.022)

    const rounds: RoundRecording[] = []
    for (let i = 0; i < 5; i++) {
      rounds.push({
        scenario: 'feel90',
        roundIndex: i,
        target: null,
        samples: [
          { t: 0, dx: 0, dy: 0 },
          { t: 100, dx: feelDx * (1 + (i - 2) * 0.02), dy: 0 },
        ],
        net: { x: feelDx * (1 + (i - 2) * 0.02), y: 0 },
        durationMs: 100,
        arenaWidth: 1920,
        arenaHeight: 1080,
      })
    }
    for (const scenario of ['flick', 'casual', 'micro'] as const) {
      for (let i = 0; i < 10; i++) {
        rounds.push({
          scenario,
          roundIndex: i,
          target: { offsetPx: { x: 200, y: 0 }, yawDeg: pointYaw, pitchDeg: 0 },
          samples: [
            { t: 0, dx: 0, dy: 0 },
            { t: 100, dx: pointDx, dy: 0 },
          ],
          net: { x: pointDx, y: 0 },
          durationMs: 100,
          arenaWidth: 1920,
          arenaHeight: 1080,
        })
      }
    }

    const rec = recommendFromRounds(setup, rounds)
    expect(rec.sensRange.center).toBeGreaterThan(0.9)
    expect(rec.sensRange.center).toBeLessThan(1.4)
    expect(rec.pointingBaselineSens).toBeGreaterThan(2)
    expect(rec.currentSens).toBe(1.13)
    expect(rec.vsCurrentRatio).not.toBeNull()
    expect(rec.dpi).toBe(400)
  })

  it('down-weights flicks when EPP is on', () => {
    expect(eppStageWeight('flick', true)).toBeLessThan(eppStageWeight('micro', true))
  })
})
