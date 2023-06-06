// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectZMultiGameMode.h"
#include "ProjectZ/Character/ProjectZCharacter.h"
#include "ProjectZ/PlayerController/ProjectZPlayerController.h"
#include "ProjectZ/PlayerState/ProjectZPlayerState.h"
#include "ProjectZ/GameState/ProjectZMultiGameState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}
AProjectZMultiGameMode::AProjectZMultiGameMode()
{
	bDelayedStart = true;
}

void AProjectZMultiGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime +WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}
void AProjectZMultiGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AProjectZPlayerController* ProjectZPlayer = Cast<AProjectZPlayerController>(*It);
		if (ProjectZPlayer)
		{
			ProjectZPlayer->OnMatchStateSet(MatchState);
		}
	}
}
void AProjectZMultiGameMode::PlayerEliminated(class AProjectZCharacter* ElimmedCharacter, class AProjectZPlayerController* VictimController, AProjectZPlayerController* AttackerController)
{
	AProjectZPlayerState* AttackerPlayerState = AttackerController ? Cast<AProjectZPlayerState>(AttackerController->PlayerState): nullptr;
	AProjectZPlayerState* VictimPlayerState = VictimController ? Cast<AProjectZPlayerState>(VictimController->PlayerState) : nullptr;

	AProjectZMultiGameState* ProjectZMultiGameState = GetGameState<AProjectZMultiGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState&&ProjectZMultiGameState)
	{
		AttackerPlayerState->AddScore(1.f);
		ProjectZMultiGameState->UpdateTopScore(AttackerPlayerState);
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
void AProjectZMultiGameMode::ShuffleArray(TArray<AActor*>& PlayerStart)
{
	for (int32 i = PlayerStart.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		if (i != j)
		{
			PlayerStart.Swap(i, j);
		}
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
		//int32 SelectionPlayerStart = FMath::RandRange(0, PlayerStarts.Num() - 1);
		ShuffleArray(PlayerStarts);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[0]);
	}
}

void AProjectZMultiGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}
