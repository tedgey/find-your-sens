import { DPI_TIERS, getGame } from './games'
import { computePathMetrics } from './path'
import type {
  Recommendation,
  RoundEstimate,
  RoundRecording,
  SensRange,
  SessionSetup,
  StageResult,
  ScenarioId,
} from './types'
import { buildDisplayConfig } from './angles'

/** Felt horizontal turn used for absolute scale (degrees). */
export const FEEL_TURN_DEG = 90

/**
 * Least-squares sens for Source-style: yaw = mx * sens * yawConst, pitch = my * sens * yawConst.
 * Mouse +y (down) corresponds to looking down (+pitch in our target model).
 */
export function impliedSensitivity(
  yawDeg: number,
  pitchDeg: number,
  mouseDx: number,
  mouseDy: number,
  yawConstant: number,
): number | null {
  const denom = yawConstant * (mouseDx * mouseDx + mouseDy * mouseDy)
  if (denom < 1e-9) return null
  const sens = (yawDeg * mouseDx + pitchDeg * mouseDy) / denom
  if (!Number.isFinite(sens) || sens <= 0) return null
  return sens
}

/** Absolute sens from a felt horizontal turn (ignores vertical drift). */
export function impliedSensFromFeelTurn(
  feelDeg: number,
  mouseDx: number,
  yawConstant: number,
): number | null {
  const dx = Math.abs(mouseDx)
  if (dx < 1e-6) return null
  const sens = feelDeg / (dx * yawConstant)
  if (!Number.isFinite(sens) || sens <= 0) return null
  return sens
}

export function roundWeight(straightness: number, lateCorrectionRatio: number): number {
  const clean = Math.max(0, straightness) * (1 - 0.65 * lateCorrectionRatio)
  return Math.max(0.05, clean)
}

export function estimateRound(
  round: RoundRecording,
  yawConstant: number,
): RoundEstimate | null {
  const metrics = computePathMetrics(round.samples, round.net)
  let sens: number | null = null

  if (round.scenario === 'feel90') {
    sens = impliedSensFromFeelTurn(FEEL_TURN_DEG, round.net.x, yawConstant)
  } else if (round.target) {
    sens = impliedSensitivity(
      round.target.yawDeg,
      round.target.pitchDeg,
      round.net.x,
      round.net.y,
      yawConstant,
    )
  }

  if (sens == null) return null
  return {
    round,
    metrics,
    impliedSens: sens,
    weight: roundWeight(metrics.straightness, metrics.lateCorrectionRatio),
  }
}

function median(values: number[]): number {
  if (values.length === 0) return 0
  const sorted = [...values].sort((a, b) => a - b)
  const mid = Math.floor(sorted.length / 2)
  return sorted.length % 2 === 0
    ? (sorted[mid - 1] + sorted[mid]) / 2
    : sorted[mid]
}

function mean(values: number[]): number {
  if (values.length === 0) return 0
  return values.reduce((a, b) => a + b, 0) / values.length
}

function stdDev(values: number[]): number {
  if (values.length < 2) return 0
  const m = mean(values)
  const v = values.reduce((acc, x) => acc + (x - m) ** 2, 0) / (values.length - 1)
  return Math.sqrt(v)
}

export function summarizeStage(
  scenario: ScenarioId,
  rounds: RoundRecording[],
  yawConstant: number,
): StageResult {
  const estimates = rounds
    .map((r) => estimateRound(r, yawConstant))
    .filter((e): e is RoundEstimate => e != null)

  const sensValues = estimates.map((e) => e.impliedSens)
  const med = median(sensValues)
  const avg = mean(sensValues)
  const sd = stdDev(sensValues)

  const nFactor = Math.min(1, estimates.length / Math.max(1, rounds.length || 1))
  const cv = med > 0 ? sd / med : 1
  const spreadFactor = Math.max(0, 1 - Math.min(cv, 1))
  const quality =
    estimates.length > 0
      ? estimates.reduce((a, e) => a + e.weight, 0) / estimates.length
      : 0
  const confidence = Math.max(0, Math.min(1, 0.35 * nFactor + 0.4 * spreadFactor + 0.25 * quality))

  return {
    scenario,
    estimates,
    medianSens: med,
    meanSens: avg,
    stdDev: sd,
    confidence,
    isAbsolute: scenario === 'feel90',
  }
}

