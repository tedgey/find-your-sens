#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SensHUD.generated.h"

UCLASS()
class FINDYOURSENS_API ASensHUD : public AHUD
{
	GENERATED_BODY()

public:
	ASensHUD();

	virtual void DrawHUD() override;

	void SetArmed(bool bInArmed) { bArmed = bInArmed; }
	void SetShowCrosshair(bool bInShow) { bShowCrosshair = bInShow; }

protected:
	bool bArmed = false;
	bool bShowCrosshair = false;
};
