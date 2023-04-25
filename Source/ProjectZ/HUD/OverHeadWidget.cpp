// Fill out your copyright notice in the Description page of Project Settings.


#include "OverHeadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void UOverHeadWidget::SetDisplayText(FString TextDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextDisplay));
	}
}

void UOverHeadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	ENetRole LocalRole = InPawn->GetLocalRole();
	FString Role;
	switch (LocalRole)
	{
	case ENetRole::ROLE_Authority:
		Role = FString("Authority");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("AutonomousProxy");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("SimulatedProxy");
		break;
	}
	FString RemoteRoleString = FString::Printf(TEXT("Remote Role: %s"), *Role);
	SetDisplayText(RemoteRoleString);
}
void UOverHeadWidget::ShowPlayerName(APawn* InPawn)
{
	const APlayerState* PlayerState = InPawn->GetPlayerState();
	if (!PlayerState || !*PlayerState->GetPlayerName() && TotalTime < GetPlayerNameTimeout)
	{
		FTimerHandle GetPlayerStateTimer;
		FTimerDelegate TryAgainDelegate;
		TryAgainDelegate.BindUFunction(this, FName("ShowPlayerName"), InPawn);
		GetWorld()->GetTimerManager().SetTimer(GetPlayerStateTimer, TryAgainDelegate, GetPlayerNameInterval, false, 0.1f);
		TotalTime += GetPlayerNameInterval;
		return;
	}
	const FString PlayerName = InPawn->GetPlayerState()->GetPlayerName();
	SetDisplayText(PlayerName);
}

void UOverHeadWidget::NativeDestruct()
{
	
	RemoveFromParent();
	Super::NativeDestruct();
}
