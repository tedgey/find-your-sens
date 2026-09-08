export type GameId = 'cs2' | 'valorant' | 'pubg' | 'apex' | 'fortnite'

/** Target-based scenarios plus felt-turn absolute calibration. */
export type ScenarioId = 'feel90' | 'flick' | 'casual' | 'micro'

export interface DisplayConfig {
  width: number
  height: number
  /** Horizontal FOV in degrees for the current aspect (resolved). */
  horizontalFovDeg: number
}

export interface GameProfile {
  id: GameId
  name: string
  /** Degrees turned per mouse count at sens 1 (Source m_yaw = 0.022). */
  yawConstant: number
  /**
   * Default FOV as the game exposes it.
   * For CS2 this is the classic 4:3-referenced FOV setting (90).
   */
  defaultFovSetting: number
  /** How the game FOV setting maps to horizontal FOV on a given aspect. */
  fovMode: 'source_4_3' | 'horizontal'
  sensDecimals: number
  settingsLabels: {
    sensitivity: string
  }
  /** True when the game uses raw mouse input (ignores Windows EPP). */
  usesRawInput: boolean
}

export type EnhancePointerPrecision = 'on' | 'off'

export interface SessionSetup {
  gameId: GameId
  dpi: number
  /** Game FOV setting; null means use game default. */
  fovSetting: number | null
  resolutionWidth: number
  resolutionHeight: number
  /**
   * Windows "Enhance pointer precision".
   * Games with raw input ignore this; browsers often still apply it.
   */
  enhancePointerPrecision: EnhancePointerPrecision
  /** Optional: what they already run in-game, for comparison only. */
  currentSens: number | null
}

export interface Point2 {
  x: number
  y: number
}

export interface PathSample {
  t: number
  dx: number
  dy: number
}

export interface TargetSpec {
  /** Pixel offset from crosshair (center), +x right, +y down. */
  offsetPx: Point2
  /** Angular offset the target represents, degrees. */
  yawDeg: number
  pitchDeg: number
}

export interface RoundRecording {
  scenario: ScenarioId
  roundIndex: number
  target: TargetSpec | null
  samples: PathSample[]
  /** Net mouse counts at click. */
  net: Point2
  durationMs: number
  /** Arena CSS pixel size used when the target angles were computed. */
  arenaWidth: number
  arenaHeight: number
}

export interface PathMetrics {
  pathLength: number
  straightLineLength: number
  straightness: number
  netDistance: number
  sampleCount: number
  lateCorrectionRatio: number
}

export interface RoundEstimate {
  round: RoundRecording
  metrics: PathMetrics
  /** Implied sensitivity at the user's typed DPI. */
  impliedSens: number
  weight: number
}

export interface StageResult {
  scenario: ScenarioId
  estimates: RoundEstimate[]
  medianSens: number
  meanSens: number
  stdDev: number
  confidence: number
  /** True when this stage sets absolute scale (felt turn), not screen pointing. */
  isAbsolute: boolean
}

export interface SensRange {
  low: number
  high: number
  center: number
}

export interface Recommendation {
  gameId: GameId
  /** DPI the sensitivity range is quoted at (user's entered DPI). */
  dpi: number
  /** Nearest common tier, for optional conversion. */
  nearestDpiTier: number
  /** Sens range if using nearestDpiTier instead of dpi. */
  sensRangeAtNearestTier: SensRange | null
  sensRange: SensRange
  confidence: number
  /** How overall confidence was assembled (for UI transparency). */
  confidenceBreakdown: {
    fromStages: number
    eppPenaltyApplied: boolean
    eppFactor: number
  }
  cmPer360: number
  eDpiRange: SensRange
  stageResults: StageResult[]
  /** Geometric ~1:1 screen-pointing baseline from target stages (diagnostic). */
  pointingBaselineSens: number | null
  currentSens: number | null
  /** measured / current when current was provided. */
  vsCurrentRatio: number | null
  assumptions: {
    fovSetting: number
    horizontalFovDeg: number
    /** Arena viewport used for angle mapping (not necessarily game res). */
    arenaWidth: number
    arenaHeight: number
    gameResolutionWidth: number
    gameResolutionHeight: number
    yawConstant: number
    enhancePointerPrecision: EnhancePointerPrecision
    usesRawInput: boolean
  }
  /** Soft warning when browser accel may disagree with in-game raw input. */
  inputNotes: string[]
}
