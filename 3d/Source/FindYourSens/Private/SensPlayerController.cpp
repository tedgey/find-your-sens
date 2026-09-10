#include "SensPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "SensAngles.h"
#include "SensGames.h"
#include "SensHUD.h"
#include "SensPawn.h"
#include "SensSession.h"
#include "SensSolver.h"
#include "SensTargetActor.h"
#include "SensTargets.h"
#include "SensUI.h"
#include "Widgets/SWeakWidget.h"

namespace
{
constexpr float DemoDuration = 1.6f;
constexpr float TargetDistanceCm = 650.f;
}

ASensPlayerController::ASensPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	PrimaryActorTick.bCanEverTick = true;
}

void ASensPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (UEnhancedInputLocalPlayerSubsystem* Sub =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (Mapping)
		{
			Sub->AddMappingContext(Mapping, 0);
		}
	}
}

void ASensPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	ClickAction = NewObject<UInputAction>(this, TEXT("SensClick"));
	ClickAction->ValueType = EInputActionValueType::Boolean;
	MouseAction = NewObject<UInputAction>(this, TEXT("SensMouse"));
	MouseAction->ValueType = EInputActionValueType::Axis2D;
	EscapeAction = NewObject<UInputAction>(this, TEXT("SensEscape"));
	EscapeAction->ValueType = EInputActionValueType::Boolean;

	Mapping = NewObject<UInputMappingContext>(this, TEXT("SensIMC"));
	Mapping->MapKey(ClickAction, EKeys::LeftMouseButton);
	Mapping->MapKey(MouseAction, EKeys::Mouse2D);
	Mapping->MapKey(EscapeAction, EKeys::Escape);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(ClickAction, ETriggerEvent::Started, this, &ASensPlayerController::OnClick);
		EIC->BindAction(MouseAction, ETriggerEvent::Triggered, this, &ASensPlayerController::OnMouse);
		EIC->BindAction(EscapeAction, ETriggerEvent::Started, this, &ASensPlayerController::OnEscape);
	}

	if (UEnhancedInputLocalPlayerSubsystem* Sub =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (Mapping)
		{
			Sub->AddMappingContext(Mapping, 0);
		}
	}
}

void ASensPlayerController::InitSession(USensSession* InSession)
{
	Session = InSession;
	if (!TargetActor && GetWorld())
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		TargetActor = GetWorld()->SpawnActor<ASensTargetActor>(Params);
	}
	if (ASensPawn* Pawn = GetSensPawn())
	{
		Pawn->SetRestRotation(FRotator(0.f, 0.f, 0.f));
		Pawn->SetActorLocation(FVector(0.f, 0.f, 96.f));
	}
	RefreshUI();
}

void ASensPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!Session || Session->Step != Sens::EAppStep::Test)
	{
		return;
	}
	if (FeelPhase == EFeelPhase::Demo && bDemoPlaying)
	{
		DemoElapsed += DeltaSeconds;
		const float T = FMath::Clamp(DemoElapsed / DemoDuration, 0.f, 1.f);
		const float Eased = 1.f - FMath::Square(1.f - T);
		if (ASensPawn* Pawn = GetSensPawn())
		{
			Pawn->SetCameraYawOffset(90.f * Eased);
		}
		if (T >= 1.f)
		{
			bDemoPlaying = false;
			RefreshUI();
		}
	}
	if (ASensHUD* HUD = GetSensHUD())
	{
		HUD->SetTravel(GetTravelLine(), GetReferenceTravelLine(), ShowTravel());
		HUD->SetArmed(bArmed);
	}
}

void ASensPlayerController::HandleOpenInfo()
{
	if (Session)
	{
		Session->OpenInfo();
		RefreshUI();
	}
}

void ASensPlayerController::HandleStartPrep()
{
	if (Session)
	{
		Session->StartPrep();
		RefreshUI();
	}
}

void ASensPlayerController::HandleSetupContinue()
{
	HandleStartPrep();
}

void ASensPlayerController::HandleStartTest()
{
	if (!Session)
	{
		return;
	}
	Session->BeginTest();
	ApplyCameraFov();
	PrepareCurrentRound();
	RefreshUI();
}

void ASensPlayerController::HandleBackToSetup()
{
	if (Session)
	{
		Session->BackToSetup();
	}
	if (ASensPawn* Pawn = GetSensPawn())
	{
		Pawn->SnapToRest();
	}
	if (TargetActor)
	{
		TargetActor->SetVisibleMarker(false);
	}
	RefreshUI();
}

