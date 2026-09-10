#include "SensGames.h"

namespace Sens
{
const TArray<int32> DpiTiers = {400, 800, 1600, 3200};

static const FGameProfile GCs2 = {
	EGameId::CS2,
	TEXT("Counter-Strike 2"),
	0.022,
	90.0,
	EFovMode::Source43,
	3,
	TEXT("sensitivity"),
	true};

static const FGameProfile GValorant = {
	EGameId::Valorant,
	TEXT("Valorant"),
	0.07,
	103.0,
	EFovMode::Horizontal,
	3,
	TEXT("Sensitivity"),
	true};

static const FGameProfile GPubg = {
	EGameId::PUBG,
	TEXT("PUBG"),
	0.022,
	90.0,
	EFovMode::Horizontal,
	2,
	TEXT("General Sensitivity"),
	true};

static const FGameProfile GApex = {
	EGameId::Apex,
	TEXT("Apex Legends"),
	0.022,
	110.0,
	EFovMode::Horizontal,
	3,
	TEXT("Mouse Sensitivity"),
	true};

static const FGameProfile GFortnite = {
	EGameId::Fortnite,
	TEXT("Fortnite"),
	0.005555,
	80.0,
	EFovMode::Horizontal,
	3,
	TEXT("X/Y Sensitivity"),
	true};

const FGameProfile& GetGame(EGameId Id)
{
	switch (Id)
	{
	case EGameId::Valorant:
		return GValorant;
	case EGameId::PUBG:
		return GPubg;
	case EGameId::Apex:
		return GApex;
	case EGameId::Fortnite:
		return GFortnite;
	case EGameId::CS2:
	default:
		return GCs2;
	}
}

TArray<EGameId> PrototypeGameOrder()
{
	return {EGameId::CS2};
}
} // namespace Sens
