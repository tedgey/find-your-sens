#include "SensPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "SensCheckIn.h"

ASensPawn::ASensPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Root);
	Camera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	Camera->bUsePawnControlRotation = false;
	Camera->bConstrainAspectRatio = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ASensPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASensPawn::SetRestRotation(FRotator Rotation)
{
	RestRotation = Rotation;
	SnapToRest();
}

void ASensPawn::SetCameraYawOffset(float YawDegrees)
{
	SetCameraAimOffset(YawDegrees, 0.f);
}

void ASensPawn::SetCameraAimOffset(float YawDegrees, float PitchDegrees)
{
	AimYawOffset = YawDegrees;
	AimPitchOffset = FMath::Clamp(PitchDegrees, static_cast<float>(-Sens::PitchClampDeg), static_cast<float>(Sens::PitchClampDeg));
	if (Camera)
	{
		Camera->SetWorldRotation(RestRotation + FRotator(AimPitchOffset, AimYawOffset, 0.f));
	}
}

void ASensPawn::SnapToRest()
{
	SetCameraAimOffset(0.f, 0.f);
}
