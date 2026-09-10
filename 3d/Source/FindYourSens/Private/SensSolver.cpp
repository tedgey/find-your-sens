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
	Stage.bIsAbsolute = true;

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

double ScenarioBlendPrior(EScenarioId Scenario)
{
	switch (Scenario)
	{
	case EScenarioId::Feel90:
		return 0.40;
	case EScenarioId::Flick:
		return 0.22;
	case EScenarioId::Casual:
		return 0.22;
	case EScenarioId::Micro:
		return 0.16;
	}
	return 0.0;
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

	const EScenarioId Scenarios[] = {
		EScenarioId::Feel90,
		EScenarioId::Flick,
		EScenarioId::Casual,
		EScenarioId::Micro};

	FRecommendation Rec;
	TOptional<double> FeelMedian;
	TArray<double> PointingValues;

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
		if (Scenario == EScenarioId::Feel90 && Stage.Estimates.Num() > 0)
		{
			FeelMedian = Stage.MedianSens;
		}
		else
		{
			for (const FRoundEstimate& E : Stage.Estimates)
			{
				PointingValues.Add(E.ImpliedSens);
			}
		}
	}

	if (PointingValues.Num() > 0)
	{
		Rec.PointingBaselineSens = Median(PointingValues);
	}

	TArray<double> StageMasses;
	StageMasses.Init(0.0, Rec.StageResults.Num());
	double PriorTotal = 0.0;
	for (const FStageResult& Stage : Rec.StageResults)
	{
		if (Stage.Estimates.Num() > 0)
		{
			PriorTotal += ScenarioBlendPrior(Stage.Scenario);
		}
	}
	if (PriorTotal < 1e-9)
	{
		PriorTotal = 1.0;
	}

	double MassTotal = 0.0;
	for (int32 I = 0; I < Rec.StageResults.Num(); ++I)
	{
		const FStageResult& Stage = Rec.StageResults[I];
		if (Stage.Estimates.Num() == 0)
		{
			continue;
		}
		const double Prior = ScenarioBlendPrior(Stage.Scenario) / PriorTotal;
		const double Mass = Prior * (0.25 + 0.75 * Stage.Confidence);
		StageMasses[I] = Mass;
		MassTotal += Mass;
	}
	if (MassTotal < 1e-9)
	{
		MassTotal = 1.0;
	}

	double BlendNumer = 0.0;
	double FromStagesNumer = 0.0;
	double FromStagesDenom = 0.0;
	TArray<double> StageCenters;
	for (int32 I = 0; I < Rec.StageResults.Num(); ++I)
	{
		FStageResult& Stage = Rec.StageResults[I];
		Stage.BlendWeight = StageMasses[I] / MassTotal;
		if (Stage.Estimates.Num() == 0)
		{
			Stage.bIsAbsolute = false;
			continue;
		}

		TArray<TPair<double, double>> StageItems;
		for (const FRoundEstimate& Est : Stage.Estimates)
		{
			StageItems.Add(TPair<double, double>(Est.ImpliedSens, Est.Weight));
		}
		const double StageCenter = WeightedMedian(StageItems);
		StageCenters.Add(StageCenter);
		BlendNumer += StageCenter * StageMasses[I];

		const double Prior = ScenarioBlendPrior(Stage.Scenario);
		FromStagesNumer += Prior * Stage.Confidence;
		FromStagesDenom += Prior;
	}

	const double BlendedSens = BlendNumer / MassTotal;
	const double BlendedStd = StdDev(StageCenters);
	const double FromStages = FromStagesDenom > 1e-9 ? FromStagesNumer / FromStagesDenom : 0.0;
	const double OverallConfidence = FromStages;

	const int32 DpiTier = PickDpiTier(Setup.Dpi);
	const double OutputDpi = Setup.Dpi;
	const FSensRange SensRange = BuildRange(
		BlendedSens,
		BlendedStd * 0.75,
		Game.SensDecimals,
		1.0);

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
			1.0);
	}

	TOptional<double> VsCurrent;
	if (Setup.CurrentSens.IsSet() && Setup.CurrentSens.GetValue() > 0.0 && SensRange.Center > 0.0)
	{
		VsCurrent = SensRange.Center / Setup.CurrentSens.GetValue();
	}

	Rec.InputNotes.Add(
		TEXT("Your quoted sensitivity blends Felt 90° with flick, casual, and micro. World markers use the same yaw math as the 90° turns. Scenarios together carry more of the pack center than Felt 90° alone."));
	if (FeelMedian.IsSet() && Rec.PointingBaselineSens.IsSet())
	{
		const double FeelVal = FeelMedian.GetValue();
		const double PointVal = Rec.PointingBaselineSens.GetValue();
		if (FeelVal > 1e-6)
		{
			const double Ratio = PointVal / FeelVal;
			if (Ratio < 0.75 || Ratio > 1.25)
			{
				Rec.InputNotes.Add(FString::Printf(
					TEXT("Flick/casual/micro median was ~%.*f while Felt 90° was ~%.*f. The pack is a weighted blend, so disagreement widens the range. Compare in-game before committing."),
					Game.SensDecimals,
					PointVal,
					Game.SensDecimals,
					FeelVal));
			}
		}
	}
	if (Setup.bCaptureIsRaw)
	{
		Rec.InputNotes.Add(
			TEXT("This Unreal test captures mouse with smoothing off and axis sensitivity 1.0, matching CS2 raw input. Windows Enhance pointer precision is not part of the quote."));
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
	Rec.bUsesRawInput = Game.bUsesRawInput;
	return Rec;
}
} // namespace Sens
