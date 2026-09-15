#include "SensUI.h"
#include "SensGames.h"
#include "SensPlayerController.h"
#include "SensSession.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SButton.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Layout/Visibility.h"

namespace
{
const FLinearColor Ink(0.91f, 0.933f, 0.96f);
const FLinearColor Muted(0.545f, 0.596f, 0.659f);
const FLinearColor Accent(0.722f, 1.f, 0.235f);
const FLinearColor Surface(0.04f, 0.055f, 0.075f, 0.94f);
const FLinearColor Line(1.f, 1.f, 1.f, 0.12f);

FSlateFontInfo Font(int32 Size, const FName Typeface = TEXT("Regular"))
{
	return FCoreStyle::GetDefaultFontStyle(Typeface, Size);
}

TSharedRef<STextBlock> Label(const FString& Text, const FLinearColor& Color, int32 Size, const FName Typeface = TEXT("Regular"))
{
	return SNew(STextBlock)
		.Text(FText::FromString(Text))
		.ColorAndOpacity(Color)
		.Font(Font(Size, Typeface))
		.AutoWrapText(true);
}

FButtonStyle MakeBtn(bool bGhost)
{
	static FButtonStyle Primary;
	static FButtonStyle Ghost;
	static bool bInit = false;
	if (!bInit)
	{
		auto Fill = [](const FLinearColor& Color) {
			FSlateBrush Brush;
			Brush.DrawAs = ESlateBrushDrawType::Box;
			Brush.TintColor = FSlateColor(Color);
			return Brush;
		};
		Primary = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
		Primary.SetNormal(Fill(Accent));
		Primary.SetHovered(Fill(FLinearColor(0.8f, 1.f, 0.4f)));
		Primary.SetPressed(Fill(Accent));
		Ghost = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
		Ghost.SetNormal(Fill(FLinearColor(0.f, 0.f, 0.f, 0.2f)));
		Ghost.SetHovered(Fill(FLinearColor(1.f, 1.f, 1.f, 0.08f)));
		Ghost.SetPressed(Fill(FLinearColor(0.f, 0.f, 0.f, 0.2f)));
		bInit = true;
	}
	return bGhost ? Ghost : Primary;
}

TSharedRef<SWidget> Btn(const FString& Text, bool bGhost, FOnClicked OnClicked)
{
	const FLinearColor TextColor = bGhost ? Ink : FLinearColor(0.04f, 0.06f, 0.03f);
	static const FButtonStyle Primary = MakeBtn(false);
	static const FButtonStyle Ghost = MakeBtn(true);
	return SNew(SButton)
		.ButtonStyle(bGhost ? &Ghost : &Primary)
		.OnClicked(OnClicked)
		.ContentPadding(FMargin(16.f, 10.f))
		[
			SNew(STextBlock)
				.Text(FText::FromString(Text))
				.Font(Font(13, TEXT("Bold")))
				.ColorAndOpacity(TextColor)
		];
}

TSharedRef<SWidget> Field(const FString& Caption, TSharedRef<SWidget> Input)
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[Label(Caption, Muted, 12)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 0)[Input];
}

FReply Click(ASensPlayerController* PC, void (ASensPlayerController::*Method)())
{
	if (PC)
	{
		(PC->*Method)();
	}
	return FReply::Handled();
}
} // namespace

