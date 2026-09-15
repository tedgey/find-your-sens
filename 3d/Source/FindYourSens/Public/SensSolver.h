#pragma once

#include "SensTypes.h"

/** Quoted pack blends Felt 90° with flick, casual, and micro world-angle stages. */
namespace Sens
{
TOptional<double> ImpliedSensitivity(double YawDeg, double PitchDeg, double MouseDx, double MouseDy, double YawConstant);
/** Unreal Mouse2D +Y is up (CS2). Solver +Y is down (browser movementY). Negate before ImpliedSensitivity. */
double SolverMouseDyFromUnreal(double UnrealMouseDy);
TOptional<double> ImpliedSensFromFeelTurn(double FeelDeg, double MouseDx, double YawConstant);
double RoundWeight(double Straightness, double LateCorrectionRatio);
TOptional<FRoundEstimate> EstimateRound(const FRoundRecording& Round, double YawConstant);
FStageResult SummarizeStage(EScenarioId Scenario, const TArray<FRoundRecording>& Rounds, double YawConstant);
double ScaleSensForDpi(double Sens, double FromDpi, double ToDpi);
double CmPer360(double Sens, double Dpi, double YawConstant);
int32 PickDpiTier(double UserDpi);
/** Share of the quoted pack before confidence scaling. Scenarios together outweigh Felt 90°. */
double ScenarioBlendPrior(EScenarioId Scenario);
FRecommendation RecommendFromRounds(const FSessionSetup& Setup, const TArray<FRoundRecording>& Rounds);
}