void ASensPlayerController::HandleRetest()
{
	if (Session)
	{
		Session->Retest();
		RefreshUI();
	}
}

void ASensPlayerController::HandleReplayDemo()
{
	StartFeelDemo();
}

void ASensPlayerController::HandleStartReplicate()
{
	FeelPhase = EFeelPhase::Replicate;
	if (ASensPawn* Pawn = GetSensPawn())
	{
		Pawn->SnapToRest();
	}
	ResetMeasurement();
	EnterCaptureInput();
	RefreshUI();
}

FString ASensPlayerController::GetHudLine() const
{
	if (!Session)
	{
		return TEXT("");
	}
	const bool bFeel = Session->CurrentScenario() == Sens::EScenarioId::Feel90;
	if (bFeel && FeelPhase == EFeelPhase::Demo)
	{
		return bDemoPlaying
			? TEXT("Watch the turn. This is what 90° of yaw looks like.")
			: TEXT("Demo ready. Press Replay if you want another look, then Start replicate.");
	}
	if (!bCapturing)
	{
		return TEXT("Click the arena to lock the pointer and begin.");
	}
	if (!bArmed)
	{
		return bFeel
			? TEXT("Click once to arm (crosshair turns green), then sweep like that turn, then click to commit.")
			: TEXT("Click once to arm (crosshair turns green), then move, then click to commit.");
	}
	return bFeel
		? TEXT("Armed. Turn until it feels like 90°. The camera stays still; watch inches and cm on the pad.")
		: TEXT("Armed. Move as you would in-game. Click when you believe you'd be on target.");
}

FString ASensPlayerController::GetScenarioHint() const
{
	if (!Session)
	{
		return TEXT("");
	}
	switch (Session->CurrentScenario())
	{
	case Sens::EScenarioId::Feel90:
		return TEXT("First watch a real 90° turn. Then copy that same mouse travel. The camera stays still; stop when it feels like you've turned 90°.");
	case Sens::EScenarioId::Flick:
		return TEXT("Consistency check: move as if snapping onto the target in one commit, then click.");
	case Sens::EScenarioId::Casual:
		return TEXT("Consistency check: move as if smoothly placing your crosshair on the target, then click.");
	case Sens::EScenarioId::Micro:
		return TEXT("Consistency check: move as if making a small correction onto the target, then click.");
	}
	return TEXT("");
}

FString ASensPlayerController::GetTravelLine() const
{
	if (!Session || Session->Setup.Dpi <= 0.0)
	{
		return TEXT("0.00 in  ·  0.0 cm");
	}
	const double Inches = FMath::Abs(Net.X) / Session->Setup.Dpi;
	const double Cm = Inches * 2.54;
	return FString::Printf(TEXT("%.2f in  ·  %.1f cm"), Inches, Cm);
}

FString ASensPlayerController::GetReferenceTravelLine() const
{
	if (!Session || !Session->Setup.CurrentSens.IsSet() || Session->Setup.CurrentSens.GetValue() <= 0.0)
	{
		return ShowTravel() ? TEXT("horizontal pad travel") : TEXT("");
	}
	const Sens::FGameProfile& Game = Sens::GetGame(Session->Setup.GameId);
	const double Cm = Sens::CmPer360(Session->Setup.CurrentSens.GetValue(), Session->Setup.Dpi, Game.YawConstant) / 4.0;
	return FString::Printf(
		TEXT("current sens ≈ %.2f in / %.1f cm for 90°"),
		Cm / 2.54,
		Cm);
}

bool ASensPlayerController::IsFeelDemo() const
{
	return Session && Session->Step == Sens::EAppStep::Test
		&& Session->CurrentScenario() == Sens::EScenarioId::Feel90
		&& FeelPhase == EFeelPhase::Demo;
}

bool ASensPlayerController::IsFeelReplicate() const
{
	return Session && Session->Step == Sens::EAppStep::Test
		&& Session->CurrentScenario() == Sens::EScenarioId::Feel90
		&& FeelPhase == EFeelPhase::Replicate;
}

bool ASensPlayerController::ShowTravel() const
{
	return IsFeelReplicate() && bArmed;
}

