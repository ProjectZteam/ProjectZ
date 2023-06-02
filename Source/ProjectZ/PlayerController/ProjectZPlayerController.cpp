// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectZPlayerController.h"
#include "ProjectZ/Character/ProjectZCharacter.h"
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

void AProjectZPlayerController::SetHUDScore(float Score)
{
	ProjectZHUD = ProjectZHUD == nullptr ? Cast<AProjectZHUD>(GetHUD()) : ProjectZHUD;
	bool bHUDValid = ProjectZHUD && ProjectZHUD->CharacterOverlay && ProjectZHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		ProjectZHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
}

void AProjectZPlayerController::SetHUDDefeats(int32 Defeats)
{
	ProjectZHUD = ProjectZHUD == nullptr ? Cast<AProjectZHUD>(GetHUD()) : ProjectZHUD;
	bool bHUDValid = ProjectZHUD && ProjectZHUD->CharacterOverlay && ProjectZHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		ProjectZHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
}

void AProjectZPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	ProjectZHUD = ProjectZHUD == nullptr ? Cast<AProjectZHUD>(GetHUD()) : ProjectZHUD;
	bool bHUDValid = ProjectZHUD && ProjectZHUD->CharacterOverlay && ProjectZHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		ProjectZHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AProjectZPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	AProjectZCharacter* ProjectZCharacter = Cast<AProjectZCharacter>(InPawn);
	if (ProjectZCharacter)
	{
		SetHUDHealth(ProjectZCharacter->GetHealth(), ProjectZCharacter->GetMaxHealth());
	}
}
