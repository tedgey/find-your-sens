import type { DisplayConfig, GameProfile, Point2, TargetSpec } from './types'

const DEG2RAD = Math.PI / 180
const RAD2DEG = 180 / Math.PI

/** Source engine: FOV setting is horizontal on 4:3; rescale to actual aspect. */
export function resolveHorizontalFovDeg(
  game: GameProfile,
  fovSetting: number,
  aspect: number,
): number {
  if (game.fovMode === 'horizontal') {
    return fovSetting
  }

  const referenceAspect = 4 / 3
  const halfRad = (fovSetting * DEG2RAD) / 2
  return 2 * Math.atan(Math.tan(halfRad) * (aspect / referenceAspect)) * RAD2DEG
}

export function verticalFovFromHorizontal(horizontalFovDeg: number, aspect: number): number {
  const halfH = (horizontalFovDeg * DEG2RAD) / 2
  return 2 * Math.atan(Math.tan(halfH) / aspect) * RAD2DEG
}

export function buildDisplayConfig(
  game: GameProfile,
  fovSetting: number,
  width: number,
  height: number,
): DisplayConfig {
  const aspect = width / height
  return {
    width,
    height,
    horizontalFovDeg: resolveHorizontalFovDeg(game, fovSetting, aspect),
  }
}

/**
 * Pixel offset from center → yaw/pitch degrees (pinhole model).
 * +x right → +yaw, +y down → +pitch (game pitch sign handled at sens solve time).
 */
export function offsetPxToAngles(offset: Point2, display: DisplayConfig): {
  yawDeg: number
  pitchDeg: number
} {
  const halfW = display.width / 2
  const halfH = display.height / 2
  const halfHFov = (display.horizontalFovDeg * DEG2RAD) / 2
  const vFov = verticalFovFromHorizontal(display.horizontalFovDeg, display.width / display.height)
  const halfVFov = (vFov * DEG2RAD) / 2

  const yawDeg = Math.atan((offset.x / halfW) * Math.tan(halfHFov)) * RAD2DEG
  const pitchDeg = Math.atan((offset.y / halfH) * Math.tan(halfVFov)) * RAD2DEG

  return { yawDeg, pitchDeg }
}

export function makeTarget(offsetPx: Point2, display: DisplayConfig): TargetSpec {
  const angles = offsetPxToAngles(offsetPx, display)
  return {
    offsetPx: { ...offsetPx },
    yawDeg: angles.yawDeg,
    pitchDeg: angles.pitchDeg,
  }
}
