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
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

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
		USensSession* S = PC ? PC->GetSession() : nullptr;
		if (S)
		{
			DpiText = FText::AsNumber(static_cast<int32>(S->Setup.Dpi));
			ResWText = FText::AsNumber(S->Setup.ResolutionWidth);
			ResHText = FText::AsNumber(S->Setup.ResolutionHeight);
			if (S->Setup.CurrentSens.IsSet())
			{
				SensText = FText::AsNumber(S->Setup.CurrentSens.GetValue());
			}
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
						Label(TEXT("We map what your hand thinks a turn should feel like, then quote a CS2 settings pack. The camera stays still while you move; a 90° demo is the only time the view yaws."), Muted, 15)
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
						Field(TEXT("Current sensitivity (optional)"), SNew(SEditableTextBox).Text(SensText).HintText(FText::FromString(TEXT("e.g. 1.13"))).OnTextChanged_Lambda([this](const FText& T) { SensText = T; }))
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
								const FString SensRaw = SensText.ToString().TrimStartAndEnd();
								S->Setup.CurrentSens.Reset();
								if (!SensRaw.IsEmpty())
								{
									S->Setup.CurrentSens = FCString::Atod(*SensRaw);
								}
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
	FText SensText;
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
			[Label(TEXT("When you intend a turn, how far does your hand actually move? That travel is the source of truth. The camera stays still on purpose so live yaw cannot train you toward a geometric target. A scripted 90° demo is the only time the view yaws, and it does not use your sensitivity."), Muted, 14)]
			+ SScrollBox::Slot().Padding(0, 16, 0, 8)
			[Label(TEXT("What the test includes"), Accent, 13, TEXT("Bold"))]
			+ SScrollBox::Slot()
			[Label(TEXT("Felt 90° (5): watch a real 90° demo, then copy that mouse travel with a still camera. Inches and cm show pad distance. Flick / Casual / Micro (10 each) are consistency checks on world markers. The view still does not follow the mouse."), Muted, 14)]
			+ SScrollBox::Slot().Padding(0, 16, 0, 8)
			[Label(TEXT("Absolute vs consistency"), Accent, 13, TEXT("Bold"))]
			+ SScrollBox::Slot()
			[Label(TEXT("Only Felt 90° sets the quoted number: sens ≈ 90 / (|counts| × m_yaw) with CS2 m_yaw = 0.022. Pointing stages feed about 30% of confidence, not the pack center."), Muted, 14)]
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
		USensSession* S = PC ? PC->GetSession() : nullptr;
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
							[Label(TEXT("Watch a short demo of a real 90° turn, then copy that mouse travel. The camera stays still while you replicate. You'll see inches and cm for horizontal pad travel. Stop when it feels like you've turned 90°. That distance sets absolute sensitivity. Later stages only check consistency."), Ink, 13)]
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						SNew(SBorder).Padding(14.f).BorderBackgroundColor(FLinearColor(1, 1, 1, 0.04f)).BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()[Label(TEXT("Enhance pointer precision"), Accent, 13, TEXT("Bold"))]
							+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
							[Label(TEXT("CS2 ignores Windows acceleration (raw input). This Unreal test also aims for raw mouse, so EPP should not change the quoted pack. We still ask so the note on results stays honest."), Ink, 13)]
							+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
								[Btn(TEXT("Leave it on"), true, FOnClicked::CreateLambda([this]() {
									if (USensSession* Sess = PC.IsValid() ? PC->GetSession() : nullptr)
									{
										Sess->Setup.EnhancePointerPrecision = Sens::EEnhancePointerPrecision::On;
									}
									return FReply::Handled();
								}))]
								+ SHorizontalBox::Slot().AutoWidth()
								[Btn(TEXT("Off for this test"), true, FOnClicked::CreateLambda([this]() {
									if (USensSession* Sess = PC.IsValid() ? PC->GetSession() : nullptr)
									{
										Sess->Setup.EnhancePointerPrecision = Sens::EEnhancePointerPrecision::Off;
									}
									return FReply::Handled();
								}))]
							]
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
	TWeakObjectPtr<ASensPlayerController> WeakPC(PC);
	USensSession* S = PC ? PC->GetSession() : nullptr;
	const Sens::EScenarioId Scenario = S ? S->CurrentScenario() : Sens::EScenarioId::Feel90;
	const FString Title = Sens::ScenarioLabel(Scenario);
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

	if (PC && PC->IsFeelDemo())
	{
		Root->AddSlot().AutoHeight().Padding(0, 12, 0, 0)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 10, 0)
			[Btn(TEXT("Replay"), true, FOnClicked::CreateLambda([WeakPC]() { return Click(WeakPC.Get(), &ASensPlayerController::HandleReplayDemo); }))]
			+ SHorizontalBox::Slot().AutoWidth()
			[Btn(TEXT("Start replicate"), false, FOnClicked::CreateLambda([WeakPC]() { return Click(WeakPC.Get(), &ASensPlayerController::HandleStartReplicate); }))]
		];
	}

	if (PC && PC->ShowTravel())
	{
		Root->AddSlot().AutoHeight().Padding(0, 16, 0, 0)[Label(Travel, Accent, 22, TEXT("Bold"))];
		Root->AddSlot().AutoHeight()[Label(Ref, Muted, 12)];
	}

	return SNew(SBox).HAlign(HAlign_Fill).VAlign(VAlign_Top).Padding(FMargin(24.f, 16.f))
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
		const FString Tag = Stage.bIsAbsolute ? TEXT("absolute") : TEXT("consistency");
		const FString Line = FString::Printf(
			TEXT("%s [%s]   med %.3f  ·  σ %.3f  ·  %d%%"),
			*Sens::ScenarioLabel(Stage.Scenario),
			*Tag,
			Stage.MedianSens,
			Stage.StdDev,
			FMath::RoundToInt(Stage.Confidence * 100.0));
		Stages->AddSlot().AutoHeight().Padding(0, 6, 0, 0)[Label(Line, Ink, 13)];
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
		TEXT("Assumed FOV setting %.0f (≈ %.1f° horizontal on the %.0fx%.0f arena). Game res %dx%d. eDPI %.1f-%.1f. Enhance pointer precision: %s."),
		Rec.FovSetting,
		Rec.HorizontalFovDeg,
		Rec.ArenaWidth,
		Rec.ArenaHeight,
		Rec.GameResolutionWidth,
		Rec.GameResolutionHeight,
		Rec.EDpiRange.Low,
		Rec.EDpiRange.High,
		Rec.EnhancePointerPrecision == Sens::EEnhancePointerPrecision::On ? TEXT("on") : TEXT("off"));

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
				[Label(TEXT("Absolute scale from Felt 90°. Target stages are consistency checks only."), Muted, 14)]
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
				[Btn(TEXT("New setup"), true, FOnClicked::CreateLambda([WeakPC]() { return Click(WeakPC.Get(), &ASensPlayerController::HandleBackToSetup); }))]
				+ SHorizontalBox::Slot().AutoWidth()
				[Btn(TEXT("Retest"), false, FOnClicked::CreateLambda([WeakPC]() { return Click(WeakPC.Get(), &ASensPlayerController::HandleRetest); }))]
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
