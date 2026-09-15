#include "SensCheckIn.h"

namespace Sens
{
namespace
{
double MedianAbs(TArray<double> Values)
{
	if (Values.Num() == 0)
	{
		return 0.0;
	}
	Values.Sort();
	const int32 N = Values.Num();
	if (N % 2 == 1)
	{
		return Values[N / 2];
	}
	return 0.5 * (Values[N / 2 - 1] + Values[N / 2]);
}

FCheckInStageSummary SummarizeStage(EScenarioId Scenario, const TArray<FCheckInRound>& Rounds)
{
	FCheckInStageSummary Out;
	Out.Scenario = Scenario;
	TArray<double> AbsErrors;
	for (const FCheckInRound& Round : Rounds)
	{
		if (Round.Scenario != Scenario)
		{
			continue;
		}
		++Out.Rounds;
		AbsErrors.Add(FMath::Abs(Round.AngularErrorDeg));
		switch (Round.Band)
		{
		case ECheckInBand::OnTarget:
			++Out.OnTarget;
			break;
		case ECheckInBand::Close:
			++Out.Close;
			break;
		case ECheckInBand::Miss:
			++Out.Miss;
			break;
		}
		if (Round.Band != ECheckInBand::OnTarget)
		{
			if (Round.bWrongWay)
			{
				++Out.Undershoot;
			}
			else if (Round.bOvershoot)
			{
				++Out.Overshoot;
			}
			else
			{
				++Out.Undershoot;
			}
		}
	}
	Out.MedianAbsErrorDeg = MedianAbs(MoveTemp(AbsErrors));
	return Out;
}
} // namespace

FVector2D MouseToAimDeltaDeg(double Dx, double Dy, double Sens, double YawConstant)
{
	const double Scale = YawConstant * Sens;
	return FVector2D(Dx * Scale, Dy * Scale);
}

void AccumulateAim(double& YawDeg, double& PitchDeg, double DYawDeg, double DPitchDeg)
{
	YawDeg += DYawDeg;
	PitchDeg = FMath::Clamp(PitchDeg + DPitchDeg, -PitchClampDeg, PitchClampDeg);
}

double AngleBetweenDeg(const FVector& A, const FVector& B)
{
	const FVector NA = A.GetSafeNormal();
	const FVector NB = B.GetSafeNormal();
	if (NA.IsNearlyZero() || NB.IsNearlyZero())
	{
		return 180.0;
	}
	const double Cos = FMath::Clamp(FVector::DotProduct(NA, NB), -1.0, 1.0);
	return FMath::RadiansToDegrees(FMath::Acos(Cos));
}

bool RayHitsSphere(const FVector& Origin, const FVector& Dir, const FVector& Center, double Radius)
{
	const FVector NormalDir = Dir.GetSafeNormal();
	if (NormalDir.IsNearlyZero() || Radius <= 0.0)
	{
		return false;
	}
	const FVector Oc = Origin - Center;
	const double B = FVector::DotProduct(Oc, NormalDir);
	const double C = Oc.SizeSquared() - Radius * Radius;
	const double Disc = B * B - C;
	if (Disc < 0.0)
	{
		return false;
	}
	const double SqrtDisc = FMath::Sqrt(Disc);
	const double T0 = -B - SqrtDisc;
	const double T1 = -B + SqrtDisc;
	return T0 >= 0.0 || T1 >= 0.0;
}

double OrbAngularRadiusDeg(double DistanceCm, double RadiusCm)
{
	if (DistanceCm <= 1.0 || RadiusCm <= 0.0)
	{
		return FeelOnTargetDeg;
	}
	return FMath::RadiansToDegrees(FMath::Atan(RadiusCm / DistanceCm));
}

ECheckInBand ClassifyBand(double AbsErrorDeg, double OnTargetDeg, bool bOnGeometry)
{
	if (bOnGeometry)
	{
		return ECheckInBand::OnTarget;
	}
	const double CloseDeg = 2.0 * OnTargetDeg;
	if (AbsErrorDeg <= OnTargetDeg)
	{
		return ECheckInBand::OnTarget;
	}
	if (AbsErrorDeg <= CloseDeg)
	{
		return ECheckInBand::Close;
	}
	return ECheckInBand::Miss;
}

double SignedArcErrorDeg(const FVector& RestForward, const FVector& TargetDir, const FVector& AimForward)
{
	double TargetDeg = 0.0;
	double AimDeg = 0.0;
	ArcAnglesDeg(RestForward, TargetDir, AimForward, TargetDeg, AimDeg);
	return AimDeg - TargetDeg;
}

void ArcAnglesDeg(const FVector& RestForward, const FVector& TargetDir, const FVector& AimForward, double& OutTargetDeg, double& OutAimDeg)
{
	OutTargetDeg = 0.0;
	OutAimDeg = 0.0;
	const FVector Rest = RestForward.GetSafeNormal();
	const FVector Target = TargetDir.GetSafeNormal();
	const FVector Aim = AimForward.GetSafeNormal();
	if (Rest.IsNearlyZero() || Target.IsNearlyZero() || Aim.IsNearlyZero())
	{
		return;
	}

	FVector Axis = FVector::CrossProduct(Rest, Target);
	if (Axis.SizeSquared() < 1e-8)
	{
		Axis = FVector::UpVector;
	}
	Axis.Normalize();

	auto AngleAboutAxis = [&](const FVector& Dir) {
		FVector Projected = Dir - Axis * FVector::DotProduct(Dir, Axis);
		if (!Projected.Normalize(1e-8))
		{
			return 0.0;
		}
		const double Cos = FMath::Clamp(FVector::DotProduct(Rest, Projected), -1.0, 1.0);
		const double Sin = FVector::DotProduct(Axis, FVector::CrossProduct(Rest, Projected));
		return FMath::RadiansToDegrees(FMath::Atan2(Sin, Cos));
	};

	OutTargetDeg = AngleAboutAxis(Target);
	OutAimDeg = AngleAboutAxis(Aim);
}

FCheckInRound ScoreFeel90Round(const FCheckInPose& Pose, double ChosenSens)
{
	FCheckInRound Out;
	Out.Scenario = EScenarioId::Feel90;
	Out.ChosenSens = ChosenSens;
	Out.AchievedYawDeg = Pose.YawDeg;
	Out.AchievedPitchDeg = Pose.PitchDeg;
	Out.SignedErrorDeg = Pose.YawDeg - FeelTurnDeg;

	const FVector Intended = Pose.RestForward.RotateAngleAxis(static_cast<float>(FeelTurnDeg), FVector::UpVector);
	Out.AngularErrorDeg = AngleBetweenDeg(Pose.AimForward, Intended);
	Out.bWrongWay = Pose.YawDeg < 0.0;
	Out.bOvershoot = !Out.bWrongWay && Pose.YawDeg > FeelTurnDeg;
	if (Out.bWrongWay)
	{
		Out.Band = ECheckInBand::Miss;
	}
	else
	{
		Out.Band = ClassifyBand(Out.AngularErrorDeg, FeelOnTargetDeg, false);
	}
	return Out;
}

FCheckInRound ScoreTargetRound(const FCheckInPose& Pose, const FCheckInTargetWorld& Target, double ChosenSens)
{
	FCheckInRound Out;
	Out.ChosenSens = ChosenSens;
	Out.AchievedYawDeg = Pose.YawDeg;
	Out.AchievedPitchDeg = Pose.PitchDeg;
	Out.Target = Target.Spec;

	const FVector ToTarget = Target.Center - Pose.Eye;
	const double Distance = ToTarget.Size();
	Out.AngularErrorDeg = AngleBetweenDeg(Pose.AimForward, ToTarget);

	double TargetArcDeg = 0.0;
	double AimArcDeg = 0.0;
	ArcAnglesDeg(Pose.RestForward, ToTarget, Pose.AimForward, TargetArcDeg, AimArcDeg);
	Out.SignedErrorDeg = AimArcDeg - TargetArcDeg;
	Out.bWrongWay = TargetArcDeg * AimArcDeg < 0.0 && FMath::Abs(AimArcDeg) > 1.0;
	Out.bOvershoot = !Out.bWrongWay && Out.SignedErrorDeg > 0.0;

	const bool bHit = RayHitsSphere(Pose.Eye, Pose.AimForward, Target.Center, Target.RadiusCm);
	const double OnTargetDeg = OrbAngularRadiusDeg(Distance, Target.RadiusCm);
	Out.Band = ClassifyBand(Out.AngularErrorDeg, OnTargetDeg, bHit);
	if (Out.bWrongWay)
	{
		Out.Band = ECheckInBand::Miss;
	}
	return Out;
}

FCheckInReport BuildCheckInReport(double ChosenSens, const TArray<FCheckInRound>& Rounds)
{
	FCheckInReport Report;
	Report.ChosenSens = ChosenSens;

	const EScenarioId Order[] = {
		EScenarioId::Feel90,
		EScenarioId::Flick,
		EScenarioId::Casual,
		EScenarioId::Micro,
	};
	for (const EScenarioId Id : Order)
	{
		const FCheckInStageSummary Stage = SummarizeStage(Id, Rounds);
		if (Stage.Rounds > 0)
		{
			Report.Stages.Add(Stage);
		}
	}

	for (const FCheckInStageSummary& Stage : Report.Stages)
	{
		if (Stage.Scenario == EScenarioId::Feel90)
		{
			Report.Notes.Add(FString::Printf(
				TEXT("Felt 90°: median %.1f° off a true 90° turn (%d on-target, %d close, %d miss)."),
				Stage.MedianAbsErrorDeg,
				Stage.OnTarget,
				Stage.Close,
				Stage.Miss));
		}
		else
		{
			Report.Notes.Add(FString::Printf(
				TEXT("%s: %d on-target, %d close, %d miss. Overshoot %d, undershoot %d. Median error %.1f°."),
				*ScenarioLabel(Stage.Scenario),
				Stage.OnTarget,
				Stage.Close,
				Stage.Miss,
				Stage.Overshoot,
				Stage.Undershoot,
				Stage.MedianAbsErrorDeg));
		}
	}

	Report.Notes.Add(TEXT("This check-in does not change your settings pack. It only shows how the chosen sensitivity felt with live look."));
	return Report;
}
} // namespace Sens
