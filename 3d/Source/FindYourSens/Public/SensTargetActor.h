#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SensTargetActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class FINDYOURSENS_API ASensTargetActor : public AActor
{
	GENERATED_BODY()

public:
	ASensTargetActor();

	void PlaceAtAngles(const FVector& Eye, const FRotator& Rest, double& YawDeg, double& PitchDeg, float DistanceCm);
	void SetVisibleMarker(bool bVisible);
	float GetOrbRadiusCm() const;

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;
};
