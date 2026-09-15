#include "SensRangeBuilder.h"
#include "Components/LightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"

static UStaticMesh* LoadBasic(const TCHAR* Path)
{
	return LoadObject<UStaticMesh>(nullptr, Path);
}

static AStaticMeshActor* SpawnMesh(
	UWorld* World,
	UStaticMesh* Mesh,
	const FVector& Location,
	const FRotator& Rotation,
	const FVector& Scale,
	const FLinearColor& Color)
{
	if (!World || !Mesh)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(Location, Rotation, Params);
	if (!Actor)
	{
		return nullptr;
	}
	Actor->SetMobility(EComponentMobility::Movable);
	UStaticMeshComponent* Comp = Actor->GetStaticMeshComponent();
	Comp->SetStaticMesh(Mesh);
	Comp->SetWorldScale3D(Scale);
	Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Comp->SetCastShadow(true);
	if (UMaterialInterface* Base = Comp->GetMaterial(0))
	{
		if (UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Base, Actor))
		{
			Mid->SetVectorParameterValue(TEXT("Color"), Color);
			Mid->SetVectorParameterValue(TEXT("BaseColor"), Color);
			Comp->SetMaterial(0, Mid);
		}
	}
	return Actor;
}

void SensRangeBuilder::Build(UWorld* World, const FVector& Origin)
{
	if (!World)
	{
		return;
	}

	UStaticMesh* Cube = LoadBasic(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (!Cube)
	{
		return;
	}

	const FLinearColor Floor(0.03f, 0.035f, 0.04f);
	const FLinearColor Wall(0.045f, 0.05f, 0.055f);
	const FLinearColor Ceiling(0.022f, 0.025f, 0.028f);

	const float HalfExtentCm = 1500.f;
	const float HeightCm = 500.f;
	const float WallZ = HeightCm * 0.5f;
	const float FloorScale = (HalfExtentCm * 2.f) / 100.f;
	const float WallH = HeightCm / 100.f;

	SpawnMesh(World, Cube, Origin + FVector(0.f, 0.f, -10.f), FRotator::ZeroRotator, FVector(FloorScale, FloorScale, 0.2f), Floor);
	SpawnMesh(World, Cube, Origin + FVector(HalfExtentCm, 0.f, WallZ), FRotator::ZeroRotator, FVector(0.2f, FloorScale, WallH), Wall);
	SpawnMesh(World, Cube, Origin + FVector(-HalfExtentCm, 0.f, WallZ), FRotator::ZeroRotator, FVector(0.2f, FloorScale, WallH), Wall);
	SpawnMesh(World, Cube, Origin + FVector(0.f, HalfExtentCm, WallZ), FRotator::ZeroRotator, FVector(FloorScale, 0.2f, WallH), Wall);
	SpawnMesh(World, Cube, Origin + FVector(0.f, -HalfExtentCm, WallZ), FRotator::ZeroRotator, FVector(FloorScale, 0.2f, WallH), Wall);
	SpawnMesh(World, Cube, Origin + FVector(0.f, 0.f, HeightCm + 10.f), FRotator::ZeroRotator, FVector(FloorScale, FloorScale, 0.2f), Ceiling);

	if (ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(Origin + FVector(0.f, 0.f, 400.f), FRotator(-70.f, 20.f, 0.f)))
	{
		Sun->SetLightColor(FLinearColor(0.85f, 0.9f, 1.f));
		if (ULightComponent* Comp = Sun->GetLightComponent())
		{
			Comp->SetIntensity(2.5f);
		}
	}
}
