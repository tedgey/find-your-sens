#pragma once

#include "SensTypes.h"

/** Port of 2d/src/math/angles.ts */
namespace Sens
{
double ResolveHorizontalFovDeg(const FGameProfile& Game, double FovSetting, double Aspect);
double VerticalFovFromHorizontal(double HorizontalFovDeg, double Aspect);
FDisplayConfig BuildDisplayConfig(const FGameProfile& Game, double FovSetting, double Width, double Height);
void OffsetPxToAngles(const FPoint2& Offset, const FDisplayConfig& Display, double& OutYawDeg, double& OutPitchDeg);
FTargetSpec MakeTarget(const FPoint2& OffsetPx, const FDisplayConfig& Display);
}
