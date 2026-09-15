#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "SensTypes.h"
#include "SensPlayerController.generated.h"

class USensSession;
class ASensHUD;
class ASensPawn;
class ASensTargetActor;
class UInputAction;
class UInputMappingContext;
class SWidget;

UCLASS()
class FINDYOURSENS_API ASensPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASensPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaSeconds) override;

	void InitSession(USensSession* InSession);
	USensSession* GetSession() const { return Session; }

	void HandleOpenInfo();
	void HandleSetupContinue();
	void HandleStartTest();
	void HandleBackToSetup();
	void HandleRetest();
	void HandleReplayDemo();
	void HandleStartReplicate();
	void HandleOpenCheckIn();
	void HandleStartCheckInTest();
	void HandleRestartCheckInSelect();
	void HandleBackToPackResults();

	FString GetHudLine() const;
	FString GetScenarioHint() const;
	FString GetTravelLine() const;
	FString GetReferenceTravelLine() const;
	bool IsFeelDemo() const;
	bool IsFeelReplicate() const;
	bool IsArmed() const { return bArmed; }
	bool ShowTravel() const;

protected:
	void OnClick(const FInputActionValue& Value);
	void OnMouse(const FInputActionValue& Value);
	void OnEscape(const FInputActionValue& Value);
	void OnReplay(const FInputActionValue& Value);

	void RefreshUI();
	void ShowMenuWidget(TSharedRef<SWidget> Widget, bool bHitTestInvisible = false);
	void ClearViewportWidget();
	void EnterMenuInput();
	void EnterCaptureInput();
	void ApplyCameraFov();
	Sens::FDisplayConfig GetDisplayConfig() const;
	void StartFeelDemo();
	void ResetMeasurement();
	void SpawnOrHideTarget();
	void CommitRound();
	void CommitCheckInRound();
	void PrepareCurrentRound();
	bool IsCheckInTest() const;
	ASensPawn* GetSensPawn() const;
	ASensHUD* GetSensHUD() const;
	void UpdateHudFlags();

	UPROPERTY()
	TObjectPtr<USensSession> Session;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> Mapping;

	UPROPERTY()
	TObjectPtr<UInputAction> ClickAction;

	UPROPERTY()
	TObjectPtr<UInputAction> MouseAction;

	UPROPERTY()
	TObjectPtr<UInputAction> EscapeAction;

	UPROPERTY()
	TObjectPtr<UInputAction> ReplayAction;

	UPROPERTY()
	TObjectPtr<ASensTargetActor> TargetActor;

	TSharedPtr<SWidget> ViewportWidget;
	TSharedPtr<SWidget> ViewportHost;

	enum class EFeelPhase : uint8
	{
		Demo,
		Replicate
	};

	EFeelPhase FeelPhase = EFeelPhase::Demo;
	bool bDemoPlaying = false;
	bool bArmed = false;
	bool bCapturing = false;
	float DemoElapsed = 0.f;
	double StartedAtMs = 0.0;
	Sens::FPoint2 Net;
	TArray<Sens::FPathSample> Samples;
	TOptional<Sens::FTargetSpec> CurrentTarget;
};
