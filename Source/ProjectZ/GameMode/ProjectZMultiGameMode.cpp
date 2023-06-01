// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectZMultiGameMode.h"
#include "ProjectZ/Character/ProjectZCharacter.h"
#include "ProjectZ/PlayerController/ProjectZPlayerController.h"
#include "ProjectZ/PlayerState/ProjectZPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void AProjectZMultiGameMode::PlayerEliminated(class AProjectZCharacter* ElimmedCharacter, class AProjectZPlayerController* VictimController, AProjectZPlayerController* AttackerController)
{
	AProjectZPlayerState* AttackerPlayerState = AttackerController ? Cast<AProjectZPlayerState>(AttackerController->PlayerState): nullptr;
	AProjectZPlayerState* VictimPlayerState = VictimController ? Cast<AProjectZPlayerState>(VictimController->PlayerState) : nullptr;
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddScore(1.f);
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddDefeats(1);
	}
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}

void AProjectZMultiGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this,APlayerStart::StaticClass(),PlayerStarts);
		int32 SelectionPlayerStart = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController,PlayerStarts[SelectionPlayerStart]);
	}
}
