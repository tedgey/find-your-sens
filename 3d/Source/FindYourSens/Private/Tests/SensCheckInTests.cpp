#include "Misc/AutomationTest.h"
#include "SensCheckIn.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSensCheckInYawInvertTest,
	"FindYourSens.CheckIn.YawInvert",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSensCheckInYawInvertTest::RunTest(const FString& Parameters)
{
	const double Sens = 1.13;
	const double YawC = 0.022;
	const double Dx = Sens::FeelTurnDeg / (Sens * YawC);
	const FVector2D Delta = Sens::MouseToAimDeltaDeg(Dx, 0.0, Sens, YawC);
	TestTrue(TEXT("90° of counts at 1.13 recovers 90° yaw"), FMath::IsNearlyEqual(Delta.X, Sens::FeelTurnDeg, 1e-4));
	TestTrue(TEXT("no pitch from horizontal counts"), FMath::IsNearlyEqual(Delta.Y, 0.0, 1e-9));

	const FVector2D PitchDelta = Sens::MouseToAimDeltaDeg(0.0, 10.0, 1.0, YawC);
	TestTrue(TEXT("mouse up looks up"), PitchDelta.Y > 0.0);
	TestTrue(TEXT("pitch uses m_pitch = m_yaw"), FMath::IsNearlyEqual(PitchDelta.Y, 10.0 * YawC, 1e-9));

	double Yaw = 0.0;
	double Pitch = 80.0;
	Sens::AccumulateAim(Yaw, Pitch, 0.0, 20.0);
	TestTrue(TEXT("pitch clamps below 89"), FMath::IsNearlyEqual(Pitch, Sens::PitchClampDeg, 1e-6));
	Pitch = -80.0;
	Sens::AccumulateAim(Yaw, Pitch, 0.0, -20.0);
	TestTrue(TEXT("pitch clamps above -89"), FMath::IsNearlyEqual(Pitch, -Sens::PitchClampDeg, 1e-6));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSensCheckInBandEdgesTest,
	"FindYourSens.CheckIn.BandEdges",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSensCheckInBandEdgesTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("<= on-target deg is on-target"), Sens::ClassifyBand(3.0, 3.0, false) == Sens::ECheckInBand::OnTarget);
	TestTrue(TEXT("just over on-target is close"), Sens::ClassifyBand(3.1, 3.0, false) == Sens::ECheckInBand::Close);
	TestTrue(TEXT("2x radius is still close"), Sens::ClassifyBand(6.0, 3.0, false) == Sens::ECheckInBand::Close);
	TestTrue(TEXT("past 2x is miss"), Sens::ClassifyBand(6.1, 3.0, false) == Sens::ECheckInBand::Miss);
	TestTrue(TEXT("geometry hit wins"), Sens::ClassifyBand(20.0, 3.0, true) == Sens::ECheckInBand::OnTarget);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSensCheckInFeel90ScoreTest,
	"FindYourSens.CheckIn.Feel90Score",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSensCheckInFeel90ScoreTest::RunTest(const FString& Parameters)
{
	Sens::FCheckInPose Pose;
	Pose.RestForward = FVector::ForwardVector;
	Pose.AimForward = FVector::ForwardVector.RotateAngleAxis(90.f, FVector::UpVector);
	Pose.YawDeg = 90.0;
	Sens::FCheckInRound On = Sens::ScoreFeel90Round(Pose, 1.13);
	TestTrue(TEXT("exact 90 is on-target"), On.Band == Sens::ECheckInBand::OnTarget);
	TestTrue(TEXT("exact 90 is not overshoot"), !On.bOvershoot && !On.bWrongWay);
	TestTrue(TEXT("exact 90 signed error ~0"), FMath::Abs(On.SignedErrorDeg) < 1e-4);

	Pose.YawDeg = 95.0;
	Pose.AimForward = FVector::ForwardVector.RotateAngleAxis(95.f, FVector::UpVector);
	Sens::FCheckInRound Over = Sens::ScoreFeel90Round(Pose, 1.13);
	TestTrue(TEXT("5° past is close"), Over.Band == Sens::ECheckInBand::Close);
	TestTrue(TEXT("5° past is overshoot"), Over.bOvershoot);

	Pose.YawDeg = 85.0;
	Pose.AimForward = FVector::ForwardVector.RotateAngleAxis(85.f, FVector::UpVector);
	Sens::FCheckInRound Under = Sens::ScoreFeel90Round(Pose, 1.13);
	TestTrue(TEXT("5° short is close"), Under.Band == Sens::ECheckInBand::Close);
	TestTrue(TEXT("5° short is not overshoot"), !Under.bOvershoot && !Under.bWrongWay);

	Pose.YawDeg = -20.0;
	Pose.AimForward = FVector::ForwardVector.RotateAngleAxis(-20.f, FVector::UpVector);
	Sens::FCheckInRound Wrong = Sens::ScoreFeel90Round(Pose, 1.13);
	TestTrue(TEXT("wrong way is miss"), Wrong.Band == Sens::ECheckInBand::Miss);
	TestTrue(TEXT("wrong way flag"), Wrong.bWrongWay);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSensCheckInTargetScoreTest,
	"FindYourSens.CheckIn.TargetScore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSensCheckInTargetScoreTest::RunTest(const FString& Parameters)
{
	Sens::FCheckInPose Pose;
	Pose.Eye = FVector(0.f, 0.f, 100.f);
	Pose.RestForward = FVector::ForwardVector;

	Sens::FCheckInTargetWorld Target;
	Target.RadiusCm = 35.0;
	Target.Spec.YawDeg = 30.0;
	Target.Spec.PitchDeg = 0.0;
	const FVector ToTarget = FVector::ForwardVector.RotateAngleAxis(30.f, FVector::UpVector) * 650.f;
	Target.Center = Pose.Eye + ToTarget;

	Pose.YawDeg = 30.0;
	Pose.AimForward = ToTarget.GetSafeNormal();
	Sens::FCheckInRound Hit = Sens::ScoreTargetRound(Pose, Target, 1.5);
	TestTrue(TEXT("looking at orb is on-target"), Hit.Band == Sens::ECheckInBand::OnTarget);
	TestTrue(TEXT("ray hits orb"), Sens::RayHitsSphere(Pose.Eye, Pose.AimForward, Target.Center, Target.RadiusCm));

	Pose.YawDeg = 40.0;
	Pose.AimForward = FVector::ForwardVector.RotateAngleAxis(40.f, FVector::UpVector);
	Sens::FCheckInRound Over = Sens::ScoreTargetRound(Pose, Target, 1.5);
	TestTrue(TEXT("past the orb is overshoot"), Over.bOvershoot);
	TestTrue(TEXT("10° past is miss"), Over.Band == Sens::ECheckInBand::Miss);

	Pose.YawDeg = 20.0;
	Pose.AimForward = FVector::ForwardVector.RotateAngleAxis(20.f, FVector::UpVector);
	Sens::FCheckInRound Under = Sens::ScoreTargetRound(Pose, Target, 1.5);
	TestTrue(TEXT("short of the orb is undershoot"), !Under.bOvershoot && !Under.bWrongWay);
	TestTrue(TEXT("10° short is miss"), Under.Band == Sens::ECheckInBand::Miss);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSensCheckInReportDiagnosticTest,
	"FindYourSens.CheckIn.ReportIsDiagnostic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSensCheckInReportDiagnosticTest::RunTest(const FString& Parameters)
{
	TArray<Sens::FCheckInRound> Rounds;
	Sens::FCheckInPose Pose;
	Pose.RestForward = FVector::ForwardVector;
	Pose.AimForward = FVector::ForwardVector.RotateAngleAxis(90.f, FVector::UpVector);
	Pose.YawDeg = 90.0;
	for (int32 I = 0; I < 5; ++I)
	{
		Sens::FCheckInRound Round = Sens::ScoreFeel90Round(Pose, 1.47);
		Round.Scenario = Sens::EScenarioId::Feel90;
		Round.RoundIndex = I;
		Rounds.Add(Round);
	}

	const Sens::FCheckInReport Report = Sens::BuildCheckInReport(1.47, Rounds);
	TestTrue(TEXT("keeps chosen sens"), FMath::IsNearlyEqual(Report.ChosenSens, 1.47));
	TestTrue(TEXT("has Felt 90 stage"), Report.Stages.Num() == 1 && Report.Stages[0].Scenario == Sens::EScenarioId::Feel90);
	TestTrue(TEXT("five on-target"), Report.Stages[0].OnTarget == 5);
	bool bMentionsPackUnchanged = false;
	for (const FString& Note : Report.Notes)
	{
		if (Note.Contains(TEXT("does not change")))
		{
			bMentionsPackUnchanged = true;
		}
	}
	TestTrue(TEXT("notes say pack is unchanged"), bMentionsPackUnchanged);
	return true;
}

#endif
