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
	int LeftMonsterCount = 0;
public:
	UFUNCTION(BlueprintCallable)
	virtual void DecreaseMonsterCount(int Count);

	UFUNCTION(BlueprintCallable)
	virtual void IncreaseMonsterCount(int Count);

	UFUNCTION(BlueprintCallable)
	virtual int GetLeftMonsterCount();

	virtual void StageClear();
};