class SSensSetup : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSensSetup) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, ASensPlayerController* InPC)
	{
		PC = InPC;
		USensSession* S = PC.IsValid() ? PC->GetSession() : nullptr;
		if (S)
		{
			DpiText = FText::AsNumber(static_cast<int32>(S->Setup.Dpi));
			ResWText = FText::AsNumber(S->Setup.ResolutionWidth);
			ResHText = FText::AsNumber(S->Setup.ResolutionHeight);
			if (S->Setup.FovSetting.IsSet())
			{
				FovText = FText::AsNumber(S->Setup.FovSetting.GetValue());
			}
		}

		ChildSlot
		[
			SNew(SBorder)
			.Padding(32.f)
			.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
			.BorderBackgroundColor(Surface)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.9f).VAlign(VAlign_Center).Padding(0, 0, 40, 0)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()[Label(TEXT("PROTOTYPE · CS2 FIRST"), Accent, 11, TEXT("Bold"))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)[Label(TEXT("Find Your Sens"), Ink, 32, TEXT("Bold"))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 12, 0, 0)
					[
						Label(TEXT("We map what your hand thinks a turn or aim move should feel like, then quote a CS2 settings pack from Felt 90°, flick, casual, and micro. The camera stays still while you move; a 90° demo is the only time the view yaws."), Muted, 15)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 16, 0, 0)
					[
						Btn(TEXT("How it works"), true, FOnClicked::CreateLambda([this]() {
							return Click(PC.Get(), &ASensPlayerController::HandleOpenInfo);
						}))
					]
				]
				+ SHorizontalBox::Slot().FillWidth(1.1f).VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()[Field(TEXT("Game"), Label(TEXT("Counter-Strike 2"), Ink, 14))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
					[
						Field(TEXT("Mouse DPI"), SNew(SEditableTextBox).Text(DpiText).OnTextChanged_Lambda([this](const FText& T) { DpiText = T; }))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0, 0, 8, 0)
						[Field(TEXT("Resolution width"), SNew(SEditableTextBox).Text(ResWText).OnTextChanged_Lambda([this](const FText& T) { ResWText = T; }))]
						+ SHorizontalBox::Slot().FillWidth(1.f)
						[Field(TEXT("Resolution height"), SNew(SEditableTextBox).Text(ResHText).OnTextChanged_Lambda([this](const FText& T) { ResHText = T; }))]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
					[
						Field(TEXT("FOV (optional, CS2 default 90)"), SNew(SEditableTextBox).Text(FovText).HintText(FText::FromString(TEXT("90"))).OnTextChanged_Lambda([this](const FText& T) { FovText = T; }))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 18, 0, 0)
					[
						Btn(TEXT("Continue"), false, FOnClicked::CreateLambda([this]() {
							if (USensSession* S = PC.IsValid() ? PC->GetSession() : nullptr)
							{
								S->Setup.Dpi = FCString::Atod(*DpiText.ToString());
								if (S->Setup.Dpi < 100.0)
								{
									S->Setup.Dpi = 400.0;
								}
								S->Setup.ResolutionWidth = FMath::Max(800, FCString::Atoi(*ResWText.ToString()));
								S->Setup.ResolutionHeight = FMath::Max(600, FCString::Atoi(*ResHText.ToString()));
								S->Setup.CurrentSens.Reset();
								const FString FovRaw = FovText.ToString().TrimStartAndEnd();
								S->Setup.FovSetting.Reset();
								if (!FovRaw.IsEmpty())
								{
									S->Setup.FovSetting = FCString::Atod(*FovRaw);
								}
								S->Setup.GameId = Sens::EGameId::CS2;
								S->Setup.bCaptureIsRaw = true;
							}
							return Click(PC.Get(), &ASensPlayerController::HandleSetupContinue);
						}))
					]
				]
			]
		];
	}

private:
	TWeakObjectPtr<ASensPlayerController> PC;
	FText DpiText;
	FText ResWText;
	FText ResHText;
	FText FovText;
};

TSharedRef<SWidget> MakeSensSetupWidget(ASensPlayerController* PC)
{
	return SNew(SSensSetup, PC);
}

