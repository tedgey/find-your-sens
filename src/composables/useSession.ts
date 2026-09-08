import { computed, reactive } from 'vue'
import {
  recommendFromRounds,
  type Recommendation,
  type RoundRecording,
  type ScenarioId,
  type SessionSetup,
} from '../math'

export interface ScenarioPlan {
  id: ScenarioId
  rounds: number
}

/** Felt turns set absolute scale; target scenarios check consistency only. */
export const SCENARIO_PLAN: ScenarioPlan[] = [
  { id: 'feel90', rounds: 5 },
  { id: 'flick', rounds: 10 },
  { id: 'casual', rounds: 10 },
  { id: 'micro', rounds: 10 },
]

export type AppStep = 'setup' | 'info' | 'prep' | 'test' | 'results'

const state = reactive({
  step: 'setup' as AppStep,
  setup: {
    gameId: 'cs2',
    dpi: 400,
    fovSetting: null as number | null,
    resolutionWidth: 1920,
    resolutionHeight: 1080,
    enhancePointerPrecision: 'on' as const,
    currentSens: null as number | null,
  } satisfies SessionSetup,
  scenarioIndex: 0,
  roundIndex: 0,
  rounds: [] as RoundRecording[],
  recommendation: null as Recommendation | null,
})

export function useSession() {
  const currentPlan = computed(
    () => SCENARIO_PLAN[state.scenarioIndex] ?? SCENARIO_PLAN[0],
  )
  const currentScenario = computed(() => currentPlan.value.id)
  const roundsThisScenario = computed(() => currentPlan.value.rounds)
  const totalRounds = SCENARIO_PLAN.reduce((a, s) => a + s.rounds, 0)
  const completedRounds = computed(() => state.rounds.length)

  function saveSetup(partial: Partial<SessionSetup>) {
    Object.assign(state.setup, partial)
  }

  function openInfo() {
    state.step = 'info'
  }

  function startPrep() {
    state.step = 'prep'
  }

  function beginTest() {
    state.scenarioIndex = 0
    state.roundIndex = 0
    state.rounds = []
    state.recommendation = null
    state.step = 'test'
  }

  function recordRound(
    recording: Omit<RoundRecording, 'scenario' | 'roundIndex'>,
  ) {
    const scenario = currentScenario.value
    state.rounds.push({
      ...recording,
      scenario,
      roundIndex: state.roundIndex,
    })

    if (state.roundIndex + 1 < roundsThisScenario.value) {
      state.roundIndex += 1
      return
    }

    if (state.scenarioIndex + 1 < SCENARIO_PLAN.length) {
      state.scenarioIndex += 1
      state.roundIndex = 0
      return
    }

    state.recommendation = recommendFromRounds(state.setup, state.rounds)
    state.step = 'results'
  }

  function retest() {
    state.recommendation = null
    state.rounds = []
    state.scenarioIndex = 0
    state.roundIndex = 0
    state.step = 'prep'
  }

  function backToSetup() {
    state.step = 'setup'
    state.recommendation = null
    state.rounds = []
    state.scenarioIndex = 0
    state.roundIndex = 0
  }

  return {
    state,
    currentScenario,
    roundsThisScenario,
    totalRounds,
    completedRounds,
    scenarioPlan: SCENARIO_PLAN,
    saveSetup,
    openInfo,
    startPrep,
    beginTest,
    recordRound,
    retest,
    backToSetup,
  }
}