function weightedMedian(items: { value: number; weight: number }[]): number {
  if (items.length === 0) return 0
  const sorted = [...items].sort((a, b) => a.value - b.value)
  const total = sorted.reduce((a, i) => a + i.weight, 0)
  let acc = 0
  for (const item of sorted) {
    acc += item.weight
    if (acc >= total / 2) return item.value
  }
  return sorted[sorted.length - 1].value
}

export function scaleSensForDpi(sens: number, fromDpi: number, toDpi: number): number {
  return sens * (fromDpi / toDpi)
}

export function cmPer360(sens: number, dpi: number, yawConstant: number): number {
  return (360 / (sens * yawConstant * dpi)) * 2.54
}

export function pickDpiTier(userDpi: number): number {
  let best: number = DPI_TIERS[0]
  let bestDist = Math.abs(userDpi - best)
  for (const tier of DPI_TIERS) {
    const d = Math.abs(userDpi - tier)
    if (d < bestDist) {
      best = tier
      bestDist = d
    }
  }
  if (Math.abs(userDpi - best) / best <= 0.05) return best
  return best
}

function buildRange(center: number, std: number, decimals: number, widen = 1): SensRange {
  const pad = Math.max(std, center * 0.04) * widen
  const low = Number((center - pad).toFixed(decimals))
  const high = Number((center + pad).toFixed(decimals))
  return {
    low: Math.max(0.001, low),
    high: Math.max(low + Number((10 ** -decimals).toFixed(decimals)), high),
    center: Number(center.toFixed(decimals)),
  }
}

/**
 * With Enhance Pointer Precision on, the browser often accelerates fast moves while
 * raw-input games do not. Prefer slower stages when blending.
 */
export function eppStageWeight(scenario: ScenarioId, eppOn: boolean): number {
  if (!eppOn) return 1
  switch (scenario) {
    case 'feel90':
      return 1.2
    case 'micro':
      return 1.45
    case 'casual':
      return 1.05
    case 'flick':
      return 0.5
  }
}