void ASensPlayerController::OnClick(const FInputActionValue& Value)
{
	if (!Session || Session->Step != Sens::EAppStep::Test)
	{
		return;
	}
	if (IsFeelDemo())
	{
		return;
	}
	if (!bCapturing)
	{
		EnterCaptureInput();
		RefreshUI();
		return;
	}
	if (!bArmed)
	{
		bArmed = true;
		StartedAtMs = 0.0;
		Samples.Reset();
		Net = {};
		UpdateHudFlags();
		RefreshUI();
		return;
	}
	CommitRound();
}

void ASensPlayerController::OnMouse(const FInputActionValue& Value)
{
	if (!bCapturing || !bArmed)
	{
		return;
	}
	if (IsFeelDemo())
	{
		return;
	}
	const FVector2D Delta = Value.Get<FVector2D>();
	if (Delta.IsNearlyZero())
	{
		return;
	}
	const double Now = FPlatformTime::Seconds() * 1000.0;
	if (StartedAtMs <= 0.0)
	{
		StartedAtMs = Now;
	}
	Sens::FPathSample Sample;
	Sample.T = Now;
	Sample.Dx = Delta.X;
	Sample.Dy = Delta.Y;
	Samples.Add(Sample);
	Net.X += Delta.X;
	Net.Y += Delta.Y;
}

void ASensPlayerController::OnEscape(const FInputActionValue& Value)
{
	if (!Session || Session->Step != Sens::EAppStep::Test)
	{
		return;
	}
	bArmed = false;
	bCapturing = false;
	ResetMeasurement();
	if (ASensPawn* Pawn = GetSensPawn())
	{
		Pawn->SnapToRest();
	}
	EnterMenuInput();
	if (Session->CurrentScenario() == Sens::EScenarioId::Feel90)
	{
		FeelPhase = EFeelPhase::Demo;
		bDemoPlaying = false;
	}
	UpdateHudFlags();
	RefreshUI();
}

void ASensPlayerController::RefreshUI()
{
	ClearViewportWidget();
	if (!Session)
	{
		return;
	}
	UpdateHudFlags();
	switch (Session->Step)
	{
	case Sens::EAppStep::Setup:
		EnterMenuInput();
		ShowMenuWidget(MakeSensSetupWidget(this));
		break;
	case Sens::EAppStep::Info:
		EnterMenuInput();
		ShowMenuWidget(MakeSensInfoWidget(this));
		break;
	case Sens::EAppStep::Prep:
		EnterMenuInput();
		ShowMenuWidget(MakeSensPrepWidget(this));
		break;
	case Sens::EAppStep::Test:
		ShowMenuWidget(MakeSensTestHudWidget(this));
		if (IsFeelDemo())
		{
			EnterMenuInput();
		}
		break;
	case Sens::EAppStep::Results:
		EnterMenuInput();
		if (ASensPawn* Pawn = GetSensPawn())
		{
			Pawn->SnapToRest();
		}
		if (TargetActor)
		{
			TargetActor->SetVisibleMarker(false);
		}
		ShowMenuWidget(MakeSensResultsWidget(this));
		break;
	}
}

void ASensPlayerController::ShowMenuWidget(TSharedRef<SWidget> Widget)
{
	ClearViewportWidget();
	ViewportWidget = Widget;
	ViewportHost = SNew(SWeakWidget).PossiblyNullContent(ViewportWidget.ToSharedRef());
	if (UGameViewportClient* VP = GetWorld() ? GetWorld()->GetGameViewport() : nullptr)
	{
		VP->AddViewportWidgetContent(ViewportHost.ToSharedRef(), 100);
	}
}

void ASensPlayerController::ClearViewportWidget()
{
	if (ViewportHost.IsValid())
	{
		if (UGameViewportClient* VP = GetWorld() ? GetWorld()->GetGameViewport() : nullptr)
		{
			VP->RemoveViewportWidgetContent(ViewportHost.ToSharedRef());
		}
	}
	ViewportHost.Reset();
	ViewportWidget.Reset();
}

void ASensPlayerController::EnterMenuInput()
{
	FInputModeGameAndUI Mode;
	Mode.SetHideCursorDuringCapture(false);
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bCapturing = false;
}

void ASensPlayerController::EnterCaptureInput()
{
	FInputModeGameOnly Mode;
	Mode.SetConsumeCaptureMouseDown(true);
	SetInputMode(Mode);
	bShowMouseCursor = false;
	bCapturing = true;
}

