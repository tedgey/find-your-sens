#pragma once

#include "CoreMinimal.h"

class UWorld;

/** Procedural AimLabs-style box: dark floor, walls, and ceiling. No clutter. */
namespace SensRangeBuilder
{
void Build(UWorld* World, const FVector& Origin);
}
