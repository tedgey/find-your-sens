#pragma once

#include "SensTypes.h"

/** Port of 2d/src/math/games.ts */
namespace Sens
{
extern const TArray<int32> DpiTiers;

const FGameProfile& GetGame(EGameId Id);
TArray<EGameId> PrototypeGameOrder();
}