void ASensPlayerController::ApplyCameraFov()
{
	ASensPawn* Pawn = GetSensPawn();
	if (!Pawn || !Session || !Pawn->GetCamera())
	{
		return;
	}
	const Sens::FGameProfile& Game = Sens::GetGame(Session->Setup.GameId);
	const double FovSetting = Session->Setup.FovSetting.IsSet()
		? Session->Setup.FovSetting.GetValue()
		: Game.DefaultFovSetting;
	FVector2D Size(1920.0, 1080.0);
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(Size);
	}
	const Sens::FDisplayConfig Display =
		Sens::BuildDisplayConfig(Game, FovSetting, Size.X, Size.Y);
	Pawn->GetCamera()->SetFieldOfView(static_cast<float>(Display.HorizontalFovDeg));
}

void ASensPlayerController::StartFeelDemo()
{
	FeelPhase = EFeelPhase::Demo;
	bDemoPlaying = true;
	DemoElapsed = 0.f;
	bArmed = false;
	ResetMeasurement();
	if (ASensPawn* Pawn = GetSensPawn())
	{
		Pawn->SnapToRest();
	}
	if (TargetActor)
	{
		TargetActor->SetVisibleMarker(false);
	}
	EnterMenuInput();
	RefreshUI();
}

void ASensPlayerController::ResetMeasurement()
{
	Samples.Reset();
	Net = {};
	StartedAtMs = 0.0;
	bArmed = false;
	UpdateHudFlags();
}

void ASensPlayerController::SpawnOrHideTarget()
{
	if (!Session || !TargetActor)
	{
		return;
	}
	if (Session->CurrentScenario() == Sens::EScenarioId::Feel90)
	{
		TargetActor->SetVisibleMarker(false);
		CurrentTarget.Reset();
		return;
	}
	CurrentTarget = Sens::SpawnWorldTarget(Session->CurrentScenario());
	ASensPawn* Pawn = GetSensPawn();
	const FVector Eye = Pawn && Pawn->GetCamera()
		? Pawn->GetCamera()->GetComponentLocation()
		: GetFocalLocation();
	const FRotator Rest = Pawn ? Pawn->GetRestRotation() : FRotator::ZeroRotator;
	TargetActor->PlaceAtAngles(
		Eye,
		Rest,
		CurrentTarget->YawDeg,
		CurrentTarget->PitchDeg,
		TargetDistanceCm);
}

void ASensPlayerController::CommitRound()
{
	if (!Session)
	{
		return;
	}
	FVector2D Size(Session->Setup.ResolutionWidth, Session->Setup.ResolutionHeight);
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(Size);
	}

	Sens::FRoundRecording Recording;
	Recording.Target = CurrentTarget;
	Recording.Samples = Samples;
	Recording.Net = Net;
	Recording.DurationMs = StartedAtMs <= 0.0 ? 0.0 : (FPlatformTime::Seconds() * 1000.0 - StartedAtMs);
	Recording.ArenaWidth = Size.X;
	Recording.ArenaHeight = Size.Y;

	const Sens::EScenarioId Prev = Session->CurrentScenario();
	Session->RecordRound(Recording);

	if (Session->Step == Sens::EAppStep::Results)
	{
		RefreshUI();
		return;
	}

	if (Session->CurrentScenario() != Prev || Session->RoundIndex == 0)
	{
		PrepareCurrentRound();
	}
	else
	{
		PrepareCurrentRound();
	}
	RefreshUI();
}

void ASensPlayerController::PrepareCurrentRound()
{
	ResetMeasurement();
	SpawnOrHideTarget();
	if (!Session)
	{
		return;
	}
	if (Session->CurrentScenario() == Sens::EScenarioId::Feel90)
	{
		StartFeelDemo();
	}
	else
	{
		FeelPhase = EFeelPhase::Replicate;
		if (ASensPawn* Pawn = GetSensPawn())
		{
			Pawn->SnapToRest();
		}
		EnterCaptureInput();
	}
	UpdateHudFlags();
}

ASensPawn* ASensPlayerController::GetSensPawn() const
{
	return Cast<ASensPawn>(GetPawn());
}

ASensHUD* ASensPlayerController::GetSensHUD() const
{
	return Cast<ASensHUD>(GetHUD());
}

void ASensPlayerController::UpdateHudFlags()
{
	if (ASensHUD* HUD = GetSensHUD())
	{
		const bool bTest = Session && Session->Step == Sens::EAppStep::Test;
		HUD->SetShowCrosshair(bTest);
		HUD->SetArmed(bArmed);
	}
}