TSharedRef<SWidget> MakeSensInfoWidget(ASensPlayerController* PC)
{
	TWeakObjectPtr<ASensPlayerController> WeakPC(PC);
	return SNew(SBorder).Padding(28.f).BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox")).BorderBackgroundColor(Surface)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()[Label(TEXT("ABOUT THIS TOOL"), Accent, 11, TEXT("Bold"))]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)[Label(TEXT("How Find Your Sens works"), Ink, 26, TEXT("Bold"))]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
				[Label(TEXT("A blind, open-loop test that turns how your hand wants to move into a DPI + in-game sensitivity pack. Not an aim trainer. Not a pro-sens converter."), Muted, 14)]
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top)
			[Btn(TEXT("Back"), true, FOnClicked::CreateLambda([WeakPC]() { return Click(WeakPC.Get(), &ASensPlayerController::HandleBackToSetup); }))]
		]
		+ SVerticalBox::Slot().FillHeight(1.f).Padding(0, 16, 0, 0)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot().Padding(0, 0, 0, 12)
			[Label(TEXT("Purpose"), Accent, 13, TEXT("Bold"))]
			+ SScrollBox::Slot()
			[Label(TEXT("When you intend a turn or a snap, how far does your hand actually move? That travel is the source of truth. During the blind finder the camera stays still on purpose so live yaw cannot train you toward a geometric target. A scripted 90° demo is the only time the view yaws in that loop, and it does not use your sensitivity."), Muted, 14)]
			+ SScrollBox::Slot().Padding(0, 16, 0, 8)
			[Label(TEXT("What the test includes"), Accent, 13, TEXT("Bold"))]
			+ SScrollBox::Slot()
			[Label(TEXT("Felt 90° (5): watch a real 90° demo, then copy that mouse travel with a still camera. Inches and cm show pad distance. Flick / Casual / Micro (10 each): world markers at different turn sizes. All four stages feed the settings pack. The view still does not follow the mouse."), Muted, 14)]
			+ SScrollBox::Slot().Padding(0, 16, 0, 8)
			[Label(TEXT("How the pack is built"), Accent, 13, TEXT("Bold"))]
			+ SScrollBox::Slot()
			[Label(TEXT("Each round maps intended degrees to mouse counts with CS2 m_yaw = 0.022 (sens ≈ degrees / (|counts| × m_yaw)). Felt 90° is one stage, not the whole quote. Flick, casual, and micro together carry about 60% of the pack center (22% / 22% / 16%); Felt 90° carries about 40%. Messy stages lose influence through confidence. Disagreement widens the range."), Muted, 14)]
			+ SScrollBox::Slot().Padding(0, 16, 0, 8)
			[Label(TEXT("Raw input"), Accent, 13, TEXT("Bold"))]
			+ SScrollBox::Slot()
			[Label(TEXT("CS2 uses raw mouse. This Unreal test does too (smoothing off, axis sensitivity 1.0)."), Muted, 14)]
			+ SScrollBox::Slot().Padding(0, 16, 0, 8)
			[Label(TEXT("After the pack"), Accent, 13, TEXT("Bold"))]
			+ SScrollBox::Slot()
			[Label(TEXT("The Unreal prototype can then run the same stages with live look at a sensitivity you pick (default: pack center). That check-in scores how close you got to 90° and whether markers were on-target, close, or a miss. It does not rewrite the pack, and it is not aim training."), Muted, 14)]
			+ SScrollBox::Slot().Padding(0, 16, 0, 8)
			[Label(TEXT("What this is not"), Accent, 13, TEXT("Bold"))]
			+ SScrollBox::Slot()
			[Label(TEXT("Not aim training. Not someone else's sens. Treat a large gap vs your current sens as experimental."), Muted, 14)]
		]
	];
}

class SSensPrep : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSensPrep) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, ASensPlayerController* InPC)
	{
		PC = InPC;
		USensSession* S = PC.IsValid() ? PC->GetSession() : nullptr;
		const Sens::FGameProfile& Game = Sens::GetGame(S ? S->Setup.GameId : Sens::EGameId::CS2);
		const int32 Total = S ? S->TotalRounds() : 35;
		const FString Lede = FString::Printf(
			TEXT("You're testing for %s at %.0f DPI · %d blind rounds · no feedback until the end."),
			*Game.Name,
			S ? S->Setup.Dpi : 400.0,
			Total);

		ChildSlot
		[
			SNew(SBorder).Padding(28.f).BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox")).BorderBackgroundColor(Surface)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()[Label(TEXT("BEFORE YOU START"), Accent, 11, TEXT("Bold"))]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)[Label(TEXT("Input prep"), Ink, 26, TEXT("Bold"))]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 16)[Label(Lede, Muted, 14)]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0, 0, 10, 0)
					[
						SNew(SBorder).Padding(14.f).BorderBackgroundColor(FLinearColor(1, 1, 1, 0.04f)).BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()[Label(TEXT("Checklist"), Accent, 13, TEXT("Bold"))]
							+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
							[Label(TEXT("1. Windows pointer speed at the middle notch (6/11).\n2. Same mousepad posture you use in-game.\n3. Fullscreen when you can (Alt+Enter).\n4. Click to commit each round. Don't \"fix\" after you stop."), Ink, 13)]
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0, 0, 10, 0)
					[
						SNew(SBorder).Padding(14.f).BorderBackgroundColor(FLinearColor(1, 1, 1, 0.04f)).BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()[Label(TEXT("What is Felt 90°?"), Accent, 13, TEXT("Bold"))]
							+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
							[Label(TEXT("Watch a short demo of a real 90° turn, then copy that mouse travel. The camera stays still while you replicate. You'll see inches and cm for horizontal pad travel. Stop when it feels like you've turned 90°. That stage is about 40% of the quoted pack, not the whole number."), Ink, 13)]
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						SNew(SBorder).Padding(14.f).BorderBackgroundColor(FLinearColor(1, 1, 1, 0.04f)).BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()[Label(TEXT("Scenarios and raw input"), Accent, 13, TEXT("Bold"))]
							+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
							[Label(TEXT("Flick, casual, and micro are world-angle stages. Together they carry about 60% of the pack center. CS2 uses raw mouse, and so does this test, so Windows Enhance pointer precision is not part of the quote."), Ink, 13)]
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 20, 0, 0)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 10, 0)
					[Btn(TEXT("Back"), true, FOnClicked::CreateLambda([this]() { return Click(PC.Get(), &ASensPlayerController::HandleBackToSetup); }))]
					+ SHorizontalBox::Slot().AutoWidth()
					[Btn(TEXT("Start test"), false, FOnClicked::CreateLambda([this]() { return Click(PC.Get(), &ASensPlayerController::HandleStartTest); }))]
				]
			]
		];
	}

