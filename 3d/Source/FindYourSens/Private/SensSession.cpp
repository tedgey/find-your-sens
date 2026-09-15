#include "SensSession.h"
#include "SensCheckIn.h"
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
	CheckInRounds.Reset();
	CheckInReport.Reset();
	CheckInSens = 0.0;
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
	CheckInRounds.Reset();
	CheckInReport.Reset();
	CheckInSens = 0.0;
	ScenarioIndex = 0;
	RoundIndex = 0;
}

void USensSession::BackToSetup()
{
	Step = Sens::EAppStep::Setup;
	Recommendation.Reset();
	Rounds.Reset();
	CheckInRounds.Reset();
	CheckInReport.Reset();
	CheckInSens = 0.0;
	ScenarioIndex = 0;
	RoundIndex = 0;
}

bool USensSession::IsLiveTest() const
{
	return Step == Sens::EAppStep::Test || Step == Sens::EAppStep::CheckInTest;
}

void USensSession::OpenCheckInSelect(bool bResetSensToCenter)
{
	if (bResetSensToCenter || CheckInSens <= 0.0)
	{
		if (Recommendation.IsSet() && Recommendation->SensRange.Center > 0.0)
		{
			CheckInSens = Recommendation->SensRange.Center;
		}
		else if (Setup.CurrentSens.IsSet() && Setup.CurrentSens.GetValue() > 0.0)
		{
			CheckInSens = Setup.CurrentSens.GetValue();
		}
		else if (CheckInSens <= 0.0)
		{
			CheckInSens = 1.0;
		}
	}
	CheckInRounds.Reset();
	CheckInReport.Reset();
	ScenarioIndex = 0;
	RoundIndex = 0;
	Step = Sens::EAppStep::CheckInSelect;
}

void USensSession::BeginCheckInTest()
{
	if (CheckInSens <= 0.0)
	{
		return;
	}
	ScenarioIndex = 0;
	RoundIndex = 0;
	CheckInRounds.Reset();
	CheckInReport.Reset();
	Step = Sens::EAppStep::CheckInTest;
}

void USensSession::RecordCheckInRound(const Sens::FCheckInRound& PayloadWithoutIds)
{
	Sens::FCheckInRound Recording = PayloadWithoutIds;
	Recording.Scenario = CurrentScenario();
	Recording.RoundIndex = RoundIndex;
	Recording.ChosenSens = CheckInSens;
	CheckInRounds.Add(Recording);

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

	CheckInReport = Sens::BuildCheckInReport(CheckInSens, CheckInRounds);
	Step = Sens::EAppStep::CheckInResults;
}

void USensSession::RestartCheckInSelect()
{
	OpenCheckInSelect(false);
}

void USensSession::BackToPackResults()
{
	CheckInRounds.Reset();
	ScenarioIndex = 0;
	RoundIndex = 0;
	if (Recommendation.IsSet())
	{
		Step = Sens::EAppStep::Results;
	}
	else
	{
		Step = Sens::EAppStep::Setup;
	}
}
