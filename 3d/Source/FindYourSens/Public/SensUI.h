#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class ASensPlayerController;

TSharedRef<SWidget> MakeSensSetupWidget(ASensPlayerController* PC);
TSharedRef<SWidget> MakeSensInfoWidget(ASensPlayerController* PC);
TSharedRef<SWidget> MakeSensPrepWidget(ASensPlayerController* PC);
TSharedRef<SWidget> MakeSensTestHudWidget(ASensPlayerController* PC);
TSharedRef<SWidget> MakeSensResultsWidget(ASensPlayerController* PC);
