import type { PathMetrics, PathSample, Point2 } from './types'

export function accumulateNet(samples: PathSample[]): Point2 {
  let x = 0
  let y = 0
  for (const s of samples) {
    x += s.dx
    y += s.dy
  }
  return { x, y }
}

export function computePathMetrics(samples: PathSample[], net: Point2): PathMetrics {
  let pathLength = 0
  for (const s of samples) {
    pathLength += Math.hypot(s.dx, s.dy)
  }

  const straightLineLength = Math.hypot(net.x, net.y)
  const straightness =
    pathLength > 1e-6 ? Math.min(1, straightLineLength / pathLength) : 0

  // Fraction of path length in the last 25% of time that reverses away from net direction.
  let lateCorrectionRatio = 0
  if (samples.length >= 4 && pathLength > 1e-6) {
    const t0 = samples[0].t
    const t1 = samples[samples.length - 1].t
    const cut = t0 + (t1 - t0) * 0.75
    const netAngle = Math.atan2(net.y, net.x)
    let late = 0
    let lateReverse = 0
    for (const s of samples) {
      if (s.t < cut) continue
      const step = Math.hypot(s.dx, s.dy)
      late += step
      const stepAngle = Math.atan2(s.dy, s.dx)
      let delta = Math.abs(stepAngle - netAngle)
      if (delta > Math.PI) delta = 2 * Math.PI - delta
      if (delta > Math.PI / 2) lateReverse += step
    }
    lateCorrectionRatio = late > 1e-6 ? lateReverse / late : 0
  }

  return {
    pathLength,
    straightLineLength,
    straightness,
    netDistance: straightLineLength,
    sampleCount: samples.length,
    lateCorrectionRatio,
  }
}
