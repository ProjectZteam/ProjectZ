// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ProjectZMultiGameState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZ_API AProjectZMultiGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(class AProjectZPlayerState* ScoringPlayer);
	UPROPERTY(Replicated)
	TArray<AProjectZPlayerState*> TopScoringPlayers;

private:
	float TopScore = 0.f;
};
