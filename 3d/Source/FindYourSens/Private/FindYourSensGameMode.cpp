#include "FindYourSensGameMode.h"
#include "SensHUD.h"
#include "SensPawn.h"
#include "SensPlayerController.h"
#include "SensRangeBuilder.h"
#include "SensSession.h"

AFindYourSensGameMode::AFindYourSensGameMode()
{
	PlayerControllerClass = ASensPlayerController::StaticClass();
	DefaultPawnClass = ASensPawn::StaticClass();
	HUDClass = ASensHUD::StaticClass();
}

void AFindYourSensGameMode::StartPlay()
{
	Super::StartPlay();
	Session = NewObject<USensSession>(this);
	SensRangeBuilder::Build(GetWorld(), FVector::ZeroVector);
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ASensPlayerController* PC = Cast<ASensPlayerController>(It->Get()))
		{
			PC->InitSession(Session);
		}
	}
}
