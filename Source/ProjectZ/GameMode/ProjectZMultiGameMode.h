// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ProjectZ/PlayerController/ProjectZPlayerController.h"
#include "ProjectZMultiGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZ_API AProjectZMultiGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(class AProjectZCharacter* ElimmedCharacter,class AProjectZPlayerController* VictimController, AProjectZPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
};
