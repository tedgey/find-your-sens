#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SensPawn.generated.h"

class UCameraComponent;
class USceneComponent;

UCLASS()
class FINDYOURSENS_API ASensPawn : public APawn
{
	GENERATED_BODY()

public:
	ASensPawn();

	virtual void Tick(float DeltaTime) override;

	UCameraComponent* GetCamera() const { return Camera; }

	void SetRestRotation(FRotator Rotation);
	FRotator GetRestRotation() const { return RestRotation; }

	void SetCameraYawOffset(float YawDegrees);
	void SetCameraAimOffset(float YawDegrees, float PitchDegrees);
	FRotator GetAimOffset() const { return FRotator(AimPitchOffset, AimYawOffset, 0.f); }
	void SnapToRest();

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;

	FRotator RestRotation = FRotator::ZeroRotator;
	float AimYawOffset = 0.f;
	float AimPitchOffset = 0.f;
};
