// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ProjectZPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZ_API AProjectZPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	void AddScore(float ScoreAmount);
	void AddDefeats(int32 DefeatsAmount);
	virtual void OnRep_Score() override;
	UFUNCTION()
	virtual void OnRep_Defeats();
private:
	UPROPERTY()
	class AProjectZCharacter* Character;
	UPROPERTY()
	class AProjectZPlayerController* Controller;
	UPROPERTY(ReplicatedUsing=OnRep_Defeats)
	int32 Defeats;

	
};
