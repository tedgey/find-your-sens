#include "SensHUD.h"
#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"

ASensHUD::ASensHUD()
{
}

void ASensHUD::SetTravel(const FString& InTravel, const FString& InReference, bool bInShowTravel)
{
	TravelLine = InTravel;
	ReferenceLine = InReference;
	bShowTravel = bInShowTravel;
}

void ASensHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas)
	{
		return;
	}

	const float CX = Canvas->ClipX * 0.5f;
	const float CY = Canvas->ClipY * 0.5f;

	if (bShowTravel && GEngine)
	{
		const float TravelY = Canvas->ClipY / 3.f;
		if (UFont* Large = GEngine->GetLargeFont())
		{
			FCanvasTextItem Travel(FVector2D(CX, TravelY), FText::FromString(TravelLine), Large, FLinearColor(0.722f, 1.f, 0.235f));
			Travel.bCentreX = true;
			Travel.bCentreY = true;
			Travel.Scale = FVector2D(1.4f, 1.4f);
			Canvas->DrawItem(Travel);
		}
		if (UFont* Small = GEngine->GetSmallFont())
		{
			FCanvasTextItem Ref(FVector2D(CX, TravelY + 22.f), FText::FromString(ReferenceLine), Small, FLinearColor(0.91f, 0.933f, 0.96f, 0.55f));
			Ref.bCentreX = true;
			Ref.bCentreY = true;
			Canvas->DrawItem(Ref);
		}
	}

	if (!bShowCrosshair)
	{
		return;
	}
	const FLinearColor Color = bArmed
		? FLinearColor(0.722f, 1.f, 0.235f, 1.f)
		: FLinearColor(0.91f, 0.933f, 0.96f, 1.f);

	FCanvasLineItem H(FVector2D(CX - 10.f, CY), FVector2D(CX + 10.f, CY));
	H.SetColor(Color);
	H.LineThickness = 1.5f;
	Canvas->DrawItem(H);

	FCanvasLineItem V(FVector2D(CX, CY - 10.f), FVector2D(CX, CY + 10.f));
	V.SetColor(Color);
	V.LineThickness = 1.5f;
	Canvas->DrawItem(V);

	if (bArmed)
	{
		const float R = 8.f;
		const int32 Segs = 16;
		FVector2D Prev(CX + R, CY);
		for (int32 I = 1; I <= Segs; ++I)
		{
			const float Ang = (2.f * PI * I) / Segs;
			const FVector2D Next(CX + R * FMath::Cos(Ang), CY + R * FMath::Sin(Ang));
			FCanvasLineItem Arc(Prev, Next);
			Arc.SetColor(Color);
			Arc.LineThickness = 1.5f;
			Canvas->DrawItem(Arc);
			Prev = Next;
		}
	}
}
