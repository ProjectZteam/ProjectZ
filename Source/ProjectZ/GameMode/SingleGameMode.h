// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SingleGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZ_API ASingleGameMode : public AGameModeBase
{
	GENERATED_BODY()


private:
	int MonsterCount;
public:
	UFUNCTION(BlueprintCallable)
	void DecreaseMonsterCount(int Count);

	virtual void StageClear();
	
	virtual void BeginPlay() override;
};
