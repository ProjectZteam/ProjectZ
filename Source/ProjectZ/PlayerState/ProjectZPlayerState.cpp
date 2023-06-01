// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectZPlayerState.h"
#include "ProjectZ/Character/ProjectZCharacter.h"
#include "ProjectZ/PlayerController/ProjectZPlayerController.h"
#include "Net/UnrealNetwork.h"
void AProjectZPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AProjectZPlayerState, Defeats);

}
void AProjectZPlayerState::AddScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);

	Character = Character == nullptr ? Cast<AProjectZCharacter>(GetPawn()) : Character;

	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AProjectZPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}
void AProjectZPlayerState::AddDefeats(int32 DefeatsAmount)
{
	
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<AProjectZCharacter>(GetPawn()) : Character;

	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AProjectZPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}
void AProjectZPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	Character = Character == nullptr ? Cast<AProjectZCharacter>(GetPawn()) : Character;

	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AProjectZPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AProjectZPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<AProjectZCharacter>(GetPawn()) : Character;

	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AProjectZPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}
