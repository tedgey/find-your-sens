#include "SensPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "SensAngles.h"
#include "SensCheckIn.h"
#include "SensGames.h"
#include "SensHUD.h"
#include "SensPawn.h"
#include "SensSession.h"
#include "SensSolver.h"
#include "SensTargetActor.h"
#include "SensTargets.h"
#include "SensUI.h"
#include "Widgets/SWeakWidget.h"
#include "Layout/Visibility.h"

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
	ReplayAction = NewObject<UInputAction>(this, TEXT("SensReplay"));
	ReplayAction->ValueType = EInputActionValueType::Boolean;

	Mapping = NewObject<UInputMappingContext>(this, TEXT("SensIMC"));
	Mapping->MapKey(ClickAction, EKeys::LeftMouseButton);
	Mapping->MapKey(MouseAction, EKeys::Mouse2D);
	Mapping->MapKey(EscapeAction, EKeys::Escape);
	Mapping->MapKey(ReplayAction, EKeys::R);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(ClickAction, ETriggerEvent::Started, this, &ASensPlayerController::OnClick);
		EIC->BindAction(MouseAction, ETriggerEvent::Triggered, this, &ASensPlayerController::OnMouse);
		EIC->BindAction(EscapeAction, ETriggerEvent::Started, this, &ASensPlayerController::OnEscape);
		EIC->BindAction(ReplayAction, ETriggerEvent::Started, this, &ASensPlayerController::OnReplay);
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
	if (ASensPawn* SensPawn = GetSensPawn())
	{
		SensPawn->SetRestRotation(FRotator(0.f, 0.f, 0.f));
		SensPawn->SetActorLocation(FVector(0.f, 0.f, 96.f));
	}
	RefreshUI();
}

void ASensPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!Session || !Session->IsLiveTest())
	{
		return;
	}
	if (FeelPhase == EFeelPhase::Demo && bDemoPlaying)
	{
		DemoElapsed += DeltaSeconds;
		const float T = FMath::Clamp(DemoElapsed / DemoDuration, 0.f, 1.f);
		const float Eased = 1.f - FMath::Square(1.f - T);
		if (ASensPawn* SensPawn = GetSensPawn())
		{
			SensPawn->SetCameraYawOffset(90.f * Eased);
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

void ASensPlayerController::HandleSetupContinue()
{
	if (Session)
	{
		Session->OpenPrep();
		RefreshUI();
	}
}

void ASensPlayerController::HandleStartTest()
{
	if (!Session || Session->Step == Sens::EAppStep::Test)
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
	if (ASensPawn* SensPawn = GetSensPawn())
	{
		SensPawn->SnapToRest();
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
	}
	HandleStartTest();
}

void ASensPlayerController::HandleOpenCheckIn()
{
	if (!Session)
	{
		return;
	}
	Session->OpenCheckInSelect(true);
	if (ASensPawn* SensPawn = GetSensPawn())
	{
		SensPawn->SnapToRest();
	}
	if (TargetActor)
	{
		TargetActor->SetVisibleMarker(false);
	}
	RefreshUI();
}

void ASensPlayerController::HandleStartCheckInTest()
{
	if (!Session || Session->Step != Sens::EAppStep::CheckInSelect)
	{
		return;
	}
	if (Session->CheckInSens <= 0.0)
	{
		return;
	}
	Session->BeginCheckInTest();
	ApplyCameraFov();
	PrepareCurrentRound();
	RefreshUI();
}

void ASensPlayerController::HandleRestartCheckInSelect()
{
	if (!Session)
	{
		return;
	}
	Session->RestartCheckInSelect();
	if (ASensPawn* SensPawn = GetSensPawn())
	{
		SensPawn->SnapToRest();
	}
	if (TargetActor)
	{
		TargetActor->SetVisibleMarker(false);
	}
	RefreshUI();
}

void ASensPlayerController::HandleBackToPackResults()
{
	if (!Session)
	{
		return;
	}
	Session->BackToPackResults();
	if (ASensPawn* SensPawn = GetSensPawn())
	{
		SensPawn->SnapToRest();
	}
	if (TargetActor)
	{
		TargetActor->SetVisibleMarker(false);
	}
	RefreshUI();
}

void ASensPlayerController::HandleReplayDemo()
{
	StartFeelDemo();
}

void ASensPlayerController::HandleStartReplicate()
{
	FeelPhase = EFeelPhase::Replicate;
	if (ASensPawn* SensPawn = GetSensPawn())
	{
		SensPawn->SnapToRest();
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
	const bool bCheckIn = IsCheckInTest();
	if (bFeel && FeelPhase == EFeelPhase::Demo)
	{
		return bDemoPlaying
			? TEXT("Watch the turn. This is what 90° of yaw looks like.")
			: TEXT("Go. Left click to copy that turn. Press R to replay.");
	}
	if (!bCapturing)
	{
		return TEXT("Click the arena to lock the pointer and begin.");
	}
	if (!bArmed)
	{
		if (bCheckIn)
		{
			return bFeel
				? TEXT("Click once to arm (crosshair turns green), then look to 90° with this sensitivity, then click to commit.")
				: TEXT("Click once to arm (crosshair turns green), then put the crosshair on the marker, then click to commit.");
		}
		return bFeel
			? TEXT("Click once to arm (crosshair turns green), then sweep like that turn, then click to commit.")
			: TEXT("Click once to arm (crosshair turns green), then move, then click to commit.");
	}
	if (bCheckIn)
	{
		return bFeel
			? TEXT("Armed. Turn until it feels like 90°. The view follows your mouse.")
			: TEXT("Armed. Put the crosshair on the marker, then click.");
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
	if (IsCheckInTest())
	{
		switch (Session->CurrentScenario())
		{
		case Sens::EScenarioId::Feel90:
			return TEXT("Watch a real 90° turn, then look there with live yaw. We measure how close you stop to 90°.");
		case Sens::EScenarioId::Flick:
			return TEXT("Flick the crosshair onto the world marker. Hits are scored. They do not change your pack.");
		case Sens::EScenarioId::Casual:
			return TEXT("Place the crosshair on the world marker smoothly. Hits are scored. They do not change your pack.");
		case Sens::EScenarioId::Micro:
			return TEXT("Make a small correction onto the world marker. Hits are scored. They do not change your pack.");
		}
		return TEXT("");
	}
	switch (Session->CurrentScenario())
	{
	case Sens::EScenarioId::Feel90:
		return TEXT("Watch a real 90° turn, then copy that mouse travel. This is one of four stages that feed the pack.");
	case Sens::EScenarioId::Flick:
		return TEXT("Snap onto the world marker in one commit, then click. This stage feeds the quoted pack.");
	case Sens::EScenarioId::Casual:
		return TEXT("Place your crosshair on the world marker smoothly, then click. This stage feeds the quoted pack.");
	case Sens::EScenarioId::Micro:
		return TEXT("Make a small correction onto the world marker, then click. This stage feeds the quoted pack.");
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
	return Session && Session->IsLiveTest()
		&& Session->CurrentScenario() == Sens::EScenarioId::Feel90
		&& FeelPhase == EFeelPhase::Demo;
}

bool ASensPlayerController::IsFeelReplicate() const
{
	return Session && Session->IsLiveTest()
		&& Session->CurrentScenario() == Sens::EScenarioId::Feel90
		&& FeelPhase == EFeelPhase::Replicate;
}

bool ASensPlayerController::IsCheckInTest() const
{
	return Session && Session->Step == Sens::EAppStep::CheckInTest;
}

bool ASensPlayerController::ShowTravel() const
{
	return IsFeelReplicate() && bArmed && !IsCheckInTest();
}

void ASensPlayerController::OnClick(const FInputActionValue& Value)
{
	if (!Session || !Session->IsLiveTest())
	{
		return;
	}
	if (IsFeelDemo())
	{
		if (!bDemoPlaying)
		{
			HandleStartReplicate();
		}
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

	if (!IsCheckInTest())
	{
		return;
	}
	ASensPawn* SensPawn = GetSensPawn();
	if (!SensPawn)
	{
		return;
	}
	const Sens::FGameProfile& Game = Sens::GetGame(Session->Setup.GameId);
	const FVector2D AimDelta = Sens::MouseToAimDeltaDeg(Delta.X, Delta.Y, Session->CheckInSens, Game.YawConstant);
	const FRotator Current = SensPawn->GetAimOffset();
	double Yaw = Current.Yaw;
	double Pitch = Current.Pitch;
	Sens::AccumulateAim(Yaw, Pitch, AimDelta.X, AimDelta.Y);
	SensPawn->SetCameraAimOffset(static_cast<float>(Yaw), static_cast<float>(Pitch));
}

void ASensPlayerController::OnEscape(const FInputActionValue& Value)
{
	if (!Session || !Session->IsLiveTest())
	{
		return;
	}
	bArmed = false;
	bCapturing = false;
	ResetMeasurement();
	if (ASensPawn* SensPawn = GetSensPawn())
	{
		SensPawn->SnapToRest();
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

void ASensPlayerController::OnReplay(const FInputActionValue& Value)
{
	if (IsFeelDemo())
	{
		HandleReplayDemo();
	}
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
		ShowMenuWidget(MakeSensTestHudWidget(this), true);
		if (IsFeelDemo())
		{
			EnterMenuInput();
		}
		break;
	case Sens::EAppStep::CheckInSelect:
		EnterMenuInput();
		if (ASensPawn* SensPawn = GetSensPawn())
		{
			SensPawn->SnapToRest();
		}
		if (TargetActor)
		{
			TargetActor->SetVisibleMarker(false);
		}
		ShowMenuWidget(MakeSensCheckInSelectWidget(this));
		break;
	case Sens::EAppStep::CheckInTest:
		ShowMenuWidget(MakeSensTestHudWidget(this), true);
		if (IsFeelDemo())
		{
			EnterMenuInput();
		}
		break;
	case Sens::EAppStep::Results:
		EnterMenuInput();
		if (ASensPawn* SensPawn = GetSensPawn())
		{
			SensPawn->SnapToRest();
		}
		if (TargetActor)
		{
			TargetActor->SetVisibleMarker(false);
		}
		ShowMenuWidget(MakeSensResultsWidget(this));
		break;
	case Sens::EAppStep::CheckInResults:
		EnterMenuInput();
		if (ASensPawn* SensPawn = GetSensPawn())
		{
			SensPawn->SnapToRest();
		}
		if (TargetActor)
		{
			TargetActor->SetVisibleMarker(false);
		}
		ShowMenuWidget(MakeSensCheckInResultsWidget(this));
		break;
	}
}

void ASensPlayerController::ShowMenuWidget(TSharedRef<SWidget> Widget, bool bHitTestInvisible)
{
	ClearViewportWidget();
	ViewportWidget = Widget;
	ViewportHost = SNew(SWeakWidget).PossiblyNullContent(ViewportWidget.ToSharedRef());
	if (bHitTestInvisible)
	{
		ViewportHost->SetVisibility(EVisibility::HitTestInvisible);
	}
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

Sens::FDisplayConfig ASensPlayerController::GetDisplayConfig() const
{
	FVector2D Size(1920.0, 1080.0);
	if (Session)
	{
		Size.X = Session->Setup.ResolutionWidth;
		Size.Y = Session->Setup.ResolutionHeight;
	}
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(Size);
	}
	const Sens::FGameProfile& Game = Sens::GetGame(Session ? Session->Setup.GameId : Sens::EGameId::CS2);
	const double FovSetting = (Session && Session->Setup.FovSetting.IsSet())
		? Session->Setup.FovSetting.GetValue()
		: Game.DefaultFovSetting;
	return Sens::BuildDisplayConfig(Game, FovSetting, Size.X, Size.Y);
}

void ASensPlayerController::ApplyCameraFov()
{
	ASensPawn* SensPawn = GetSensPawn();
	if (!SensPawn || !SensPawn->GetCamera())
	{
		return;
	}
	const Sens::FDisplayConfig Display = GetDisplayConfig();
	SensPawn->GetCamera()->bConstrainAspectRatio = false;
	SensPawn->GetCamera()->SetFieldOfView(static_cast<float>(Display.HorizontalFovDeg));
}

void ASensPlayerController::StartFeelDemo()
{
	FeelPhase = EFeelPhase::Demo;
	bDemoPlaying = true;
	DemoElapsed = 0.f;
	bArmed = false;
	ResetMeasurement();
	if (ASensPawn* SensPawn = GetSensPawn())
	{
		SensPawn->SnapToRest();
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
	CurrentTarget = Sens::SpawnWorldTarget(Session->CurrentScenario(), GetDisplayConfig());
	ASensPawn* SensPawn = GetSensPawn();
	const FVector Eye = SensPawn && SensPawn->GetCamera()
		? SensPawn->GetCamera()->GetComponentLocation()
		: GetFocalLocation();
	const FRotator Rest = SensPawn ? SensPawn->GetRestRotation() : FRotator::ZeroRotator;
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
	if (IsCheckInTest())
	{
		CommitCheckInRound();
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

	Session->RecordRound(Recording);

	if (Session->Step == Sens::EAppStep::Results)
	{
		RefreshUI();
		return;
	}

	PrepareCurrentRound();
	RefreshUI();
}

void ASensPlayerController::CommitCheckInRound()
{
	if (!Session)
	{
		return;
	}

	Sens::FCheckInPose Pose;
	if (ASensPawn* SensPawn = GetSensPawn())
	{
		const FRotator Aim = SensPawn->GetAimOffset();
		Pose.YawDeg = Aim.Yaw;
		Pose.PitchDeg = Aim.Pitch;
		Pose.RestForward = SensPawn->GetRestRotation().Vector();
		if (UCameraComponent* Cam = SensPawn->GetCamera())
		{
			Pose.Eye = Cam->GetComponentLocation();
			Pose.AimForward = Cam->GetForwardVector();
		}
	}

	Sens::FCheckInRound Scored;
	if (Session->CurrentScenario() == Sens::EScenarioId::Feel90)
	{
		Scored = Sens::ScoreFeel90Round(Pose, Session->CheckInSens);
	}
	else
	{
		Sens::FCheckInTargetWorld World;
		if (CurrentTarget.IsSet())
		{
			World.Spec = CurrentTarget.GetValue();
		}
		if (TargetActor)
		{
			World.Center = TargetActor->GetActorLocation();
			World.RadiusCm = TargetActor->GetOrbRadiusCm();
		}
		Scored = Sens::ScoreTargetRound(Pose, World, Session->CheckInSens);
	}

	Session->RecordCheckInRound(Scored);

	if (Session->Step == Sens::EAppStep::CheckInResults)
	{
		RefreshUI();
		return;
	}

	PrepareCurrentRound();
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
		if (ASensPawn* SensPawn = GetSensPawn())
		{
			SensPawn->SnapToRest();
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
		const bool bTest = Session && Session->IsLiveTest();
		HUD->SetShowCrosshair(bTest);
		HUD->SetArmed(bArmed);
	}
}
