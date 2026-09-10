#include "Misc/AutomationTest.h"
#include "SensAngles.h"
#include "SensGames.h"
#include "SensSolver.h"
#include "SensTargets.h"

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

static Sens::FRoundRecording MakeFeelRound(int32 Index, double SensValue)
{
	const double FeelDx = Sens::FeelTurnDeg / (SensValue * 0.022);
	Sens::FRoundRecording R;
	R.Scenario = Sens::EScenarioId::Feel90;
	R.RoundIndex = Index;
	const double Dx = FeelDx * (1.0 + (Index - 2) * 0.02);
	R.Samples.Add({0.0, 0.0, 0.0});
	R.Samples.Add({100.0, Dx, 0.0});
	R.Net = {Dx, 0.0};
	R.DurationMs = 100.0;
	R.ArenaWidth = 1920.0;
	R.ArenaHeight = 1080.0;
	return R;
}

static Sens::FRoundRecording MakePointRound(Sens::EScenarioId Scenario, int32 Index, double SensValue)
{
	const double PointYaw = 30.0;
	const double PointDx = PointYaw / (SensValue * 0.022);
	Sens::FRoundRecording R;
	R.Scenario = Scenario;
	R.RoundIndex = Index;
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
	return R;
}

static TArray<Sens::FRoundRecording> MakeSessionRounds(double FeelSens, double PointSens)
{
	TArray<Sens::FRoundRecording> Rounds;
	for (int32 I = 0; I < 5; ++I)
	{
		Rounds.Add(MakeFeelRound(I, FeelSens));
	}
	const Sens::EScenarioId Pointing[] = {
		Sens::EScenarioId::Flick,
		Sens::EScenarioId::Casual,
		Sens::EScenarioId::Micro};
	for (Sens::EScenarioId Scenario : Pointing)
	{
		for (int32 I = 0; I < 10; ++I)
		{
			Rounds.Add(MakePointRound(Scenario, I, PointSens));
		}
	}
	return Rounds;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSensQuoteBlendsScenariosTest,
	"FindYourSens.Math.QuoteBlendsScenarios",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSensQuoteBlendsScenariosTest::RunTest(const FString& Parameters)
{
	Sens::FSessionSetup Setup;
	Setup.GameId = Sens::EGameId::CS2;
	Setup.Dpi = 400.0;
	Setup.ResolutionWidth = 1920;
	Setup.ResolutionHeight = 1080;
	Setup.CurrentSens = 1.13;
	Setup.bCaptureIsRaw = true;

	const Sens::FRecommendation Rec = Sens::RecommendFromRounds(Setup, MakeSessionRounds(1.13, 2.4));
	TestTrue(TEXT("pack sits between Felt 1.13 and scenario 2.4"), Rec.SensRange.Center > 1.5 && Rec.SensRange.Center < 2.2);
	TestTrue(TEXT("scenarios pull the pack off Felt-only"), Rec.SensRange.Center > 1.4);
	TestTrue(TEXT("Felt 90 still pulls the pack off scenario-only"), Rec.SensRange.Center < 2.3);
	TestTrue(TEXT("pointing baseline stays high"), Rec.PointingBaselineSens.IsSet() && Rec.PointingBaselineSens.GetValue() > 2.0);
	TestTrue(TEXT("keeps current sens"), Rec.CurrentSens.IsSet() && FMath::IsNearlyEqual(Rec.CurrentSens.GetValue(), 1.13));
	TestTrue(TEXT("dpi 400"), FMath::IsNearlyEqual(Rec.Dpi, 400.0));

	double BlendSum = 0.0;
	double ScenarioShare = 0.0;
	double FeelShare = 0.0;
	for (const Sens::FStageResult& Stage : Rec.StageResults)
	{
		BlendSum += Stage.BlendWeight;
		if (Stage.Scenario == Sens::EScenarioId::Feel90)
		{
			FeelShare = Stage.BlendWeight;
		}
		else
		{
			ScenarioShare += Stage.BlendWeight;
		}
	}
	TestTrue(TEXT("blend weights sum to 1"), FMath::IsNearlyEqual(BlendSum, 1.0, 1e-4));
	TestTrue(TEXT("scenarios outweigh Felt 90 in the pack"), ScenarioShare > FeelShare);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSensQuoteWhenStagesAgreeTest,
	"FindYourSens.Math.QuoteWhenStagesAgree",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSensQuoteWhenStagesAgreeTest::RunTest(const FString& Parameters)
{
	Sens::FSessionSetup Setup;
	Setup.GameId = Sens::EGameId::CS2;
	Setup.Dpi = 400.0;
	Setup.bCaptureIsRaw = true;
	const Sens::FRecommendation Rec = Sens::RecommendFromRounds(Setup, MakeSessionRounds(1.13, 1.13));
	TestTrue(TEXT("agreed stages quote ~1.13"), Rec.SensRange.Center > 1.05 && Rec.SensRange.Center < 1.21);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSensScenarioBlendPriorTest,
	"FindYourSens.Math.ScenarioBlendPrior",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSensScenarioBlendPriorTest::RunTest(const FString& Parameters)
{
	const double Feel = Sens::ScenarioBlendPrior(Sens::EScenarioId::Feel90);
	const double Scenarios =
		Sens::ScenarioBlendPrior(Sens::EScenarioId::Flick)
		+ Sens::ScenarioBlendPrior(Sens::EScenarioId::Casual)
		+ Sens::ScenarioBlendPrior(Sens::EScenarioId::Micro);
	TestTrue(TEXT("priors sum to 1"), FMath::IsNearlyEqual(Feel + Scenarios, 1.0, 1e-6));
	TestTrue(TEXT("scenarios outweigh Felt 90"), Scenarios > Feel);
	TestTrue(TEXT("Felt 90 still has a real vote"), Feel >= 0.35);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSensWorldTargetOnScreenTest,
	"FindYourSens.Math.WorldTargetOnScreen",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSensWorldTargetOnScreenTest::RunTest(const FString& Parameters)
{
	const Sens::FGameProfile& Game = Sens::GetGame(Sens::EGameId::CS2);
	const Sens::FDisplayConfig Display = Sens::BuildDisplayConfig(Game, 90.0, 1920.0, 1080.0);
	const double MaxX = Display.Width * 0.45;
	const double MaxY = Display.Height * 0.45;
	const Sens::EScenarioId Scenarios[] = {
		Sens::EScenarioId::Flick,
		Sens::EScenarioId::Casual,
		Sens::EScenarioId::Micro,
	};
	for (int32 I = 0; I < 200; ++I)
	{
		for (const Sens::EScenarioId Scenario : Scenarios)
		{
			const Sens::FTargetSpec Spec = Sens::SpawnWorldTarget(Scenario, Display);
			TestTrue(TEXT("orb stays inside horizontal view"), FMath::Abs(Spec.OffsetPx.X) <= MaxX + 1e-6);
			TestTrue(TEXT("orb stays inside vertical view"), FMath::Abs(Spec.OffsetPx.Y) <= MaxY + 1e-6);
		}
	}
	return true;
}

#endif
