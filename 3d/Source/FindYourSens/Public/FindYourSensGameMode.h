#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FindYourSensGameMode.generated.h"

class USensSession;

UCLASS()
class FINDYOURSENS_API AFindYourSensGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFindYourSensGameMode();
	virtual void StartPlay() override;

	UPROPERTY()
	TObjectPtr<USensSession> Session;
};