private:
	TWeakObjectPtr<ASensPlayerController> PC;
};

TSharedRef<SWidget> MakeSensPrepWidget(ASensPlayerController* PC)
{
	return SNew(SSensPrep, PC);
}

TSharedRef<SWidget> MakeSensTestHudWidget(ASensPlayerController* PC)
{
	USensSession* S = PC ? PC->GetSession() : nullptr;
	const Sens::EScenarioId Scenario = S ? S->CurrentScenario() : Sens::EScenarioId::Feel90;
	const bool bCheckIn = S && S->Step == Sens::EAppStep::CheckInTest;
	const FString Title = bCheckIn
		? FString::Printf(TEXT("CHECK-IN · %s"), *Sens::ScenarioLabel(Scenario))
		: Sens::ScenarioLabel(Scenario);
	const FString RoundLine = S
		? FString::Printf(TEXT("Round %d / %d"), S->RoundIndex + 1, S->RoundsThisScenario())
		: TEXT("");
	const FString Hint = PC ? PC->GetScenarioHint() : TEXT("");
	const FString Directive = PC ? PC->GetHudLine() : TEXT("");
	const FString Travel = PC ? PC->GetTravelLine() : TEXT("");
	const FString Ref = PC ? PC->GetReferenceTravelLine() : TEXT("");

	TSharedRef<SVerticalBox> Root = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()[Label(Title.ToUpper(), Accent, 14, TEXT("Bold"))]
				+ SVerticalBox::Slot().AutoHeight()[Label(RoundLine, Muted, 12)]
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[Label(Hint, Muted, 13)]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
		[Label(Directive, Accent, 16, TEXT("Bold"))];

	if (PC && PC->ShowTravel())
	{
		Root->AddSlot().AutoHeight().Padding(0, 16, 0, 0)[Label(Travel, Accent, 22, TEXT("Bold"))];
		Root->AddSlot().AutoHeight()[Label(Ref, Muted, 12)];
	}

	return SNew(SBox)
		.Visibility(EVisibility::HitTestInvisible)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Top)
		.Padding(FMargin(24.f, 16.f))
	[
		SNew(SBorder).Padding(16.f).BorderBackgroundColor(FLinearColor(0.02f, 0.03f, 0.04f, 0.55f)).BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		[Root]
	];
}

