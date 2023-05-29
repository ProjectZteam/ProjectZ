// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectZPlayerController.h"
#include "ProjectZ//HUD/ProjectZHUD.h"
#include "ProjectZ/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
void AProjectZPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ProjectZHUD=Cast<AProjectZHUD>(GetHUD());


}
void AProjectZPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	ProjectZHUD = ProjectZHUD == nullptr ? Cast<AProjectZHUD>(GetHUD()) : ProjectZHUD;
	bool bHUDValid = ProjectZHUD && ProjectZHUD->CharacterOverlay && ProjectZHUD->CharacterOverlay->HealthBar && ProjectZHUD->CharacterOverlay->HealthText;
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		ProjectZHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		ProjectZHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}