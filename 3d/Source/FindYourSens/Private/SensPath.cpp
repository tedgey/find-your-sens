#include "SensPath.h"

namespace Sens
{
FPoint2 AccumulateNet(const TArray<FPathSample>& Samples)
{
	FPoint2 Net;
	for (const FPathSample& S : Samples)
	{
		Net.X += S.Dx;
		Net.Y += S.Dy;
	}
	return Net;
}

FPathMetrics ComputePathMetrics(const TArray<FPathSample>& Samples, const FPoint2& Net)
{
	FPathMetrics Out;
	double PathLength = 0.0;
	for (const FPathSample& S : Samples)
	{
		PathLength += FMath::Sqrt(S.Dx * S.Dx + S.Dy * S.Dy);
	}

	const double StraightLineLength = FMath::Sqrt(Net.X * Net.X + Net.Y * Net.Y);
	const double Straightness = PathLength > 1e-6 ? FMath::Min(1.0, StraightLineLength / PathLength) : 0.0;

	double LateCorrectionRatio = 0.0;
	if (Samples.Num() >= 4 && PathLength > 1e-6)
	{
		const double T0 = Samples[0].T;
		const double T1 = Samples.Last().T;
		const double Cut = T0 + (T1 - T0) * 0.75;
		const double NetAngle = FMath::Atan2(Net.Y, Net.X);
		double Late = 0.0;
		double LateReverse = 0.0;
		for (const FPathSample& S : Samples)
		{
			if (S.T < Cut)
			{
				continue;
			}
			const double Step = FMath::Sqrt(S.Dx * S.Dx + S.Dy * S.Dy);
			Late += Step;
			const double StepAngle = FMath::Atan2(S.Dy, S.Dx);
			double Delta = FMath::Abs(StepAngle - NetAngle);
			if (Delta > PI)
			{
				Delta = 2.0 * PI - Delta;
			}
			if (Delta > PI / 2.0)
			{
				LateReverse += Step;
			}
		}
		LateCorrectionRatio = Late > 1e-6 ? LateReverse / Late : 0.0;
	}

	Out.PathLength = PathLength;
	Out.StraightLineLength = StraightLineLength;
	Out.Straightness = Straightness;
	Out.NetDistance = StraightLineLength;
	Out.SampleCount = Samples.Num();
	Out.LateCorrectionRatio = LateCorrectionRatio;
	return Out;
}
} // namespace Sens