TSharedRef<SWidget> MakeSensResultsWidget(ASensPlayerController* PC)
{
	TWeakObjectPtr<ASensPlayerController> WeakPC(PC);
	USensSession* S = PC ? PC->GetSession() : nullptr;
	if (!S || !S->Recommendation.IsSet())
	{
		return Label(TEXT("No results"), Muted, 14);
	}
	const Sens::FRecommendation& Rec = S->Recommendation.GetValue();
	const Sens::FGameProfile& Game = Sens::GetGame(Rec.GameId);
	const FString Band = FString::Printf(TEXT("%.3f - %.3f"), Rec.SensRange.Low, Rec.SensRange.High);
	const FString Center = FString::Printf(TEXT("center %.3f"), Rec.SensRange.Center);
	const FString Conf = FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Rec.Confidence * 100.0));
	const FString Cm = FString::Printf(TEXT("%.1f cm / 360°"), Rec.CmPer360);

	TSharedRef<SVerticalBox> Stages = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[Label(TEXT("Per scenario"), Accent, 13, TEXT("Bold"))];
	for (const Sens::FStageResult& Stage : Rec.StageResults)
	{
		const FString Tag = FString::Printf(TEXT("pack %d%%"), FMath::RoundToInt(Stage.BlendWeight * 100.0));
		const FString StageLine = FString::Printf(
			TEXT("%s [%s]   med %.3f  ·  σ %.3f  ·  %d%%"),
			*Sens::ScenarioLabel(Stage.Scenario),
			*Tag,
			Stage.MedianSens,
			Stage.StdDev,
			FMath::RoundToInt(Stage.Confidence * 100.0));
		Stages->AddSlot().AutoHeight().Padding(0, 6, 0, 0)[Label(StageLine, Ink, 13)];
	}

	TSharedRef<SVerticalBox> Notes = SNew(SVerticalBox);
	for (const FString& Note : Rec.InputNotes)
	{
		Notes->AddSlot().AutoHeight().Padding(0, 0, 0, 8)[Label(Note, Ink, 13)];
	}

	FString Compare;
	if (Rec.CurrentSens.IsSet())
	{
		Compare = FString::Printf(TEXT("You run %.3f"), Rec.CurrentSens.GetValue());
		if (Rec.VsCurrentRatio.IsSet())
		{
			Compare += FString::Printf(TEXT("  ·  measured is %.2fx that"), Rec.VsCurrentRatio.GetValue());
		}
	}

	const FString Assume = FString::Printf(
		TEXT("Assumed FOV setting %.0f (≈ %.1f° horizontal on the %.0fx%.0f arena). Game res %dx%d. eDPI %.1f-%.1f. Raw mouse capture (CS2-style)."),
		Rec.FovSetting,
		Rec.HorizontalFovDeg,
		Rec.ArenaWidth,
		Rec.ArenaHeight,
		Rec.GameResolutionWidth,
		Rec.GameResolutionHeight,
		Rec.EDpiRange.Low,
		Rec.EDpiRange.High);

	return SNew(SBorder).Padding(28.f).BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox")).BorderBackgroundColor(Surface)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()[Label(TEXT("YOUR SETTINGS PACK"), Accent, 11, TEXT("Bold"))]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)[Label(Game.Name, Ink, 26, TEXT("Bold"))]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)
				[Label(TEXT("Quoted from all four stages. Flick, casual, and micro carry more of the center than Felt 90° alone. Capture is raw, like CS2."), Muted, 14)]
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
				[Btn(TEXT("New setup"), true, FOnClicked::CreateLambda([WeakPC]() { return Click(WeakPC.Get(), &ASensPlayerController::HandleBackToSetup); }))]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
				[Btn(TEXT("Retest"), true, FOnClicked::CreateLambda([WeakPC]() { return Click(WeakPC.Get(), &ASensPlayerController::HandleRetest); }))]
				+ SHorizontalBox::Slot().AutoWidth()
				[Btn(TEXT("Test your sensitivity"), false, FOnClicked::CreateLambda([WeakPC]() { return Click(WeakPC.Get(), &ASensPlayerController::HandleOpenCheckIn); }))]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 16, 0, 0)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f)[Label(FString::Printf(TEXT("DPI\n%.0f"), Rec.Dpi), Ink, 18, TEXT("Bold"))]
			+ SHorizontalBox::Slot().FillWidth(1.2f)[Label(Game.SensitivityLabel + TEXT("\n") + Band + TEXT("\n") + Center, Ink, 16, TEXT("Bold"))]
			+ SHorizontalBox::Slot().FillWidth(1.f)[Label(TEXT("Confidence\n") + Conf + TEXT("\n") + Cm, Ink, 16, TEXT("Bold"))]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 12, 0, 0)[Label(Compare, Ink, 14)]
		+ SVerticalBox::Slot().FillHeight(1.f).Padding(0, 16, 0, 0)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()[Stages]
			+ SScrollBox::Slot().Padding(0, 16, 0, 0)[Label(Assume, Muted, 12)]
			+ SScrollBox::Slot().Padding(0, 12, 0, 0)[Notes]
		]
	];
}

