#include "Misc/AutomationTest.h"
#include "SensAngles.h"
#include "SensGames.h"
#include "SensSolver.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSensCs2FovTest,
	"FindYourSens.Math.Cs2Fov",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSensCs2FovTest::RunTest(const FString& Parameters)
{
	const Sens::FGameProfile& Game = Sens::GetGame(Sens::EGameId::CS2);
	const Sens::FDisplayConfig Display = Sens::BuildDisplayConfig(Game, 90.0, 1920.0, 1080.0);
	TestTrue(TEXT("Source 90 FOV expands on 16:9"), Display.HorizontalFovDeg > 100.0 && Display.HorizontalFovDeg < 110.0);

	double Yaw = 0.0;
	double Pitch = 0.0;
	Sens::OffsetPxToAngles({400.0, 0.0}, Display, Yaw, Pitch);
	TestTrue(TEXT("center-right offset is +yaw"), Yaw > 0.0);
	TestTrue(TEXT("no pitch on horizontal offset"), FMath::Abs(Pitch) < 1e-6);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSensImpliedTest,
	"FindYourSens.Math.ImpliedSens",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSensImpliedTest::RunTest(const FString& Parameters)
{
	const double YawConstant = 0.022;
	const double YawDeg = 30.0;
	const double MouseDx = YawDeg / (1.0 * YawConstant);
	const TOptional<double> Recovered = Sens::ImpliedSensitivity(YawDeg, 0.0, MouseDx, 0.0, YawConstant);
	TestTrue(TEXT("recovers 1.0"), Recovered.IsSet() && FMath::IsNearlyEqual(Recovered.GetValue(), 1.0, 1e-5));

	const double FeelDx = Sens::FeelTurnDeg / (1.13 * YawConstant);
	const TOptional<double> Feel = Sens::ImpliedSensFromFeelTurn(Sens::FeelTurnDeg, FeelDx, YawConstant);
	TestTrue(TEXT("recovers felt 1.13"), Feel.IsSet() && FMath::IsNearlyEqual(Feel.GetValue(), 1.13, 1e-5));

	TestTrue(TEXT("scale 800->400"), FMath::IsNearlyEqual(Sens::ScaleSensForDpi(1.0, 800.0, 400.0), 2.0));
	TestTrue(TEXT("scale 800->1600"), FMath::IsNearlyEqual(Sens::ScaleSensForDpi(1.0, 800.0, 1600.0), 0.5));

	const double Cm = Sens::CmPer360(1.0, 800.0, 0.022);
	TestTrue(TEXT("cm/360 band"), Cm > 50.0 && Cm < 53.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSensQuoteFeelNotPointingTest,
	"FindYourSens.Math.QuoteFeelNotPointing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSensQuoteFeelNotPointingTest::RunTest(const FString& Parameters)
{
	Sens::FSessionSetup Setup;
	Setup.GameId = Sens::EGameId::CS2;
	Setup.Dpi = 400.0;
	Setup.ResolutionWidth = 1920;
	Setup.ResolutionHeight = 1080;
	Setup.EnhancePointerPrecision = Sens::EEnhancePointerPrecision::Off;
	Setup.CurrentSens = 1.13;
	Setup.bCaptureIsRaw = true;

	const double FeelSens = 1.13;
	const double FeelDx = Sens::FeelTurnDeg / (FeelSens * 0.022);
	const double PointSens = 2.4;
	const double PointYaw = 30.0;
	const double PointDx = PointYaw / (PointSens * 0.022);

	TArray<Sens::FRoundRecording> Rounds;
	for (int32 I = 0; I < 5; ++I)
	{
		Sens::FRoundRecording R;
		R.Scenario = Sens::EScenarioId::Feel90;
		R.RoundIndex = I;
		const double Dx = FeelDx * (1.0 + (I - 2) * 0.02);
		R.Samples.Add({0.0, 0.0, 0.0});
		R.Samples.Add({100.0, Dx, 0.0});
		R.Net = {Dx, 0.0};
		R.DurationMs = 100.0;
		R.ArenaWidth = 1920.0;
		R.ArenaHeight = 1080.0;
		Rounds.Add(R);
	}

	const Sens::EScenarioId Pointing[] = {
		Sens::EScenarioId::Flick,
		Sens::EScenarioId::Casual,
		Sens::EScenarioId::Micro};
	for (Sens::EScenarioId Scenario : Pointing)
	{
		for (int32 I = 0; I < 10; ++I)
		{
			Sens::FRoundRecording R;
			R.Scenario = Scenario;
			R.RoundIndex = I;
			Sens::FTargetSpec Target;
			Target.OffsetPx = {200.0, 0.0};
			Target.YawDeg = PointYaw;
			Target.PitchDeg = 0.0;
			R.Target = Target;
			R.Samples.Add({0.0, 0.0, 0.0});
			R.Samples.Add({100.0, PointDx, 0.0});
			R.Net = {PointDx, 0.0};
			R.DurationMs = 100.0;
			R.ArenaWidth = 1920.0;
			R.ArenaHeight = 1080.0;
			Rounds.Add(R);
		}
	}

	const Sens::FRecommendation Rec = Sens::RecommendFromRounds(Setup, Rounds);
	TestTrue(TEXT("quotes Felt ~1.13 not pointing ~2.4"), Rec.SensRange.Center > 0.9 && Rec.SensRange.Center < 1.4);
	TestTrue(TEXT("pointing baseline stays high"), Rec.PointingBaselineSens.IsSet() && Rec.PointingBaselineSens.GetValue() > 2.0);
	TestTrue(TEXT("keeps current sens"), Rec.CurrentSens.IsSet() && FMath::IsNearlyEqual(Rec.CurrentSens.GetValue(), 1.13));
	TestTrue(TEXT("dpi 400"), FMath::IsNearlyEqual(Rec.Dpi, 400.0));
	TestTrue(TEXT("raw capture skips EPP haircut"), !Rec.bEppPenaltyApplied);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSensEppStageWeightTest,
	"FindYourSens.Math.EppStageWeight",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSensEppStageWeightTest::RunTest(const FString& Parameters)
{
	TestTrue(
		TEXT("down-weights flicks when EPP is on"),
		Sens::EppStageWeight(Sens::EScenarioId::Flick, true)
			< Sens::EppStageWeight(Sens::EScenarioId::Micro, true));
	return true;
}

#endif
