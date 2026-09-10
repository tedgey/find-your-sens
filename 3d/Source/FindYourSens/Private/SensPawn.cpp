#include "SensPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"

ASensPawn::ASensPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Root);
	Camera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	Camera->bUsePawnControlRotation = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
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
	if (Camera)
	{
		Camera->SetWorldRotation(RestRotation + FRotator(0.f, YawDegrees, 0.f));
	}
}

void ASensPawn::SnapToRest()
{
	if (Camera)
	{
		Camera->SetWorldRotation(RestRotation);
	}
}
