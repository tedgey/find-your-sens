#pragma once

#include "CoreMinimal.h"
#include "SensTypes.h"

namespace Sens
{
constexpr double FeelOnTargetDeg = 3.0;
constexpr double PitchClampDeg = 89.0;
constexpr double DefaultOrbRadiusCm = 35.0;

struct FCheckInPose
{
	double YawDeg = 0.0;
	double PitchDeg = 0.0;
	FVector Eye = FVector::ZeroVector;
	FVector AimForward = FVector::ForwardVector;
	FVector RestForward = FVector::ForwardVector;
};

struct FCheckInTargetWorld
{
	FVector Center = FVector::ZeroVector;
	double RadiusCm = DefaultOrbRadiusCm;
	FTargetSpec Spec;
};

FVector2D MouseToAimDeltaDeg(double Dx, double Dy, double Sens, double YawConstant);
void AccumulateAim(double& YawDeg, double& PitchDeg, double DYawDeg, double DPitchDeg);

double AngleBetweenDeg(const FVector& A, const FVector& B);
bool RayHitsSphere(const FVector& Origin, const FVector& Dir, const FVector& Center, double Radius);
double OrbAngularRadiusDeg(double DistanceCm, double RadiusCm);
ECheckInBand ClassifyBand(double AbsErrorDeg, double OnTargetDeg, bool bOnGeometry);
double SignedArcErrorDeg(const FVector& RestForward, const FVector& TargetDir, const FVector& AimForward);
void ArcAnglesDeg(const FVector& RestForward, const FVector& TargetDir, const FVector& AimForward, double& OutTargetDeg, double& OutAimDeg);

FCheckInRound ScoreFeel90Round(const FCheckInPose& Pose, double ChosenSens);
FCheckInRound ScoreTargetRound(const FCheckInPose& Pose, const FCheckInTargetWorld& Target, double ChosenSens);
FCheckInReport BuildCheckInReport(double ChosenSens, const TArray<FCheckInRound>& Rounds);
} // namespace Sens
