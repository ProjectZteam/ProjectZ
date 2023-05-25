// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectZHUD.h"

void AProjectZHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDSet.CrosshairSpread;
		if (HUDSet.CrosshairsCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDSet.CrosshairsCenter, ViewportCenter,Spread);
		}
		if (HUDSet.CrosshairsRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDSet.CrosshairsRight, ViewportCenter, Spread);
		}
		if (HUDSet.CrosshairsLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDSet.CrosshairsLeft, ViewportCenter, Spread);
		}
		if (HUDSet.CrosshairsTop)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDSet.CrosshairsTop, ViewportCenter, Spread);
		}
		if (HUDSet.CrosshairsBottom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDSet.CrosshairsBottom, ViewportCenter, Spread);
		}
	}
}

void AProjectZHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter,FVector2D Spread)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f)+Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f)+Spread.Y
	);

	//���ڸ��
	// �׸� �ؽ��� ������, �׸���ġ XY ��ǥ, �ؽ��� �ʺ�� ����, �ؽ��� UV ��ǥ �� ������ �����Ͽ� ����
	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		FLinearColor::White
	);
}
