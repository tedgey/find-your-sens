import type { GameId, GameProfile } from './types'

export const DPI_TIERS = [400, 800, 1600, 3200] as const

export const GAMES: Record<GameId, GameProfile> = {
  cs2: {
    id: 'cs2',
    name: 'Counter-Strike 2',
    yawConstant: 0.022,
    defaultFovSetting: 90,
    fovMode: 'source_4_3',
    sensDecimals: 3,
    usesRawInput: true,
    settingsLabels: {
      sensitivity: 'sensitivity',
    },
  },
  valorant: {
    id: 'valorant',
    name: 'Valorant',
    // Valorant yaw is effectively 0.07 deg per count at sens 1 (widely used constant).
    yawConstant: 0.07,
    defaultFovSetting: 103,
    fovMode: 'horizontal',
    sensDecimals: 3,
    usesRawInput: true,
    settingsLabels: {
      sensitivity: 'Sensitivity',
    },
  },
  pubg: {
    id: 'pubg',
    name: 'PUBG',
    yawConstant: 0.022,
    defaultFovSetting: 90,
    fovMode: 'horizontal',
    sensDecimals: 2,
    usesRawInput: true,
    settingsLabels: {
      sensitivity: 'General Sensitivity',
    },
  },
  apex: {
    id: 'apex',
    name: 'Apex Legends',
    yawConstant: 0.022,
    defaultFovSetting: 110,
    fovMode: 'horizontal',
    sensDecimals: 3,
    usesRawInput: true,
    settingsLabels: {
      sensitivity: 'Mouse Sensitivity',
    },
  },
  fortnite: {
    id: 'fortnite',
    name: 'Fortnite',
    // Fortnite uses % / x and a different pipeline; placeholder until pack ships.
    yawConstant: 0.005555,
    defaultFovSetting: 80,
    fovMode: 'horizontal',
    sensDecimals: 3,
    usesRawInput: true,
    settingsLabels: {
      sensitivity: 'X/Y Sensitivity',
    },
  },
}

export const PROTOTYPE_GAME_ORDER: GameId[] = ['cs2']

export function getGame(id: GameId): GameProfile {
  return GAMES[id]
}
