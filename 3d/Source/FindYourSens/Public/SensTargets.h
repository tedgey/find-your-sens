#pragma once

#include "SensTypes.h"

/** Port of 2d/src/math/targets.ts, plus 3D world-angle spawn. */
namespace Sens
{
struct FSpawnRange
{
	double MinAngleDeg = 0.0;
	double MaxAngleDeg = 0.0;
};

FSpawnRange GetScenarioRange(EScenarioId Scenario);
FTargetSpec SpawnTarget(EScenarioId Scenario, const FDisplayConfig& Display);
/** World-space yaw/pitch in the scenario band, clamped inside the view. */
FTargetSpec SpawnWorldTarget(EScenarioId Scenario, const FDisplayConfig& Display);
}
