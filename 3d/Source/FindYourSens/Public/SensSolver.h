#pragma once

#include "SensTypes.h"

/** Port of 2d/src/math/solver.ts. Quoted pack is Felt 90° only. */
namespace Sens
{
TOptional<double> ImpliedSensitivity(double YawDeg, double PitchDeg, double MouseDx, double MouseDy, double YawConstant);
TOptional<double> ImpliedSensFromFeelTurn(double FeelDeg, double MouseDx, double YawConstant);
double RoundWeight(double Straightness, double LateCorrectionRatio);
TOptional<FRoundEstimate> EstimateRound(const FRoundRecording& Round, double YawConstant);
FStageResult SummarizeStage(EScenarioId Scenario, const TArray<FRoundRecording>& Rounds, double YawConstant);
double ScaleSensForDpi(double Sens, double FromDpi, double ToDpi);
double CmPer360(double Sens, double Dpi, double YawConstant);
int32 PickDpiTier(double UserDpi);
double EppStageWeight(EScenarioId Scenario, bool bEppOn);
FRecommendation RecommendFromRounds(const FSessionSetup& Setup, const TArray<FRoundRecording>& Rounds);
}
