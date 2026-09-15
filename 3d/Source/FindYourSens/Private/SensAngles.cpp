#include "SensAngles.h"

namespace Sens
{
constexpr double Deg2Rad = PI / 180.0;
constexpr double Rad2Deg = 180.0 / PI;

double ResolveHorizontalFovDeg(const FGameProfile& Game, double FovSetting, double Aspect)
{
	if (Game.FovMode == EFovMode::Horizontal)
	{
		return FovSetting;
	}
	const double ReferenceAspect = 4.0 / 3.0;
	const double HalfRad = (FovSetting * Deg2Rad) / 2.0;
	return 2.0 * FMath::Atan(FMath::Tan(HalfRad) * (Aspect / ReferenceAspect)) * Rad2Deg;
}

double VerticalFovFromHorizontal(double HorizontalFovDeg, double Aspect)
{
	const double HalfH = (HorizontalFovDeg * Deg2Rad) / 2.0;
	return 2.0 * FMath::Atan(FMath::Tan(HalfH) / Aspect) * Rad2Deg;
}

FDisplayConfig BuildDisplayConfig(const FGameProfile& Game, double FovSetting, double Width, double Height)
{
	const double Aspect = Width / Height;
	FDisplayConfig Display;
	Display.Width = Width;
	Display.Height = Height;
	Display.HorizontalFovDeg = ResolveHorizontalFovDeg(Game, FovSetting, Aspect);
	return Display;
}

void OffsetPxToAngles(const FPoint2& Offset, const FDisplayConfig& Display, double& OutYawDeg, double& OutPitchDeg)
{
	const double HalfW = Display.Width / 2.0;
	const double HalfH = Display.Height / 2.0;
	const double HalfHFov = (Display.HorizontalFovDeg * Deg2Rad) / 2.0;
	const double VFov = VerticalFovFromHorizontal(Display.HorizontalFovDeg, Display.Width / Display.Height);
	const double HalfVFov = (VFov * Deg2Rad) / 2.0;

	OutYawDeg = FMath::Atan((Offset.X / HalfW) * FMath::Tan(HalfHFov)) * Rad2Deg;
	OutPitchDeg = FMath::Atan((Offset.Y / HalfH) * FMath::Tan(HalfVFov)) * Rad2Deg;
}

FTargetSpec MakeTarget(const FPoint2& OffsetPx, const FDisplayConfig& Display)
{
	FTargetSpec Spec;
	Spec.OffsetPx = OffsetPx;
	OffsetPxToAngles(OffsetPx, Display, Spec.YawDeg, Spec.PitchDeg);
	return Spec;
}
} // namespace Sens