export function recommendFromRounds(
  setup: SessionSetup,
  rounds: RoundRecording[],
): Recommendation {
  const game = getGame(setup.gameId)
  const fovSetting = setup.fovSetting ?? game.defaultFovSetting
  const eppOn = setup.enhancePointerPrecision === 'on'

  const scenarios: ScenarioId[] = ['feel90', 'flick', 'casual', 'micro']
  const stageResults = scenarios.map((scenario) =>
    summarizeStage(
      scenario,
      rounds.filter((r) => r.scenario === scenario),
      game.yawConstant,
    ),
  )

  const feelStage = stageResults.find((s) => s.scenario === 'feel90')
  const pointingStages = stageResults.filter((s) => s.scenario !== 'feel90')

  // Absolute scale from felt turns only. Target stages measure screen-pointing (~1:1), not FPS yaw.
  const feelItems: { value: number; weight: number }[] = []
  if (feelStage) {
    for (const est of feelStage.estimates) {
      feelItems.push({
        value: est.impliedSens,
        weight: est.weight * (0.25 + 0.75 * feelStage.confidence),
      })
    }
  }

  const blendedSens = weightedMedian(feelItems)
  const allFeelSens = feelItems.map((i) => i.value)
  const blendedStd = stdDev(allFeelSens)

  const pointingValues = pointingStages.flatMap((s) => s.estimates.map((e) => e.impliedSens))
  const pointingBaselineSens = pointingValues.length ? median(pointingValues) : null

  let fromStages = feelStage?.confidence ?? 0
  // Path quality on target stages supports confidence without setting absolute scale.
  if (pointingStages.length) {
    const pathConf =
      pointingStages.reduce((a, s) => a + s.confidence, 0) / pointingStages.length
    fromStages = 0.7 * fromStages + 0.3 * pathConf
  }

  const eppPenaltyApplied = eppOn && game.usesRawInput
  const eppFactor = eppPenaltyApplied ? 0.78 : 1
  const overallConfidence = fromStages * eppFactor

  const dpiTier = pickDpiTier(setup.dpi)
  const outputDpi = setup.dpi

  const sensRange = buildRange(
    blendedSens,
    blendedStd * 0.75,
    game.sensDecimals,
    eppPenaltyApplied ? 1.25 : 1,
  )
  const eDpiRange: SensRange = {
    low: Number((sensRange.low * outputDpi).toFixed(1)),
    high: Number((sensRange.high * outputDpi).toFixed(1)),
    center: Number((sensRange.center * outputDpi).toFixed(1)),
  }

  const sensRangeAtNearestTier =
    dpiTier !== outputDpi
      ? buildRange(
          scaleSensForDpi(sensRange.center, outputDpi, dpiTier),
          scaleSensForDpi(blendedStd * 0.75, outputDpi, dpiTier),
          game.sensDecimals,
          eppPenaltyApplied ? 1.25 : 1,
        )
      : null

  const currentSens = setup.currentSens
  const vsCurrentRatio =
    currentSens && currentSens > 0 && sensRange.center > 0
      ? sensRange.center / currentSens
      : null

  const inputNotes: string[] = []
  inputNotes.push(
    'Your quoted sensitivity comes from the felt 90° turns. Flick/casual/micro targets check consistency; alone they tend to measure desktop-style screen pointing (~1:1), not CS2 turn feel.',
  )
  if (pointingBaselineSens != null) {
    inputNotes.push(
      `Target-stage pointing baseline was ~${pointingBaselineSens.toFixed(game.sensDecimals)} (often near geometric 1:1). That is expected and not what we quote.`,
    )
  }
  if (eppPenaltyApplied) {
    inputNotes.push(
      `${game.name} uses raw input, so Windows Enhance pointer precision does not apply in-game. This browser test may still accelerate fast moves. Overall confidence was reduced from ${Math.round(fromStages * 100)}% to ${Math.round(overallConfidence * 100)}% for that mismatch.`,
    )
  }
  if (vsCurrentRatio != null && (vsCurrentRatio < 0.7 || vsCurrentRatio > 1.4)) {
    inputNotes.push(
      `This sits notably ${vsCurrentRatio > 1 ? 'above' : 'below'} your current ${currentSens}. Treat it as experimental and compare in-game before committing.`,
    )
  }

  const arenaWidth = rounds[0]?.arenaWidth ?? setup.resolutionWidth
  const arenaHeight = rounds[0]?.arenaHeight ?? setup.resolutionHeight
  const arenaDisplay = buildDisplayConfig(game, fovSetting, arenaWidth, arenaHeight)

  return {
    gameId: setup.gameId,
    dpi: outputDpi,
    nearestDpiTier: dpiTier,
    sensRangeAtNearestTier,
    sensRange,
    confidence: Math.max(0, Math.min(1, overallConfidence)),
    confidenceBreakdown: {
      fromStages: Math.max(0, Math.min(1, fromStages)),
      eppPenaltyApplied,
      eppFactor,
    },
    cmPer360: cmPer360(sensRange.center, outputDpi, game.yawConstant),
    eDpiRange,
    stageResults,
    pointingBaselineSens,
    currentSens,
    vsCurrentRatio,
    assumptions: {
      fovSetting,
      horizontalFovDeg: arenaDisplay.horizontalFovDeg,
      arenaWidth,
      arenaHeight,
      gameResolutionWidth: setup.resolutionWidth,
      gameResolutionHeight: setup.resolutionHeight,
      yawConstant: game.yawConstant,
      enhancePointerPrecision: setup.enhancePointerPrecision,
      usesRawInput: game.usesRawInput,
    },
    inputNotes,
  }
}
