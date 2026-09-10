#include "SensTargetActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
const FLinearColor OrbGreen(0.1f, 6.f, 0.175f);
constexpr float OrbScale = 0.7f;
constexpr float FloorTopCm = 0.f;
constexpr float CeilingBottomCm = 500.f;
constexpr float ClearanceCm = 24.f;
}

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

	UMaterialInterface* Base = nullptr;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Emissive(
		TEXT("/Engine/EngineMaterials/EmissiveMeshMaterial.EmissiveMeshMaterial"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Solid(
		TEXT("/Engine/EngineDebugMaterials/M_SimpleOpaque.M_SimpleOpaque"));
	if (Emissive.Succeeded())
	{
		Base = Emissive.Object;
	}
	else if (Solid.Succeeded())
	{
		Base = Solid.Object;
	}
	if (Base)
	{
		if (UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Base, this))
		{
			Mid->SetVectorParameterValue(TEXT("Color"), OrbGreen);
			Mid->SetVectorParameterValue(TEXT("EmissiveColor"), OrbGreen);
			Mid->SetVectorParameterValue(TEXT("BaseColor"), OrbGreen);
			Mesh->SetMaterial(0, Mid);
		}
		else
		{
			Mesh->SetMaterial(0, Base);
		}
	}

	Mesh->SetWorldScale3D(FVector(OrbScale));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCastShadow(false);
	SetActorHiddenInGame(true);
	Mesh->SetHiddenInGame(true);
}

void ASensTargetActor::PlaceAtAngles(
	const FVector& Eye,
	const FRotator& Rest,
	double& YawDeg,
	double& PitchDeg,
	float DistanceCm)
{
	const FRotator Offset(static_cast<float>(-PitchDeg), static_cast<float>(YawDeg), 0.f);
	const FRotator WorldRot = Rest + Offset;
	FVector Loc = Eye + WorldRot.RotateVector(FVector::ForwardVector) * DistanceCm;

	const float RadiusCm = OrbScale * 50.f;
	const float MinZ = FloorTopCm + RadiusCm + ClearanceCm;
	const float MaxZ = CeilingBottomCm - RadiusCm - ClearanceCm;
	Loc.Z = FMath::Clamp(Loc.Z, MinZ, MaxZ);
	SetActorLocation(Loc);

	const FVector Local = Rest.UnrotateVector(Loc - Eye);
	const float Horiz = FMath::Sqrt(Local.X * Local.X + Local.Y * Local.Y);
	YawDeg = FMath::RadiansToDegrees(FMath::Atan2(Local.Y, Local.X));
	PitchDeg = -FMath::RadiansToDegrees(FMath::Atan2(Local.Z, FMath::Max(Horiz, 1.f)));

	SetVisibleMarker(true);
}

void ASensTargetActor::SetVisibleMarker(bool bVisible)
{
	SetActorHiddenInGame(!bVisible);
	if (Mesh)
	{
		Mesh->SetHiddenInGame(!bVisible);
	}
}
