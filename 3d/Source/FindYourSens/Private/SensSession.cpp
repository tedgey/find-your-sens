#include "SensSession.h"
#include "SensSolver.h"

USensSession::USensSession()
{
	Setup.bCaptureIsRaw = true;
}

const TArray<Sens::FScenarioPlan>& USensSession::Plan()
{
	static const TArray<Sens::FScenarioPlan> Plans = {
		{Sens::EScenarioId::Feel90, 5},
		{Sens::EScenarioId::Flick, 10},
		{Sens::EScenarioId::Casual, 10},
		{Sens::EScenarioId::Micro, 10},
	};
	return Plans;
}

Sens::EScenarioId USensSession::CurrentScenario() const
{
	const TArray<Sens::FScenarioPlan>& Plans = Plan();
	if (!Plans.IsValidIndex(ScenarioIndex))
	{
		return Sens::EScenarioId::Feel90;
	}
	return Plans[ScenarioIndex].Id;
}

int32 USensSession::RoundsThisScenario() const
{
	const TArray<Sens::FScenarioPlan>& Plans = Plan();
	if (!Plans.IsValidIndex(ScenarioIndex))
	{
		return 0;
	}
	return Plans[ScenarioIndex].Rounds;
}

int32 USensSession::TotalRounds() const
{
	int32 Total = 0;
	for (const Sens::FScenarioPlan& P : Plan())
	{
		Total += P.Rounds;
	}
	return Total;
}

void USensSession::OpenInfo()
{
	Step = Sens::EAppStep::Info;
}

void USensSession::OpenPrep()
{
	Step = Sens::EAppStep::Prep;
}

void USensSession::BeginTest()
{
	ScenarioIndex = 0;
	RoundIndex = 0;
	Rounds.Reset();
	Recommendation.Reset();
	Step = Sens::EAppStep::Test;
}

void USensSession::RecordRound(const Sens::FRoundRecording& PayloadWithoutIds)
{
	Sens::FRoundRecording Recording = PayloadWithoutIds;
	Recording.Scenario = CurrentScenario();
	Recording.RoundIndex = RoundIndex;
	Rounds.Add(Recording);

	if (RoundIndex + 1 < RoundsThisScenario())
	{
		++RoundIndex;
		return;
	}

	if (ScenarioIndex + 1 < Plan().Num())
	{
		++ScenarioIndex;
		RoundIndex = 0;
		return;
	}

	Recommendation = Sens::RecommendFromRounds(Setup, Rounds);
	Step = Sens::EAppStep::Results;
}

void USensSession::Retest()
{
	Recommendation.Reset();
	Rounds.Reset();
	ScenarioIndex = 0;
	RoundIndex = 0;
}

void USensSession::BackToSetup()
{
	Step = Sens::EAppStep::Setup;
	Recommendation.Reset();
	Rounds.Reset();
	ScenarioIndex = 0;
	RoundIndex = 0;
}
