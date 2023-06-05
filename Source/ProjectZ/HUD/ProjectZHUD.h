// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ProjectZHUD.generated.h"

//�𸮾� ���� USTRUCT ���̹� �������� �տ� �ݵ�� F ������������ ������ ����
USTRUCT(BlueprintType)
struct FHUDSet
{
	GENERATED_BODY()
public:
	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	float CrosshairSpread;
	FLinearColor CrosshairsColor;
};
/**
*
*/

UCLASS()
class PROJECTZ_API AProjectZHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;
	void AddCharacterOverlay();
protected:
	virtual void BeginPlay() override;
private:
	FHUDSet HUDSet;
	void DrawCrosshair(UTexture2D* Textrue,FVector2D ViewportCenter,FVector2D Sprad,FLinearColor CrosshairColor);
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 15.f;
public:
	FORCEINLINE void SetHUDSet(const FHUDSet& Set) { HUDSet = Set; }
	
};
