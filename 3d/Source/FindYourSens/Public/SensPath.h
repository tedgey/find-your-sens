#pragma once

#include "SensTypes.h"

/** Port of 2d/src/math/path.ts */
namespace Sens
{
FPoint2 AccumulateNet(const TArray<FPathSample>& Samples);
FPathMetrics ComputePathMetrics(const TArray<FPathSample>& Samples, const FPoint2& Net);
}
