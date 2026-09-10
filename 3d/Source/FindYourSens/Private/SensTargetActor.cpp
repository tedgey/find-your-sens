#include "SensTargetActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ASensTargetActor::ASensTargetActor()
{
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (Sphere.Succeeded())
	{
		Mesh->SetStaticMesh(Sphere.Object);
	}
	Mesh->SetWorldScale3D(FVector(0.18f));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetHiddenInGame(true);
}

void ASensTargetActor::PlaceAtAngles(
	const FVector& Eye,
	const FRotator& Rest,
	double YawDeg,
	double PitchDeg,
	float DistanceCm)
{
	const FRotator Offset(static_cast<float>(-PitchDeg), static_cast<float>(YawDeg), 0.f);
	const FRotator WorldRot = Rest + Offset;
	const FVector Loc = Eye + WorldRot.RotateVector(FVector::ForwardVector) * DistanceCm;
	SetActorLocation(Loc);
	SetVisibleMarker(true);

	if (UMaterialInterface* Base = Mesh->GetMaterial(0))
	{
		if (UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Base, this))
		{
			Mid->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.722f, 1.f, 0.235f));
			Mesh->SetMaterial(0, Mid);
		}
	}
}

void ASensTargetActor::SetVisibleMarker(bool bVisible)
{
	SetActorHiddenInGame(!bVisible);
}