class SSensCheckInSelect : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSensCheckInSelect) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, ASensPlayerController* InPC)
	{
		PC = InPC;
		USensSession* S = PC.IsValid() ? PC->GetSession() : nullptr;
		if (S && S->Recommendation.IsSet())
		{
			const Sens::FSensRange& Range = S->Recommendation->SensRange;
			Low = Range.Low;
			High = Range.High;
			if (High <= Low)
			{
				High = Low + 0.001;
			}
			if (S->CheckInSens <= 0.0)
			{
				S->CheckInSens = Range.Center;
			}
			SensValue = S->CheckInSens;
		}
		SensText = FText::FromString(FString::Printf(TEXT("%.3f"), SensValue));

		const Sens::FGameProfile& Game = Sens::GetGame(S ? S->Setup.GameId : Sens::EGameId::CS2);
		const FString Band = FString::Printf(TEXT("%.3f - %.3f"), Low, High);
		const FString Lede = FString::Printf(
			TEXT("Your pack quoted %s (center %.3f). The slider stays in that band. Type a number to try something outside it. Live look uses this sensitivity; the pack does not change."),
			*Band,
			S && S->Recommendation.IsSet() ? S->Recommendation->SensRange.Center : SensValue);

		ChildSlot
		[
			SNew(SBorder).Padding(28.f).BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox")).BorderBackgroundColor(Surface)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()[Label(TEXT("TEST YOUR SENSITIVITY"), Accent, 11, TEXT("Bold"))]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)[Label(Game.Name, Ink, 26, TEXT("Bold"))]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 16)[Label(Lede, Muted, 14)]
				+ SVerticalBox::Slot().AutoHeight()
				[Label(Game.SensitivityLabel, Muted, 12)]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(0, 8, 0, 0)
				[
					SNew(SBox).WidthOverride(240.f).MinDesiredHeight(22.f)
					[
						SNew(SSlider)
						.Value_Lambda([this]() { return GetSliderValue(); })
						.OnValueChanged_Lambda([this](float Value) { OnSliderChanged(Value); })
					]
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(0, 12, 0, 0)
				[
					SNew(SBox).WidthOverride(240.f)
					[
						SNew(SEditableTextBox)
						.Text_Lambda([this]() { return SensText; })
						.OnTextChanged(this, &SSensCheckInSelect::OnSensTextChanged)
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 24, 0, 0)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 10, 0)
					[Btn(TEXT("Back to pack"), true, FOnClicked::CreateLambda([this]() {
						return Click(PC.Get(), &ASensPlayerController::HandleBackToPackResults);
					}))]
					+ SHorizontalBox::Slot().AutoWidth()
					[Btn(TEXT("Start check-in"), false, FOnClicked::CreateLambda([this]() {
						CommitSensToSession();
						return Click(PC.Get(), &ASensPlayerController::HandleStartCheckInTest);
					}))]
				]
			]
		];
	}

private:
	float GetSliderValue() const
	{
		const double Span = High - Low;
		if (Span <= 0.0)
		{
			return 0.5f;
		}
		return static_cast<float>(FMath::Clamp((SensValue - Low) / Span, 0.0, 1.0));
	}

	void OnSliderChanged(float Value)
	{
		SensValue = Low + static_cast<double>(Value) * (High - Low);
		SensText = FText::FromString(FString::Printf(TEXT("%.3f"), SensValue));
		CommitSensToSession();
	}

	void OnSensTextChanged(const FText& Text)
	{
		SensText = Text;
		const double Parsed = FCString::Atod(*Text.ToString());
		if (Parsed > 0.0)
		{
			SensValue = Parsed;
			CommitSensToSession();
		}
	}

	void CommitSensToSession()
	{
		if (USensSession* S = PC.IsValid() ? PC->GetSession() : nullptr)
		{
			if (SensValue > 0.0)
			{
				S->CheckInSens = SensValue;
			}
		}
	}

	TWeakObjectPtr<ASensPlayerController> PC;
	double Low = 0.5;
	double High = 2.0;
	double SensValue = 1.0;
	FText SensText;
};

TSharedRef<SWidget> MakeSensCheckInSelectWidget(ASensPlayerController* PC)
{
	return SNew(SSensCheckInSelect, PC);
}

