import type { ScenarioId } from './types'
import { makeTarget } from './angles'
import type { DisplayConfig, TargetSpec } from './types'

export interface SpawnRange {
  /** Minimum angular magnitude from crosshair, degrees. */
  minAngleDeg: number
  maxAngleDeg: number
}

export const SCENARIO_RANGES: Record<Exclude<ScenarioId, 'feel90'>, SpawnRange> = {
  flick: { minAngleDeg: 18, maxAngleDeg: 48 },
  casual: { minAngleDeg: 12, maxAngleDeg: 32 },
  micro: { minAngleDeg: 2.5, maxAngleDeg: 8 },
}

function rand(min: number, max: number): number {
  return min + Math.random() * (max - min)
}

/** Convert desired yaw/pitch degrees into pixel offset for this display. */
function anglesToOffsetPx(
  yawDeg: number,
  pitchDeg: number,
  display: DisplayConfig,
): { x: number; y: number } {
  const halfW = display.width / 2
  const halfH = display.height / 2
  const aspect = display.width / display.height
  const halfHFov = (display.horizontalFovDeg * Math.PI) / 360
  const vFov =
    2 *
    Math.atan(Math.tan(halfHFov) / aspect) *
    (180 / Math.PI)
  const halfVFov = (vFov * Math.PI) / 360

  const x = (Math.tan((yawDeg * Math.PI) / 180) / Math.tan(halfHFov)) * halfW
  const y = (Math.tan((pitchDeg * Math.PI) / 180) / Math.tan(halfVFov)) * halfH
  return { x, y }
}

/**
 * Spawn a target at a random direction with angular magnitude in the scenario band.
 * Keeps the point inside ~90% of the half-frame so it stays visible.
 */
export function spawnTarget(scenario: Exclude<ScenarioId, 'feel90'>, display: DisplayConfig): TargetSpec {
  const range = SCENARIO_RANGES[scenario]
  const magnitude = rand(range.minAngleDeg, range.maxAngleDeg)
  const theta = rand(0, Math.PI * 2)
  let yawDeg = Math.cos(theta) * magnitude
  let pitchDeg = Math.sin(theta) * magnitude

  let offset = anglesToOffsetPx(yawDeg, pitchDeg, display)
  const maxX = display.width * 0.45
  const maxY = display.height * 0.45
  const scale = Math.max(Math.abs(offset.x) / maxX, Math.abs(offset.y) / maxY, 1)
  if (scale > 1) {
    offset = { x: offset.x / scale, y: offset.y / scale }
  }

  return makeTarget(offset, display)
}
