#include "SensTargets.h"
#include "SensAngles.h"

namespace Sens
{
FSpawnRange GetScenarioRange(EScenarioId Scenario)
{
	switch (Scenario)
	{
	case EScenarioId::Flick:
		return {18.0, 48.0};
	case EScenarioId::Casual:
		return {12.0, 32.0};
	case EScenarioId::Micro:
		return {2.5, 8.0};
	default:
		return {0.0, 0.0};
	}
}

static double RandRange(double Min, double Max)
{
	return Min + FMath::FRand() * (Max - Min);
}

static FPoint2 AnglesToOffsetPx(double YawDeg, double PitchDeg, const FDisplayConfig& Display)
{
	const double HalfW = Display.Width / 2.0;
	const double HalfH = Display.Height / 2.0;
	const double Aspect = Display.Width / Display.Height;
	const double HalfHFov = (Display.HorizontalFovDeg * PI) / 360.0;
	const double VFov = 2.0 * FMath::Atan(FMath::Tan(HalfHFov) / Aspect) * (180.0 / PI);
	const double HalfVFov = (VFov * PI) / 360.0;

	FPoint2 Offset;
	Offset.X = (FMath::Tan((YawDeg * PI) / 180.0) / FMath::Tan(HalfHFov)) * HalfW;
	Offset.Y = (FMath::Tan((PitchDeg * PI) / 180.0) / FMath::Tan(HalfVFov)) * HalfH;
	return Offset;
}

FTargetSpec SpawnTarget(EScenarioId Scenario, const FDisplayConfig& Display)
{
	const FSpawnRange Range = GetScenarioRange(Scenario);
	const double Magnitude = RandRange(Range.MinAngleDeg, Range.MaxAngleDeg);
	const double Theta = RandRange(0.0, PI * 2.0);
	double YawDeg = FMath::Cos(Theta) * Magnitude;
	double PitchDeg = FMath::Sin(Theta) * Magnitude;

	FPoint2 Offset = AnglesToOffsetPx(YawDeg, PitchDeg, Display);
	const double MaxX = Display.Width * 0.45;
	const double MaxY = Display.Height * 0.45;
	const double Scale = FMath::Max(FMath::Max(FMath::Abs(Offset.X) / MaxX, FMath::Abs(Offset.Y) / MaxY), 1.0);
	if (Scale > 1.0)
	{
		Offset.X /= Scale;
		Offset.Y /= Scale;
	}
	return MakeTarget(Offset, Display);
}

FTargetSpec SpawnWorldTarget(EScenarioId Scenario, const FDisplayConfig& Display)
{
	return SpawnTarget(Scenario, Display);
}
} // namespace Sens
