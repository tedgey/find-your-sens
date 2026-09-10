#include "SensHUD.h"
#include "Engine/Canvas.h"

ASensHUD::ASensHUD()
{
}

void ASensHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!bShowCrosshair || !Canvas)
	{
		return;
	}

	const float CX = Canvas->ClipX * 0.5f;
	const float CY = Canvas->ClipY * 0.5f;
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
