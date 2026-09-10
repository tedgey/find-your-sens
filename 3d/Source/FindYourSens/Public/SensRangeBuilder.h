#pragma once

#include "CoreMinimal.h"

class UWorld;

/** Procedural first-person range: floor, walls, posts at 45° so a 90° demo reads. */
namespace SensRangeBuilder
{
void Build(UWorld* World, const FVector& Origin);
}
