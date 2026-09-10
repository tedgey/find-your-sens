#pragma once

#include "CoreMinimal.h"

/**
 * Plain C++ types ported from 2d/src/math/types.ts.
 * Not UHT types so the solver can stay a header-friendly math module.
 */
namespace Sens
{
enum class EGameId : uint8
{
	CS2,
	Valorant,
	PUBG,
	Apex,
	Fortnite
};

enum class EScenarioId : uint8
{
	Feel90,
	Flick,
	Casual,
	Micro
};

enum class EAppStep : uint8
{
	Setup,
	Info,
	Prep,
	Test,
	Results
};

enum class EFovMode : uint8
{
	Source43,
	Horizontal
};

struct FGameProfile
{
	EGameId Id = EGameId::CS2;
	FString Name;
	double YawConstant = 0.022;
	double DefaultFovSetting = 90.0;
	EFovMode FovMode = EFovMode::Source43;
	int32 SensDecimals = 3;
	FString SensitivityLabel;
	bool bUsesRawInput = true;
};

struct FSessionSetup
{
	EGameId GameId = EGameId::CS2;
	double Dpi = 400.0;
	TOptional<double> FovSetting;
	int32 ResolutionWidth = 1920;
	int32 ResolutionHeight = 1080;
	TOptional<double> CurrentSens;
	/** Unreal capture uses raw mouse, matching CS2. */
	bool bCaptureIsRaw = true;
};

struct FDisplayConfig
{
	double Width = 1920.0;
	double Height = 1080.0;
	double HorizontalFovDeg = 90.0;
};

struct FPoint2
{
	double X = 0.0;
	double Y = 0.0;
};

struct FPathSample
{
	double T = 0.0;
	double Dx = 0.0;
	double Dy = 0.0;
};

struct FTargetSpec
{
	FPoint2 OffsetPx;
	double YawDeg = 0.0;
	double PitchDeg = 0.0;
};

struct FRoundRecording
{
	EScenarioId Scenario = EScenarioId::Feel90;
	int32 RoundIndex = 0;
	TOptional<FTargetSpec> Target;
	TArray<FPathSample> Samples;
	FPoint2 Net;
	double DurationMs = 0.0;
	double ArenaWidth = 1920.0;
	double ArenaHeight = 1080.0;
};

struct FPathMetrics
{
	double PathLength = 0.0;
	double StraightLineLength = 0.0;
	double Straightness = 0.0;
	double NetDistance = 0.0;
	int32 SampleCount = 0;
	double LateCorrectionRatio = 0.0;
};

struct FRoundEstimate
{
	FRoundRecording Round;
	FPathMetrics Metrics;
	double ImpliedSens = 0.0;
	double Weight = 0.0;
};

struct FStageResult
{
	EScenarioId Scenario = EScenarioId::Feel90;
	TArray<FRoundEstimate> Estimates;
	double MedianSens = 0.0;
	double MeanSens = 0.0;
	double StdDev = 0.0;
	double Confidence = 0.0;
	/** Normalized share of the quoted pack (priors x stage confidence). */
	double BlendWeight = 0.0;
	bool bIsAbsolute = false;
};

struct FSensRange
{
	double Low = 0.0;
	double High = 0.0;
	double Center = 0.0;
};

struct FRecommendation
{
	EGameId GameId = EGameId::CS2;
	double Dpi = 400.0;
	int32 NearestDpiTier = 400;
	TOptional<FSensRange> SensRangeAtNearestTier;
	FSensRange SensRange;
	double Confidence = 0.0;
	double FromStages = 0.0;
	double CmPer360 = 0.0;
	FSensRange EDpiRange;
	TArray<FStageResult> StageResults;
	TOptional<double> PointingBaselineSens;
	TOptional<double> CurrentSens;
	TOptional<double> VsCurrentRatio;
	double FovSetting = 90.0;
	double HorizontalFovDeg = 90.0;
	double ArenaWidth = 1920.0;
	double ArenaHeight = 1080.0;
	int32 GameResolutionWidth = 1920;
	int32 GameResolutionHeight = 1080;
	double YawConstant = 0.022;
	bool bUsesRawInput = true;
	TArray<FString> InputNotes;
};

struct FScenarioPlan
{
	EScenarioId Id = EScenarioId::Feel90;
	int32 Rounds = 5;
};

inline FString ScenarioLabel(EScenarioId Id)
{
	switch (Id)
	{
	case EScenarioId::Feel90:
		return TEXT("Felt 90°");
	case EScenarioId::Flick:
		return TEXT("Flick");
	case EScenarioId::Casual:
		return TEXT("Casual");
	case EScenarioId::Micro:
		return TEXT("Micro");
	}
	return TEXT("Unknown");
}

constexpr double FeelTurnDeg = 90.0;
} // namespace Sens
