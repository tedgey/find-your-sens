#include "SensSolver.h"
#include "SensAngles.h"
#include "SensGames.h"
#include "SensPath.h"

namespace Sens
{
static double RoundToDecimals(double Value, int32 Decimals)
{
	const double Scale = FMath::Pow(10.0, static_cast<double>(Decimals));
	return FMath::RoundToDouble(Value * Scale) / Scale;
}

static double Median(TArray<double> Values)
{
	if (Values.Num() == 0)
	{
		return 0.0;
	}
	Values.Sort();
	const int32 Mid = Values.Num() / 2;
	if (Values.Num() % 2 == 0)
	{
		return (Values[Mid - 1] + Values[Mid]) / 2.0;
	}
	return Values[Mid];
}

static double Mean(const TArray<double>& Values)
{
	if (Values.Num() == 0)
	{
		return 0.0;
	}
	double Sum = 0.0;
	for (double V : Values)
	{
		Sum += V;
	}
	return Sum / Values.Num();
}

static double StdDev(const TArray<double>& Values)
{
	if (Values.Num() < 2)
	{
		return 0.0;
	}
	const double M = Mean(Values);
	double Acc = 0.0;
	for (double X : Values)
	{
		Acc += (X - M) * (X - M);
	}
	return FMath::Sqrt(Acc / (Values.Num() - 1));
}

static double WeightedMedian(TArray<TPair<double, double>> Items)
{
	if (Items.Num() == 0)
	{
		return 0.0;
	}
	Items.Sort([](const TPair<double, double>& A, const TPair<double, double>& B) {
		return A.Key < B.Key;
	});
	double Total = 0.0;
	for (const auto& Item : Items)
	{
		Total += Item.Value;
	}
	double Acc = 0.0;
	for (const auto& Item : Items)
	{
		Acc += Item.Value;
		if (Acc >= Total / 2.0)
		{
			return Item.Key;
		}
	}
	return Items.Last().Key;
}

TOptional<double> ImpliedSensitivity(
	double YawDeg,
	double PitchDeg,
	double MouseDx,
	double MouseDy,
	double YawConstant)
{
	const double Denom = YawConstant * (MouseDx * MouseDx + MouseDy * MouseDy);
	if (Denom < 1e-9)
	{
		return {};
	}
	const double Sens = (YawDeg * MouseDx + PitchDeg * MouseDy) / Denom;
	if (!FMath::IsFinite(Sens) || Sens <= 0.0)
	{
		return {};
	}
	return Sens;
}

TOptional<double> ImpliedSensFromFeelTurn(double FeelDeg, double MouseDx, double YawConstant)
{
	const double Dx = FMath::Abs(MouseDx);
	if (Dx < 1e-6)
	{
		return {};
	}
	const double Sens = FeelDeg / (Dx * YawConstant);
	if (!FMath::IsFinite(Sens) || Sens <= 0.0)
	{
		return {};
	}
	return Sens;
}

double RoundWeight(double Straightness, double LateCorrectionRatio)
{
	const double Clean = FMath::Max(0.0, Straightness) * (1.0 - 0.65 * LateCorrectionRatio);
	return FMath::Max(0.05, Clean);
}

TOptional<FRoundEstimate> EstimateRound(const FRoundRecording& Round, double YawConstant)
{
	FRoundEstimate Out;
	Out.Round = Round;
	Out.Metrics = ComputePathMetrics(Round.Samples, Round.Net);
	TOptional<double> Sens;
	if (Round.Scenario == EScenarioId::Feel90)
	{
		Sens = ImpliedSensFromFeelTurn(FeelTurnDeg, Round.Net.X, YawConstant);
	}
	else if (Round.Target.IsSet())
	{
		const FTargetSpec& Target = Round.Target.GetValue();
		Sens = ImpliedSensitivity(Target.YawDeg, Target.PitchDeg, Round.Net.X, Round.Net.Y, YawConstant);
	}
	if (!Sens.IsSet())
	{
		return {};
	}
	Out.ImpliedSens = Sens.GetValue();
	Out.Weight = RoundWeight(Out.Metrics.Straightness, Out.Metrics.LateCorrectionRatio);
	return Out;
}

FStageResult SummarizeStage(EScenarioId Scenario, const TArray<FRoundRecording>& Rounds, double YawConstant)
{
	FStageResult Stage;
	Stage.Scenario = Scenario;
	Stage.bIsAbsolute = Scenario == EScenarioId::Feel90;

	TArray<double> SensValues;
	for (const FRoundRecording& Round : Rounds)
	{
		TOptional<FRoundEstimate> Est = EstimateRound(Round, YawConstant);
		if (Est.IsSet())
		{
			Stage.Estimates.Add(Est.GetValue());
			SensValues.Add(Est.GetValue().ImpliedSens);
		}
	}

	Stage.MedianSens = Median(SensValues);
	Stage.MeanSens = Mean(SensValues);
	Stage.StdDev = StdDev(SensValues);

	const double NFactor = FMath::Min(1.0, static_cast<double>(Stage.Estimates.Num()) / FMath::Max(1, Rounds.Num()));
	const double Cv = Stage.MedianSens > 0.0 ? Stage.StdDev / Stage.MedianSens : 1.0;
	const double SpreadFactor = FMath::Max(0.0, 1.0 - FMath::Min(Cv, 1.0));
	double Quality = 0.0;
	if (Stage.Estimates.Num() > 0)
	{
		for (const FRoundEstimate& E : Stage.Estimates)
		{
			Quality += E.Weight;
		}
		Quality /= Stage.Estimates.Num();
	}
	Stage.Confidence = FMath::Clamp(0.35 * NFactor + 0.4 * SpreadFactor + 0.25 * Quality, 0.0, 1.0);
	return Stage;
}

double ScaleSensForDpi(double SensValue, double FromDpi, double ToDpi)
{
	return SensValue * (FromDpi / ToDpi);
}

double CmPer360(double SensValue, double Dpi, double YawConstant)
{
	return (360.0 / (SensValue * YawConstant * Dpi)) * 2.54;
}

int32 PickDpiTier(double UserDpi)
{
	int32 Best = DpiTiers[0];
	double BestDist = FMath::Abs(UserDpi - Best);
	for (int32 Tier : DpiTiers)
	{
		const double D = FMath::Abs(UserDpi - Tier);
		if (D < BestDist)
		{
			Best = Tier;
			BestDist = D;
		}
	}
	return Best;
}

double EppStageWeight(EScenarioId Scenario, bool bEppOn)
{
	if (!bEppOn)
	{
		return 1.0;
	}
	switch (Scenario)
	{
	case EScenarioId::Feel90:
		return 1.2;
	case EScenarioId::Micro:
		return 1.45;
	case EScenarioId::Casual:
		return 1.05;
	case EScenarioId::Flick:
		return 0.5;
	}
	return 1.0;
}

static FSensRange BuildRange(double Center, double Std, int32 Decimals, double Widen)
{
	const double Pad = FMath::Max(Std, Center * 0.04) * Widen;
	double Low = RoundToDecimals(Center - Pad, Decimals);
	double High = RoundToDecimals(Center + Pad, Decimals);
	const double Step = FMath::Pow(10.0, -static_cast<double>(Decimals));
	FSensRange Range;
	Range.Low = FMath::Max(0.001, Low);
	Range.High = FMath::Max(Range.Low + RoundToDecimals(Step, Decimals), High);
	Range.Center = RoundToDecimals(Center, Decimals);
	return Range;
}

FRecommendation RecommendFromRounds(const FSessionSetup& Setup, const TArray<FRoundRecording>& Rounds)
{
	const FGameProfile& Game = GetGame(Setup.GameId);
	const double FovSetting = Setup.FovSetting.IsSet() ? Setup.FovSetting.GetValue() : Game.DefaultFovSetting;
	const bool bEppOn = Setup.EnhancePointerPrecision == EEnhancePointerPrecision::On;

	const EScenarioId Scenarios[] = {
		EScenarioId::Feel90,
		EScenarioId::Flick,
		EScenarioId::Casual,
		EScenarioId::Micro};

	FRecommendation Rec;
	TArray<FStageResult> PointingStages;
	FStageResult FeelStage;
	bool bHasFeel = false;

	for (EScenarioId Scenario : Scenarios)
	{
		TArray<FRoundRecording> Filtered;
		for (const FRoundRecording& R : Rounds)
		{
			if (R.Scenario == Scenario)
			{
				Filtered.Add(R);
			}
		}
		FStageResult Stage = SummarizeStage(Scenario, Filtered, Game.YawConstant);
		Rec.StageResults.Add(Stage);
		if (Scenario == EScenarioId::Feel90)
		{
			FeelStage = Stage;
			bHasFeel = true;
		}
		else
		{
			PointingStages.Add(Stage);
		}
	}

	TArray<TPair<double, double>> FeelItems;
	if (bHasFeel)
	{
		for (const FRoundEstimate& Est : FeelStage.Estimates)
		{
			FeelItems.Add(TPair<double, double>(
				Est.ImpliedSens,
				Est.Weight * (0.25 + 0.75 * FeelStage.Confidence)));
		}
	}

	const double BlendedSens = WeightedMedian(FeelItems);
	TArray<double> AllFeelSens;
	for (const auto& Item : FeelItems)
	{
		AllFeelSens.Add(Item.Key);
	}
	const double BlendedStd = StdDev(AllFeelSens);

	TArray<double> PointingValues;
	for (const FStageResult& Stage : PointingStages)
	{
		for (const FRoundEstimate& E : Stage.Estimates)
		{
			PointingValues.Add(E.ImpliedSens);
		}
	}
	if (PointingValues.Num() > 0)
	{
		Rec.PointingBaselineSens = Median(PointingValues);
	}

	double FromStages = bHasFeel ? FeelStage.Confidence : 0.0;
	if (PointingStages.Num() > 0)
	{
		double PathConf = 0.0;
		for (const FStageResult& S : PointingStages)
		{
			PathConf += S.Confidence;
		}
		PathConf /= PointingStages.Num();
		FromStages = 0.7 * FromStages + 0.3 * PathConf;
	}

	// Browser tests haircut EPP. This Unreal capture path uses raw mouse, so skip it.
	const bool bEppPenaltyApplied = bEppOn && Game.bUsesRawInput && !Setup.bCaptureIsRaw;
	const double EppFactor = bEppPenaltyApplied ? 0.78 : 1.0;
	const double OverallConfidence = FromStages * EppFactor;

	const int32 DpiTier = PickDpiTier(Setup.Dpi);
	const double OutputDpi = Setup.Dpi;
	const FSensRange SensRange = BuildRange(
		BlendedSens,
		BlendedStd * 0.75,
		Game.SensDecimals,
		bEppPenaltyApplied ? 1.25 : 1.0);

	FSensRange EDpi;
	EDpi.Low = RoundToDecimals(SensRange.Low * OutputDpi, 1);
	EDpi.High = RoundToDecimals(SensRange.High * OutputDpi, 1);
	EDpi.Center = RoundToDecimals(SensRange.Center * OutputDpi, 1);

	TOptional<FSensRange> SensRangeAtNearestTier;
	if (DpiTier != static_cast<int32>(OutputDpi))
	{
		SensRangeAtNearestTier = BuildRange(
			ScaleSensForDpi(SensRange.Center, OutputDpi, DpiTier),
			ScaleSensForDpi(BlendedStd * 0.75, OutputDpi, DpiTier),
			Game.SensDecimals,
			bEppPenaltyApplied ? 1.25 : 1.0);
	}

	TOptional<double> VsCurrent;
	if (Setup.CurrentSens.IsSet() && Setup.CurrentSens.GetValue() > 0.0 && SensRange.Center > 0.0)
	{
		VsCurrent = SensRange.Center / Setup.CurrentSens.GetValue();
	}

	Rec.InputNotes.Add(
		TEXT("Your quoted sensitivity comes from the felt 90° turns. Flick/casual/micro targets check consistency; alone they tend to measure desktop-style screen pointing (~1:1), not CS2 turn feel."));
	if (Rec.PointingBaselineSens.IsSet())
	{
		Rec.InputNotes.Add(FString::Printf(
			TEXT("Target-stage pointing baseline was ~%.*f (often near geometric 1:1). That is expected and not what we quote."),
			Game.SensDecimals,
			Rec.PointingBaselineSens.GetValue()));
	}
	if (Setup.bCaptureIsRaw)
	{
		Rec.InputNotes.Add(
			TEXT("This Unreal test captures mouse with smoothing off and axis sensitivity 1.0 (raw-leaning). Enhance pointer precision does not change the quoted confidence the way the browser test does."));
	}
	else if (bEppPenaltyApplied)
	{
		Rec.InputNotes.Add(FString::Printf(
			TEXT("%s uses raw input, so Windows Enhance pointer precision does not apply in-game. This capture path may still accelerate fast moves. Overall confidence was reduced from %d%% to %d%% for that mismatch."),
			*Game.Name,
			FMath::RoundToInt(FromStages * 100.0),
			FMath::RoundToInt(OverallConfidence * 100.0)));
	}
	if (VsCurrent.IsSet() && (VsCurrent.GetValue() < 0.7 || VsCurrent.GetValue() > 1.4))
	{
		Rec.InputNotes.Add(FString::Printf(
			TEXT("This sits notably %s your current %s. Treat it as experimental and compare in-game before committing."),
			VsCurrent.GetValue() > 1.0 ? TEXT("above") : TEXT("below"),
			*FString::SanitizeFloat(Setup.CurrentSens.GetValue())));
	}

	const double ArenaWidth = Rounds.Num() > 0 ? Rounds[0].ArenaWidth : Setup.ResolutionWidth;
	const double ArenaHeight = Rounds.Num() > 0 ? Rounds[0].ArenaHeight : Setup.ResolutionHeight;
	const FDisplayConfig ArenaDisplay = BuildDisplayConfig(Game, FovSetting, ArenaWidth, ArenaHeight);

	Rec.GameId = Setup.GameId;
	Rec.Dpi = OutputDpi;
	Rec.NearestDpiTier = DpiTier;
	Rec.SensRangeAtNearestTier = SensRangeAtNearestTier;
	Rec.SensRange = SensRange;
	Rec.Confidence = FMath::Clamp(OverallConfidence, 0.0, 1.0);
	Rec.FromStages = FMath::Clamp(FromStages, 0.0, 1.0);
	Rec.bEppPenaltyApplied = bEppPenaltyApplied;
	Rec.EppFactor = EppFactor;
	Rec.CmPer360 = CmPer360(SensRange.Center, OutputDpi, Game.YawConstant);
	Rec.EDpiRange = EDpi;
	Rec.CurrentSens = Setup.CurrentSens;
	Rec.VsCurrentRatio = VsCurrent;
	Rec.FovSetting = FovSetting;
	Rec.HorizontalFovDeg = ArenaDisplay.HorizontalFovDeg;
	Rec.ArenaWidth = ArenaWidth;
	Rec.ArenaHeight = ArenaHeight;
	Rec.GameResolutionWidth = Setup.ResolutionWidth;
	Rec.GameResolutionHeight = Setup.ResolutionHeight;
	Rec.YawConstant = Game.YawConstant;
	Rec.EnhancePointerPrecision = Setup.EnhancePointerPrecision;
	Rec.bUsesRawInput = Game.bUsesRawInput;
	return Rec;
}
} // namespace Sens
