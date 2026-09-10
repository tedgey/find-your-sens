#include "SensRangeBuilder.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
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
	if (UMaterialInterface* Base = Comp->GetMaterial(0))
	{
		if (UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Base, Actor))
		{
			Mid->SetVectorParameterValue(TEXT("Color"), Color);
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
	UStaticMesh* Cylinder = LoadBasic(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (!Cube)
	{
		return;
	}

	const FLinearColor Floor(0.02f, 0.025f, 0.03f);
	const FLinearColor Wall(0.04f, 0.05f, 0.06f);
	const FLinearColor Post(0.35f, 0.38f, 0.42f);
	const FLinearColor Accent(0.15f, 0.18f, 0.12f);

	SpawnMesh(World, Cube, Origin + FVector(0.f, 0.f, -10.f), FRotator::ZeroRotator, FVector(40.f, 40.f, 0.2f), Floor);

	const float WallZ = 200.f;
	const float WallH = 4.2f;
	SpawnMesh(World, Cube, Origin + FVector(2000.f, 0.f, WallZ), FRotator::ZeroRotator, FVector(0.2f, 40.f, WallH), Wall);
	SpawnMesh(World, Cube, Origin + FVector(-2000.f, 0.f, WallZ), FRotator::ZeroRotator, FVector(0.2f, 40.f, WallH), Wall);
	SpawnMesh(World, Cube, Origin + FVector(0.f, 2000.f, WallZ), FRotator::ZeroRotator, FVector(40.f, 0.2f, WallH), Wall);
	SpawnMesh(World, Cube, Origin + FVector(0.f, -2000.f, WallZ), FRotator::ZeroRotator, FVector(40.f, 0.2f, WallH), Wall);

	if (Cylinder)
	{
		for (int32 I = 0; I < 8; ++I)
		{
			const float Yaw = I * 45.f;
			const FVector Dir = FRotator(0.f, Yaw, 0.f).RotateVector(FVector::ForwardVector);
			const FVector Loc = Origin + Dir * 700.f + FVector(0.f, 0.f, 140.f);
			SpawnMesh(World, Cylinder, Loc, FRotator::ZeroRotator, FVector(0.35f, 0.35f, 2.8f), Post);
		}
	}

	SpawnMesh(World, Cube, Origin + FVector(900.f, -250.f, 40.f), FRotator::ZeroRotator, FVector(1.2f, 1.2f, 0.8f), Accent);
	SpawnMesh(World, Cube, Origin + FVector(1100.f, 180.f, 60.f), FRotator::ZeroRotator, FVector(0.8f, 2.4f, 1.2f), Accent);

	if (ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(Origin + FVector(0.f, 0.f, 800.f), FRotator(-50.f, 30.f, 0.f)))
	{
		Sun->SetLightColor(FLinearColor(1.f, 0.96f, 0.9f));
		if (ULightComponent* Comp = Sun->GetLightComponent())
		{
			Comp->SetIntensity(4.f);
		}
	}
	if (ASkyLight* Sky = World->SpawnActor<ASkyLight>(Origin + FVector(0.f, 0.f, 600.f), FRotator::ZeroRotator))
	{
		if (USkyLightComponent* Comp = Sky->GetLightComponent())
		{
			Comp->SetIntensity(0.6f);
			Comp->SetLightColor(FLinearColor(0.55f, 0.65f, 0.8f));
		}
	}
	if (AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(Origin, FRotator::ZeroRotator))
	{
		if (UExponentialHeightFogComponent* Comp = Fog->GetComponent())
		{
			Comp->SetFogDensity(0.02f);
			Comp->SetFogHeightFalloff(0.15f);
			Comp->SetFogInscatteringColor(FLinearColor(0.08f, 0.1f, 0.12f));
		}
	}
}