TSharedRef<SWidget> MakeSensCheckInResultsWidget(ASensPlayerController* PC)
{
	TWeakObjectPtr<ASensPlayerController> WeakPC(PC);
	USensSession* S = PC ? PC->GetSession() : nullptr;
	if (!S || !S->CheckInReport.IsSet())
	{
		return Label(TEXT("No check-in report"), Muted, 14);
	}
	const Sens::FCheckInReport& Report = S->CheckInReport.GetValue();
	const Sens::FGameProfile& Game = Sens::GetGame(S->Setup.GameId);
	const FString SensLine = FString::Printf(TEXT("tested at %.3f"), Report.ChosenSens);
	const FString PackNote = S->Recommendation.IsSet()
		? FString::Printf(TEXT("Pack still quotes %.3f - %.3f (center %.3f). This report does not change it."),
			S->Recommendation->SensRange.Low,
			S->Recommendation->SensRange.High,
			S->Recommendation->SensRange.Center)
		: TEXT("This report does not change your pack.");

	FString FeelHeadline = TEXT("Felt 90° not run");
	FString HitHeadline = TEXT("No marker stages");
	for (const Sens::FCheckInStageSummary& Stage : Report.Stages)
	{
		if (Stage.Scenario == Sens::EScenarioId::Feel90)
		{
			FeelHeadline = FString::Printf(TEXT("median %.1f° off 90°"), Stage.MedianAbsErrorDeg);
		}
	}
	int32 OnTarget = 0;
	int32 Close = 0;
	int32 Miss = 0;
	int32 MarkerRounds = 0;
	for (const Sens::FCheckInStageSummary& Stage : Report.Stages)
	{
		if (Stage.Scenario == Sens::EScenarioId::Feel90)
		{
			continue;
		}
		OnTarget += Stage.OnTarget;
		Close += Stage.Close;
		Miss += Stage.Miss;
		MarkerRounds += Stage.Rounds;
	}
	if (MarkerRounds > 0)
	{
		HitHeadline = FString::Printf(TEXT("%d on-target · %d close · %d miss"), OnTarget, Close, Miss);
	}

	TSharedRef<SVerticalBox> Stages = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[Label(TEXT("Per scenario"), Accent, 13, TEXT("Bold"))];
	for (const Sens::FCheckInStageSummary& Stage : Report.Stages)
	{
		const FString StageLine = FString::Printf(
			TEXT("%s   %d on-target  ·  %d close  ·  %d miss  ·  overshoot %d / undershoot %d  ·  med %.1f°"),
			*Sens::ScenarioLabel(Stage.Scenario),
			Stage.OnTarget,
			Stage.Close,
			Stage.Miss,
			Stage.Overshoot,
			Stage.Undershoot,
			Stage.MedianAbsErrorDeg);
		Stages->AddSlot().AutoHeight().Padding(0, 6, 0, 0)[Label(StageLine, Ink, 13)];
	}

	TSharedRef<SVerticalBox> Notes = SNew(SVerticalBox);
	for (const FString& Note : Report.Notes)
	{
		Notes->AddSlot().AutoHeight().Padding(0, 0, 0, 8)[Label(Note, Ink, 13)];
	}

	return SNew(SBorder).Padding(28.f).BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox")).BorderBackgroundColor(Surface)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()[Label(TEXT("CHECK-IN"), Accent, 11, TEXT("Bold"))]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)[Label(Game.Name, Ink, 26, TEXT("Bold"))]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)[Label(PackNote, Muted, 14)]
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
				[Btn(TEXT("Retry finding your sens"), true, FOnClicked::CreateLambda([WeakPC]() { return Click(WeakPC.Get(), &ASensPlayerController::HandleBackToSetup); }))]
				+ SHorizontalBox::Slot().AutoWidth()
				[Btn(TEXT("Change sens and go again"), false, FOnClicked::CreateLambda([WeakPC]() { return Click(WeakPC.Get(), &ASensPlayerController::HandleRestartCheckInSelect); }))]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 16, 0, 0)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f)[Label(Game.SensitivityLabel + TEXT("\n") + SensLine, Ink, 18, TEXT("Bold"))]
			+ SHorizontalBox::Slot().FillWidth(1.2f)[Label(TEXT("Felt 90°\n") + FeelHeadline, Ink, 16, TEXT("Bold"))]
			+ SHorizontalBox::Slot().FillWidth(1.2f)[Label(TEXT("Markers\n") + HitHeadline, Ink, 16, TEXT("Bold"))]
		]
		+ SVerticalBox::Slot().FillHeight(1.f).Padding(0, 16, 0, 0)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()[Stages]
			+ SScrollBox::Slot().Padding(0, 16, 0, 0)[Notes]
		]
	];
}
