#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SensTypes.h"
#include "SensSession.generated.h"

UCLASS()
class FINDYOURSENS_API USensSession : public UObject
{
	GENERATED_BODY()

public:
	USensSession();

	Sens::FSessionSetup Setup;
	Sens::EAppStep Step = Sens::EAppStep::Setup;
	int32 ScenarioIndex = 0;
	int32 RoundIndex = 0;
	TArray<Sens::FRoundRecording> Rounds;
	TOptional<Sens::FRecommendation> Recommendation;
	double CheckInSens = 0.0;
	TArray<Sens::FCheckInRound> CheckInRounds;
	TOptional<Sens::FCheckInReport> CheckInReport;

	static const TArray<Sens::FScenarioPlan>& Plan();
	Sens::EScenarioId CurrentScenario() const;
	int32 RoundsThisScenario() const;
	int32 TotalRounds() const;
	int32 CompletedRounds() const { return Rounds.Num(); }
	bool IsLiveTest() const;

	void OpenInfo();
	void OpenPrep();
	void BeginTest();
	void RecordRound(const Sens::FRoundRecording& PayloadWithoutIds);
	void Retest();
	void BackToSetup();
	void OpenCheckInSelect(bool bResetSensToCenter);
	void BeginCheckInTest();
	void RecordCheckInRound(const Sens::FCheckInRound& PayloadWithoutIds);
	void RestartCheckInSelect();
	void BackToPackResults();
};
