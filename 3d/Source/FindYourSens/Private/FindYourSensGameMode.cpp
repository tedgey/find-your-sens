#include "FindYourSensGameMode.h"
#include "SensHUD.h"
#include "SensPawn.h"
#include "SensPlayerController.h"

AFindYourSensGameMode::AFindYourSensGameMode()
{
	PlayerControllerClass = ASensPlayerController::StaticClass();
	DefaultPawnClass = ASensPawn::StaticClass();
	HUDClass = ASensHUD::StaticClass();
}
